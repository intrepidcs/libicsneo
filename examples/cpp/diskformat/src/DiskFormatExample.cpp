#include <iostream>
#include <string>

#include "icsneo/icsneocpp.h"
#include "icsneo/disk/diskdetails.h"

int main() {
	std::cout << "Running libicsneo " << icsneo::GetVersion() << std::endl;

	std::cout << "\nFinding devices... " << std::flush;
	auto devices = icsneo::FindAllDevices();
	std::cout << "OK, " << devices.size() << " device" << (devices.size() == 1 ? "" : "s") << " found" << std::endl;

	if(devices.empty()) {
		std::cout << "error: no devices found" << std::endl;
		return -1;
	}

	// List devices and let the user pick one
	for(size_t i = 0; i < devices.size(); i++) {
		std::cout << "  [" << i << "] " << devices[i]->describe() << std::endl;
	}

	size_t choice = 0;
	if(devices.size() > 1) {
		std::cout << "Select a device [0-" << (devices.size() - 1) << "]: ";
		std::cin >> choice;
		if(choice >= devices.size()) {
			std::cout << "error: invalid selection" << std::endl;
			return -1;
		}
	}

	auto& device = devices[choice];
	std::cout << "\nOpening " << device->describe() << "... " << std::flush;
	if(!device->open()) {
		std::cout << "FAIL" << std::endl;
		std::cout << "error: " << icsneo::GetLastError() << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	// Check that this device supports disk formatting
	if(!device->supportsDiskFormatting()) {
		std::cout << "error: " << device->describe() << " does not support disk formatting" << std::endl;
		device->close();
		return -1;
	}

	std::cout << "Disk count: " << device->getDiskCount() << std::endl;

	// Query the current disk state from the device
	std::cout << "\nQuerying disk details... " << std::flush;
	auto details = device->getDiskDetails();
	if(!details) {
		std::cout << "FAIL" << std::endl;
		std::cout << "error: " << icsneo::GetLastError() << std::endl;
		device->close();
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "  Layout        : " << (details->layout == icsneo::DiskLayout::RAID0 ? "RAID0" : "Spanned") << std::endl;
	for(size_t i = 0; i < details->disks.size(); i++) {
		const auto& disk = details->disks[i];
		std::cout << "  Disk [" << i << "]:" << std::endl;
		std::cout << "    Present     : " << (disk.present     ? "yes" : "no") << std::endl;
		std::cout << "    Initialized : " << (disk.initialized ? "yes" : "no") << std::endl;
		std::cout << "    Formatted   : " << (disk.formatted   ? "yes" : "no") << std::endl;
		if(disk.present) {
			uint64_t bytes = disk.size();
			std::cout << "    Size        : " << (bytes / (1024 * 1024)) << " MB"
			          << " (" << disk.sectors << " sectors x " << disk.bytesPerSector << " bytes)" << std::endl;
		}
	}

	// Build a format configuration.
	// We keep the existing layout and re-use the disk geometry reported by the device.
	// The 'formatted' flag must be true for each disk you want the device to format.
	icsneo::DiskDetails formatConfig;
	formatConfig.layout = details->layout;
	formatConfig.fullFormat = false; // Quick format; set to true for a full (slow) format
	formatConfig.disks = details->disks;

	// Mark all present disks for formatting
	bool anyPresent = false;
	for(auto& disk : formatConfig.disks) {
		if(disk.present) {
			disk.formatted = true;
			anyPresent = true;
		}
	}

	if(!anyPresent) {
		std::cout << "\nerror: no disks are present in the device" << std::endl;
		device->close();
		return -1;
	}

	std::cout << "\nThis will format the disk(s) in " << device->describe() << "." << std::endl;
	std::cout << "All existing data will be lost. Continue? [y/N]: ";
	std::string confirm;
	std::cin >> confirm;
	if(confirm != "y" && confirm != "Y") {
		std::cout << "Aborted." << std::endl;
		device->close();
		return 0;
	}

	std::cout << "\nStarting format..." << std::endl;

	// Progress callback — called every 500 ms while formatting
	auto progressHandler = [](uint64_t sectorsFormatted, uint64_t sectorsTotal) -> icsneo::Device::DiskFormatDirective {
		double pct = sectorsTotal > 0 ? (100.0 * sectorsFormatted / sectorsTotal) : 0.0;
		std::cout << "\r  Progress: " << sectorsFormatted << " / " << sectorsTotal
		          << " sectors  (" << static_cast<int>(pct) << "%)" << std::flush;
		return icsneo::Device::DiskFormatDirective::Continue;
	};

	bool success = device->formatDisk(formatConfig, progressHandler);
	std::cout << std::endl; // newline after progress line

	if(!success) {
		std::cout << "error: format failed: " << icsneo::GetLastError() << std::endl;
		device->close();
		return -1;
	}
	std::cout << "Format complete!" << std::endl;

	// Verify by re-querying disk details
	std::cout << "\nVerifying disk state after format... " << std::flush;
	auto postDetails = device->getDiskDetails();
	if(!postDetails) {
		std::cout << "FAIL (could not re-query disk details)" << std::endl;
	} else {
		std::cout << "OK" << std::endl;
		for(size_t i = 0; i < postDetails->disks.size(); i++) {
			const auto& disk = postDetails->disks[i];
			std::cout << "  Disk [" << i << "] formatted: " << (disk.formatted ? "yes" : "no") << std::endl;
		}
	}

	device->close();
	return 0;
}
