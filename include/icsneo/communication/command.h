#ifndef __COMMAND_H_
#define __COMMAND_H_

namespace icsneo {

enum class Command : uint8_t {
	EnableNetworkCommunication = 0x07,
	RequestSerialNumber = 0xA1,
	SetSettings = 0xA4, // Previously known as RED_CMD_SET_BAUD_REQ, follow up with SaveSettings to write to EEPROM
	GetSettings = 0xA5, // Previously known as RED_CMD_READ_BAUD_REQ
	SaveSettings = 0xA6,
	SetDefaultSettings = 0xA8 // Follow up with SaveSettings to write to EEPROM
};

}

#endif