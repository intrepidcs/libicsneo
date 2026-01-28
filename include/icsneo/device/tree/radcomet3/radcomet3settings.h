#ifndef __RADCOMET3SETTINGS_H_
#define __RADCOMET3SETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	// ECU ID used in CAN communications.
	// TX ID = ECU ID with bit28 cleared,
	// RX ID = ECUID with bit28 set,
	// ECU ID = 0 implies ECU ID = serial no with bit 27 set
	uint32_t ecu_id;
	uint16_t perf_en;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t disableUsbCheckOnBoot : 1;
		uint16_t reserved : 14;
	} flags;
	uint16_t network_enabled_on_boot;
	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;
	uint16_t iso_msg_termination_1;
	uint64_t network_enables;
	uint64_t network_enables_2;
	uint64_t termination_enables;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	RAD_REPORTING_SETTINGS reporting;
	int16_t iso15765_separation_time_offset;
	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;
	RAD_GPTP_SETTINGS gPTP;
	STextAPISettings text_api;
	// Ethernet 10/100/1000
	ETHERNET_SETTINGS2 ethernet;
	// Ethernet General
	AE_GENERAL_SETTINGS aeGen;
	// 100/1000T1
	ETHERNET_SETTINGS2 ethT1;
	AE_SETTINGS ae_01;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s1;
	ETHERNET10T1S_SETTINGS t1s1;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s2;
	ETHERNET10T1S_SETTINGS t1s2;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s3;
	ETHERNET10T1S_SETTINGS t1s3;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s4;
	ETHERNET10T1S_SETTINGS t1s4;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s5;
	ETHERNET10T1S_SETTINGS t1s5;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s6;
	ETHERNET10T1S_SETTINGS t1s6;
	LIN_SETTINGS lin1;
	// 10T1S Extended Settings
	ETHERNET10T1S_SETTINGS_EXT t1s1Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s2Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s3Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s4Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s5Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s6Ext;
} radcomet3_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radcomet3_settings_t) == 674, "RADComet3 settings size mismatch");

#include <iostream>

class RADComet3Settings : public IDeviceSettings {
public:
	RADComet3Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radcomet3_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radcomet3_settings_t>();
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
		auto cfg = getStructurePointer<radcomet3_settings_t>();
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

	const ETHERNET_SETTINGS2* getEthernetSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radcomet3_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::ETHERNET_01:
				return &(cfg->ethernet);
			case Network::NetID::AE_01:
				return &(cfg->ethT1);
			case Network::NetID::AE_02:
				return &(cfg->ethT1s1);
			case Network::NetID::AE_03:
				return &(cfg->ethT1s2);
			case Network::NetID::AE_04:
				return &(cfg->ethT1s3);
			case Network::NetID::AE_05:
				return &(cfg->ethT1s4);
			case Network::NetID::AE_06:
				return &(cfg->ethT1s5);
			case Network::NetID::AE_07:
				return &(cfg->ethT1s6);
			default:
				return nullptr;
		}
	}

	const AE_SETTINGS* getAESettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radcomet3_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::AE_01:
				return &(cfg->ae_01);
			default:
				return nullptr;
		}
	}

	std::optional<bool> isT1SPLCAEnabledFor(Network net) const override {
		const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
		if(t1s == nullptr)
			return std::nullopt;

		return std::make_optional((t1s->flags & ETHERNET10T1S_SETTINGS_FLAG_ENABLE_PLCA) != 0);
	}

	bool setT1SPLCAFor(Network net, bool enable) override {
		ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
		if(t1s == nullptr)
			return false;

		if(enable)
			t1s->flags |= ETHERNET10T1S_SETTINGS_FLAG_ENABLE_PLCA;
		else
			t1s->flags &= ~ETHERNET10T1S_SETTINGS_FLAG_ENABLE_PLCA;

		return true;
	}

	std::optional<uint8_t> getT1SLocalIDFor(Network net) const override {
		const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
		if(t1s == nullptr)
			return std::nullopt;

		return std::make_optional(t1s->local_id);
	}

	bool setT1SLocalIDFor(Network net, uint8_t id) override {
		ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
		if(t1s == nullptr)
			return false;

		t1s->local_id = id;
		return true;
	}

	std::optional<uint8_t> getT1SMaxNodesFor(Network net) const override {
		const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
		if(t1s == nullptr)
			return std::nullopt;

		return std::make_optional(t1s->max_num_nodes);
	}

	bool setT1SMaxNodesFor(Network net, uint8_t nodes) override {
		ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
		if(t1s == nullptr)
			return false;

		t1s->max_num_nodes = nodes;
		return true;
	}

	std::optional<uint8_t> getT1STxOppTimerFor(Network net) const override {
		const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
		if(t1s == nullptr)
			return std::nullopt;

		return std::make_optional(t1s->to_timer);
	}

	bool setT1STxOppTimerFor(Network net, uint8_t timer) override {
		ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
		if(t1s == nullptr)
			return false;

		t1s->to_timer = timer;
		return true;
	}

	std::optional<uint8_t> getT1SMaxBurstFor(Network net) const override {
		const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
		if(t1s == nullptr)
			return std::nullopt;

		return std::make_optional(t1s->max_burst_count);
	}

	bool setT1SMaxBurstFor(Network net, uint8_t burst) override {
		ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
		if(t1s == nullptr)
			return false;

		t1s->max_burst_count = burst;
		return true;
	}

	std::optional<uint8_t> getT1SBurstTimerFor(Network net) const override {
		const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
		if(t1s == nullptr)
			return std::nullopt;

		return std::make_optional(t1s->burst_timer);
	}

	bool setT1SBurstTimerFor(Network net, uint8_t timer) override {
		ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
		if(t1s == nullptr)
			return false;

		t1s->burst_timer = timer;
		return true;
	}

	bool setPhyRoleFor(Network net, AELinkMode mode) override {
		if (mode != AE_LINK_AUTO && mode != AE_LINK_MASTER && mode != AE_LINK_SLAVE) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}

		AE_SETTINGS* ae = getMutableAESettingsFor(net);
		if (ae == nullptr) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}

		ae->ucConfigMode = static_cast<uint8_t>(mode);

		ETHERNET_SETTINGS2* ethSettings = getMutableEthernetSettingsFor(net);
		if (ethSettings == nullptr) {
			return false;
		}
		
		uint8_t& flags2 = ethSettings->flags2;
		
		switch (mode) {
			case AE_LINK_AUTO:
				flags2 |= ETHERNET_SETTINGS2_FLAGS2_LINK_MODE_AUTO;
				break;
			case AE_LINK_MASTER:
				flags2 &= ~ETHERNET_SETTINGS2_FLAGS2_LINK_MODE_AUTO;
				flags2 &= ~ETHERNET_SETTINGS2_FLAGS2_LINK_MODE_SLAVE;
				break;
			case AE_LINK_SLAVE:
				flags2 &= ~ETHERNET_SETTINGS2_FLAGS2_LINK_MODE_AUTO;
				flags2 |= ETHERNET_SETTINGS2_FLAGS2_LINK_MODE_SLAVE;
				break;
		}

		return true;
	}

	bool setPhyEnableFor(Network net, bool enable) override {
		auto cfg = getMutableStructurePointer<radcomet3_settings_t>();
		if (cfg == nullptr)
			return false;

		if (net.getType() != Network::Type::Ethernet && net.getType() != Network::Type::AutomotiveEthernet) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}

		auto coreMini = net.getCoreMini();
		if (!coreMini.has_value()) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}

		const uint64_t networkID = static_cast<uint64_t>(coreMini.value());
		uint64_t bitfields[2] = { cfg->network_enables, cfg->network_enables_2 };
		const bool success = enable ? 
			SetNetworkEnabled(bitfields, 2, networkID) :
			ClearNetworkEnabled(bitfields, 2, networkID);
		
		if (!success) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}

		cfg->network_enables = bitfields[0];
		cfg->network_enables_2 = bitfields[1];

		return true;
	}

	std::optional<AELinkMode> getPhyRoleFor(Network net) const override {
		const AE_SETTINGS* ae = getAESettingsFor(net);
		if (ae == nullptr) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return std::nullopt;
		}

		switch (ae->ucConfigMode) {
			case 0:
				return std::make_optional(AE_LINK_AUTO);
			case 1:
				return std::make_optional(AE_LINK_MASTER);
			case 2:
				return std::make_optional(AE_LINK_SLAVE);
			default:
				return std::make_optional(AE_LINK_AUTO);
		}
	}

	std::optional<bool> getPhyEnableFor(Network net) const override {
		auto cfg = getStructurePointer<radcomet3_settings_t>();
		if (cfg == nullptr) {
			report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
			return std::nullopt;
		}

		if (net.getType() != Network::Type::Ethernet && net.getType() != Network::Type::AutomotiveEthernet) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return std::nullopt;
		}

		auto coreMini = net.getCoreMini();
		if (!coreMini.has_value()) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return std::nullopt;
		}

		const uint64_t networkID = static_cast<uint64_t>(coreMini.value());
		const uint64_t bitfields[2] = { cfg->network_enables, cfg->network_enables_2 };
		return GetNetworkEnabled(bitfields, 2, networkID);
	}

	std::vector<EthPhyLinkMode> getSupportedPhyLinkModesFor(Network net) const override {
		switch(net.getNetID()) {
			case Network::NetID::ETHERNET_01:
				return {
					ETH_LINK_MODE_AUTO_NEGOTIATION,
					ETH_LINK_MODE_10MBPS_FULLDUPLEX,
					ETH_LINK_MODE_100MBPS_FULLDUPLEX,
					ETH_LINK_MODE_1GBPS_FULLDUPLEX
				};
			
			case Network::NetID::AE_01:
				return {
					ETH_LINK_MODE_AUTO_NEGOTIATION,
					ETH_LINK_MODE_100MBPS_FULLDUPLEX,
					ETH_LINK_MODE_1GBPS_FULLDUPLEX
				};
			
			case Network::NetID::AE_02:
			case Network::NetID::AE_03:
			case Network::NetID::AE_04:
			case Network::NetID::AE_05:
			case Network::NetID::AE_06:
			case Network::NetID::AE_07:
				return {ETH_LINK_MODE_10MBPS_HALFDUPLEX};
			
			default:
				return {};
		}
	}

	bool setPhyLinkModeFor(Network net, EthPhyLinkMode mode) override {
		auto supported = getSupportedPhyLinkModesFor(net);
		if (std::find(supported.begin(), supported.end(), mode) == supported.end()) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}

		auto cfg = getMutableStructurePointer<radcomet3_settings_t>();
		if (cfg == nullptr)
			return false;

		if (net.getNetID() == Network::NetID::AE_01) {
			AE_SETTINGS* ae = getMutableAESettingsFor(net);
			if (ae == nullptr) {
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return false;
			}

			switch (mode) {
				case ETH_LINK_MODE_AUTO_NEGOTIATION:
					ae->link_spd = 3;
					cfg->ethT1.link_speed = 2;
					cfg->ethT1.flags |= ETHERNET_SETTINGS2_FLAG_AUTO_NEG;
					cfg->ethT1.flags |= ETHERNET_SETTINGS2_FLAG_FULL_DUPLEX;
					break;
				case ETH_LINK_MODE_100MBPS_FULLDUPLEX:
					ae->link_spd = 1;
					cfg->ethT1.link_speed = 1;
					cfg->ethT1.flags &= ~ETHERNET_SETTINGS2_FLAG_AUTO_NEG;
					cfg->ethT1.flags |= ETHERNET_SETTINGS2_FLAG_FULL_DUPLEX;
					break;
				case ETH_LINK_MODE_1GBPS_FULLDUPLEX:
					ae->link_spd = 2;
					cfg->ethT1.link_speed = 2;
					cfg->ethT1.flags &= ~ETHERNET_SETTINGS2_FLAG_AUTO_NEG;
					cfg->ethT1.flags |= ETHERNET_SETTINGS2_FLAG_FULL_DUPLEX;
					break;
				default:
					report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
					return false;
			}
			
		} else if (net.getNetID() == Network::NetID::ETHERNET_01) {
			switch (mode) {
				case ETH_LINK_MODE_AUTO_NEGOTIATION:
					cfg->ethernet.flags |= ETHERNET_SETTINGS2_FLAG_AUTO_NEG;
					cfg->ethernet.link_speed = 2;
					cfg->ethernet.flags |= ETHERNET_SETTINGS2_FLAG_FULL_DUPLEX;
					break;
				case ETH_LINK_MODE_10MBPS_FULLDUPLEX:
					cfg->ethernet.link_speed = 0;
					cfg->ethernet.flags &= ~ETHERNET_SETTINGS2_FLAG_AUTO_NEG;
					cfg->ethernet.flags |= ETHERNET_SETTINGS2_FLAG_FULL_DUPLEX;
					break;
				case ETH_LINK_MODE_100MBPS_FULLDUPLEX:
					cfg->ethernet.link_speed = 1;
					cfg->ethernet.flags &= ~ETHERNET_SETTINGS2_FLAG_AUTO_NEG;
					cfg->ethernet.flags |= ETHERNET_SETTINGS2_FLAG_FULL_DUPLEX;
					break;
				case ETH_LINK_MODE_1GBPS_FULLDUPLEX:
					cfg->ethernet.link_speed = 2;
					cfg->ethernet.flags &= ~ETHERNET_SETTINGS2_FLAG_AUTO_NEG;
					cfg->ethernet.flags |= ETHERNET_SETTINGS2_FLAG_FULL_DUPLEX;
					break;
				default:
					report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
					return false;
			}
		}

		return true;
	}

	std::optional<EthPhyLinkMode> getPhyLinkModeFor(Network net) const override {
		auto cfg = getStructurePointer<radcomet3_settings_t>();
		if (cfg == nullptr) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return std::nullopt;
		}

		if (net.getNetID() == Network::NetID::ETHERNET_01) {
			if (cfg->ethernet.flags & ETHERNET_SETTINGS2_FLAG_AUTO_NEG) {
				return ETH_LINK_MODE_AUTO_NEGOTIATION;
			}
			
			bool fullDuplex = (cfg->ethernet.flags & ETHERNET_SETTINGS2_FLAG_FULL_DUPLEX) != 0;
			
			switch (cfg->ethernet.link_speed) {
				case 0:
					return fullDuplex ? ETH_LINK_MODE_10MBPS_FULLDUPLEX 
					                  : ETH_LINK_MODE_10MBPS_HALFDUPLEX;
				case 1:
					return fullDuplex ? ETH_LINK_MODE_100MBPS_FULLDUPLEX 
					                  : ETH_LINK_MODE_100MBPS_HALFDUPLEX;
				case 2:
					return fullDuplex ? ETH_LINK_MODE_1GBPS_FULLDUPLEX 
					                  : ETH_LINK_MODE_1GBPS_HALFDUPLEX;
				default:
					report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
					return std::nullopt;
			}

		} else if (net.getNetID() == Network::NetID::AE_01) {
			const AE_SETTINGS* ae = &cfg->ae_01;
			
			// Check auto-negotiate
			if (ae->link_spd == 3 || (cfg->ethT1.flags & ETHERNET_SETTINGS2_FLAG_AUTO_NEG)) {
				return ETH_LINK_MODE_AUTO_NEGOTIATION;
			}
			
			// T1 is always full-duplex
			switch (ae->link_spd) {
				case 1: // 100 Mbps
					return ETH_LINK_MODE_100MBPS_FULLDUPLEX;
				case 2: // 1000 Mbps
					return ETH_LINK_MODE_1GBPS_FULLDUPLEX;
				default:
					report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
					return std::nullopt;
			}

		} else if (net.getNetID() >= Network::NetID::AE_02 && net.getNetID() <= Network::NetID::AE_07) {
			// 10BASE-T1S ports - half-duplex only
			return ETH_LINK_MODE_10MBPS_HALFDUPLEX;

		} else {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return std::nullopt;
		}
	}

private:
	const ETHERNET10T1S_SETTINGS* getT1SSettingsFor(Network net) const {
		auto cfg = getStructurePointer<radcomet3_settings_t>();
		if(cfg == nullptr)
			return nullptr;

		switch(net.getNetID()) {
			case Network::NetID::AE_02: return &(cfg->t1s1);
			case Network::NetID::AE_03: return &(cfg->t1s2);
			case Network::NetID::AE_04: return &(cfg->t1s3);
			case Network::NetID::AE_05: return &(cfg->t1s4);
			case Network::NetID::AE_06: return &(cfg->t1s5);
			case Network::NetID::AE_07: return &(cfg->t1s6);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}

	ETHERNET10T1S_SETTINGS* getMutableT1SSettingsFor(Network net) {
		auto cfg = getMutableStructurePointer<radcomet3_settings_t>();
		if(cfg == nullptr)
			return nullptr;

		switch(net.getNetID()) {
			case Network::NetID::AE_02: return &(cfg->t1s1);
			case Network::NetID::AE_03: return &(cfg->t1s2);
			case Network::NetID::AE_04: return &(cfg->t1s3);
			case Network::NetID::AE_05: return &(cfg->t1s4);
			case Network::NetID::AE_06: return &(cfg->t1s5);
			case Network::NetID::AE_07: return &(cfg->t1s6);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif
