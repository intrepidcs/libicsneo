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

#include <icsneo/icsneoc2types.h>

typedef icsneoc2_devicetype_t devicetype_t;

namespace icsneo {

class DeviceType {
public:
	// This enum used to be a bitfield, but has since become an enum as we have more than 32 devices
	// Adding something? Make sure you update the type string and C-compatible defines below!
	enum Enum : icsneoc2_devicetype_t {
		Unknown = icsneoc2_devicetype_unknown,
		BLUE = icsneoc2_devicetype_blue,
		ECU_AVB = icsneoc2_devicetype_ecu_avb,
		RADSupermoon = icsneoc2_devicetype_rad_supermoon,
		DW_VCAN = icsneoc2_devicetype_dw_vcan,
		RADMoon2 = icsneoc2_devicetype_rad_moon2,
		RADMars = icsneoc2_devicetype_rad_mars,
		VCAN4_1 = icsneoc2_devicetype_vcan41,
		FIRE = icsneoc2_devicetype_fire,
		RADPluto = icsneoc2_devicetype_rad_pluto,
		VCAN4_2EL = icsneoc2_devicetype_vcan42_el,
		RADIO_CANHUB = icsneoc2_devicetype_radio_canhub,
		NEOECU12 = icsneoc2_devicetype_neo_ecu12,
		OBD2_LCBADGE = icsneoc2_devicetype_obd2_lc_badge,
		RADMoonDuo = icsneoc2_devicetype_rad_moon_duo,
		FIRE3 = icsneoc2_devicetype_fire3,
		VCAN3 = icsneoc2_devicetype_vcan3,
		RADJupiter = icsneoc2_devicetype_rad_jupiter,
		VCAN4_IND = icsneoc2_devicetype_vcan4_industrial,
		RADGigastar = icsneoc2_devicetype_rad_gigastar,
		RED2 = icsneoc2_devicetype_red2,
		EtherBADGE = icsneoc2_devicetype_etherbadge,
		RAD_A2B = icsneoc2_devicetype_rad_a2b,
		RADEpsilon = icsneoc2_devicetype_rad_epsilon,
		RADGalaxy2 = icsneoc2_devicetype_rad_galaxy2,
		RADMoon3 = icsneoc2_devicetype_rad_moon3,
		RADComet = icsneoc2_devicetype_rad_comet,
		FIRE3_FlexRay = icsneoc2_devicetype_fire3_flexray,
		Connect = icsneoc2_devicetype_connect,
		RADComet3 = icsneoc2_devicetype_rad_comet3,
		RADMoonT1S = icsneoc2_devicetype_rad_moon_t1s,
		RADGigastar2 = icsneoc2_devicetype_rad_gigastar2,
		RED = icsneoc2_devicetype_red,
		ECU = icsneoc2_devicetype_ecu,
		IEVB = icsneoc2_devicetype_ievb,
		Pendant = icsneoc2_devicetype_pendant,
		OBD2_PRO = icsneoc2_devicetype_obd2_pro,
		ECUChip_UART = icsneoc2_devicetype_ecuchip_uart,
		PLASMA = icsneoc2_devicetype_plasma,
		//DONT_REUSE0 = , // Previously FIRE_VNET
		NEOAnalog = icsneoc2_devicetype_neo_analog,
		CT_OBD = icsneoc2_devicetype_ct_obd,
		//DONT_REUSE1 = (0x00010000), // Previously PLASMA_1_12
		//DONT_REUSE2 = (0x00020000), // Previously PLASMA_1_13
		ION = icsneoc2_devicetype_ion,
		RADStar = icsneoc2_devicetype_rad_star,
		//DONT_REUSE3 = (0x00100000), // Previously ION3
		VCAN4_4 = icsneoc2_devicetype_vcan44,
		VCAN4_2 = icsneoc2_devicetype_vcan42,
		CMProbe = icsneoc2_devicetype_cm_probe,
		EEVB = icsneoc2_devicetype_eevb,
		VCANrf = icsneoc2_devicetype_vcan_rf,
		FIRE2 = icsneoc2_devicetype_fire2,
		Flex = icsneoc2_devicetype_flex,
		RADGalaxy = icsneoc2_devicetype_rad_galaxy,
		RADStar2 = icsneoc2_devicetype_rad_star2,
		VividCAN = icsneoc2_devicetype_vividcan,
		OBD2_SIM = icsneoc2_devicetype_obd2_sim
	};

	/**
	 * Get the generic product name for this device type.
	 * 
	 * Note that device->getProductName() should always be preferred where available,
	 * as the product name may change based on device-specific factors, such as serial
	 * number.
	 */
	template<typename T>
	static std::string GetGenericProductName(T deviceType) {
		// Adding something? Make sure you update DEVICE_TYPE_LONGEST_NAME at the top!
		switch(static_cast<icsneoc2_devicetype_t>(deviceType)) {
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
			case RADGalaxy2:
				return "RAD-Galaxy 2";
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
			case RADMoonT1S:
				return "RAD-Moon T1S";
			case Connect:
				return "neoVI Connect";
			case RADGigastar2:
				return "RAD-Gigastar 2";
			// Intentionally don't use default so that the compiler throws a warning when something is added
		}
		return "Unknown neoVI";
	}

	DeviceType() { value = DeviceType::Enum::Unknown; }
	DeviceType(devicetype_t netid) { value = (DeviceType::Enum)netid; }
	DeviceType(DeviceType::Enum netid) { value = netid; }
	icsneoc2_devicetype_t getDeviceType() const { return value; }
	std::string getGenericProductName() const { return DeviceType::GetGenericProductName(getDeviceType()); }
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
#define ICSNEO_DEVICETYPE_RADGALAXY2 ((devicetype_t)0x00000021)
#define ICSNEO_DEVICETYPE_RADMoon3 ((devicetype_t)0x00000023)
#define ICSNEO_DEVICETYPE_RADCOMET ((devicetype_t)0x00000024)
#define ICSNEO_DEVICETYPE_FIRE3FLEXRAY ((devicetype_t)0x00000025)
#define ICSNEO_DEVICETYPE_CONNECT ((devicetype_t)0x00000026)
#define ICSNEO_DEVICETYPE_RADCOMET3 ((devicetype_t)0x00000027)
#define ICSNEO_DEVICETYPE_RADMOONT1S ((devicetype_t)0x00000028)
#define ICSNEO_DEVICETYPE_RADGIGASTAR2 ((devicetype_t)0x00000029)
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
