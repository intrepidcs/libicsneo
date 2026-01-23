#ifndef __NEOVIFIRE3T1SLINSETTINGS_H_
#define __NEOVIFIRE3T1SLINSETTINGS_H_

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
	uint16_t perf_en;
	uint16_t network_enabled_on_boot;
	uint16_t misc_io_on_report_events;
	uint16_t pwr_man_enable;
	int16_t iso15765_separation_time_offset;
	uint16_t slaveVnetA;
	uint32_t reserved;
	uint64_t termination_enables;
	uint64_t network_enables;
	uint32_t pwr_man_timeout;
	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;
	CAN_SETTINGS can3;
	CANFD_SETTINGS canfd3;
	CAN_SETTINGS can4;
	CANFD_SETTINGS canfd4;
	CAN_SETTINGS can5;
	CANFD_SETTINGS canfd5;
	CAN_SETTINGS can6;
	CANFD_SETTINGS canfd6;
	CAN_SETTINGS can7;
	CANFD_SETTINGS canfd7;
	CAN_SETTINGS can8;
	CANFD_SETTINGS canfd8;
	LIN_SETTINGS lin1;
	LIN_SETTINGS lin2;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;
	uint16_t iso_msg_termination_1;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_2;
	uint16_t iso_parity_2;
	uint16_t iso_msg_termination_2;
	ETHERNET_SETTINGS ethernet_1;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSync;
	STextAPISettings text_api;
	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t busMessagesToAndroid : 1;
		uint32_t reserved1 : 1;
		uint32_t enableDefaultLogger : 1;
		uint32_t enableDefaultUpload : 1;
		uint32_t reserved : 26;
	} flags;
	DISK_SETTINGS disk;
	uint16_t misc_io_report_period;
	uint16_t ain_threshold;
	uint16_t misc_io_analog_enable;
	uint16_t digitalIoThresholdTicks;
	uint16_t digitalIoThresholdEnable;
	uint16_t misc_io_initial_ddr;
	uint16_t misc_io_initial_latch;
	ETHERNET_SETTINGS2 ethernet2_1;
	ETHERNET_SETTINGS ethernet_2;
	ETHERNET_SETTINGS2 ethernet2_2;
	Fire3LinuxSettings os_settings;
	RAD_GPTP_SETTINGS gPTP;

	/* VEM */
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s1;
	ETHERNET10T1S_SETTINGS t1s1;
	ETHERNET10T1S_SETTINGS_EXT t1s1Ext;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s2;
	ETHERNET10T1S_SETTINGS t1s2;
	ETHERNET10T1S_SETTINGS_EXT t1s2Ext;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s3;
	ETHERNET10T1S_SETTINGS t1s3;
	ETHERNET10T1S_SETTINGS_EXT t1s3Ext;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s4;
	ETHERNET10T1S_SETTINGS t1s4;
	ETHERNET10T1S_SETTINGS_EXT t1s4Ext;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s5;
	ETHERNET10T1S_SETTINGS t1s5;
	ETHERNET10T1S_SETTINGS_EXT t1s5Ext;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s6;
	ETHERNET10T1S_SETTINGS t1s6;
	ETHERNET10T1S_SETTINGS_EXT t1s6Ext;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s7;
	ETHERNET10T1S_SETTINGS t1s7;
	ETHERNET10T1S_SETTINGS_EXT t1s7Ext;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s8;
	ETHERNET10T1S_SETTINGS t1s8;
	ETHERNET10T1S_SETTINGS_EXT t1s8Ext;

	LIN_SETTINGS lin3;
	LIN_SETTINGS lin4;
	LIN_SETTINGS lin5;
	LIN_SETTINGS lin6;
	LIN_SETTINGS lin7;
	LIN_SETTINGS lin8;
	LIN_SETTINGS lin9;
	LIN_SETTINGS lin10;

	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_3;
	uint16_t iso_parity_3;
	uint16_t iso_msg_termination_3;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_4;
	uint16_t iso_parity_4;
	uint16_t iso_msg_termination_4;

	uint16_t iso_tester_pullup_enable;

	uint64_t network_enables_5;

	CMP_GLOBAL_DATA cmp_global_data;
	CMP_NETWORK_DATA cmp_stream_data[CMP_STREAMS_FIRE3T1SLIN];
	uint32_t networkTimeSync;
} neovifire3t1slin_settings_t;

typedef struct {
	uint8_t backupPowerGood;
	uint8_t backupPowerEnabled;
	uint8_t usbHostPowerEnabled;
	uint8_t ethernetActivationLineEnabled;
	EthernetNetworkStatus ethernetStatus;
} neovifire3t1slin_status_t;
#pragma pack(pop)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __cplusplus

static_assert(sizeof(neovifire3t1slin_settings_t) == 1594, "NeoVIFire3T1SLIN settings size mismatch");

#include <iostream>

class NeoVIFIRE3T1SLINSettings : public IDeviceSettings {
public:
	NeoVIFIRE3T1SLINSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(neovifire3t1slin_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire3t1slin_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->can1);
			case Network::NetID::DWCAN_08:
				return &(cfg->can2);
			case Network::NetID::DWCAN_02:
				return &(cfg->can3);
			case Network::NetID::DWCAN_03:
				return &(cfg->can4);
			case Network::NetID::DWCAN_04:
				return &(cfg->can5);
			case Network::NetID::DWCAN_05:
				return &(cfg->can6);
			case Network::NetID::DWCAN_06:
				return &(cfg->can7);
			case Network::NetID::DWCAN_07:
				return &(cfg->can8);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire3t1slin_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->canfd1);
			case Network::NetID::DWCAN_08:
				return &(cfg->canfd2);
			case Network::NetID::DWCAN_02:
				return &(cfg->canfd3);
			case Network::NetID::DWCAN_03:
				return &(cfg->canfd4);
			case Network::NetID::DWCAN_04:
				return &(cfg->canfd5);
			case Network::NetID::DWCAN_05:
				return &(cfg->canfd6);
			case Network::NetID::DWCAN_06:
				return &(cfg->canfd7);
			case Network::NetID::DWCAN_07:
				return &(cfg->canfd8);
			default:
				return nullptr;
		}
	}

	virtual std::vector<TerminationGroup> getTerminationGroups() const override {
		return {
			{
				Network(Network::NetID::DWCAN_01),
				Network(Network::NetID::DWCAN_03),
				Network(Network::NetID::DWCAN_05),
				Network(Network::NetID::DWCAN_07)
			},
			{
				Network(Network::NetID::DWCAN_08),
				Network(Network::NetID::DWCAN_02),
				Network(Network::NetID::DWCAN_04),
				Network(Network::NetID::DWCAN_06)
			}
		};
	}

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire3t1slin_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LIN_01:
				return &(cfg->lin1);
			case Network::NetID::LIN_02:
				return &(cfg->lin2);
			case Network::NetID::LIN_03:
				return &(cfg->lin3);
			case Network::NetID::LIN_04:
				return &(cfg->lin4);
			case Network::NetID::LIN_05:
				return &(cfg->lin5);
			case Network::NetID::LIN_06:
				return &(cfg->lin6);
			case Network::NetID::LIN_07:
				return &(cfg->lin7);
			case Network::NetID::LIN_08:
				return &(cfg->lin8);
			case Network::NetID::LIN_09:
				return &(cfg->lin9);
			case Network::NetID::LIN_10:
				return &(cfg->lin10);
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
		auto cfg = getStructurePointer<neovifire3t1slin_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		
		switch(net.getNetID()) {
			case Network::NetID::AE_01: return &(cfg->t1s1);
			case Network::NetID::AE_02: return &(cfg->t1s2);
			case Network::NetID::AE_03: return &(cfg->t1s3);
			case Network::NetID::AE_04: return &(cfg->t1s4);
			case Network::NetID::AE_05: return &(cfg->t1s5);
			case Network::NetID::AE_06: return &(cfg->t1s6);
			case Network::NetID::AE_07: return &(cfg->t1s7);
			case Network::NetID::AE_08: return &(cfg->t1s8);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}

	ETHERNET10T1S_SETTINGS* getMutableT1SSettingsFor(Network net) {
		auto cfg = getMutableStructurePointer<neovifire3t1slin_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		
		switch(net.getNetID()) {
			case Network::NetID::AE_01: return &(cfg->t1s1);
			case Network::NetID::AE_02: return &(cfg->t1s2);
			case Network::NetID::AE_03: return &(cfg->t1s3);
			case Network::NetID::AE_04: return &(cfg->t1s4);
			case Network::NetID::AE_05: return &(cfg->t1s5);
			case Network::NetID::AE_06: return &(cfg->t1s6);
			case Network::NetID::AE_07: return &(cfg->t1s7);
			case Network::NetID::AE_08: return &(cfg->t1s8);
			default:
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return nullptr;
		}
	}

protected:
	ICSNEO_UNALIGNED(const uint64_t*) getTerminationEnables() const override {
		auto cfg = getStructurePointer<neovifire3t1slin_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		return &cfg->termination_enables;
	}
};

}

#endif // __cplusplus

#endif