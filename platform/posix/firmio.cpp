#include "icsneo/platform/firmio.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/communication/message/serialnumbermessage.h"
#include <dirent.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/resource.h>

using namespace icsneo;

#define PHY_ADDR_BASE (0x1E000000) // IPC0
#define MMAP_LEN (0x1000000)
#define FIRMIO_DEV "/dev/firmio"
#define COM_VER (0xC000)
#define memory_barrier() __sync_synchronize()

void FirmIO::Find(std::vector<FoundDevice>& found) {
	FirmIO temp([](APIEvent::Type, APIEvent::Severity) {});
	if(!temp.open())
		return;

	std::vector<uint8_t> payload = {
		((1 << 4) | (uint8_t)Network::NetID::Main51), // Packet size of 1 on NETID_MAIN51
		(uint8_t)Command::RequestSerialNumber
	};
	payload.push_back(Packetizer::ICSChecksum(payload));
	payload.insert(payload.begin(), 0xAA);
	temp.write(payload);

	Packetizer packetizer([](APIEvent::Type, APIEvent::Severity) {});
	Decoder decoder([](APIEvent::Type, APIEvent::Severity) {});
	using namespace std::chrono;
	// Get an absolute wall clock to compare to
	const auto overallTimeout = steady_clock::now() + milliseconds(200);
	size_t lastBufferSize = 0;
	while (steady_clock::now() < overallTimeout) 
	{
		temp.waitForRx(lastBufferSize + 1, milliseconds(100));
		bool havePacket = packetizer.input(temp.getReadBuffer());
		lastBufferSize = temp.getReadBuffer().size();

		if(!havePacket)
			continue; // A full packet has not yet been read out

		for(const auto& packet : packetizer.output()) {
			std::shared_ptr<Message> message;
			if(!decoder.decode(message, packet))
				continue; // Malformed packet

			const auto serial = std::dynamic_pointer_cast<SerialNumberMessage>(message);
			if(!serial || serial->deviceSerial.size() != 6)
				continue; // Not a serial number message

			FoundDevice foundDevice;
			// Don't need a handle, only one device will be found
			// Setting one anyway in case anyone checks for 0 as invalid handle
			foundDevice.handle = 1;
			memcpy(foundDevice.serial, serial->deviceSerial.c_str(), sizeof(foundDevice.serial) - 1);
			foundDevice.serial[sizeof(foundDevice.serial) - 1] = '\0';

			foundDevice.makeDriver = [](const device_eventhandler_t& report, neodevice_t&) {
				return std::unique_ptr<Driver>(new FirmIO(report));
			};

			found.push_back(foundDevice);
			break; // never going to find two!
		}
	}
}

FirmIO::~FirmIO() {
	if(isOpen())
		close();
}

bool FirmIO::open() {
	if(isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}

	fd = ::open(FIRMIO_DEV, O_RDWR);
	if(!isOpen()) {
		//std::cout << "Open of " << ttyPath.c_str() << " failed with " << strerror(errno) << ' ';
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	vbase = reinterpret_cast<uint8_t*>(mmap(nullptr, MMAP_LEN, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, PHY_ADDR_BASE));
	if(vbase == MAP_FAILED) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	header = reinterpret_cast<ComHeader*>(vbase);
	if(header->comVer != COM_VER) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	// Swapping the in and out ptrs here, what the device considers out, we consider in
	out.emplace(header->msgqPtrIn.offset + vbase, header->msgqIn.offset + vbase);
	in.emplace(header->msgqPtrOut.offset + vbase, header->msgqOut.offset + vbase);
	outMemory.emplace(vbase + header->shmIn.offset, header->shmIn.size, vbase, PHY_ADDR_BASE);

	// Flush any messages that are stuck in the pipe
	Msg msg;
	std::vector<Msg> toFree;

	int i = 0;
	while(!in->isEmpty() && i++ < 10000) {
		if(!in->read(&msg))
			break;

		switch(msg.command) {
		case Msg::Command::ComData: {
			if(toFree.empty() || toFree.back().payload.free.refCount == 6) {
				toFree.emplace_back();
				toFree.back().command = Msg::Command::ComFree;
				toFree.back().payload.free.refCount = 0;
			}

			// Add this ref to the list of payloads to free
			// After we process these, we'll send this list back to the device
			// so that it can free these entries
			toFree.back().payload.free.ref[toFree.back().payload.free.refCount] = msg.payload.data.ref;
			toFree.back().payload.free.refCount++;
			break;
		}
		}
	}

	// std::cout << "Flushed " << std::dec << i << " freeing " << toFree.size() << std::endl;

	auto endTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
	while(std::chrono::steady_clock::now() < endTime && !toFree.empty()) {
		bool pass = false;
		{
			std::scoped_lock lk(outMutex);
			pass = out->write(&toFree.back());
		}
		if (!pass)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		toFree.pop_back();
	}

	// Create threads
	readThread = std::thread(&FirmIO::readTask, this);
	//logThread = std::thread(&FirmIO::logTask, this);
	writeThread = std::thread(&FirmIO::writeTask, this);
	
	return true;
}

bool FirmIO::isOpen() {
	return fd >= 0; // Negative fd indicates error or not opened yet
}

bool FirmIO::close() {
	if(!isOpen() && !isDisconnected()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	setIsClosing(true);

	if(readThread.joinable())
		readThread.join();

	if (writeThread.joinable())
		writeThread.join();

	// if(logThread.joinable())
	// 	logThread.join();


	setIsClosing(false);
	setIsDisconnected(false);

	int ret = 0;
	if(vbase != nullptr) {
		ret |= munmap(vbase, MMAP_LEN);
		vbase = nullptr;
	}

	ret |= ::close(fd);
	fd = -1;

	if(ret == 0) {
		return true;
	} else {
		report(APIEvent::Type::DriverFailedToClose, APIEvent::Severity::Error);
		return false;
	}
}

void FirmIO::readTask() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	Msg msg;
	std::vector<Msg::Ref> toFree;
	toFree.reserve(outMemory->getTotalBlocks());

	// attempt to elevate the thread priority. PRIO_MIN is actually the highest priority but the lowest value.
	int err = setpriority(PRIO_PROCESS, 0, -1);
	if (err != 0) {
		std::cerr << "FirmIO::readTask setpriority failed : " << strerror(errno)  << std::endl;
	}

	while(!isClosing() && !isDisconnected()) {
		fd_set rfds = {0};
		struct timeval tv = {0};
		FD_SET(fd, &rfds);
		tv.tv_usec = 50000; // 50ms
		int ret = ::select(fd + 1, &rfds, NULL, NULL, &tv);
		if(ret < 0)
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
		if(ret <= 0)
			continue;

		uint32_t interruptCount = 0;
		ret = ::read(fd, &interruptCount, sizeof(interruptCount));
		if(ret < 0)
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
		if(ret < int(sizeof(interruptCount)) || interruptCount < 1)
			continue;

		while(in->read(&msg)) {
			switch(msg.command) {
			case Msg::Command::ComData: {

				toFree.push_back(msg.payload.data.ref);
				++num_read;

				// Translate the physical address back to our virtual address space
				uint8_t* addr = reinterpret_cast<uint8_t*>(msg.payload.data.addr - PHY_ADDR_BASE + vbase);
				while (!pushRx(addr, msg.payload.data.len)) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1)); // back-off so reading thread can empty the buffer
					if (isClosing() || isDisconnected()) {
						break;
					}
				}
			}
			break;
			case Msg::Command::ComFree: {
				std::scoped_lock lk(outMutex);
				for(uint32_t i = 0; i < msg.payload.free.refCount; i++)
					outMemory->free(reinterpret_cast<uint8_t*>(msg.payload.free.ref[i]));
				break;
			}
			default:
				// std::cout << "invalid command: " << std::hex << static_cast<uint32_t>(msg.command) << std::dec << std::endl;
				break;
			}
			if (isClosing() || isDisconnected())
				break;
		}
		while (toFree.size()) {
			Msg freeMsg = { Msg::Command::ComFree };
			freeMsg.payload.free.refCount = std::min(static_cast<uint32_t>(toFree.size()), 6u);
			for (size_t i = 0; i < freeMsg.payload.free.refCount; ++i) {
				freeMsg.payload.free.ref[i] = toFree[i];
			}
			std::scoped_lock lk(outMutex);
			if (!out->write(&freeMsg)) {
				break;
			}
			num_freed += freeMsg.payload.free.refCount;
			toFree.erase(toFree.begin(), toFree.begin() + freeMsg.payload.free.refCount);
		}
	}
	while (toFree.size())
	{
		Msg freeMsg = { Msg::Command::ComFree };
		freeMsg.payload.free.refCount = std::min(static_cast<uint32_t>(toFree.size()), 6u);
		for (size_t i = 0; i < freeMsg.payload.free.refCount; ++i) {
			freeMsg.payload.free.ref[i] = toFree[i];
		}
		std::scoped_lock lk(outMutex);
		if (!out->write(&freeMsg)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		toFree.erase(toFree.begin(), toFree.begin() + freeMsg.payload.free.refCount);
	}
	// std::cout << "FirmIO readTask exiting: " << "closing=" << isClosing() << " disconnected=" << isDisconnected() << std::endl;
}

