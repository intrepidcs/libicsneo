#ifndef __RADGIGASTARSETTINGS_H_
#define __RADGIGASTARSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint32_t ecu_id;

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

	uint16_t network_enables;
	uint16_t network_enables_2;

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

	STextAPISettings text_api;
	uint64_t termination_enables;

	DISK_SETTINGS disk;

	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t disableUsbCheckOnBoot : 1;
		uint16_t reserved : 14;
	} flags;
	ETHERNET_SETTINGS2 ethernet1;
	ETHERNET_SETTINGS2 ethernet2;

	LIN_SETTINGS lin1;

	AE_GENERAL_SETTINGS aeGen;
	AE_SETTINGS ae_01;
	AE_SETTINGS ae_02;

	SERDESCAM_SETTINGS serdescam1;
	SERDESPOC_SETTINGS serdespoc;
	LOGGER_SETTINGS logger;
	SERDESCAM_SETTINGS serdescam2;
	SERDESCAM_SETTINGS serdescam3;
	SERDESCAM_SETTINGS serdescam4;
	RAD_REPORTING_SETTINGS reporting;
	uint16_t network_enables_4;
	SERDESGEN_SETTINGS serdesgen;

	RAD_GPTP_SETTINGS gPTP;
	uint64_t network_enables_5;
} radgigastar_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radgigastar_settings_t) == 710, "RADGigastar settings size mismatch");

#include <iostream>

class RADGigastarSettings : public IDeviceSettings {
public:
	RADGigastarSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radgigastar_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgigastar_settings_t>();
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
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgigastar_settings_t>();
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
			default:
				return nullptr;
		}
	}

	virtual std::vector<TerminationGroup> getTerminationGroups() const override {
		return {
			{
				Network(Network::NetID::DWCAN_01),
				Network(Network::NetID::DWCAN_02),
				Network(Network::NetID::DWCAN_03),
				Network(Network::NetID::DWCAN_04)
			},
			{
				Network(Network::NetID::DWCAN_08),
				Network(Network::NetID::DWCAN_05)
			}
		};
	}

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgigastar_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LIN_01:
				return &(cfg->lin1);
			default:
				return nullptr;
		}
	}

protected:
	ICSNEO_UNALIGNED(const uint64_t*) getTerminationEnables() const override {
		auto cfg = getStructurePointer<radgigastar_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		return &cfg->termination_enables;
	}
};

typedef struct {
	uint8_t unused[3];
	uint8_t ethernetActivationLineEnabled;
} radgigastar_status_t;

}

#endif // __cplusplus

#endif