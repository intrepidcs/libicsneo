#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <icsneo/device/devicetype.h>
#include <icsneo/icsneotypes.h>

namespace icsneo {

void init_devicetype(pybind11::module_& m) {
	pybind11::class_<DeviceType> deviceType(m, "DeviceType");
	// deviceType.def("get_device_type", &DeviceType::getDeviceType);
	// deviceType.def("get_generic_product_name", &DeviceType::getGenericProductName);

	pybind11::enum_<_icsneo_devicetype_t>(m, "icsneo_devicetype_t")
		.value("Unknown", icsneo_devicetype_unknown)
		.value("BLUE", icsneo_devicetype_blue)
		.value("ECU_AVB", icsneo_devicetype_ecu_avb)
		.value("RADSupermoon", icsneo_devicetype_rad_supermoon)
		.value("DW_VCAN", icsneo_devicetype_dw_vcan)
		.value("RADMoon2", icsneo_devicetype_rad_moon2)
		.value("RADMars", icsneo_devicetype_rad_mars)
		.value("VCAN4_1", icsneo_devicetype_vcan41)
		.value("FIRE", icsneo_devicetype_fire)
		.value("RADPluto", icsneo_devicetype_rad_pluto)
		.value("VCAN4_2EL", icsneo_devicetype_vcan42_el)
		.value("RADIO_CANHUB", icsneo_devicetype_radio_canhub)
		.value("NEOECU12", icsneo_devicetype_neo_ecu12)
		.value("OBD2_LCBADGE", icsneo_devicetype_obd2_lc_badge)
		.value("RADMoonDuo", icsneo_devicetype_rad_moon_duo)
		.value("FIRE3", icsneo_devicetype_fire3)
		.value("VCAN3", icsneo_devicetype_vcan3)
		.value("RADJupiter", icsneo_devicetype_rad_jupiter)
		.value("VCAN4_IND", icsneo_devicetype_vcan4_industrial)
		.value("RADGigastar", icsneo_devicetype_rad_gigastar)
		.value("RED2", icsneo_devicetype_red2)
		.value("EtherBADGE", icsneo_devicetype_etherbadge)
		.value("RAD_A2B", icsneo_devicetype_rad_a2b)
		.value("RADEpsilon", icsneo_devicetype_rad_epsilon)
		.value("RADMoon3", icsneo_devicetype_rad_moon3)
		.value("RADComet", icsneo_devicetype_rad_comet)
		.value("FIRE3_FlexRay", icsneo_devicetype_fire3_flexray)
		.value("Connect", icsneo_devicetype_connect)
		.value("RADComet3", icsneo_devicetype_rad_comet3)
		.value("RADMoonT1S", icsneo_devicetype_rad_moon_t1s)
		.value("RADGigastar2", icsneo_devicetype_rad_gigastar2)
		.value("RED", icsneo_devicetype_red)
		.value("ECU", icsneo_devicetype_ecu)
		.value("IEVB", icsneo_devicetype_ievb)
		.value("Pendant", icsneo_devicetype_pendant)
		.value("OBD2_PRO", icsneo_devicetype_obd2_pro)
		.value("ECUChip_UART", icsneo_devicetype_ecuchip_uart)
		.value("PLASMA", icsneo_devicetype_plasma)
		.value("NEOAnalog", icsneo_devicetype_neo_analog)
		.value("CT_OBD", icsneo_devicetype_ct_obd)
		.value("ION", icsneo_devicetype_ion)
		.value("RADStar", icsneo_devicetype_rad_star)
		.value("VCAN4_4", icsneo_devicetype_vcan44)
		.value("VCAN4_2", icsneo_devicetype_vcan42)
		.value("CMProbe", icsneo_devicetype_cm_probe)
		.value("EEVB", icsneo_devicetype_eevb)
		.value("FIRE2", icsneo_devicetype_fire2)
		.value("Flex", icsneo_devicetype_flex)
		.value("RADGalaxy", icsneo_devicetype_rad_galaxy)
		.value("RADStar2", icsneo_devicetype_rad_star2)
		.value("VividCAN", icsneo_devicetype_vividcan)
		.value("OBD2_SIM", icsneo_devicetype_obd2_sim);
}

} // namespace icsneo 

