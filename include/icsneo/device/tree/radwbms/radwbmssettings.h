#ifndef __RADWBMSSETTINGS_H_
#define __RADWBMSSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif // __cplusplus

#pragma pack(push, 2)
typedef struct
{
	uint16_t perf_en;

	uint64_t termination_enables;

	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;

	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;

	uint16_t network_enables;
	uint16_t network_enables_2;
	uint16_t network_enables_3;

	int16_t iso15765_separation_time_offset;

	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t enablePcEthernetComm : 1;
		uint32_t reserved : 29;
	} flags;

	ETHERNET_SETTINGS ethernet;
	ETHERNET_SETTINGS2 ethernet2;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;
	uint16_t network_enabled_on_boot;

	uint8_t rsvd[10]; //Was sWILBridgeConfig

	sSPI_PORT_SETTINGS spi_config;

	sWIL_CONNECTION_SETTINGS wbms_wil_1;
	sWIL_CONNECTION_SETTINGS wbms_wil_2;

	uint16_t wil1_nwk_metadata_buff_count;
	uint16_t wil2_nwk_metadata_buff_count;

	WBMSGatewaySettings gateway;

	uint16_t network_enables_4;
	uint64_t network_enables_5;
} radwbms_settings_t;

#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radwbms_settings_t) == 156, "RAD-wBMS settings size mismatch");

#include <iostream>

class RADwBMSSettings : public IDeviceSettings {
public:
	RADwBMSSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radwbms_settings_t)) {}
};

}

#endif // __cplusplus

#endif // __RADWBMSSETTINGS_H_