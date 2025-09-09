#ifndef __MULTICHANNELCOMMUNICATION_H_
#define __MULTICHANNELCOMMUNICATION_H_

#ifdef __cplusplus

#include "icsneo/communication/communication.h"
#include "icsneo/communication/driver.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/third-party/readerwriterqueue/readerwriterqueue.h"

namespace icsneo {

class MultiChannelCommunication : public Communication {
public:
	MultiChannelCommunication(
		device_eventhandler_t err,
		std::unique_ptr<Driver> com,
		std::function<std::unique_ptr<Packetizer>()> makeConfiguredPacketizer,
		std::unique_ptr<Encoder> e,
		std::unique_ptr<Decoder> md,
		size_t vnetCount);
	void spawnThreads() override;
	void joinThreads() override;
	bool sendPacket(std::vector<uint8_t>& bytes) override;

	enum class CommandType : uint8_t {
		PlasmaReadRequest = 0x10, // Status read request to HSC
		PlasmaStatusResponse = 0x11, // Status response by HSC
		HostPC_to_Vnet1 = 0x20, // Host PC data to Vnet module-1
		Vnet1_to_HostPC = 0x21, // Vnet module-1 data to host PC
		HostPC_to_Vnet2 = 0x30, // Host PC data to Vnet module-2
		Vnet2_to_HostPC = 0x31, // Vnet module-2 data to host PC
		HostPC_to_Vnet3 = 0x40, // Host PC data to Vnet module-3
		Vnet3_to_HostPC = 0x41, // Vnet module-3 data to host PC
		HostPC_to_SDCC1 = 0x50, // Host PC data to write to SDCC-1
		HostPC_from_SDCC1 = 0x51, // Host PC wants data read from SDCC-1
		SDCC1_to_HostPC = 0x52, // SDCC-1 data to host PC
		HostPC_to_SDCC2 = 0x60, // Host PC data to write to SDCC-2
		HostPC_from_SDCC2 = 0x61, // Host PC wants data read from SDCC-2
		SDCC2_to_HostPC = 0x62, // SDCC-2 data to host PC
		PC_to_LSOC = 0x70, // Host PC data to LSOCC
		LSOCC_to_PC = 0x71, // LSOCC data to host PC
		HostPC_to_Microblaze = 0x80, // Host PC data to microblaze processor
		Microblaze_to_HostPC = 0x81 // Microblaze processor data to host PC
	};

protected:
	bool preprocessPacket(std::deque<uint8_t>& usbReadFifo);

private:
	static bool CommandTypeIsValid(CommandType cmd) {
		switch(cmd) {
			case CommandType::PlasmaReadRequest:
			case CommandType::PlasmaStatusResponse:
			case CommandType::HostPC_to_Vnet1:
			case CommandType::Vnet1_to_HostPC:
			case CommandType::HostPC_to_Vnet2:
			case CommandType::Vnet2_to_HostPC:
			case CommandType::HostPC_to_Vnet3:
			case CommandType::Vnet3_to_HostPC:
			case CommandType::HostPC_to_SDCC1:
			case CommandType::HostPC_from_SDCC1:
			case CommandType::SDCC1_to_HostPC:
			case CommandType::HostPC_to_SDCC2:
			case CommandType::HostPC_from_SDCC2:
			case CommandType::SDCC2_to_HostPC:
			case CommandType::PC_to_LSOC:
			case CommandType::LSOCC_to_PC:
			case CommandType::HostPC_to_Microblaze:
			case CommandType::Microblaze_to_HostPC:
				return true;
			default:
				return false;
		}	
	}
	static bool CommandTypeHasAddress(CommandType cmd) {
		// Check CommandTypeIsValid before this, you will get false on an invalid command
		switch(cmd) {
			case CommandType::HostPC_to_SDCC1:
			case CommandType::HostPC_from_SDCC1:
			case CommandType::HostPC_to_SDCC2:
			case CommandType::HostPC_from_SDCC2:
				return true;
			default:
				return false;
		}
	}
	static uint16_t CommandTypeDefinesLength(CommandType cmd) {
		// Check CommandTypeIsValid before this, you will get 0 on an invalid command
		switch(cmd) {
			case CommandType::PlasmaStatusResponse:
				return 2;
			default:
				return 0; // Length is defined by following bytes in message
		}
	}

	enum class PreprocessState {
		SearchForCommand,
		ParseAddress,
		ParseLength,
		GetData
	};
	PreprocessState state = PreprocessState::SearchForCommand;
	uint16_t currentCommandLength;
	CommandType currentCommandType;
	size_t currentReadIndex = 0;

	const size_t numVnets;
	std::thread hidReadThread;
	std::vector<std::thread> vnetThreads;
	std::vector< moodycamel::BlockingReaderWriterQueue< std::vector<uint8_t> > > vnetQueues;
	void hidReadTask();
	void vnetReadTask(size_t vnetIndex);

    RingBuffer packetRB;
    std::mutex ringBufMutex;
    std::condition_variable ringBufCV;
};

}

#endif // __cplusplus

#endif