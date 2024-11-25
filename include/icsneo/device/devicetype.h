#pragma once

// Hold the length of the longest name, so that C applications can allocate memory accordingly
// Currently the longest is "Intrepid Ethernet Evaluation Board"
#define ICSNEO_DEVICETYPE_LONGEST_NAME (35 + 1) // Add 1 so that if someone forgets, they still have space for null terminator
#define ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION (ICSNEO_DEVICETYPE_LONGEST_NAME + 7) // 6 character serial, plus space

#include <ostream>
#include <cstdint>
#include <string>
#include <icsneo/icsneotypes.h>


namespace icsneo {

class DeviceType {
public:
	/**
	 * Get the generic product name for this device type.
	 * 
	 * Note that device->getProductName() should always be preferred where available,
	 * as the product name may change based on device-specific factors, such as serial
	 * number.
	 */
	template<typename T>
	static std::string getGenericProductName(T deviceType) {
		icsneo_devicetype_t t = static_cast<icsneo_devicetype_t>(deviceType);
		// Adding something? Make sure you update DEVICE_TYPE_LONGEST_NAME at the top!
		switch((icsneo_devicetype_t)t) {
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
			case RADMoonT1S:
				return "RAD-Moon T1S";
			case Connect:
				return "neoVI Connect";
			case RADGigastar2:
				return "RAD-Gigastar 2";
			case DONT_REUSE0:
			case DONT_REUSE1:
			case DONT_REUSE2:
			case DONT_REUSE3:
				return "Unknown neoVI";
			// Intentionally don't use default so that the compiler throws a warning when something is added
		}
		return "Unknown neoVI";
	}

	DeviceType(icsneo_devicetype_t device_type) { deviceType = device_type; }
	icsneo_devicetype_t getDeviceType() const { return deviceType; }

	// Returns the generic name of the device - This doesn't include the serial.
	std::string getProductName() const { return DeviceType::getGenericProductName(getDeviceType()); }

private:
	icsneo_devicetype_t deviceType;
};

}; // namespace icsneo
