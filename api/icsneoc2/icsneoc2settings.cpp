#include "icsneo/icsneoc2.h"
#include "icsneo/icsneoc2settings.h"
#include "icsneo/device/device.h"
#include "icsneo/device/devicefinder.h"
#include "icsneo/icsneocpp.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/message/linmessage.h"
#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/communication/packet/canpacket.h"
#include "icsneo/communication/io.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <optional>
#include <sstream>

using namespace icsneo;

#include "icsneoc2_internal.h"

icsneoc2_error_t icsneoc2_settings_apply_defaults(icsneoc2_device_t* device, bool save) {
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!device->device->settings->applyDefaults(!save)) {
		return icsneoc2_error_set_settings_failure;
	}

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_apply(icsneoc2_device_t* device) {
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!device->device->settings->apply()) {
		return icsneoc2_error_set_settings_failure;
	}

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_refresh(icsneoc2_device_t* device) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->refresh()) {
        return icsneoc2_error_get_settings_failure;
    }

    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_baudrate_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, int64_t* baudrate) {
	if(!baudrate) {
		return icsneoc2_error_invalid_parameters;
	}
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	auto baudrate_value = device->device->settings->getBaudrateFor(Network(netid));
	if(baudrate_value < 0) {
		*baudrate = 0;
		return icsneoc2_error_invalid_type;
	}
	*baudrate = baudrate_value;

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_baudrate_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, int64_t baudrate) {
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!device->device->settings->setBaudrateFor(Network(netid), baudrate)) {
		return icsneoc2_error_set_settings_failure;
	}

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_canfd_baudrate_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, int64_t* baudrate) {
	if(!baudrate) {
		return icsneoc2_error_invalid_parameters;
	}
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	*baudrate = device->device->settings->getFDBaudrateFor(Network(netid));
	if(*baudrate < 0) {
		*baudrate = 0;
		return icsneoc2_error_invalid_type;
	}

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_canfd_baudrate_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, int64_t baudrate) {
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!device->device->settings->setFDBaudrateFor(Network(netid), baudrate)) {
		return icsneoc2_error_set_settings_failure;
	}

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_termination_is_supported(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* supported) {
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!supported) {
		return icsneoc2_error_invalid_parameters;
	}
	*supported = device->device->settings->isTerminationSupportedFor(Network(static_cast<Network::NetID>(netid)));

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_termination_can_enable(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* can_enable) {
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!can_enable) {
		return icsneoc2_error_invalid_parameters;
	}
	*can_enable = device->device->settings->canTerminationBeEnabledFor(Network(static_cast<Network::NetID>(netid)));

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_termination_is_enabled(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* enabled) {
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!enabled) {
		return icsneoc2_error_invalid_parameters;
	}
	*enabled = device->device->settings->isTerminationEnabledFor(Network(static_cast<Network::NetID>(netid))).value_or(false);

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_termination_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool enable) {
	// Make sure the device is valid
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!device->device->settings->setTerminationFor(Network(static_cast<Network::NetID>(netid)), enable)) {
		return icsneoc2_error_set_settings_failure;
	}

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_commander_resistor_enabled(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* enabled) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!enabled) {
        return icsneoc2_error_invalid_parameters;
    }
    *enabled = device->device->settings->isCommanderResistorEnabledFor(Network(static_cast<Network::NetID>(netid))).value_or(false);

    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_commander_resistor_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool enable) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setCommanderResistorFor(Network(static_cast<Network::NetID>(netid)), enable)) {
        return icsneoc2_error_set_settings_failure;
    }

    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_lin_mode_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_lin_mode_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getLINModeFor(network); result.has_value()) {
        *value = static_cast<icsneoc2_lin_mode_t>(result.value());
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_lin_mode_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_lin_mode_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setLINModeFor(network, static_cast<LINMode>(value))) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_lin_commander_response_time_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getLINCommanderResponseTimeFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_lin_commander_response_time_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setLINCommanderResponseTimeFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_phy_enable_get(icsneoc2_device_t* device, uint8_t index, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->getPhyEnable(index); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = false;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_phy_enable_set(icsneoc2_device_t* device, uint8_t index, bool value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setPhyEnable(index, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_phy_mode_get(icsneoc2_device_t* device, uint8_t index, icsneoc2_ae_link_mode_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->getPhyMode(index); result.has_value()) {
        *value = static_cast<icsneoc2_ae_link_mode_t>(result.value());
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_phy_mode_set(icsneoc2_device_t* device, uint8_t index, icsneoc2_ae_link_mode_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setPhyMode(index, static_cast<AELinkMode>(value))) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_phy_speed_get(icsneoc2_device_t* device, uint8_t index, icsneoc2_eth_phy_link_mode_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->getPhySpeed(index); result.has_value()) {
        *value = static_cast<icsneoc2_eth_phy_link_mode_t>(result.value());
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}
icsneoc2_error_t icsneoc2_settings_phy_speed_set(icsneoc2_device_t* device, uint8_t index, icsneoc2_eth_phy_link_mode_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setPhySpeed(index, static_cast<EthPhyLinkMode>(value))) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_phy_role_for_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_ae_link_mode_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getPhyRoleFor(network); result.has_value()) {
        *value = static_cast<icsneoc2_ae_link_mode_t>(result.value());
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_phy_role_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_ae_link_mode_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setPhyRoleFor(network, static_cast<AELinkMode>(value))) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_phy_link_mode_for_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_eth_phy_link_mode_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getPhyLinkModeFor(network); result.has_value()) {
        *value = static_cast<icsneoc2_eth_phy_link_mode_t>(result.value());
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_phy_link_mode_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_eth_phy_link_mode_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setPhyLinkModeFor(network, static_cast<EthPhyLinkMode>(value))) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_phy_enable_for_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getPhyEnableFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = false;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_phy_enable_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setPhyEnableFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_supported_phy_link_modes_for(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_eth_phy_link_mode_t** link_modes, size_t* link_modes_count) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!link_modes || !link_modes_count) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    auto modes = device->device->settings->getSupportedPhyLinkModesFor(network);
    *link_modes_count = std::minmax(modes.size(), *link_modes_count).first;
    memcpy(*link_modes, modes.data(), *link_modes_count * sizeof(icsneoc2_eth_phy_link_mode_t));

    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_is_plca_enabled_for(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->isT1SPLCAEnabledFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = false;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_plca_enabled_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SPLCAFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_local_id_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value){
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getT1SLocalIDFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_local_id_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value){
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SLocalIDFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_max_nodes_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getT1SMaxNodesFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_max_nodes_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SMaxNodesFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_tx_opp_timer_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getT1STxOppTimerFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_tx_opp_timer_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1STxOppTimerFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_burst_timer_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getT1SBurstTimerFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_burst_timer_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SBurstTimerFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_max_burst_timer_for_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getT1SMaxBurstFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_max_burst_timer_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SMaxBurstFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_local_id_alternate_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getT1SLocalIDAlternateFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_local_id_alternate_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SLocalIDAlternateFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_is_termination_enabled_for(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->isT1STerminationEnabledFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = false;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_termination_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1STerminationFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_is_bus_decoding_beacons_enabled_for(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->isT1SBusDecodingBeaconsEnabledFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = false;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_bus_decoding_beacons_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SBusDecodingBeaconsFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_is_bus_decoding_all_enabled_for(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->isT1SBusDecodingAllEnabledFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = false;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_bus_decoding_all_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SBusDecodingAllFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_multi_id_enable_mask_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getT1SMultiIDEnableMaskFor(network); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_multi_id_enable_mask_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SMultiIDEnableMaskFor(network, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_t1s_multi_id_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t index, uint8_t* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(auto result = device->device->settings->getT1SMultiIDFor(network, index); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    } else {
        *value = 0;
        return icsneoc2_error_get_settings_failure;
    }
}

icsneoc2_error_t icsneoc2_settings_t1s_multi_id_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t index, uint8_t value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    auto network = Network(static_cast<Network::NetID>(netid));
    if(!device->device->settings->setT1SMultiIDFor(network, index, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_misc_io_analog_output_enabled_set(icsneoc2_device_t* device, uint8_t pin, uint8_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setMiscIOAnalogOutputEnabled(pin, value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}


icsneoc2_error_t icsneoc2_settings_misc_io_analog_output_set(icsneoc2_device_t* device, uint8_t pin, icsneoc2_misc_io_analog_voltage_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setMiscIOAnalogOutput(pin, static_cast<MiscIOAnalogVoltage>(value))) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_linux_boot_enabled_get(icsneoc2_device_t* device, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->getLinuxBootEnabled(); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    }
    return icsneoc2_error_get_settings_failure;
}

icsneoc2_error_t icsneoc2_settings_linux_boot_enabled_set(icsneoc2_device_t* device, bool value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setLinuxBootEnabled(value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_external_wifi_antenna_enabled_get(icsneoc2_device_t* device, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->getExternalWifiAntennaEnabled(); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    }
    return icsneoc2_error_get_settings_failure;
}

icsneoc2_error_t icsneoc2_settings_external_wifi_antenna_enabled_set(icsneoc2_device_t* device, bool value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setExternalWifiAntennaEnabled(value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_linux_configuration_port_get(icsneoc2_device_t* device, icsneoc2_linux_configuration_port_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->getLinuxConfigurationPort(); result.has_value()) {
        *value = static_cast<icsneoc2_linux_configuration_port_t>(result.value());
        return icsneoc2_error_success;
    }
    return icsneoc2_error_get_settings_failure;
}

icsneoc2_error_t icsneoc2_settings_linux_configuration_port_set(icsneoc2_device_t* device, icsneoc2_linux_configuration_port_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setLinuxConfigurationPort(static_cast<LinuxConfigurationPort>(value))) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_disabled_get(icsneoc2_device_t* device, bool* value) {
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    *value = device->device->settings->disabled;
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_readonly_get(icsneoc2_device_t* device, bool* value) {
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    *value = device->device->settings->readonly;
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_gptp_profile_get(icsneoc2_device_t* device, icsneoc2_gptp_profile_t* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->getGPTPProfile(); result.has_value()) {
        *value = static_cast<icsneoc2_gptp_profile_t>(result.value());
        return icsneoc2_error_success;
    }
    return icsneoc2_error_get_settings_failure;
}

icsneoc2_error_t icsneoc2_settings_gptp_profile_set(icsneoc2_device_t* device, icsneoc2_gptp_profile_t value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(value >= icsneoc2_gptp_profile_maxsize) {
        return icsneoc2_error_invalid_parameters;
    }
    if(!device->device->settings->setGPTPProfile(static_cast<RADGPTPProfile>(value))) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_gptp_role_get(icsneoc2_device_t* device, icsneoc2_gptp_role_t* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->getGPTPRole(); result.has_value()) {
        *value = static_cast<icsneoc2_gptp_role_t>(result.value());
        return icsneoc2_error_success;
    }
    return icsneoc2_error_get_settings_failure;
}

icsneoc2_error_t icsneoc2_settings_gptp_role_set(icsneoc2_device_t* device, icsneoc2_gptp_role_t value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(value >= icsneoc2_gptp_role_maxsize) {
        return icsneoc2_error_invalid_parameters;
    }
    if(!device->device->settings->setGPTPRole(static_cast<RADGPTPRole>(value))) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_gptp_enabled_port_get(icsneoc2_device_t* device, uint8_t* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->getGPTPEnabledPort(); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    }
    return icsneoc2_error_get_settings_failure;
}

icsneoc2_error_t icsneoc2_settings_gptp_enabled_port_set(icsneoc2_device_t* device, uint8_t value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setGPTPEnabledPort(value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_settings_gptp_clock_syntonization_enabled_get(icsneoc2_device_t* device, bool* value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!value) {
        return icsneoc2_error_invalid_parameters;
    }
    if(auto result = device->device->settings->isGPTPClockSyntonizationEnabled(); result.has_value()) {
        *value = result.value();
        return icsneoc2_error_success;
    }
    return icsneoc2_error_get_settings_failure;
}

icsneoc2_error_t icsneoc2_settings_gptp_clock_syntonization_enabled_set(icsneoc2_device_t* device, bool value) {
    auto res = icsneoc2_device_is_valid(device);
    if(res != icsneoc2_error_success) {
        return res;
    }
    if(!device->device->settings->setGPTPClockSyntonizationEnabled(value)) {
        return icsneoc2_error_set_settings_failure;
    }
    return icsneoc2_error_success;
}
