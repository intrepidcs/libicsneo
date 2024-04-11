#ifndef __DEVICETYPE_H_
#define __DEVICETYPE_H_

// Hold the length of the longest name, so that C applications can allocate memory accordingly
// Currently the longest is "Intrepid Ethernet Evaluation Board"
#define ICSNEO_DEVICETYPE_LONGEST_NAME (35 + 1) // Add 1 so that if someone forgets, they still have space for null terminator
#define ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION (ICSNEO_DEVICETYPE_LONGEST_NAME + 7) // 6 character serial, plus space

#ifndef __cplusplus
#include <stdint.h>

typedef uint32_t devicetype_t;
#else
#include <ostream>
#include <cstdint>

typedef uint32_t devicetype_t;

namespace icsneo {

class DeviceType {
public:
	// This enum used to be a bitfield, but has since become an enum as we have more than 32 devices
	// Adding something? Make sure you update the type string and C-compatible defines below!
	enum Enum : devicetype_t {
		Unknown = (0x00000000),
		BLUE = (0x00000001),
		ECU_AVB = (0x00000002),
		RADSupermoon = (0x00000003),
		DW_VCAN = (0x00000004),
		RADMoon2 = (0x00000005),
		RADMars = (0x00000006),
		VCAN4_1 = (0x00000007),
		FIRE = (0x00000008),
		RADPluto = (0x00000009),
		VCAN4_2EL = (0x0000000a),
		RADIO_CANHUB = (0x0000000b),
		NEOECU12 = (0x0000000c),
		OBD2_LCBADGE = (0x0000000d),
		RADMoonDuo = (0x0000000e),
		FIRE3 = (0x0000000f),
		VCAN3 = (0x00000010),
		RADJupiter = (0x00000011),
		VCAN4_IND = (0x00000012),
		RADGigastar = (0x00000013),
		RED2 = (0x00000014),
		EtherBADGE = (0x00000016),
		RAD_A2B = (0x00000017),
		RADEpsilon = (0x00000018),
		RADMoon3 = (0x00000023),
		RADComet = (0x00000024),
		FIRE3_FlexRay = (0x00000025),
		RADComet3 = (0x00000027),
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

