#ifndef __RADPLUTOSETTINGS_H_
#define __RADPLUTOSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#define PLUTO_MAX_MAC_CONFIG_ENTRIES (5)

#pragma pack(push, 2)
typedef struct {
	/* Performance Test */
	uint16_t perf_en;

	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;
	LIN_SETTINGS lin1;

	uint16_t network_enables;
	uint16_t network_enables_2;
	uint16_t network_enables_3;
	uint64_t termination_enables;
	uint16_t misc_io_analog_enable;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t network_enabled_on_boot;

	/* ISO15765-2 Transport Layer */
	int16_t iso15765_separation_time_offset;
	uint16_t iso9141_kwp_enable_reserved;
	uint16_t iso_tester_pullup_enable;
	uint16_t iso_parity;
	uint16_t iso_msg_termination;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings;
	ETHERNET_SETTINGS ethernet;

	STextAPISettings text_api;
	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t enablePcEthernetComm : 1;
		uint32_t reserved : 29;
	} flags;

	struct
	{
		uint8_t mode[PLUTO_MAX_MAC_CONFIG_ENTRIES];
		uint8_t speed[PLUTO_MAX_MAC_CONFIG_ENTRIES];
		uint8_t enablePhy[PLUTO_MAX_MAC_CONFIG_ENTRIES];
		uint8_t ae1Select;
		uint8_t usbSelect;
		uint8_t pad;
	} custom;

	ETHERNET_SETTINGS2 ethernet2;
} radpluto_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radpluto_settings_t) == 322, "RAD-Pluto Settings are not packed correctly!");

#include <iostream>

class RADPlutoSettings : public IDeviceSettings {
public:
	RADPlutoSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radpluto_settings_t)) {
	}

	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radpluto_settings_t>();
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
		auto cfg = getStructurePointer<radpluto_settings_t>();
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

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radpluto_settings_t>();
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