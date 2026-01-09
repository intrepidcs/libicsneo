#ifndef __RADMOONT1SSETTINGS_H_
#define __RADMOONT1SSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t disableUsbCheckOnBoot : 1;
		uint16_t reserved : 14;
	} flags;
	uint16_t network_enabled_on_boot;
	uint64_t network_enables;
	uint64_t network_enables_2;
	RAD_REPORTING_SETTINGS reporting;
	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;
	// Ethernet 10/100/1000
	ETHERNET_SETTINGS2 ethernet;
	// Ethernet General
	AE_GENERAL_SETTINGS aeGen;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s;
	ETHERNET10T1S_SETTINGS t1s;
	// 10T1S Extended Settings
	ETHERNET10T1S_SETTINGS_EXT t1sExt;
	RAD_GPTP_SETTINGS gPTP;
} radmoont1s_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radmoont1s_settings_t) == 160, "RADMoonT1S settings size mismatch");

#include <iostream>

class RADMoonT1SSettings : public IDeviceSettings {
public:
	RADMoonT1SSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radmoont1s_settings_t)) {}

	std::optional<bool> isT1SPLCAEnabledFor(Network net) const override {
		const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
		if(t1s == nullptr)
			return std::nullopt;

		return std::make_optional<bool>((t1s->flags & ETHERNET10T1S_SETTINGS_FLAG_ENABLE_PLCA) != 0);
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
		auto cfg = getStructurePointer<radmoont1s_settings_t>();
		if(cfg == nullptr)
			return nullptr;

		switch(net.getNetID()) {
			case Network::NetID::AE_01:
				return &(cfg->t1s);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}

	ETHERNET10T1S_SETTINGS* getMutableT1SSettingsFor(Network net) {
		auto cfg = getMutableStructurePointer<radmoont1s_settings_t>();
		if(cfg == nullptr)
			return nullptr;

		switch(net.getNetID()) {
			case Network::NetID::AE_01:
				return &(cfg->t1s);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}

	const ETHERNET10T1S_SETTINGS_EXT* getT1SSettingsExtFor(Network net) const {
		auto cfg = getStructurePointer<radmoont1s_settings_t>();
		if(cfg == nullptr)
			return nullptr;

		switch(net.getNetID()) {
			case Network::NetID::AE_01:
				return &(cfg->t1sExt);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}

	ETHERNET10T1S_SETTINGS_EXT* getMutableT1SSettingsExtFor(Network net) {
		auto cfg = getMutableStructurePointer<radmoont1s_settings_t>();
		if(cfg == nullptr)
			return nullptr;

		switch(net.getNetID()) {
			case Network::NetID::AE_01:
				return &(cfg->t1sExt);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif // __RADMOONT1SSETTINGS_H_
