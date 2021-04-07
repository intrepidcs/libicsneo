#ifndef __COMMAND_H_
#define __COMMAND_H_

#ifdef __cplusplus

namespace icsneo {

enum class Command : uint8_t {
	EnableNetworkCommunication = 0x07,
	EnableNetworkCommunicationEx = 0x08,
	RequestSerialNumber = 0xA1,
	SetSettings = 0xA4, // Previously known as RED_CMD_SET_BAUD_REQ, follow up with SaveSettings to write to EEPROM
	//GetSettings = 0xA5, // Previously known as RED_CMD_READ_BAUD_REQ, now unused
	SaveSettings = 0xA6,
	UpdateLEDState = 0xA7,
	SetDefaultSettings = 0xA8, // Follow up with SaveSettings to write to EEPROM
	RequestStatusUpdate = 0xBC,
	ReadSettings = 0xC7, // Previously known as 3G_READ_SETTINGS_EX
	MiscControl = 0xE7,
	FlexRayControl = 0xF3
};

}

#endif // __cplusplus

#endif