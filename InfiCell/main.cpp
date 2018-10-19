#include <Harpoon/include/harpoon.h>
#include <Windows.h>
#pragma comment(lib, "Harpoon.lib")
using namespace hp;

const char DotE[] = "DungeonoftheEndless.exe";
const size_t startTime = 60;						//Max start time (until process exists) of 60 seconds
const size_t minStartTime = 10;						//Min start time (if process already exists) of 10 seconds
const size_t waitTime = 20;							//Max wait time (until mono exists) of 20 seconds

//The one executing this should determine to use 32-bit or 64-bit
int main() {

	if (!Harpoon::isActive(DotE))
		ShellExecuteA(0, 0, "\"steam://rungameid/249050\"", 0, 0, SW_HIDE);

	size_t counter = startTime;

	while (!Harpoon::isActive(DotE)) {

		if (counter == 0) {
			printf("InfiCell failed; couldn't startup %s\n", DotE);
			return 1;
		}

		--counter;
		Sleep(1000);

	}

	while (counter >= (startTime - minStartTime)) {
		--counter;
		Sleep(1000);
	}

	HarpoonError res = Harpoon::hook(DotE, waitTime);

	if (res != "") {
		printf("Harpoon failed:\n%s\n", res.c_str());
		return 2;
	}

	return 0;

}