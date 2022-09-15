#ifndef __RADA2BSETTINGS_H_
#define __RADA2BSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct
{
	uint8_t tdmMode;
	uint8_t upstreamChannelOffset;
	uint8_t downstreamChannelOffset;
	uint8_t nodeType;
	/* 
	 * bit0: 16-bit channel width
	 */
	uint8_t flags;
	uint8_t reserved[15];
} rada2b_monitor_settings_t;

typedef struct {
	uint16_t perf_en;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t : 15;
	} flags;
	uint16_t network_enabled_on_boot;
	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;
	LIN_SETTINGS lin1;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;
	uint16_t iso_msg_termination_1;
	uint64_t network_enables;
	uint64_t termination_enables;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	RAD_REPORTING_SETTINGS reporting;
	DISK_SETTINGS disk;
	LOGGER_SETTINGS logger;
	int16_t iso15765_separation_time_offset;
	rada2b_monitor_settings_t a2b_monitor;
	rada2b_monitor_settings_t a2b_node;
	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;
	ETHERNET_SETTINGS2 ethernet;
} rada2b_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class RADA2BSettings : public IDeviceSettings {
public:
	RADA2BSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(rada2b_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<rada2b_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->can1);
			case Network::NetID::HSCAN2:
				return &(cfg->can2);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<rada2b_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->canfd1);
			case Network::NetID::HSCAN2:
				return &(cfg->canfd2);
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif