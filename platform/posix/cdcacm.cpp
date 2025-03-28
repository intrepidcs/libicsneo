#include "icsneo/platform/cdcacm.h"
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
#include <sys/select.h>

using namespace icsneo;

CDCACM::~CDCACM() {
	awaitModeChangeComplete();
	if(isOpen())
		close();
}

bool CDCACM::open() {
	if(isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}

	const std::string& ttyPath = HandleToTTY(device.handle);
	if(ttyPath.empty()) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	// Some devices can take a while to boot
	for(int i = 0; i != 50; ++i) {
		fd = ::open(ttyPath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

		if(fd != -1) {
			// Opened successfully, check if it's the same inode as the disallowed one
			// The inode is disallowed if we're expecting the device to re-enumerate (modeChanging)
			if(disallowedInode.has_value()) {
				struct stat fileStat = {};
				if(fstat(fd, &fileStat) >= 0) {
					if(fileStat.st_ino != *disallowedInode)
						break;
				}
			} else {
				break;
			}

			// This is still the old device, wait and try again
			::close(fd);
			fd = -1;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	disallowedInode.reset();
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
	readThread = std::thread(&CDCACM::readTask, this);
	writeThread = std::thread(&CDCACM::writeTask, this);
	
	return true;
}

bool CDCACM::isOpen() {
	return fd >= 0; // Negative fd indicates error or not opened yet
}

bool CDCACM::close() {
	if(!isOpen() && !isDisconnected()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	setIsClosing(true);

	if(readThread.joinable())
		readThread.join();
	
	if(writeThread.joinable())
		writeThread.join();

	setIsClosing(false);
	setIsDisconnected(false);

	if(modeChanging) {
		// We're expecting this inode to go away after we close the device
		// In order to block waiting for this to happen, we first need to
		// get the inode.
		struct stat fileStat = {};
		if(fstat(fd, &fileStat) >= 0)
			disallowedInode = fileStat.st_ino;
	}

	int ret = ::close(fd);
	fd = -1;

	clearBuffers();

	if(modeChanging) {
		modeChanging = false;
		return open(); // Reopen the reenumerated device
	}
	
	if(ret == 0) {
		return true;
	} else {
		report(APIEvent::Type::DriverFailedToClose, APIEvent::Severity::Error);
		return false;
	}
}

void CDCACM::modeChangeIncoming() {
	modeChanging = true;
}

void CDCACM::awaitModeChangeComplete() {
	std::unique_lock<std::mutex> lk(modeChangeMutex);
	if(modeChanging && !modeChangeThread.joinable()) // Waiting for the thread to start
		modeChangeCV.wait_for(lk, std::chrono::seconds(1), [this] { return modeChangeThread.joinable(); });
	if(modeChangeThread.joinable())
		modeChangeThread.join();
}

void CDCACM::readTask() {
	constexpr size_t READ_BUFFER_SIZE = 2048;
	uint8_t readbuf[READ_BUFFER_SIZE];
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!isClosing() && !isDisconnected()) {
		fd_set rfds = {0};
		struct timeval tv = {};
		FD_SET(fd, &rfds);
		tv.tv_usec = 50000; // 50ms
		::select(fd + 1, &rfds, NULL, NULL, &tv);
		auto bytesRead = ::read(fd, readbuf, READ_BUFFER_SIZE);
		if(bytesRead > 0) {
#if 0 // Perhaps helpful for debugging :)
			std::cout << "Read data: (" << bytesRead << ')' << std::hex << std::endl;
			for(int i = 0; i < bytesRead; i += 16) {
				for(int j = 0; j < std::min<int>(bytesRead - i, 16); j++)
					std::cout << std::setw(2) << std::setfill('0') << uint32_t(readbuf[i+j]) << ' ';
				std::cout << std::endl;
			}
			std::cout << std::dec << std::endl;
#endif
			pushRx(readbuf, bytesRead);
		} else {
			if(modeChanging) {
				// We were expecting a disconnect for reenumeration
				modeChangeThread = std::thread([this] {
					modeChangeCV.notify_all();
					// Requesting thread is responsible for calling close. This allows for more flexibility
				});
				break;
			} else if(!isClosing() && !fdIsValid() && !isDisconnected()) {
				setIsDisconnected(true);
				report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
			}
		}
	}
}

void CDCACM::writeTask() {
	WriteOperation writeOp;
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!isClosing() && !isDisconnected()) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		const ssize_t totalWriteSize = (ssize_t)writeOp.bytes.size();
		ssize_t totalWritten = 0;
		while(totalWritten < totalWriteSize) {
			const ssize_t writeSize = totalWriteSize - totalWritten;
			ssize_t actualWritten = ::write(fd, writeOp.bytes.data() + totalWritten, writeSize);
			if(actualWritten != writeSize) {
				// If we partially wrote, it's probably EAGAIN but it won't have been set
				// so don't signal an error unless it's < 0, we'll come back around and
				// get a -1 to see the real error.
				if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
					// We filled the TX FIFO, use select to wait for it to become available again
					fd_set wfds = {0};
					struct timeval tv = {};
					FD_SET(fd, &wfds);
					tv.tv_usec = 50000; // 50ms
					::select(fd + 1, nullptr, &wfds, nullptr, &tv);
				} else if (actualWritten < 0) {
					if(!fdIsValid()) {
						if(!isDisconnected()) {
							setIsDisconnected(true);
							report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
						}
					} else
						report(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
					break;
				}
			}
			if(actualWritten > 0) {
#if 0 // Perhaps helpful for debugging :)
				std::cout << "Wrote data: (" << actualWritten << ')' << std::hex << std::endl;
				for(int i = 0; i < actualWritten; i += 16) {
					for(int j = 0; j < std::min<int>(actualWritten - i, 16); j++)
						std::cout << std::setw(2) << std::setfill('0') << uint32_t(writeOp.bytes[totalWritten+i+j]) << ' ';
					std::cout << std::endl;
				}
				std::cout << std::dec << std::endl;
#endif

				totalWritten += actualWritten;
			}
		}
	}
}

bool CDCACM::fdIsValid() {
	struct termios tty = {};
	return tcgetattr(fd, &tty) == 0 ? true : false;
}
