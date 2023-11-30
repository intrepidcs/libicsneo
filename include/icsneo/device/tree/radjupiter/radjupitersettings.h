#ifndef __RADJUPITERSETTINGS_H_
#define __RADJUPITERSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#define RADJUPITER_NUM_PORTS 8

#define JUPITER_PTP_ROLE_DISABLED 0
#define JUPITER_PTP_ROLE_MASTER 1
#define JUPITER_PTP_ROLE_SLAVE 2

#pragma pack(push, 2)
typedef struct {
	uint32_t neighborPropDelay; //ns
	int8_t initLogPDelayReqInterval; // log2ms
	int8_t initLogSyncInterval; // log2ms
	int8_t operationLogPDelayReqInterval; // log2ms
	int8_t operationLogSyncInterval; // log2ms
	uint8_t gPTPportRole[RADJUPITER_NUM_PORTS]; // The 6th port is used for CoreMini
} radjupiter_ptp_params_t;

typedef struct {
	uint8_t phyMode[RADJUPITER_NUM_PORTS];
	uint8_t enablePhy[RADJUPITER_NUM_PORTS];
	uint8_t port7Select;
	uint8_t port8Select;
	uint8_t port8Speed;
	uint8_t port8Legacy;
	uint8_t spoofMacFlag;
	uint8_t spoofedMac[6];
	uint8_t pad;
	radjupiter_ptp_params_t ptpParams;
} radjupiter_switch_settings_t;

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
	struct {
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t enablePcEthernetComm : 1;
		uint32_t reserved : 29;
	} flags;

	radjupiter_switch_settings_t switchSettings;
	ETHERNET_SETTINGS2 ethernet2;
} radjupiter_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radjupiter_settings_t) == 348, "RAD-Jupiter Settings are not packed correctly!");

#include <iostream>

class RADJupiterSettings : public IDeviceSettings {
public:
	RADJupiterSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radjupiter_settings_t)) {}

	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radjupiter_settings_t>();
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
		auto cfg = getStructurePointer<radjupiter_settings_t>();
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
	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radjupiter_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LIN:
				return &(cfg->lin1);
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif