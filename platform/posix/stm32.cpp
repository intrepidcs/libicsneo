#include "icsneo/platform/stm32.h"
#include <dirent.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

using namespace icsneo;

class Directory {
public:
	class Listing {
	public:
		Listing(std::string newName, uint8_t newType) : name(newName), type(newType) {}
		const std::string& getName() const { return name; }
		uint8_t getType() const { return type; }
	private:
		std::string name;
		uint8_t type;
	};
	Directory(std::string directory) {
		dir = opendir(directory.c_str());
	}
	~Directory() {
		if(openedSuccessfully())
			closedir(dir);
		dir = nullptr;
	}
	bool openedSuccessfully() { return dir != nullptr; }
	std::vector<Listing> ls() {
		std::vector<Listing> results;
		struct dirent* entry;
		while((entry = readdir(dir)) != nullptr) {
			std::string name = entry->d_name;
			if(name != "." && name != "..") // Ignore parent and self
				results.emplace_back(name, entry->d_type);
		}
		return results;
	}
private:
	DIR* dir;
};

class USBSerialGetter {
public:
	USBSerialGetter(std::string usbid) {
		std::stringstream ss;
		auto colonpos = usbid.find(":");
		if(colonpos == std::string::npos) {
			succeeded = false;
			return;
		}

		ss << "/sys/bus/usb/devices/" << usbid.substr(0, colonpos) << "/serial";
		try {
			std::ifstream reader(ss.str());
			std::getline(reader, serial);
		} catch(...) {
			succeeded = false;
			return;
		}

		succeeded = true;
	}
	bool success() const { return succeeded; }
	const std::string& getSerial() const { return serial; }
private:
	bool succeeded;
	std::string serial;
};

std::vector<neodevice_t> STM32::FindByProduct(int product) {
	std::vector<neodevice_t> found;

	Directory directory("/sys/bus/usb/drivers/cdc_acm"); // Query the STM32 driver
	if(!directory.openedSuccessfully())
		return found;

	std::vector<std::string> foundusbs;
	for(auto& entry : directory.ls()) {
		/* This directory will have directories (links) for all devices using the cdc_acm driver (as STM32 devices do)
		 * There will also be other files and directories providing information about the driver in here. We want to ignore them.
		 * Devices will be named like "7-2:1.0" where 7 is the enumeration for the USB controller, 2 is the device enumeration on
		 * that specific controller (will change if the device is unplugged and replugged), 1 is the device itself and 0 is 
		 * enumeration for different services provided by the device. We're looking for the service that provides TTY.
		 * For now we find the directories with a digit for the first character, these are likely to be our USB devices.
		 */
		if(isdigit(entry.getName()[0]) && entry.getType() == DT_LNK)
			foundusbs.emplace_back(entry.getName());
	}

	// Pair the USB and TTY if found
	std::map<std::string, std::string> foundttys;
	for(auto& usb : foundusbs) {
		std::stringstream ss;
		ss << "/sys/bus/usb/drivers/cdc_acm/" << usb << "/tty";
		Directory devicedir(ss.str());
		if(!devicedir.openedSuccessfully()) // The tty directory doesn't exist, because this is not the tty service we want
			continue;

		auto listing = devicedir.ls();
		if(listing.size() != 1) // We either got no serial ports or multiple, either way no good
			continue;

		foundttys.insert(std::make_pair(usb, listing[0].getName()));
	}

	// We're going to remove from the map if this is not the product we're looking for
	for(auto iter = foundttys.begin(); iter != foundttys.end(); ) {
		const auto& dev = *iter;
		const std::string matchString = "PRODUCT=";
		std::stringstream ss;
		ss << "/sys/class/tty/" << dev.second << "/device/uevent"; // Read the uevent file, which contains should have a line like "PRODUCT=93c/1101/100"
		std::ifstream fs(ss.str());
		std::string productLine;
		size_t pos = std::string::npos;
		do {
			std::getline(fs, productLine, '\n');
		} while(((pos = productLine.find(matchString)) == std::string::npos) && !fs.eof());

		if(pos != 0) { // We did not find a product line... weird
			iter = foundttys.erase(iter); // Remove the element, this also moves iter forward for us
			continue;
		}

		size_t firstSlashPos = productLine.find('/', matchString.length());
		if(firstSlashPos == std::string::npos) {
			iter = foundttys.erase(iter);
			continue;
		}
		size_t pidpos = firstSlashPos + 1;

		std::string vidstr = productLine.substr(matchString.length(), firstSlashPos - matchString.length());
		std::string pidstr = productLine.substr(pidpos, productLine.find('/', pidpos) - pidpos); // In hex like "1101" or "93c"

		uint16_t vid, pid;
		try {
			vid = (uint16_t)std::stoul(vidstr, nullptr, 16);
			pid = (uint16_t)std::stoul(pidstr, nullptr, 16);
		} catch(...) {
			iter = foundttys.erase(iter); // We could not parse the numbers
			continue;
		}

		if(vid != INTREPID_USB_VENDOR_ID || pid != product) {
			iter = foundttys.erase(iter); // Not the right VID or PID, remove
			continue;
		}
		iter++; // If the loop ends without erasing the iter from the map, the item is good
	}

	// At this point, foundttys contains the the devices we want
	
	// Get the serial number, create the neodevice_t
	for(auto& dev : foundttys) {
		neodevice_t device;

		USBSerialGetter getter(dev.first);
		if(!getter.success())
			continue; // Failure, could not get serial number

		// In ttyACM0, we want the i to be the first character of the number
		size_t i;
		for(i = 0; i < dev.second.length(); i++) {
			if(isdigit(dev.second[i]))
				break;
		}
		// Now we try to parse the number so we have a handle for later
		try {
			device.handle = (neodevice_handle_t)std::stoul(dev.second.substr(i));
			/* The TTY numbering starts at zero, but we want to keep zero for an undefined
			 * handle, so add a constant, and we'll subtract that constant in the open function.
			 */
			device.handle += HANDLE_OFFSET;
		} catch(...) {
			continue; // Somehow this failed, have to toss the device
		}
		
		device.serial[getter.getSerial().copy(device.serial, sizeof(device.serial)-1)] = '\0';

		found.push_back(device); // Finally, add device to search results
	}

	return found;
}

bool STM32::open() {
	if(isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}
	std::stringstream ss;
	ss << "/dev/ttyACM" << (int)(device.handle - HANDLE_OFFSET);
	fd = ::open(ss.str().c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if(!isOpen()) {
		//std::cout << "Open of " << ss.str().c_str() << " failed with " << strerror(errno) << ' ';
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	struct termios tty = {};
	struct termios compare = {};

	if(tcgetattr(fd, &tty) != 0) {
		close();
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	tty.c_cflag |= (CLOCAL | CREAD); // Ignore modem controls
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8; // 8-bit characters
	tty.c_cflag &= ~PARENB; // No parity bit
	tty.c_cflag &= ~CSTOPB; // One stop bit
	tty.c_cflag &= ~CRTSCTS; // No hardware flow control

	// Non-canonical mode
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	// Fetch bytes as they become available
	// See http://man7.org/linux/man-pages/man3/termios.3.html
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 1; // 100ms timeout (1 decisecond, what?)

	if(tcsetattr(fd, TCSAFLUSH, &tty) != 0) { // Flushes input and output buffers as well as setting settings
		close();
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	if(tcgetattr(fd, &compare) != 0 || memcmp(&tty, &compare, sizeof(struct termios)) != 0) {
		close();
		return false;
	}

	// Create threads
	readThread = std::thread(&STM32::readTask, this);
	writeThread = std::thread(&STM32::writeTask, this);
	
	return true;
}

bool STM32::isOpen() {
	return fd >= 0; // Negative fd indicates error or not opened yet
}

bool STM32::close() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	closing = true;

	if(readThread.joinable())
		readThread.join();
	
	if(writeThread.joinable())
		writeThread.join();

	closing = false;

	int ret = ::close(fd);
	fd = -1;

	uint8_t flush;
	WriteOperation flushop;
	while (readQueue.try_dequeue(flush)) {}
	while (writeQueue.try_dequeue(flushop)) {}
	
	if(ret == 0) {
		return true;
	} else {
		report(APIEvent::Type::DriverFailedToClose, APIEvent::Severity::Error);
		return false;
	}
}

void STM32::readTask() {
	constexpr size_t READ_BUFFER_SIZE = 8;
	uint8_t readbuf[READ_BUFFER_SIZE];
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!closing) {
		auto bytesRead = ::read(fd, readbuf, READ_BUFFER_SIZE);
		if(bytesRead > 0)
			readQueue.enqueue_bulk(readbuf, bytesRead);
	}
}

void STM32::writeTask() {
	WriteOperation writeOp;
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!closing) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		const ssize_t writeSize = (ssize_t)writeOp.bytes.size();
		ssize_t actualWritten = ::write(fd, writeOp.bytes.data(), writeSize);
		if(actualWritten != writeSize)
			report(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
		onWrite();
	}
}