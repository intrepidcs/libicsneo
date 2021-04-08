#ifndef __ETHERBADGESETTINGS_H_
#define __ETHERBADGESETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en;//2

	CAN_SETTINGS can1;//12
	CANFD_SETTINGS canfd1;//10
	CAN_SETTINGS can2;//12
	CANFD_SETTINGS canfd2;//10
	LIN_SETTINGS lin1;//10

	uint16_t network_enables;//2
	uint16_t network_enables_2;//2
	uint16_t network_enables_3;//2
	uint64_t termination_enables;//8

	uint32_t pwr_man_timeout;//4
	uint16_t pwr_man_enable;//2

	uint16_t network_enabled_on_boot;//2

	/* ISO15765-2 Transport Layer */
	int16_t iso15765_separation_time_offset;//2
	uint16_t iso9141_kwp_enable_reserved;//2
	uint16_t iso_tester_pullup_enable;//2
	uint16_t iso_parity;//2
	uint16_t iso_msg_termination;//2
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings;//114
	ETHERNET_SETTINGS ethernet;//8

	uint16_t misc_io_initial_ddr;//2
	uint16_t misc_io_initial_latch;//2
	uint16_t misc_io_report_period;//2
	uint16_t misc_io_on_report_events;//2
	uint16_t misc_io_analog_enable;//2
	uint16_t ain_sample_period;//2
	uint16_t ain_threshold;//2

	STextAPISettings text_api;//72

	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t enablePcEthernetComm : 1;
		uint32_t reserved : 29;
	} flags;//4
} etherbadge_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class EtherBADGESettings : public IDeviceSettings {
public:
	EtherBADGESettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(etherbadge_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<etherbadge_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->can1);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<etherbadge_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->canfd1);
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif