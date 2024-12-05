#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <icsneo/device/devicetype.h>
#include <icsneo/icsneotypes.h>

namespace icsneo {

void init_devicetype(pybind11::module_& m) {
	pybind11::class_<DeviceType> deviceType(m, "DeviceType");
	deviceType.def("get_device_type", &DeviceType::getDeviceType);
	deviceType.def(
		"get_generic_product_name", [](DeviceType &self) -> std::string {
			return self.getGenericProductName(self.getDeviceType());
		});

	pybind11::enum_<_icsneo_devicetype_t>(m, "icsneo_devicetype_t")
		.value("unknown", icsneo_devicetype_unknown)
		.value("blue", icsneo_devicetype_blue)
		.value("ecu_avb", icsneo_devicetype_ecu_avb)
		.value("rad_supermoon", icsneo_devicetype_rad_supermoon)
		.value("dw_vcan", icsneo_devicetype_dw_vcan)
		.value("rad_moon2", icsneo_devicetype_rad_moon2)
		.value("rad_mars", icsneo_devicetype_rad_mars)
		.value("vcan41", icsneo_devicetype_vcan41)
		.value("fire", icsneo_devicetype_fire)
		.value("rad_pluto", icsneo_devicetype_rad_pluto)
		.value("vcan42_el", icsneo_devicetype_vcan42_el)
		.value("radio_canhub", icsneo_devicetype_radio_canhub)
		.value("neo_ecu12", icsneo_devicetype_neo_ecu12)
		.value("obd2_lc_badge", icsneo_devicetype_obd2_lc_badge)
		.value("rad_moon_duo", icsneo_devicetype_rad_moon_duo)
		.value("fire3", icsneo_devicetype_fire3)
		.value("vcan3", icsneo_devicetype_vcan3)
		.value("rad_jupiter", icsneo_devicetype_rad_jupiter)
		.value("vcan4_industrial", icsneo_devicetype_vcan4_industrial)
		.value("rad_gigastar", icsneo_devicetype_rad_gigastar)
		.value("red2", icsneo_devicetype_red2)
		.value("etherbadge", icsneo_devicetype_etherbadge)
		.value("rad_a2b", icsneo_devicetype_rad_a2b)
		.value("rad_epsilon", icsneo_devicetype_rad_epsilon)
		.value("rad_moon3", icsneo_devicetype_rad_moon3)
		.value("rad_comet", icsneo_devicetype_rad_comet)
		.value("fire3_flexray", icsneo_devicetype_fire3_flexray)
		.value("connect", icsneo_devicetype_connect)
		.value("rad_comet3", icsneo_devicetype_rad_comet3)
		.value("rad_moon_t1s", icsneo_devicetype_rad_moon_t1s)
		.value("rad_gigastar2", icsneo_devicetype_rad_gigastar2)
		.value("red", icsneo_devicetype_red)
		.value("ecu", icsneo_devicetype_ecu)
		.value("ievb", icsneo_devicetype_ievb)
		.value("pendant", icsneo_devicetype_pendant)
		.value("obd2_pro", icsneo_devicetype_obd2_pro)
		.value("plasma", icsneo_devicetype_plasma)
		.value("ion", icsneo_devicetype_ion)
		.value("rad_star", icsneo_devicetype_rad_star)
		.value("vcan44", icsneo_devicetype_vcan44)
		.value("vcan42", icsneo_devicetype_vcan42)
		.value("cm_probe", icsneo_devicetype_cm_probe)
		.value("eevb", icsneo_devicetype_eevb)
		.value("fire2", icsneo_devicetype_fire2)
		.value("flex", icsneo_devicetype_flex)
		.value("rad_galaxy", icsneo_devicetype_rad_galaxy)
		.value("rad_star2", icsneo_devicetype_rad_star2)
		.value("vividcan", icsneo_devicetype_vividcan)
		.value("obd2_sim", icsneo_devicetype_obd2_sim);
}

} // namespace icsneo 

