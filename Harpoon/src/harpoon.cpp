#define PSAPI_VERSION 1
#include "harpoon.h"
#include <Windows.h>
#include <locale>
#include <codecvt>
#include <assert.h>
#include <process.h>
#include <fstream>
#include <psapi.h>
#include <vector>
#include <unordered_map>
#include <DbgHelp.h>
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Dbghelp.lib")
using namespace hp;

HarpoonError HarpoonError_None = "";

void print(std::string str) {
	printf("%s", str.c_str());
}

void (*Harpoon::infoCallback)(std::string) = print;

template<typename T>
HarpoonError checkErr(T func, char *str) {

	if (func == 0) {

		HRESULT last = GetLastError();
		HarpoonError err = str;

		LPTSTR errorText = NULL;

		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, last, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errorText,  0, NULL);

		if (errorText != NULL) {
			err += std::string(": ") + errorText;
			LocalFree(errorText);
		}

		return err;

	} 
	
	return HarpoonError_None;

}

std::string getProcessName(DWORD id) {

	char processName[MAX_PATH];

	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, id);

	if (process != NULL) {

		HMODULE module;
		DWORD nameLength;

		if (EnumProcessModules(process, &module, sizeof(module), &nameLength)) {
			GetModuleBaseNameA(process, module, processName, MAX_PATH);
			return processName;
		}

	}

	CloseHandle(process);
	return "";

}

DWORD findProcess(std::string str) {

	DWORD processes[1024], processCount;

	if (!EnumProcesses(processes, sizeof(processes), &processCount))
		return 0;

	DWORD current = 0;

	for (DWORD i = 0; i < processCount; ++i)
		if (getProcessName(current = processes[i]) == str)
			return current;

	return 0;
}

bool Harpoon::isActive(std::string process) {
	return findProcess(process) != 0;
}

LPVOID allocBuffer(HANDLE process, void *start, size_t length, bool exec, HarpoonError &err) {

	LPVOID remote = VirtualAllocEx(process, NULL, length, MEM_COMMIT | MEM_RESERVE, exec ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE);
	err = checkErr(remote, "Couldn't allocate space into process");

	if (err == HarpoonError_None) {

		err = checkErr(WriteProcessMemory(process, remote, (LPVOID)start, length, NULL), "Couldn't write buffer into memory");

		if (err == HarpoonError_None)
			return remote;

		VirtualFreeEx(process, remote, length, MEM_RELEASE);

	}
	
	return nullptr;
}

struct RemoteDll {
	std::string name, fileName;
	HINSTANCE remote;
};

RemoteDll getRemoteDll(HANDLE process, std::string module, HarpoonError &err) {

	HINSTANCE handles[2048];
	DWORD needed = 0;
	EnumProcessModulesEx(process, handles, sizeof(handles), &needed, LIST_MODULES_ALL);

	char name[MAX_PATH + 1], fileName[MAX_PATH + 1];

	for (DWORD i = 0; i < needed / sizeof(handles[0]); ++i) {

		GetModuleBaseNameA(process, handles[i], name, sizeof(name));
		GetModuleFileNameExA(process, handles[i], fileName, sizeof(fileName));

		MODULEINFO modInfo;
		GetModuleInformation(process, handles[i], &modInfo, sizeof(modInfo));

		if (module == name)
			return { name, fileName, handles[i] };

	}

	err = checkErr(false, "Couldn't find specified module");
	return {};
}

std::unordered_map<std::string, void*> getFunctions(RemoteDll dll, std::vector<std::string> functions, HarpoonError &err) {

	HINSTANCE local = LoadLibraryExA(dll.fileName.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);

	if ((err = checkErr(local, "Couldn't load remote library")) != HarpoonError_None)
		return {};

	std::unordered_map<std::string, void*> funcs;

	for (std::string &str : functions)
		funcs[str] = (void*)((char*)GetProcAddress(local, str.c_str()) - (char*)local + (char*)dll.remote);

	if (funcs.size() != functions.size())
		err = "Couldn't find one of the functions";

	FreeLibrary(local);
	return funcs;

}

