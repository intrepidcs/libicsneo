// Usage:
// ./libicsneocpp-coremini [DEVICE_SERIAL] [COREMINI_SCRIPT_PATH]


#include <iostream>
#include <fstream>
#include <icsneo/icsneocpp.h>

void displayUsage() {
	std::cout << "Usage:\n";
	std::cout << "./libicsneocpp-coremini [DEVICE_SERIAL] [COREMINI_SCRIPT_PATH]\n";
}


int main(int argc, char** argv) {
	std::vector<std::string> arguments(argv, argv + argc);

	if(arguments.size() != 3) {
		displayUsage();
		return EXIT_FAILURE;
	}


	std::cout << icsneo::GetVersion() << std::endl;

	const auto& devices = icsneo::FindAllDevices();

	auto it = std::find_if(
		devices.begin(),
		devices.end(),
		[&arguments](const auto &d)
		{ return d->getSerial() == arguments[1]; });

	if(it == devices.end()) {
		std::cout << "Failed to find device." << std::endl;
		return EXIT_FAILURE;
	}


	std::shared_ptr<icsneo::Device> device = *it;
	if(!device->open()) {
		std::cout << "Failed to open device." << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return EXIT_FAILURE;
	}

	if(!device->goOnline()) {
		std::cout << "Failed to go online." << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return EXIT_FAILURE;
	}

	if (!device->uploadCoremini(std::make_unique<std::ifstream>(arguments[2], std::ios::binary), icsneo::Disk::MemoryType::Flash))
	{
		std::cout << "Failed to upload coremini" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
	}

	if (!device->startScript(icsneo::Disk::MemoryType::Flash))
	{
		std::cout << "Failed to start script" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
	}

	device->goOffline();
	device->close();
	return 0;
}