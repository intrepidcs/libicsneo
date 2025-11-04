#ifndef __NEOVIFIRE2SETTINGS_H_
#define __NEOVIFIRE2SETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

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

	/* reserved for DWCAN_06/7, LSFT2, etc.. */
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
	DISK_SETTINGS disk;
	ETHERNET_SETTINGS2 ethernet2; // supercedes ethernet settings
} neovifire2_settings_t;

typedef struct {
	uint8_t backupPowerGood;
	uint8_t backupPowerEnabled;
	uint8_t usbHostPowerEnabled;
	uint8_t ethernetActivationLineEnabled;
	EthernetNetworkStatus ethernetStatus;
} neovifire2_status_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(neovifire2_settings_t) == 936, "NeoVIFire2 settings size mismatch");

#include <iostream>

class NeoVIFIRE2Settings : public IDeviceSettings {
public:
	NeoVIFIRE2Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(neovifire2_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire2_settings_t>();
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
			case Network::NetID::LSFTCAN_01:
				return &(cfg->lsftcan1);
			case Network::NetID::LSFTCAN_02:
				return &(cfg->lsftcan2);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire2_settings_t>();
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
	const CAN_SETTINGS* getLSFTCANSettingsFor(Network net) const override { return getCANSettingsFor(net); }
	const SWCAN_SETTINGS* getSWCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::SWCAN_01:
				return &(cfg->swcan1);
			case Network::NetID::SWCAN_02:
				return &(cfg->swcan2);
			default:
				return nullptr;
		}
	}

	virtual std::vector<TerminationGroup> getTerminationGroups() const override {
		return {
			{
				Network(Network::NetID::DWCAN_01),
				Network(Network::NetID::DWCAN_03),
				Network(Network::NetID::DWCAN_05),
				Network(Network::NetID::DWCAN_07)
			},
			{
				Network(Network::NetID::DWCAN_08),
				Network(Network::NetID::DWCAN_02),
				Network(Network::NetID::DWCAN_04),
				Network(Network::NetID::DWCAN_06)
			}
		};
	}

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LIN_01:
				return &(cfg->lin1);
			case Network::NetID::LIN_02:
				return &(cfg->lin2);
			case Network::NetID::LIN_03:
				return &(cfg->lin3);
			case Network::NetID::LIN_04:
				return &(cfg->lin4);
			case Network::NetID::LIN_05:
				return &(cfg->lin5);
			case Network::NetID::LIN_06:
				return &(cfg->lin6);
			default:
				return nullptr;
		}
	}

protected:
	ICSNEO_UNALIGNED(const uint64_t*) getTerminationEnables() const override {
		auto cfg = getStructurePointer<neovifire2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		return &cfg->termination_enables;
	}
};

}

#endif // __cplusplus

#endif