HarpoonError hook(DWORD processId, size_t wait) {

	//Load dll into memory

	const char *harpoonLoader = OUTDIR "HarpoonLoader.dll";

	HMODULE dllh = LoadLibraryA(harpoonLoader);

	if (dllh == nullptr)
		return "Couldn't find HarpoonLoader";

	MODULEINFO info;
	GetModuleInformation(GetCurrentProcess(), dllh, &info, sizeof(info));

	size_t dllLength = info.SizeOfImage;

	//Get initialize offset
	size_t init = size_t((char*)GetProcAddress(dllh, "initialize") - (char*)dllh);

	//Get process

	HANDLE process = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD |
		PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
		FALSE, processId
	);

	HarpoonError err;

	if ((err = checkErr(process, "Couldn't open process")) != HarpoonError_None)
		return err;

	//Load our dll into the host's process

	PVOID harpoon = allocBuffer(process, (void*) dllh, dllLength, true, err);

	if (err != HarpoonError_None) {
		CloseHandle(process);
		return err;
	}

	FreeLibrary(dllh);

	//Wait for mono

	RemoteDll mono;

	do {

		mono = getRemoteDll(process, "mono.dll", err);

		if (err == HarpoonError_None)
			break;

		if(wait != 0)
			--wait;

		Sleep(1000);

	} while (wait != 0);

	if (err != HarpoonError_None) {
		VirtualFreeEx(process, harpoon, dllLength, MEM_RELEASE);
		CloseHandle(process);
		return err;
	}

	//Setup a command to get our functions from DLL
	std::vector<std::string> funcs = {
		"mono_domain_get",
		"mono_thread_attach",
		"mono_get_root_domain",
		"mono_domain_assembly_open",
		"mono_class_from_name",
		"mono_class_get_method_from_name",
		"mono_runtime_invoke",
		"mono_assembly_get_image"
	};

	auto functionMap = getFunctions(mono, funcs, err);

	if (err != HarpoonError_None)
		return err;

	//These are our strings (and functions); but we can't use those directly
	//This is because this program will be moved and any address (not to kernel32) will have to be moved

	constexpr size_t pad = sizeof(void*);

	//TODO: Instead of OUTDIR use current working directory

	std::string csdll = OUTDIR "Harpoon.Core.dll";
	size_t datSiz = 36 + pad * 8 + csdll.size() + 1;

	char *data = new char[datSiz];
	memset(data, 0, datSiz);

	memcpy(data,
		"Harpoon.Core\0"
		"HarpoonCore\0"
		"Initialize\0"
		"Harpoon.Core.dll\0", 
	53);

	*(void**)(data + 36) = functionMap["mono_domain_get"];
	*(void**)(data + 36 + pad) = functionMap["mono_thread_attach"];
	*(void**)(data + 36 + pad * 2) = functionMap["mono_get_root_domain"];
	*(void**)(data + 36 + pad * 3) = functionMap["mono_domain_assembly_open"];
	*(void**)(data + 36 + pad * 4) = functionMap["mono_class_from_name"];
	*(void**)(data + 36 + pad * 5) = functionMap["mono_class_get_method_from_name"];
	*(void**)(data + 36 + pad * 6) = functionMap["mono_runtime_invoke"];
	*(void**)(data + 36 + pad * 7) = functionMap["mono_assembly_get_image"];

	memcpy(data + 36 + pad * 8, csdll.c_str(), csdll.size());

	LPVOID dat = allocBuffer(process, data, datSiz, false, err);

	std::vector<char> chars(data, data + datSiz);					//Just for testing the buffer that we sent
	delete[] data;

	if (err != HarpoonError_None) {
		VirtualFreeEx(process, harpoon, dllLength, MEM_RELEASE);
		CloseHandle(process);
		return err;
	}

	//Run our remote function
	PTHREAD_START_ROUTINE initialize = (PTHREAD_START_ROUTINE) ((PCHAR) harpoon + init);

	HANDLE thread = CreateRemoteThread(process, NULL, 0, initialize, dat, 0, NULL);
	
	if ((err = checkErr(thread, "Couldn't start library on remote process")) != HarpoonError_None) {
		VirtualFreeEx(process, dat, datSiz, MEM_RELEASE);
		VirtualFreeEx(process, harpoon, dllLength, MEM_RELEASE);
		CloseHandle(process);
		return err;
	}

	//Wait for it to finish and stop
	WaitForSingleObject(thread, INFINITE);

	CloseHandle(thread);

	VirtualFreeEx(process, dat, datSiz, MEM_RELEASE);
	VirtualFreeEx(process, harpoon, dllLength, MEM_RELEASE);
	CloseHandle(process);

	return "";

}

HarpoonError Harpoon::hook(std::string process, size_t wait) {

	DWORD id = findProcess(process);

	if (id == 0)
		return std::string("Couldn't resolve \"") + process + "\"";

	Harpoon::infoCallback(std::string("Resolved ") + process + " as " + std::to_string(id));

	return ::hook(id, wait);

}