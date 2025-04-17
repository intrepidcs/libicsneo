#ifndef __RADCOMET3SETTINGS_H_
#define __RADCOMET3SETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	// ECU ID used in CAN communications.
	// TX ID = ECU ID with bit28 cleared,
	// RX ID = ECUID with bit28 set,
	// ECU ID = 0 implies ECU ID = serial no with bit 27 set
	uint32_t ecu_id;
	uint16_t perf_en;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t disableUsbCheckOnBoot : 1;
		uint16_t reserved : 14;
	} flags;
	uint16_t network_enabled_on_boot;
	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;
	uint16_t iso_msg_termination_1;
	uint64_t network_enables;
	uint64_t network_enables_2;
	uint64_t termination_enables;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	RAD_REPORTING_SETTINGS reporting;
	int16_t iso15765_separation_time_offset;
	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;
	RAD_GPTP_SETTINGS gPTP;
	STextAPISettings text_api;
	// Ethernet 10/100/1000
	ETHERNET_SETTINGS2 ethernet;
	// Ethernet General
	AE_GENERAL_SETTINGS aeGen;
	// 100/1000T1
	ETHERNET_SETTINGS2 ethT1;
	AE_SETTINGS ae_01;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s1;
	ETHERNET10T1S_SETTINGS t1s1;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s2;
	ETHERNET10T1S_SETTINGS t1s2;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s3;
	ETHERNET10T1S_SETTINGS t1s3;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s4;
	ETHERNET10T1S_SETTINGS t1s4;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s5;
	ETHERNET10T1S_SETTINGS t1s5;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s6;
	ETHERNET10T1S_SETTINGS t1s6;
	LIN_SETTINGS lin1;
	// 10T1S Extended Settings
	ETHERNET10T1S_SETTINGS_EXT t1s1Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s2Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s3Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s4Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s5Ext;
	ETHERNET10T1S_SETTINGS_EXT t1s6Ext;
} radcomet3_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radcomet3_settings_t) == 674, "RADComet3 settings size mismatch");

#include <iostream>

class RADComet3Settings : public IDeviceSettings {
public:
	RADComet3Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radcomet3_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radcomet3_settings_t>();
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
		auto cfg = getStructurePointer<radcomet3_settings_t>();
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
};

}

#endif // __cplusplus

#endif
