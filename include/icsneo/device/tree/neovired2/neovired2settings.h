#ifndef __NEOVIRED2SETTINGS_H_
#define __NEOVIRED2SETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
#endif

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en;
	uint16_t network_enabled_on_boot;
	uint16_t misc_io_on_report_events;
	uint16_t pwr_man_enable;
	int16_t iso15765_separation_time_offset;
	uint16_t slaveVnetA;
	uint32_t reserved;
	uint64_t termination_enables;
	union {
		uint64_t word;
		struct
		{
			uint16_t network_enables;
			uint16_t network_enables_2;
			uint16_t network_enables_3;
		};
	} network_enables;
	uint32_t pwr_man_timeout;
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
	LIN_SETTINGS lin1;
	LIN_SETTINGS lin2;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;
	uint16_t iso_msg_termination_1;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_2;
	uint16_t iso_parity_2;
	uint16_t iso_msg_termination_2;
	ETHERNET_SETTINGS ethernet;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSync;
	STextAPISettings text_api;
	struct {
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t busMessagesToAndroid : 1;
		uint32_t enablePcEthernetComm : 1;
		uint32_t enableDefaultLogger : 1;
		uint32_t enableDefaultUpload : 1;
		uint32_t reserved : 26;
	} flags;
	DISK_SETTINGS disk;
	uint16_t misc_io_report_period;
	uint16_t ain_threshold;
	uint16_t misc_io_analog_enable;
	uint16_t digitalIoThresholdTicks;
	uint16_t digitalIoThresholdEnable;
	uint16_t misc_io_initial_ddr;
	uint16_t misc_io_initial_latch;
	ETHERNET_SETTINGS2 ethernet2_1;
	ETHERNET_SETTINGS ethernet_2;
	ETHERNET_SETTINGS2 ethernet2_2;
	Fire3LinuxSettings os_settings;
	RAD_GPTP_SETTINGS gPTP;
	uint16_t iso_tester_pullup_enable;
	CMP_GLOBAL_DATA cmp_global_data;
	CMP_NETWORK_DATA cmp_stream_data[CMP_STREAMS_RED2];
	uint32_t networkTimeSync;
} neovired2_settings_t;

typedef struct {
	uint8_t backupPowerGood;
	uint8_t backupPowerEnabled;
	uint8_t usbHostPowerEnabled;
	uint8_t ethernetActivationLineEnabled;
	EthernetNetworkStatus ethernetStatus;
} neovired2_status_t;
#pragma pack(pop)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __cplusplus

#include <iostream>

class NeoVIRED2Settings : public IDeviceSettings {
public:
	NeoVIRED2Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(neovired2_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovired2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->can1);
			case Network::NetID::DWCAN_02:
				return &(cfg->can2);
			case Network::NetID::DWCAN_03:
				return &(cfg->can3);
			case Network::NetID::DWCAN_04:
				return &(cfg->can4);
			case Network::NetID::DWCAN_05:
				return &(cfg->can5);
			case Network::NetID::DWCAN_06:
				return &(cfg->can6);
			case Network::NetID::DWCAN_07:
				return &(cfg->can7);
			case Network::NetID::DWCAN_08:
				return &(cfg->can8);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovired2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->canfd1);
			case Network::NetID::DWCAN_02:
				return &(cfg->canfd2);
			case Network::NetID::DWCAN_03:
				return &(cfg->canfd3);
			case Network::NetID::DWCAN_04:
				return &(cfg->canfd4);
			case Network::NetID::DWCAN_05:
				return &(cfg->canfd5);
			case Network::NetID::DWCAN_06:
				return &(cfg->canfd6);
			case Network::NetID::DWCAN_07:
				return &(cfg->canfd7);
			case Network::NetID::DWCAN_08:
				return &(cfg->canfd8);
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
		auto cfg = getStructurePointer<neovired2_settings_t>();
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

protected:
	ICSNEO_UNALIGNED(const uint64_t*) getTerminationEnables() const override {
		auto cfg = getStructurePointer<neovired2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		return &cfg->termination_enables;
	}
};

}

#endif // __cplusplus

#endif