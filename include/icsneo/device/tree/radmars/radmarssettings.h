#ifndef __RADMARSSETTINGS_H_
#define __RADMARSSETTINGS_H_

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
	CAN_SETTINGS can7;
	CANFD_SETTINGS canfd7;
	CAN_SETTINGS can8;
	CANFD_SETTINGS canfd8;

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
	uint8_t rsvd1[8];// previously ETHERNET_SETTINGS
	uint8_t rsvd2[8];// previously ETHERNET10G_SETTINGS

	DISK_SETTINGS disk;

	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t disableUsbCheckOnBoot : 1;
		uint16_t reserved : 14;
	} flags;
	ETHERNET_SETTINGS2 ethernet;

	SERDESCAM_SETTINGS serdescam1;
	ETHERNET10G_SETTINGS ethernet10g;

	LIN_SETTINGS lin1;

	SERDESPOC_SETTINGS serdespoc;
	LOGGER_SETTINGS logger;
	SERDESCAM_SETTINGS serdescam2;
	SERDESCAM_SETTINGS serdescam3;
	SERDESCAM_SETTINGS serdescam4;

	ETHERNET_SETTINGS2 ethernet2;
	uint16_t network_enables_4;
	RAD_REPORTING_SETTINGS reporting;
} radmars_settings_t;

typedef struct {
	uint8_t unused[3];
	uint8_t ethernetActivationLineEnabled;
} radmars_status_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radmars_settings_t) == 666, "RAD-Mars settings size mismatch");

#include <iostream>

class RADMarsSettings : public IDeviceSettings {
public:
	RADMarsSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radmars_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radmars_settings_t>();
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
		auto cfg = getStructurePointer<radmars_settings_t>();
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

	virtual std::vector<TerminationGroup> getTerminationGroups() const override {
		return {
			{
				Network(Network::NetID::HSCAN),
				Network(Network::NetID::HSCAN3),
				Network(Network::NetID::HSCAN5),
				Network(Network::NetID::HSCAN7)
			},
			{
				Network(Network::NetID::MSCAN),
				Network(Network::NetID::HSCAN2),
				Network(Network::NetID::HSCAN4),
				Network(Network::NetID::HSCAN6)
			}
		};
	}

protected:
	ICSNEO_UNALIGNED(const uint64_t*) getTerminationEnables() const override {
		auto cfg = getStructurePointer<radmars_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		return &cfg->termination_enables;
	}
};

}

#endif // __cplusplus

#endif