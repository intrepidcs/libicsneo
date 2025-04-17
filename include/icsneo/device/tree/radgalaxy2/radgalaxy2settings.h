#ifndef __RADGALAXY2SETTINGS_H_
#define __RADGALAXY2SETTINGS_H_

#include "icsneo/device/idevicesettings.h"
#include <stdint.h>

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)

typedef struct {
	uint32_t ecu_id;
	uint16_t perf_en;

	/* CAN */
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

	// SWCAN_SETTINGS swcan1;	G2 does not have SWCAN.
	uint16_t network_enables;
	// SWCAN_SETTINGS swcan2;	G2 does not have SWCAN.
	uint16_t network_enables_2;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t network_enabled_on_boot;

	/* ISO15765-2 Transport Layer */
	uint16_t iso15765_separation_time_offset;

	/* ISO9141 - Keyword */
	uint16_t iso_9141_kwp_enable_reserved;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;

	uint16_t iso_msg_termination_1;

	uint16_t idle_wakeup_network_enables_1;
	uint16_t idle_wakeup_network_enables_2;

	/* reserved for T1 networks such as BR1, BR2, etc.. */
	uint16_t network_enables_3;
	uint16_t idle_wakeup_network_enables_3;

	STextAPISettings text_api;

	uint64_t termination_enables; // New feature unlike Galaxy.

	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t reserved : 15;
	} flags;

	LIN_SETTINGS lin1;

	AE_GENERAL_SETTINGS aeGen;
	AE_SETTINGS ae_01;
	AE_SETTINGS ae_02;
	AE_SETTINGS ae_03;
	AE_SETTINGS ae_04;
	AE_SETTINGS ae_05;
	AE_SETTINGS ae_06;
	AE_SETTINGS ae_07;
	AE_SETTINGS ae_08;
	AE_SETTINGS ae_09;
	AE_SETTINGS ae_10;
	AE_SETTINGS ae_11;
	AE_SETTINGS ae_12;
	AE_SETTINGS ae_13;
	AE_SETTINGS ae_14;
	AE_SETTINGS ae_15;
	AE_SETTINGS ae_16;

	ETHERNET10G_SETTINGS ethernet10g;
	ETHERNET10G_SETTINGS ethernet10g_2;
	ETHERNET10G_SETTINGS ethernet10g_3;

	uint16_t network_enables_4;
	RAD_REPORTING_SETTINGS reporting;
	RAD_GPTP_SETTINGS gPTP;

	uint64_t network_enables_5;

	LIN_SETTINGS lin2;
} radgalaxy2_settings_t;

typedef struct {
	uint8_t unused[3];
	uint8_t ethernetActivationLineEnabled;
} radgalaxy2_status_t;

#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class RADGalaxy2Settings : public IDeviceSettings {
public:
	RADGalaxy2Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radgalaxy2_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgalaxy2_settings_t>();
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
		auto cfg = getStructurePointer<radgalaxy2_settings_t>();
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

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgalaxy2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LIN_01:
				return &(cfg->lin1);
			case Network::NetID::LIN_02:
				return &(cfg->lin2);
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif