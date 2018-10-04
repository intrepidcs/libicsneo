#ifndef __NEOVIFIRE2SETTINGS_H_
#define __NEOVIFIRE2SETTINGS_H_

#include <stdint.h>
#include "device/include/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en;

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

	CAN_SETTINGS lsftcan1;
	CAN_SETTINGS lsftcan2;

	LIN_SETTINGS lin1;
	uint16_t misc_io_initial_ddr;
	LIN_SETTINGS lin2;
	uint16_t misc_io_initial_latch;
	LIN_SETTINGS lin3;
	uint16_t misc_io_report_period;
	LIN_SETTINGS lin4;
	uint16_t misc_io_on_report_events;
	LIN_SETTINGS lin5;
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

	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_2;
	uint16_t iso_parity_2;

	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_3;
	uint16_t iso_parity_3;

	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_4;
	uint16_t iso_parity_4;

	uint16_t iso_msg_termination_1;
	uint16_t iso_msg_termination_2;
	uint16_t iso_msg_termination_3;
	uint16_t iso_msg_termination_4;

	uint16_t idle_wakeup_network_enables_1;
	uint16_t idle_wakeup_network_enables_2;

	/* reserved for HSCAN6/7, LSFT2, etc.. */
	uint16_t network_enables_3;
	uint16_t idle_wakeup_network_enables_3;

	uint16_t can_switch_mode;
	STextAPISettings text_api;
	uint64_t termination_enables;
	LIN_SETTINGS lin6;
	ETHERNET_SETTINGS ethernet;
	uint16_t slaveVnetA;
	uint16_t slaveVnetB;
	struct {
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t busMessagesToAndroid : 1;
		uint32_t enablePcEthernetComm : 1;
		uint32_t enableDefaultLogger : 1;
		uint32_t enableDefaultUpload : 1;
		uint32_t reserved : 26;
	} flags;
	uint16_t digitalIoThresholdTicks;
	uint16_t digitalIoThresholdEnable;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSync;
} neovifire2_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class NeoVIFIRE2Settings : public IDeviceSettings {
public:
	NeoVIFIRE2Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(neovifire2_settings_t)) {}
	CAN_SETTINGS* getCANSettingsFor(Network net) override {
		auto cfg = getStructurePointer<neovifire2_settings_t>();
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
	// CANFD_SETTINGS* getCANFDSettingsFor(Network net) override { return nullptr; }
};

}

#endif // __cplusplus

#endif