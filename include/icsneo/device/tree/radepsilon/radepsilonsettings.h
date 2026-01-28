#ifndef __RADEPSILONSETTINGS_H_
#define __RADEPSILONSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"
#include "icsneo/communication/network.h"
#include <optional>

#ifdef __cplusplus

namespace icsneo {

#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
#endif

#pragma pack(push, 2)
#define RADEPSILON_MAX_PHY 18
typedef struct {
	uint8_t phyMode[RADEPSILON_MAX_PHY];
	uint8_t enablePhy[RADEPSILON_MAX_PHY];
	uint8_t speed[RADEPSILON_MAX_PHY];
	uint8_t legacy[RADEPSILON_MAX_PHY];
	uint8_t spoofedMac[6];
	uint8_t spoofMacFlag;
	uint8_t pad;
} radepsilon_switch_settings_t;

typedef struct {
    /* Performance Test */
    uint16_t perf_en;

    CAN_SETTINGS can1;
    CANFD_SETTINGS canfd1;
    CAN_SETTINGS can2;
    CANFD_SETTINGS canfd2;
    LIN_SETTINGS lin1;

    uint16_t network_enables[3];
    uint64_t termination_enables;
    uint16_t misc_io_analog_enable;

    uint32_t pwr_man_timeout;
    uint16_t pwr_man_enable;

    uint16_t network_enabled_on_boot;

    /* ISO15765-2 Transport Layer */
    int16_t iso15765_separation_time_offset;
    uint16_t iso9141_kwp_enable_reserved;
    uint16_t iso_tester_pullup_enable;
    uint16_t iso_parity;
    uint16_t iso_msg_termination;
    ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings;
    ETHERNET_SETTINGS ethernet;

    STextAPISettings text_api;

    struct
    {
        uint32_t disableUsbCheckOnBoot : 1;
        uint32_t enableLatencyTest : 1;
        uint32_t enablePcEthernetComm : 1;
        uint32_t reserved : 29;
    } flags;

    radepsilon_switch_settings_t switchSettings;
    ETHERNET_SETTINGS2 ethernet2;
    uint16_t misc_io_on_report_events;
    DISK_SETTINGS disk;
} radepsilon_settings_t;
#pragma pack(pop)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __cplusplus

static_assert(sizeof(radepsilon_settings_t) == 400, "RADEpsilon settings size mismatch");

#include <iostream>

class RADEpsilonSettings : public IDeviceSettings {
public:
	RADEpsilonSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radepsilon_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radepsilon_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->can1);
			case Network::NetID::DWCAN_02:
				return &(cfg->can2);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radepsilon_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->canfd1);
			case Network::NetID::DWCAN_02:
				return &(cfg->canfd2);
			default:
				return nullptr;
		}
	}

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radepsilon_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LIN_01:
				return &(cfg->lin1);
			default:
				return nullptr;
		}
	}

	bool setPhyMode(uint8_t index, AELinkMode mode) override {
		if (index > RADEPSILON_MAX_PHY) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}
		auto cfg = getMutableStructurePointer<radepsilon_settings_t>();
		if (cfg == nullptr) {
			return false;
		}
		EpsilonPhyMode epsilonMode;
		switch (mode) {
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return false;
			case AE_LINK_AUTO:
				epsilonMode = EpsilonPhyMode::Auto;
				break;
			case AE_LINK_SLAVE:
				epsilonMode = EpsilonPhyMode::Slave;
				break;
			case AE_LINK_MASTER:
				epsilonMode = EpsilonPhyMode::Master;
				break;

		}
		cfg->switchSettings.phyMode[index] = static_cast<uint8_t>(epsilonMode);
		return true;
	}

	bool setPhyEnable(uint8_t index, bool enable) override {
		if (index > RADEPSILON_MAX_PHY) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}
		auto cfg = getMutableStructurePointer<radepsilon_settings_t>();
		if (cfg == nullptr) {
			return false;
		}
		cfg->switchSettings.enablePhy[index] = enable;
		return true;
	}

	bool setPhySpeed(uint8_t index, EthPhyLinkMode mode) override {
		if (index > RADEPSILON_MAX_PHY) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}
		auto cfg = getMutableStructurePointer<radepsilon_settings_t>();
		if (cfg == nullptr) {
			return false;
		}
		EpsilonPhySpeed epsilonSpeed;
		switch (mode) {
			case ETH_LINK_MODE_AUTO_NEGOTIATION:
				// Auto-negotiate - default to 1G base speed
				epsilonSpeed = EpsilonPhySpeed::Speed1000;
				break;
			case ETH_LINK_MODE_100MBPS_FULLDUPLEX:
				epsilonSpeed = EpsilonPhySpeed::Speed100;
				break;
			case ETH_LINK_MODE_1GBPS_FULLDUPLEX:
				epsilonSpeed = EpsilonPhySpeed::Speed1000;
				break;
			case ETH_LINK_MODE_10GBPS_FULLDUPLEX:
				epsilonSpeed = EpsilonPhySpeed::Speed10000;
				break;
			// Reject half-duplex modes - automotive T1 is full-duplex only
			case ETH_LINK_MODE_10MBPS_HALFDUPLEX:
			case ETH_LINK_MODE_10MBPS_FULLDUPLEX:
			case ETH_LINK_MODE_100MBPS_HALFDUPLEX:
			case ETH_LINK_MODE_1GBPS_HALFDUPLEX:
			case ETH_LINK_MODE_2_5GBPS_FULLDUPLEX:
			case ETH_LINK_MODE_5GBPS_FULLDUPLEX:
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return false;
		}
		cfg->switchSettings.speed[index] = static_cast<uint8_t>(epsilonSpeed);
		return true;
	}

	std::optional<AELinkMode> getPhyMode(uint8_t index) override {
		if (index > RADEPSILON_MAX_PHY) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return std::nullopt;
		}
		auto cfg = getStructurePointer<radepsilon_settings_t>();
		if (cfg == nullptr) {
			return std::nullopt;
		}
		AELinkMode mode;
		switch (static_cast<EpsilonPhyMode>(cfg->switchSettings.phyMode[index])) {
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return std::nullopt;
			case EpsilonPhyMode::Auto:
				mode = AE_LINK_AUTO;
				break;
			case EpsilonPhyMode::Slave:
				mode = AE_LINK_SLAVE;
				break;
			case EpsilonPhyMode::Master:
				mode = AE_LINK_MASTER;
				break;
		}
		return std::make_optional(mode);
	}

	std::optional<bool> getPhyEnable(uint8_t index) override {
		if (index > RADEPSILON_MAX_PHY) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return std::nullopt;
		}
		auto cfg = getStructurePointer<radepsilon_settings_t>();
		if (cfg == nullptr) {
			return false;
		}
		return std::make_optional(static_cast<bool>(cfg->switchSettings.enablePhy[index]));
	}

	std::optional<EthPhyLinkMode> getPhySpeed(uint8_t index) override {
		if (index > RADEPSILON_MAX_PHY) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return std::nullopt;
		}
		auto cfg = getStructurePointer<radepsilon_settings_t>();
		if (cfg == nullptr) {
			return std::nullopt;
		}
		// Automotive Ethernet T1 is always full-duplex
		switch (static_cast<EpsilonPhySpeed>(cfg->switchSettings.speed[index])) {
			case EpsilonPhySpeed::Speed100:
				return ETH_LINK_MODE_100MBPS_FULLDUPLEX;
			case EpsilonPhySpeed::Speed1000:
				return ETH_LINK_MODE_1GBPS_FULLDUPLEX;
			case EpsilonPhySpeed::Speed10000:
				return ETH_LINK_MODE_10GBPS_FULLDUPLEX;
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return std::nullopt;
		}
	}

private:
	enum class EpsilonPhyMode : uint8_t {
		Auto = 0,
		Master = 1,
		Slave = 2,
	};
	enum class EpsilonPhySpeed : uint8_t {
		Speed100 = 0,
		Speed1000 = 1,
		Speed10000 = 2,
	};
	
};

}; // namespace icsneo

#endif // __cplusplus

#endif // __RADEPSILONSETTINGS_H_