void FirmIO::writeTask() {
	constexpr uint32_t genInterrupt = 0x01;
	std::pair<std::optional<WriteOperation>, uint8_t*> op;
	while (!isClosing() && !isDisconnected()) {
		if (!op.first) {
			writeQueue.wait_dequeue_timed(op.first, std::chrono::milliseconds(100));
			continue;
		}

		if (!op.second) {
			op.second = outMemory->alloc(static_cast<uint32_t>(op.first->bytes.size()));
			if (op.second == nullptr) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			memcpy(op.second, op.first->bytes.data(), op.first->bytes.size());
		}

		Msg msg = { Msg::Command::ComData };
		msg.payload.data.addr = outMemory->translate(op.second);
		msg.payload.data.len = op.first->bytes.size();
		msg.payload.data.ref = reinterpret_cast<Msg::Ref>(op.second);

		
		std::scoped_lock lk(outMutex);
		if(!out->write(&msg))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		++num_written;
		::write(fd, &genInterrupt, sizeof(genInterrupt));
		op.first.reset();
		op.second = nullptr;
	}
	std::cout << "FirmIO writeTask exiting: " << "closing=" << isClosing() << " disconnected=" << isDisconnected() << std::endl;
}

bool FirmIO::writeInternal(const std::vector<uint8_t>& bytes) {
	if(bytes.empty() || bytes.size() > Mempool::BlockSize)
	{
		// std::cout << "Invalid write size of " << bytes.size() << std::endl;
		return false;
	}
	
	return writeQueue.enqueue(WriteOperation(bytes));
}

bool FirmIO::MsgQueue::read(Msg* msg) {
	if(isEmpty()) // Contains memory_barrier()
		return false;

	memcpy(msg, &msgs[info->tail], sizeof(*msg));
	info->tail = (info->tail + 1) & (info->size - 1);
	memory_barrier();
	return true;
}

bool FirmIO::MsgQueue::write(const Msg* msg) {
	if(isFull()) // Contains memory_barrier()
		return false;

	memcpy(&msgs[info->head], msg, sizeof(*msg));
	info->head = (info->head + 1) & (info->size - 1);
	memory_barrier();
	return true;
}

bool FirmIO::MsgQueue::isEmpty() const {
	memory_barrier();
	return info->head == info->tail;
}

bool FirmIO::MsgQueue::isFull() const {
	memory_barrier();
	return ((info->head + 1) & (info->size - 1)) == info->tail;
}

FirmIO::Mempool::Mempool(uint8_t* start, uint32_t size, uint8_t* virt, PhysicalAddress phys)
	: blocks(size / BlockSize), usedBlocks(0),
	  virtualAddress(virt), physicalAddress(phys) {
	size_t idx = 0;
	for(BlockInfo& block : blocks) {
		block.status = BlockInfo::Status::Free;
		block.addr = start + idx * BlockSize;
		idx++;
	}
}

uint8_t* FirmIO::Mempool::alloc(uint32_t size) {
	if(usedBlocks == blocks.size())
		return nullptr;

	if(size > BlockSize)
		return nullptr;

	auto found = std::find_if(blocks.begin(), blocks.end(), [](const BlockInfo& b) {
		return b.status == BlockInfo::Status::Free;
	});
	if(found == blocks.end())
		return nullptr; // No free blocks, inconsistency with usedBlocks

	found->status = BlockInfo::Status::Used;
	usedBlocks++;
	return found->addr;
}

bool FirmIO::Mempool::free(uint8_t* addr) {
	auto found = std::find_if(blocks.begin(), blocks.end(), [&addr](const BlockInfo& b) {
		return b.addr == addr;
	});

	if(found == blocks.end()) {
		// std::cout << "failed to free block address " << std::hex << reinterpret_cast<uintptr_t>(addr) << std::dec << std::endl;
		return false; // Invalid address
	}

	if(found->status != BlockInfo::Status::Used) {
		// std::cout << "invalid state for free of block address " << std::hex << reinterpret_cast<uintptr_t>(addr) << std::dec << std::endl;
		return false; // Double free
	}

	--usedBlocks;
	found->status = BlockInfo::Status::Free;
	return true;
}

FirmIO::Mempool::PhysicalAddress FirmIO::Mempool::translate(uint8_t* addr) const {
	return reinterpret_cast<PhysicalAddress>(addr - virtualAddress + physicalAddress);
}

// void FirmIO::logTask()
// {
// 	while (!isClosing() && !isDisconnected()) {
// 		std::cout << "FirmIO Stats: RX Count: " << num_read << " TX Count: " << num_written << " Used Blocks (out): " << outMemory->getUsedBlocks() << " Freed Blocks: " << num_freed << std::endl;
// 		std::this_thread::sleep_for(std::chrono::seconds(1));
// 	}
// 	std::cout << "FirmIO logTask exiting: " << "closing=" << isClosing() << " disconnected=" << isDisconnected() << std::endl;
// }