	/**
	 * Get the generic product name for this device type.
	 * 
	 * Note that device->getProductName() should always be preferred where available,
	 * as the product name may change based on device-specific factors, such as serial
	 * number.
	 */
	static const char* GetGenericProductName(DeviceType::Enum type) {
		// Adding something? Make sure you update DEVICE_TYPE_LONGEST_NAME at the top!
		switch(type) {
			case Unknown:
				return "Unknown";
			case BLUE:
				return "neoVI BLUE";
			case ECU_AVB:
				return "neoECU AVB/TSN";
			case RADSupermoon:
				return "RAD-Supermoon";
			case DW_VCAN:
				return "DW_VCAN";
			case RADMoon2:
				return "RAD-Moon 2";
			case RADMars:
				return "RAD-Mars";
			case VCAN4_1:
				return "ValueCAN 4-1";
			case FIRE:
				return "neoVI FIRE";
			case RADPluto:
				return "RAD-Pluto";
			case VCAN4_2EL:
				return "ValueCAN 4-2EL";
			case RADIO_CANHUB:
				return "RAD-IO2 CANHub";
			case NEOECU12:
				return "neoECU 12";
			case OBD2_LCBADGE:
				return "neoOBD2 LC BADGE";
			case RADMoonDuo:
				return "RAD-Moon Duo";
			case FIRE3:
				return "neoVI FIRE 3";
			case VCAN3:
				return "ValueCAN 3";
			case RADJupiter:
				return "RAD-Jupiter";
			case VCAN4_IND:
				return "ValueCAN 4 Industrial";
			case RADGigastar:
				return "RAD-Gigastar";
			case RED2:
				return "neoVI RED 2";
			case EtherBADGE:
				return "EtherBADGE";
			case RAD_A2B:
				return "RAD-A2B";
			case RADEpsilon:
				return "RAD-Epsilon";
			case RADMoon3:
				return "RAD-Moon 3";
			case RADComet:
				return "RAD-Comet";
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
				return "RAD-Star";
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
				return "RAD-Galaxy";
			case RADStar2:
				return "RAD-Star 2";
			case VividCAN:
				return "VividCAN";
			case OBD2_SIM:
				return "neoOBD2 SIM";
			case FIRE3_FlexRay:
				return "neoVI FIRE3 FlexRay";
			case RADComet3:
				return "RAD-Comet 3";
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
	std::string getGenericProductName() const { return GetGenericProductName(getDeviceType()); }
	operator devicetype_t() const { return getDeviceType(); }

private:
	DeviceType::Enum value;
};

}

#endif // __cplusplus

#ifdef __ICSNEOC_H_ // We are using the C API, so we want C-compatible defines
#define ICSNEO_DEVICETYPE_UNKNOWN ((devicetype_t)0x00000000)
#define ICSNEO_DEVICETYPE_BLUE ((devicetype_t)0x00000001)
#define ICSNEO_DEVICETYPE_ECU_AVB ((devicetype_t)0x00000002)
#define ICSNEO_DEVICETYPE_RADSUPERMOON ((devicetype_t)0x00000003)
#define ICSNEO_DEVICETYPE_DW_VCAN ((devicetype_t)0x00000004)
#define ICSNEO_DEVICETYPE_RADMOON2 ((devicetype_t)0x00000005)
#define ICSNEO_DEVICETYPE_RADMARS ((devicetype_t)0x00000006)
#define ICSNEO_DEVICETYPE_VCAN4_1 ((devicetype_t)0x00000007)
#define ICSNEO_DEVICETYPE_FIRE ((devicetype_t)0x00000008)
#define ICSNEO_DEVICETYPE_RADPLUTO ((devicetype_t)0x00000009)
#define ICSNEO_DEVICETYPE_VCAN4_2EL ((devicetype_t)0x0000000a)
#define ICSNEO_DEVICETYPE_RADIO_CANHUB ((devicetype_t)0x0000000b)
#define ICSNEO_DEVICETYPE_NEOECU12 ((devicetype_t)0x0000000c)
#define ICSNEO_DEVICETYPE_OBD2_LCBADGE ((devicetype_t)0x0000000d)
#define ICSNEO_DEVICETYPE_RADMOONDUO ((devicetype_t)0x0000000e)
#define ICSNEO_DEVICETYPE_FIRE3 ((devicetype_t)0x0000000f)
#define ICSNEO_DEVICETYPE_VCAN3 ((devicetype_t)0x00000010)
#define ICSNEO_DEVICETYPE_RADJUPITER ((devicetype_t)0x00000011)
#define ICSNEO_DEVICETYPE_VCAN4_IND ((devicetype_t)0x00000012)
#define ICSNEO_DEVICETYPE_RADGIGASTAR ((devicetype_t)0x00000013)
#define ICSNEO_DEVICETYPE_RED2 ((devicetype_t)0x00000014)
#define ICSNEO_DEVICETYPE_ETHERBADGE ((devicetype_t)0x00000016)
#define ICSNEO_DEVICETYPE_RAD_A2B ((devicetype_t)0x00000017)
#define ICSNEO_DEVICETYPE_RADEPSILON ((devicetype_t)0x00000018)
#define ICSNEO_DEVICETYPE_RADMoon3 ((devicetype_t)0x00000023)
#define ICSNEO_DEVICETYPE_RADCOMET ((devicetype_t)0x00000024)
#define ICSNEO_DEVICETYPE_FIRE3FLEXRAY ((devicetype_t)0x00000025)
#define ICSNEO_DEVICETYPE_RADCOMET3 ((devicetype_t)0x00000027)
#define ICSNEO_DEVICETYPE_RED ((devicetype_t)0x00000040)
#define ICSNEO_DEVICETYPE_ECU ((devicetype_t)0x00000080)
#define ICSNEO_DEVICETYPE_IEVB ((devicetype_t)0x00000100)
#define ICSNEO_DEVICETYPE_PENDANT ((devicetype_t)0x00000200)
#define ICSNEO_DEVICETYPE_OBD2_PRO ((devicetype_t)0x00000400)
#define ICSNEO_DEVICETYPE_ECUCHIP_UART ((devicetype_t)0x00000800)
#define ICSNEO_DEVICETYPE_PLASMA ((devicetype_t)0x00001000)
#define ICSNEO_DEVICETYPE_DONT_REUSE0 ((devicetype_t)0x00002000) // Previously FIRE_VNET
#define ICSNEO_DEVICETYPE_NEOANALOG ((devicetype_t)0x00004000)
#define ICSNEO_DEVICETYPE_CT_OBD ((devicetype_t)0x00008000)
#define ICSNEO_DEVICETYPE_DONT_REUSE1 ((devicetype_t)0x00010000) // Previously PLASMA_1_12
#define ICSNEO_DEVICETYPE_DONT_REUSE2 ((devicetype_t)0x00020000) // Previously PLASMA_1_13
#define ICSNEO_DEVICETYPE_ION ((devicetype_t)0x00040000)
#define ICSNEO_DEVICETYPE_RADSTAR ((devicetype_t)0x00080000)
#define ICSNEO_DEVICETYPE_DONT_REUSE3 ((devicetype_t)0x00100000) // Previously ION3
#define ICSNEO_DEVICETYPE_VCAN4_4 ((devicetype_t)0x00200000)
#define ICSNEO_DEVICETYPE_VCAN4_2 ((devicetype_t)0x00400000)
#define ICSNEO_DEVICETYPE_CMPROBE ((devicetype_t)0x00800000)
#define ICSNEO_DEVICETYPE_EEVB ((devicetype_t)0x01000000)
#define ICSNEO_DEVICETYPE_VCANRF ((devicetype_t)0x02000000)
#define ICSNEO_DEVICETYPE_FIRE2 ((devicetype_t)0x04000000)
#define ICSNEO_DEVICETYPE_FLEX ((devicetype_t)0x08000000)
#define ICSNEO_DEVICETYPE_RADGALAXY ((devicetype_t)0x10000000)
#define ICSNEO_DEVICETYPE_RADSTAR2 ((devicetype_t)0x20000000)
#define ICSNEO_DEVICETYPE_VIVIDCAN ((devicetype_t)0x40000000)
#define ICSNEO_DEVICETYPE_OBD2_SIM ((devicetype_t)0x80000000)
#endif

#endif