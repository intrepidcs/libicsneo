#ifndef __MULTICHANNELCOMMUNICATION_H_
#define __MULTICHANNELCOMMUNICATION_H_

#include "communication/include/communication.h"
#include "communication/include/icommunication.h"

namespace icsneo {

class MultiChannelCommunication : public Communication {
public:
	MultiChannelCommunication(std::shared_ptr<ICommunication> com) : Communication(com) {}
	void spawnThreads();
	void joinThreads();
	bool sendCommand(Communication::Command cmd, std::vector<uint8_t> arguments);

protected:
	bool preprocessPacket(std::deque<uint8_t>& usbReadFifo);

private:
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
			case CommandType::SDCC1_to_HostPC:
			case CommandType::SDCC2_to_HostPC:
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

	std::thread mainChannelReadThread;
	void readTask();
};

}

#endif