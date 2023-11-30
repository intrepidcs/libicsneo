#ifndef __RADGALAXYSETTINGS_H_
#define __RADGALAXYSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en;

	OP_ETH_GENERAL_SETTINGS opEthGen;
	OP_ETH_SETTINGS opEth1;
	OP_ETH_SETTINGS opEth2;
	OP_ETH_SETTINGS opEth3;
	OP_ETH_SETTINGS opEth4;
	OP_ETH_SETTINGS opEth5;
	OP_ETH_SETTINGS opEth6;
	OP_ETH_SETTINGS opEth7;
	OP_ETH_SETTINGS opEth8;
	OP_ETH_SETTINGS opEth9;
	OP_ETH_SETTINGS opEth10;
	OP_ETH_SETTINGS opEth11;
	OP_ETH_SETTINGS opEth12;

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

	/* Native CAN are either LS1/LS2 or SW1/SW2 */
	SWCAN_SETTINGS swcan1;
	uint16_t network_enables;
	SWCAN_SETTINGS swcan2;
	uint16_t network_enables_2;

	LIN_SETTINGS lin1;
	uint16_t misc_io_initial_ddr;
	uint16_t misc_io_initial_latch;
	uint16_t misc_io_report_period;
	uint16_t misc_io_on_report_events;
	uint16_t misc_io_analog_enable;
	uint16_t ain_sample_period;
	uint16_t ain_threshold;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t network_enabled_on_boot;

	uint16_t iso15765_separation_time_offset;

	uint16_t iso_9141_kwp_enable_reserved;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;

	uint16_t iso_msg_termination_1;

	uint16_t idle_wakeup_network_enables_1;
	uint16_t idle_wakeup_network_enables_2;

	uint16_t network_enables_3;
	uint16_t idle_wakeup_network_enables_3;

	uint16_t can_switch_mode;
	STextAPISettings text_api;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	uint16_t hwComLatencyTestEn;
	RAD_REPORTING_SETTINGS reporting;
	DISK_SETTINGS disk;
	LOGGER_SETTINGS logger;

	ETHERNET_SETTINGS2 ethernet1;// DAQ port on label, NETID_ETHERNET
	ETHERNET_SETTINGS2 ethernet2;// LAN port on label, NETID_ETHERNET2
	uint16_t network_enables_4;
	
	RAD_GPTP_SETTINGS gPTP;
} radgalaxy_settings_t;

typedef struct {
	uint8_t unused[3];
	uint8_t ethernetActivationLineEnabled;
} radgalaxy_status_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class RADGalaxySettings : public IDeviceSettings {
public:
	RADGalaxySettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radgalaxy_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgalaxy_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->can1);
			case Network::NetID::MSCAN:
				return &(cfg->can2);
			case Network::NetID::HSCAN2:
				return &(cfg->can3);
			case Network::NetID::HSCAN3:
				return &(cfg->can4);
			case Network::NetID::HSCAN4:
				return &(cfg->can5);
			case Network::NetID::HSCAN5:
				return &(cfg->can6);
			case Network::NetID::HSCAN6:
				return &(cfg->can7);
			case Network::NetID::HSCAN7:
				return &(cfg->can8);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgalaxy_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->canfd1);
			case Network::NetID::MSCAN:
				return &(cfg->canfd2);
			case Network::NetID::HSCAN2:
				return &(cfg->canfd3);
			case Network::NetID::HSCAN3:
				return &(cfg->canfd4);
			case Network::NetID::HSCAN4:
				return &(cfg->canfd5);
			case Network::NetID::HSCAN5:
				return &(cfg->canfd6);
			case Network::NetID::HSCAN6:
				return &(cfg->canfd7);
			case Network::NetID::HSCAN7:
				return &(cfg->canfd8);
			default:
				return nullptr;
		}
	}
	const SWCAN_SETTINGS* getSWCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgalaxy_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::SWCAN:
				return &(cfg->swcan1);
			case Network::NetID::SWCAN2:
				return &(cfg->swcan2);
			default:
				return nullptr;
		}
	}

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgalaxy_settings_t>();
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