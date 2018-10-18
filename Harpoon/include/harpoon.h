#pragma once
#include <string>

namespace hp {

	typedef std::string HarpoonError;

	class Harpoon {

	public:

		//Inject HarpoonLoader into a named process and run it
		//time = time in seconds to wait until mono is found
		static HarpoonError hook(std::string process, size_t time);

		//Check if process is active
		static bool isActive(std::string process);

		//For info callbacks (default points to printf wrapper)
		static void (*infoCallback)(std::string);

	};

}