#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <icsneo/device/devicetype.h>

namespace icsneo {

void init_devicetype(pybind11::module_& m) {
	pybind11::class_<DeviceType> deviceType(m, "DeviceType");
	pybind11::enum_<icsneo_devicetype_t>(deviceType, "Enum")
		.value("Unknown", _icsneo_devicetype_t::Unknown)
		.value("BLUE", _icsneo_devicetype_t::BLUE)
		.value("ECU_AVB", _icsneo_devicetype_t::ECU_AVB)
		.value("RADSupermoon", _icsneo_devicetype_t::RADSupermoon)
		.value("DW_VCAN", _icsneo_devicetype_t::DW_VCAN)
		.value("RADMoon2", _icsneo_devicetype_t::RADMoon2)
		.value("RADMars", _icsneo_devicetype_t::RADMars)
		.value("VCAN4_1", _icsneo_devicetype_t::VCAN4_1)
		.value("FIRE", _icsneo_devicetype_t::FIRE)
		.value("RADPluto", _icsneo_devicetype_t::RADPluto)
		.value("VCAN4_2EL", _icsneo_devicetype_t::VCAN4_2EL)
		.value("RADIO_CANHUB", _icsneo_devicetype_t::RADIO_CANHUB)
		.value("NEOECU12", _icsneo_devicetype_t::NEOECU12)
		.value("OBD2_LCBADGE", _icsneo_devicetype_t::OBD2_LCBADGE)
		.value("RADMoonDuo", _icsneo_devicetype_t::RADMoonDuo)
		.value("FIRE3", _icsneo_devicetype_t::FIRE3)
		.value("VCAN3", _icsneo_devicetype_t::VCAN3)
		.value("RADJupiter", _icsneo_devicetype_t::RADJupiter)
		.value("VCAN4_IND", _icsneo_devicetype_t::VCAN4_IND)
		.value("RADGigastar", _icsneo_devicetype_t::RADGigastar)
		.value("RED2", _icsneo_devicetype_t::RED2)
		.value("EtherBADGE", _icsneo_devicetype_t::EtherBADGE)
		.value("RAD_A2B", _icsneo_devicetype_t::RAD_A2B)
		.value("RADEpsilon", _icsneo_devicetype_t::RADEpsilon)
		.value("RADMoon3", _icsneo_devicetype_t::RADMoon3)
		.value("RADComet", _icsneo_devicetype_t::RADComet)
		.value("FIRE3_FlexRay", _icsneo_devicetype_t::FIRE3_FlexRay)
		.value("Connect", _icsneo_devicetype_t::Connect)
		.value("RADComet3", _icsneo_devicetype_t::RADComet3)
		.value("RADMoonT1S", _icsneo_devicetype_t::RADMoonT1S)
		.value("RADGigastar2", _icsneo_devicetype_t::RADGigastar2)
		.value("RED", _icsneo_devicetype_t::RED)
		.value("ECU", _icsneo_devicetype_t::ECU)
		.value("IEVB", _icsneo_devicetype_t::IEVB)
		.value("Pendant", _icsneo_devicetype_t::Pendant)
		.value("OBD2_PRO", _icsneo_devicetype_t::OBD2_PRO)
		.value("ECUChip_UART", _icsneo_devicetype_t::ECUChip_UART)
		.value("PLASMA", _icsneo_devicetype_t::PLASMA)
		.value("DONT_REUSE0", _icsneo_devicetype_t::DONT_REUSE0)
		.value("NEOAnalog", _icsneo_devicetype_t::NEOAnalog)
		.value("CT_OBD", _icsneo_devicetype_t::CT_OBD)
		.value("DONT_REUSE1", _icsneo_devicetype_t::DONT_REUSE1)
		.value("DONT_REUSE2", _icsneo_devicetype_t::DONT_REUSE2)
		.value("ION", _icsneo_devicetype_t::ION)
		.value("RADStar", _icsneo_devicetype_t::RADStar)
		.value("DONT_REUSE3", _icsneo_devicetype_t::DONT_REUSE3)
		.value("VCAN4_4", _icsneo_devicetype_t::VCAN4_4)
		.value("VCAN4_2", _icsneo_devicetype_t::VCAN4_2)
		.value("CMProbe", _icsneo_devicetype_t::CMProbe)
		.value("EEVB", _icsneo_devicetype_t::EEVB)
		.value("VCANrf", _icsneo_devicetype_t::VCANrf)
		.value("FIRE2", _icsneo_devicetype_t::FIRE2)
		.value("Flex", _icsneo_devicetype_t::Flex)
		.value("RADGalaxy", _icsneo_devicetype_t::RADGalaxy)
		.value("RADStar2", _icsneo_devicetype_t::RADStar2)
		.value("VividCAN", _icsneo_devicetype_t::VividCAN)
		.value("OBD2_SIM", _icsneo_devicetype_t::OBD2_SIM);
	deviceType.def(pybind11::init<icsneo_devicetype_t>());
	deviceType.def("get_device_type", &_icsneo_devicetype_t::getDeviceType);
	deviceType.def("get_generic_product_name", &_icsneo_devicetype_t::getGenericProductName);
}

} // namespace icsneo 

