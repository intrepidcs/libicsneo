#ifndef __FIRMIO_POSIX_H_
#define __FIRMIO_POSIX_H_

#ifdef __cplusplus

#include "icsneo/device/neodevice.h"
#include "icsneo/device/founddevice.h"
#include "icsneo/communication/driver.h"
#include "icsneo/api/eventmanager.h"
#include <optional>
#include <string>

namespace icsneo {

// This driver is only relevant for communication communication between
// Linux and CoreMini from the onboard processor of the device, you
// likely do not want it enabled for your build.
class FirmIO : public Driver {
public:
	static void Find(std::vector<FoundDevice>& foundDevices);

	using Driver::Driver; // Inherit constructor
	~FirmIO();
	bool open() override;
	bool isOpen() override;
	bool close() override;
	driver_finder_t getFinder() override { return FirmIO::Find; }

private:
	std::thread readThread, writeThread;

	void readTask();
	void writeTask();

	bool writeQueueFull() override;
	bool writeQueueAlmostFull() override;
	bool writeInternal(const std::vector<uint8_t>& bytes) override;

	struct DataInfo {
		uint32_t type;
		uint32_t offset;
		uint32_t size;
	};

	struct ComHeader {
		uint32_t comVer;
		struct DataInfo msgqPtrOut;
		struct DataInfo msgqOut;
		struct DataInfo shmOut;
		struct DataInfo msgqPtrIn;
		struct DataInfo msgqIn;
		struct DataInfo shmIn;
	};

	struct Msg {
		using Ref = uintptr_t;
		enum class Command : uint32_t {
			ComData = 0xAA000000,
			ComFree = 0xAA000001,
			ComReset = 0xAA000002,
		};
		struct Data {
			uint32_t addr;
			uint32_t len;
			Ref ref;
			uint32_t addr1;
			uint32_t len1;
			Ref ref1;
			uint32_t reserved;
		};
		struct Free {
			uint32_t refCount;
			Ref ref[6];
		};
		union Payload {
			Data data;
			Free free;
		};
		Command command;
		Payload payload;
	};

	class MsgQueue { // mq_t
	public:
		MsgQueue(void* infoPtr, void* msgsPtr)
			: info(reinterpret_cast<MsgQueueInfo*>(infoPtr)), msgs(reinterpret_cast<Msg*>(msgsPtr)) {}

		bool read(Msg* msg);
		bool write(const Msg* msg);
		bool isEmpty() const;
		bool isFull() const;

	private:
		struct MsgQueueInfo { // These variables are mmaped, don't change their order or add anything
			uint32_t head;
			uint32_t tail;
			uint32_t size;
			uint32_t reserved[4];
		};
		MsgQueueInfo* const info;
		Msg* const msgs;
	};

	class Mempool {
	public:
		using PhysicalAddress = uintptr_t;
		static constexpr const size_t BlockSize = 4096;

		Mempool(uint8_t* start, uint32_t size, uint8_t* virt, PhysicalAddress phys);
		uint8_t* alloc(uint32_t size);
		bool free(uint8_t* addr);
		PhysicalAddress translate(uint8_t* addr) const;

	private:
		struct BlockInfo {
			enum class Status : uint32_t {
				Free = 0,
				Used = 1,
			};
			Status status;
			uint8_t* addr;
		};

		std::vector<BlockInfo> blocks;
		std::atomic<uint32_t> usedBlocks;

		uint8_t* const virtualAddress;
		const PhysicalAddress physicalAddress;
	};

	int fd = -1;
	uint8_t* vbase = nullptr;
	volatile ComHeader* header = nullptr;

	std::optional<MsgQueue> in;

	std::mutex outMutex;
	std::optional<MsgQueue> out;
	std::optional<Mempool> outMemory;
};

}

#endif // __cplusplus

#endif // __FIRMIO_POSIX_H_