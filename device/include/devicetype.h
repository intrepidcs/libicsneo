#ifndef __NETWORKID_H_
#define __NETWORKID_H_

#include <cstdint>
#include <ostream>

namespace icsneo {

class DeviceType {
public:
	// This enum used to be a bitfield, but has since become an enum as we have more than 32 devices
	enum : uint32_t {
		UNKNOWN = (0x00000000),
		BLUE = (0x00000001),
		ECU_AVB = (0x00000002),
		RADSUPERMOON = (0x00000003),
		DW_VCAN = (0x00000004),
		RADMOON2 = (0x00000005),
		RADGIGALOG = (0x00000006),
		VCAN41 = (0x00000007),
		FIRE = (0x00000008),
		RADPLUTO = (0x00000009),
		VCAN42_EL = (0x0000000a),
		RADIO_CANHUB = (0x0000000b),
		VCAN3 = (0x00000010),
		RED = (0x00000040),
		ECU = (0x00000080),
		IEVB = (0x00000100),
		PENDANT = (0x00000200),
		OBD2_PRO = (0x00000400),
		ECUCHIP_UART = (0x00000800),
		PLASMA = (0x00001000),
		DONT_REUSE0 = (0x00002000), // Previously FIRE_VNET
		NEOANALOG = (0x00004000),
		CT_OBD = (0x00008000),
		DONT_REUSE1 = (0x00010000), // Previously PLASMA_1_12
		DONT_REUSE2 = (0x00020000), // Previously PLASMA_1_13
		ION = (0x00040000),
		RADSTAR = (0x00080000),
		DONT_REUSE3 = (0x00100000), // Previously ION3
		VCAN44 = (0x00200000),
		VCAN42 = (0x00400000),
		CMPROBE = (0x00800000),
		EEVB = (0x01000000),
		VCANRF = (0x02000000),
		FIRE2 = (0x04000000),
		FLEX = (0x08000000),
		RADGALAXY = (0x10000000),
		RADSTAR2 = (0x20000000),
		VIVIDCAN = (0x40000000),
		OBD2_SIM = (0x80000000)
	};
	// static const char* GetNetIDString(NetID netid) {
	// 	switch(netid) {
	// 		default:
	// 			return "Invalid Network";
	// 	}
	// }

	// Network() { setValue(NetID::Invalid); }
	// Network(uint16_t netid) { setValue((NetID)netid); }
	// Network(NetID netid) { setValue(netid); }
	// NetID getNetID() const { return value; }
	// Type getType() const { return type; }
	// friend std::ostream& operator<<(std::ostream& os, const Network& network) {
	// 	os << GetNetIDString(network.getNetID());
	// 	return os;
	// }

private:
	// NetID value; // Always use setValue so that value and type stay in sync
	// Type type;
	// void setValue(NetID id) {
	// 	value = id;
	// 	type = GetTypeOfNetID(value);
	// }
};

};

#endif