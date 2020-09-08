#ifndef __VIVIDCANSETTINGS_H_
#define __VIVIDCANSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint32_t ecu_id;

	CAN_SETTINGS can1;// 12 bytes
	SWCAN_SETTINGS swcan1;// 14 bytes
	CAN_SETTINGS lsftcan1;// 12 bytes

	uint16_t network_enables;
	uint16_t network_enabled_on_boot;

	uint16_t iso15765_separation_time_offset;

	uint16_t perf_en;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t can_switch_mode;
	uint16_t termination_enables;

	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t reserved : 30;
	} flags;
} vividcan_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class VividCANSettings : public IDeviceSettings {
public:
	VividCANSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(vividcan_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<vividcan_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->can1);
			case Network::NetID::LSFTCAN:
				return &(cfg->lsftcan1);
			default:
				return nullptr;
		}
	}
	const CAN_SETTINGS* getLSFTCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<vividcan_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LSFTCAN:
				return &(cfg->lsftcan1);
			default:
				return nullptr;
		}
	}
	const SWCAN_SETTINGS* getSWCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<vividcan_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::SWCAN:
				return &(cfg->swcan1);
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif