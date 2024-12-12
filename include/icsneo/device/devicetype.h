#pragma once

// Hold the length of the longest name, so that C applications can allocate memory accordingly
// Currently the longest is "Intrepid Ethernet Evaluation Board"
#define ICSNEO_DEVICETYPE_LONGEST_NAME (35 + 1) // Add 1 so that if someone forgets, they still have space for null terminator
#define ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION (ICSNEO_DEVICETYPE_LONGEST_NAME + 7) // 6 character serial, plus space


#include <icsneo/icsneotypes.h>

#ifdef __cplusplus
#include <cstdint>
#include <string>


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
		// Adding something? Make sure you update DEVICE_TYPE_LONGEST_NAME at the top!
		switch(static_cast<icsneo_devicetype_t>(deviceType)) {
			case icsneo_devicetype_unknown:
				return "Unknown";
			case icsneo_devicetype_blue:
				return "neoVI BLUE";
			case icsneo_devicetype_ecu_avb:
				return "neoECU AVB/TSN";
			case icsneo_devicetype_rad_supermoon:
				return "RAD-Supermoon";
			case icsneo_devicetype_dw_vcan:
				return "DW_VCAN";
			case icsneo_devicetype_rad_moon2:
				return "RAD-Moon 2";
			case icsneo_devicetype_rad_mars:
				return "RAD-Mars";
			case icsneo_devicetype_vcan41:
				return "ValueCAN 4-1";
			case icsneo_devicetype_fire:
				return "neoVI FIRE";
			case icsneo_devicetype_rad_pluto:
				return "RAD-Pluto";
			case icsneo_devicetype_vcan42_el:
				return "ValueCAN 4-2EL";
			case icsneo_devicetype_radio_canhub:
				return "RAD-IO2 CANHub";
			case icsneo_devicetype_neo_ecu12:
				return "neoECU 12";
			case icsneo_devicetype_obd2_lc_badge:
				return "neoOBD2 LC BADGE";
			case icsneo_devicetype_rad_moon_duo:
				return "RAD-Moon Duo";
			case icsneo_devicetype_fire3:
				return "neoVI FIRE 3";
			case icsneo_devicetype_vcan3:
				return "ValueCAN 3";
			case icsneo_devicetype_rad_jupiter:
				return "RAD-Jupiter";
			case icsneo_devicetype_vcan4_industrial:
				return "ValueCAN 4 Industrial";
			case icsneo_devicetype_rad_gigastar:
				return "RAD-Gigastar";
			case icsneo_devicetype_red2:
				return "neoVI RED 2";
			case icsneo_devicetype_etherbadge:
				return "EtherBADGE";
			case icsneo_devicetype_rad_a2b:
				return "RAD-A2B";
			case icsneo_devicetype_rad_epsilon:
				return "RAD-Epsilon";
			case icsneo_devicetype_rad_moon3:
				return "RAD-Moon 3";
			case icsneo_devicetype_rad_comet:
				return "RAD-Comet";
			case icsneo_devicetype_red:
				return "neoVI RED";
			case icsneo_devicetype_ecu:
				return "neoECU";
			case icsneo_devicetype_ievb:
				return "IEVB";
			case icsneo_devicetype_pendant:
				return "Pendant";
			case icsneo_devicetype_obd2_pro:
				return "neoOBD2 PRO";
			case icsneo_devicetype_ecuchip_uart:
				return "neoECU Chip UART";
			case icsneo_devicetype_plasma:
				return "neoVI PLASMA";
			case icsneo_devicetype_neo_analog:
				return "NEOAnalog";
			case icsneo_devicetype_ct_obd:
				return "CT_OBD";
			case icsneo_devicetype_ion:
				return "neoVI ION";
			case icsneo_devicetype_rad_star:
				return "RAD-Star";
			case icsneo_devicetype_vcan44:
				return "ValueCAN 4-4";
			case icsneo_devicetype_vcan42:
				return "ValueCAN 4-2";
			case icsneo_devicetype_cm_probe:
				return "CMProbe";
			case icsneo_devicetype_eevb:
				return "Intrepid Ethernet Evaluation Board";
			case icsneo_devicetype_vcan_rf:
				return "ValueCAN.rf";
			case icsneo_devicetype_fire2:
				return "neoVI FIRE 2";
			case icsneo_devicetype_flex:
				return "neoVI Flex";
			case icsneo_devicetype_rad_galaxy:
				return "RAD-Galaxy";
			case icsneo_devicetype_rad_galaxy2:
				return "RAD-Galaxy 2";
			case icsneo_devicetype_rad_star2:
				return "RAD-Star 2";
			case icsneo_devicetype_vividcan:
				return "VividCAN";
			case icsneo_devicetype_obd2_sim:
				return "neoOBD2 SIM";
			case icsneo_devicetype_fire3_flexray:
				return "neoVI FIRE3 FlexRay";
			case icsneo_devicetype_rad_comet3:
				return "RAD-Comet 3";
			case icsneo_devicetype_rad_moon_t1s:
				return "RAD-Moon T1S";
			case icsneo_devicetype_connect:
				return "neoVI Connect";
			case icsneo_devicetype_rad_gigastar2:
				return "RAD-Gigastar 2";
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

