// Usage:
// ./libicsneocpp-coremini [DEVICE_SERIAL] [COREMINI_SCRIPT_PATH] [FLASH | SD]


#include <iostream>
#include <fstream>
#include <icsneo/icsneocpp.h>

void displayUsage() {
	std::cout << "Usage:\n";
	std::cout << "./libicsneocpp-coremini [DEVICE_SERIAL] [COREMINI_SCRIPT_PATH] [FLASH | SD]\n";
}


int main(int argc, char** argv) {
	std::vector<std::string> arguments(argv, argv + argc);

	if(arguments.size() != 4) {
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

	std::string memTypeString = arguments[3];

	icsneo::Disk::MemoryType type;
	if(memTypeString == "FLASH") {
		type = icsneo::Disk::MemoryType::Flash;
	} else if(memTypeString == "SD") {
		type = icsneo::Disk::MemoryType::SD;
	} else {
		std::cout << "Incorrect memory type option" << std::endl;
		displayUsage();
		return EXIT_FAILURE;
	}

	std::ifstream coreminiFile(arguments[2], std::ios::binary);

	if (!device->uploadCoremini(coreminiFile, type)) {
		std::cout << "Failed to upload coremini" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
	}

	if (!device->startScript(type)) {
		std::cout << "Failed to start script" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
	}

	device->close();
	return 0;
}