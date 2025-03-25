#ifndef __RADEPSILONSETTINGS_H_
#define __RADEPSILONSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"
#include "icsneo/communication/network.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
#endif

#pragma pack(push, 2)
#define RADEPSILON_MAX_PHY 18
typedef struct {
	uint8_t phyMode[RADEPSILON_MAX_PHY];
	uint8_t enablePhy[RADEPSILON_MAX_PHY];
	uint8_t speed[RADEPSILON_MAX_PHY];
	uint8_t legacy[RADEPSILON_MAX_PHY];
	uint8_t spoofedMac[6];
	uint8_t spoofMacFlag;
	uint8_t pad;
} radepsilon_switch_settings_t;

typedef struct {
    /* Performance Test */
    uint16_t perf_en;

    CAN_SETTINGS can1;
    CANFD_SETTINGS canfd1;
    CAN_SETTINGS can2;
    CANFD_SETTINGS canfd2;
    LIN_SETTINGS lin1;

    uint16_t network_enables[3];
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

    radepsilon_switch_settings_t switchSettings;
    ETHERNET_SETTINGS2 ethernet2;
    uint16_t misc_io_on_report_events;
    DISK_SETTINGS disk;
} radepsilon_settings_t;
#pragma pack(pop)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __cplusplus

#include <iostream>

class RADEpsilonSettings : public IDeviceSettings {
public:
	RADEpsilonSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radepsilon_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radepsilon_settings_t>();
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
		auto cfg = getStructurePointer<radepsilon_settings_t>();
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
		auto cfg = getStructurePointer<radepsilon_settings_t>();
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

}; // namespace icsneo

#endif // __cplusplus

#endif // __RADEPSILONSETTINGS_H_
