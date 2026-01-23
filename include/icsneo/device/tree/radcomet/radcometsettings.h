#ifndef __RADCOMETSETTINGS_H_
#define __RADCOMETSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
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
	union
	{
		uint64_t word;
		struct
		{
			uint16_t network_enables;
			uint16_t network_enables_2;
			uint16_t network_enables_3;
			uint16_t network_enables_4;
		};
	} network_enables;
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
	uint64_t network_enables_5;
	LIN_SETTINGS lin1;
	// 10T1S Extended settings
	ETHERNET10T1S_SETTINGS_EXT t1s1Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s2Ext;
} radcomet_settings_t;
#pragma pack(pop)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __cplusplus

static_assert(sizeof(radcomet_settings_t) == 498, "RADComet settings size mismatch");

#include <iostream>

class RADCometSettings : public IDeviceSettings {
public:
	RADCometSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radcomet_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radcomet_settings_t>();
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
		auto cfg = getStructurePointer<radcomet_settings_t>();
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

private:
	const ETHERNET10T1S_SETTINGS* getT1SSettingsFor(Network net) const {
		auto cfg = getStructurePointer<radcomet_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		
		switch(net.getNetID()) {
			case Network::NetID::AE_02: return &(cfg->t1s1);
			case Network::NetID::AE_03: return &(cfg->t1s2);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}

	ETHERNET10T1S_SETTINGS* getMutableT1SSettingsFor(Network net) {
		auto cfg = getMutableStructurePointer<radcomet_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		
		switch(net.getNetID()) {
			case Network::NetID::AE_02: return &(cfg->t1s1);
			case Network::NetID::AE_03: return &(cfg->t1s2);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif
