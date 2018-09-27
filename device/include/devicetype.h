#ifndef __DEVICETYPE_H_
#define __DEVICETYPE_H_

typedef uint32_t devicetype_t;

// Hold the length of the longest name, so that C applications can allocate memory accordingly
// Currently the longest is "Intrepid Ethernet Evaluation Board"
#define DEVICE_TYPE_LONGEST_NAME (35 + 1) // Add 1 so that if someone forgets, they still have space for null terminator

#ifndef __cplusplus
#include <stdint.h>
#else
#include <ostream>
#include <cstdint>

namespace icsneo {

class DeviceType {
public:
	// This enum used to be a bitfield, but has since become an enum as we have more than 32 devices
	enum Enum : devicetype_t {
		Unknown = (0x00000000),
		BLUE = (0x00000001),
		ECU_AVB = (0x00000002),
		RADSupermoon = (0x00000003),
		DW_VCAN = (0x00000004),
		RADMoon2 = (0x00000005),
		RADGigalog = (0x00000006),
		VCAN4_1 = (0x00000007),
		FIRE = (0x00000008),
		RADPluto = (0x00000009),
		VCAN4_2EL = (0x0000000a),
		RADIO_CANHUB = (0x0000000b),
		VCAN3 = (0x00000010),
		RED = (0x00000040),
		ECU = (0x00000080),
		IEVB = (0x00000100),
		Pendant = (0x00000200),
		OBD2_PRO = (0x00000400),
		ECUChip_UART = (0x00000800),
		PLASMA = (0x00001000),
		DONT_REUSE0 = (0x00002000), // Previously FIRE_VNET
		NEOAnalog = (0x00004000),
		CT_OBD = (0x00008000),
		DONT_REUSE1 = (0x00010000), // Previously PLASMA_1_12
		DONT_REUSE2 = (0x00020000), // Previously PLASMA_1_13
		ION = (0x00040000),
		RADStar = (0x00080000),
		DONT_REUSE3 = (0x00100000), // Previously ION3
		VCAN4_4 = (0x00200000),
		VCAN4_2 = (0x00400000),
		CMProbe = (0x00800000),
		EEVB = (0x01000000),
		VCANrf = (0x02000000),
		FIRE2 = (0x04000000),
		Flex = (0x08000000),
		RADGalaxy = (0x10000000),
		RADStar2 = (0x20000000),
		VividCAN = (0x40000000),
		OBD2_SIM = (0x80000000)
	};
	static const char* GetDeviceTypeString(DeviceType::Enum type) {
		// Adding something? Make sure you update DEVICE_TYPE_LONGEST_NAME at the top!
		switch(type) {
			case Unknown:
				return "Unknown";
			case BLUE:
				return "neoVI BLUE";
			case ECU_AVB:
				return "neoECU AVB";
			case RADSupermoon:
				return "RADSupermoon";
			case DW_VCAN:
				return "DW_VCAN";
			case RADMoon2:
				return "RADMoon 2";
			case RADGigalog:
				return "RADGigalog";
			case VCAN4_1:
				return "ValueCAN 4-1";
			case FIRE:
				return "neoVI FIRE";
			case RADPluto:
				return "RADPluto";
			case VCAN4_2EL:
				return "ValueCAN 4-2EL";
			case RADIO_CANHUB:
				return "RADIO_CANHUB";
			case VCAN3:
				return "ValueCAN 3";
			case RED:
				return "neoVI RED";
			case ECU:
				return "neoECU";
			case IEVB:
				return "IEVB";
			case Pendant:
				return "Pendant";
			case OBD2_PRO:
				return "neoOBD2 PRO";
			case ECUChip_UART:
				return "neoECU Chip UART";
			case PLASMA:
				return "neoVI PLASMA";
			case NEOAnalog:
				return "NEOAnalog";
			case CT_OBD:
				return "CT_OBD";
			case ION:
				return "neoVI ION";
			case RADStar:
				return "RADStar";
			case VCAN4_4:
				return "ValueCAN 4-4";
			case VCAN4_2:
				return "ValueCAN 4-2";
			case CMProbe:
				return "CMProbe";
			case EEVB:
				return "Intrepid Ethernet Evaluation Board";
			case VCANrf:
				return "ValueCAN.rf";
			case FIRE2:
				return "neoVI FIRE 2";
			case Flex:
				return "neoVI Flex";
			case RADGalaxy:
				return "RADGalaxy";
			case RADStar2:
				return "RADStar 2";
			case VividCAN:
				return "VividCAN";
			case OBD2_SIM:
				return "neoOBD2-SIM";
			case DONT_REUSE0:
			case DONT_REUSE1:
			case DONT_REUSE2:
			case DONT_REUSE3:
				// Intentionally don't use default so that the compiler throws a warning when something is added
				return "Unknown neoVI";
		}
		return "Unknown neoVI";
	}

	DeviceType() { value = DeviceType::Enum::Unknown; }
	DeviceType(devicetype_t netid) { value = (DeviceType::Enum)netid; }
	DeviceType(DeviceType::Enum netid) { value = netid; }
	DeviceType::Enum getDeviceType() const { return value; }
	std::string toString() const { return GetDeviceTypeString(getDeviceType()); }
	friend std::ostream& operator<<(std::ostream& os, const DeviceType& DeviceType) {
		os << DeviceType.toString();
		return os;
	}

private:
	DeviceType::Enum value;
};

}

#endif // __cplusplus

#endif