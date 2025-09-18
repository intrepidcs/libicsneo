#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/device/devicetype.h"

namespace icsneo {

void init_devicetype(pybind11::module_& m) {
	pybind11::class_<DeviceType> deviceType(m, "DeviceType");
	pybind11::enum_<DeviceType::Enum>(deviceType, "Enum")
		.value("Unknown", DeviceType::Enum::Unknown)
		.value("BLUE", DeviceType::Enum::BLUE)
		.value("ECU_AVB", DeviceType::Enum::ECU_AVB)
		.value("RADSupermoon", DeviceType::Enum::RADSupermoon)
		.value("DW_VCAN", DeviceType::Enum::DW_VCAN)
		.value("RADMoon2", DeviceType::Enum::RADMoon2)
		.value("RADMars", DeviceType::Enum::RADMars)
		.value("VCAN4_1", DeviceType::Enum::VCAN4_1)
		.value("FIRE", DeviceType::Enum::FIRE)
		.value("RADPluto", DeviceType::Enum::RADPluto)
		.value("VCAN4_2EL", DeviceType::Enum::VCAN4_2EL)
		.value("RADIO_CANHUB", DeviceType::Enum::RADIO_CANHUB)
		.value("NEOECU12", DeviceType::Enum::NEOECU12)
		.value("OBD2_LCBADGE", DeviceType::Enum::OBD2_LCBADGE)
		.value("RADMoonDuo", DeviceType::Enum::RADMoonDuo)
		.value("FIRE3", DeviceType::Enum::FIRE3)
		.value("VCAN3", DeviceType::Enum::VCAN3)
		.value("RADJupiter", DeviceType::Enum::RADJupiter)
		.value("VCAN4_IND", DeviceType::Enum::VCAN4_IND)
		.value("RADGigastar", DeviceType::Enum::RADGigastar)
		.value("RED2", DeviceType::Enum::RED2)
		.value("EtherBADGE", DeviceType::Enum::EtherBADGE)
		.value("RAD_A2B", DeviceType::Enum::RAD_A2B)
		.value("RADEpsilon", DeviceType::Enum::RADEpsilon)
		.value("RADEpsilonXL", DeviceType::Enum::RADEpsilonXL)
		.value("RADGalaxy2", DeviceType::Enum::RADGalaxy2)
		.value("RADMoon3", DeviceType::Enum::RADMoon3)
		.value("RADComet", DeviceType::Enum::RADComet)
		.value("FIRE3_FlexRay", DeviceType::Enum::FIRE3_FlexRay)
		.value("FIRE3_T1S_LIN", DeviceType::Enum::FIRE3_T1S_LIN)
		.value("FIRE3_T1S_SENT", DeviceType::Enum::FIRE3_T1S_SENT)
		.value("Connect", DeviceType::Enum::Connect)
		.value("RADComet3", DeviceType::Enum::RADComet3)
		.value("RADMoonT1S", DeviceType::Enum::RADMoonT1S)
		.value("RADGigastar2", DeviceType::Enum::RADGigastar2)
		.value("RED", DeviceType::Enum::RED)
		.value("ECU", DeviceType::Enum::ECU)
		.value("IEVB", DeviceType::Enum::IEVB)
		.value("Pendant", DeviceType::Enum::Pendant)
		.value("OBD2_PRO", DeviceType::Enum::OBD2_PRO)
		.value("ECUChip_UART", DeviceType::Enum::ECUChip_UART)
		.value("PLASMA", DeviceType::Enum::PLASMA)
		.value("DONT_REUSE0", DeviceType::Enum::DONT_REUSE0)
		.value("NEOAnalog", DeviceType::Enum::NEOAnalog)
		.value("CT_OBD", DeviceType::Enum::CT_OBD)
		.value("DONT_REUSE1", DeviceType::Enum::DONT_REUSE1)
		.value("DONT_REUSE2", DeviceType::Enum::DONT_REUSE2)
		.value("ION", DeviceType::Enum::ION)
		.value("RADStar", DeviceType::Enum::RADStar)
		.value("DONT_REUSE3", DeviceType::Enum::DONT_REUSE3)
		.value("VCAN4_4", DeviceType::Enum::VCAN4_4)
		.value("VCAN4_2", DeviceType::Enum::VCAN4_2)
		.value("CMProbe", DeviceType::Enum::CMProbe)
		.value("EEVB", DeviceType::Enum::EEVB)
		.value("VCANrf", DeviceType::Enum::VCANrf)
		.value("FIRE2", DeviceType::Enum::FIRE2)
		.value("Flex", DeviceType::Enum::Flex)
		.value("RADGalaxy", DeviceType::Enum::RADGalaxy)
		.value("RADStar2", DeviceType::Enum::RADStar2)
		.value("VividCAN", DeviceType::Enum::VividCAN)
		.value("OBD2_SIM", DeviceType::Enum::OBD2_SIM);
	deviceType.def(pybind11::init<DeviceType::Enum>());
	deviceType.def("get_device_type", &DeviceType::getDeviceType);
	deviceType.def("get_generic_product_name", &DeviceType::getGenericProductName);
}

} // namespace icsneo 

