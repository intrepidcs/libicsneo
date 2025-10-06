#ifndef __RADSTAR2SETTINGS_H_
#define __RADSTAR2SETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en;

	AE_GENERAL_SETTINGS aeGen;
	AE_SETTINGS ae_01;
	AE_SETTINGS ae_02;

	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;

	uint16_t network_enables;
	uint16_t network_enables_2;

	LIN_SETTINGS lin1;
	uint16_t misc_io_initial_ddr;
	uint16_t misc_io_initial_latch;
	uint16_t misc_io_report_period;
	uint16_t misc_io_on_report_events;
	uint16_t misc_io_analog_enable;
	uint16_t ain_sample_period;
	uint16_t ain_threshold;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t network_enabled_on_boot;

	uint16_t iso15765_separation_time_offset;

	uint16_t iso_9141_kwp_enable_reserved;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;

	uint16_t iso_msg_termination_1;

	uint16_t idle_wakeup_network_enables_1;
	uint16_t idle_wakeup_network_enables_2;

	uint16_t network_enables_3;
	uint16_t idle_wakeup_network_enables_3;

	uint16_t can_switch_mode;
	STextAPISettings text_api;
	uint16_t pc_com_mode;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	uint16_t hwComLatencyTestEn;
	RAD_REPORTING_SETTINGS reporting;
} radstar2_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class RADStar2Settings : public IDeviceSettings {
public:
	RADStar2Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radstar2_settings_t)) {
	}
	
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radstar2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->can1);
			case Network::NetID::DWCAN_08:
				return &(cfg->can2);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radstar2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->canfd1);
			case Network::NetID::DWCAN_08:
				return &(cfg->canfd2);
			default:
				return nullptr;
		}
	}

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radstar2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LIN_01:
				return &(cfg->lin1);
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif