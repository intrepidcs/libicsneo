#ifndef __VALUECAN3SETTINGS_H_
#define __VALUECAN3SETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	CAN_SETTINGS can1;
	CAN_SETTINGS can2;

	uint16_t network_enables;
	uint16_t network_enabled_on_boot;

	uint16_t iso15765_separation_time_offset;

	uint16_t perf_en;

	uint16_t misc_io_initial_ddr;
	uint16_t misc_io_initial_latch;
	uint16_t misc_io_report_period;
	uint16_t misc_io_on_report_events;
} valuecan3_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class ValueCAN3Settings : public IDeviceSettings {
public:
	ValueCAN3Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(valuecan3_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<valuecan3_settings_t>();
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
};

}

#endif // __cplusplus

#endif