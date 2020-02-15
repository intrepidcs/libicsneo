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

bool STM32::open() {
	if(isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}

	const std::string& ttyPath = HandleToTTY(device.handle);
	if(ttyPath.empty()) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	fd = ::open(ttyPath.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if(!isOpen()) {
		//std::cout << "Open of " << ttyPath.c_str() << " failed with " << strerror(errno) << ' ';
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