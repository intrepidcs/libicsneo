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
	uint64_t network_enables_5;
} radgalaxy_settings_t;

typedef struct {
	uint8_t unused[3];
	uint8_t ethernetActivationLineEnabled;
} radgalaxy_status_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radgalaxy_settings_t) == 776, "RADGalaxy settings size mismatch");

#include <iostream>

class RADGalaxySettings : public IDeviceSettings {
public:
	RADGalaxySettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radgalaxy_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgalaxy_settings_t>();
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
		auto cfg = getStructurePointer<radgalaxy_settings_t>();
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
	const SWCAN_SETTINGS* getSWCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgalaxy_settings_t>();
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

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgalaxy_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LIN_01:
				return &(cfg->lin1);
			default:
				return nullptr;
		}
	}

	bool setMiscIOAnalogOutputEnabled(uint8_t pin, bool enabled) override {
		if(!settingsLoaded) {
			report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
			return false;
		}

		if(disabled) {
			report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
			return false;
		}

		if(readonly) {
			report(APIEvent::Type::SettingsReadOnly, APIEvent::Severity::Error);
			return false;
		}

		if(pin < 1 || pin > 2) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}

		auto cfg = getMutableStructurePointer<radgalaxy_settings_t>();
		if(cfg == nullptr) {
			report(APIEvent::Type::SettingNotAvaiableDevice, APIEvent::Severity::Error);
			return false;
		}

		const uint16_t bitMask = 1 << (pin - 1);

		if(enabled) {
			// Set pin as output and enable analog mode
			cfg->misc_io_initial_ddr |= bitMask;
			cfg->misc_io_analog_enable |= bitMask;
		} else {
			// Disable analog mode (leave DDR as-is)
			cfg->misc_io_analog_enable &= ~bitMask;
		}

		return true;
	}

	bool setMiscIOAnalogOutput(uint8_t pin, MiscIOAnalogVoltage voltage) override {
		if(!settingsLoaded) {
			report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
			return false;
		}

		if(disabled) {
			report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
			return false;
		}

		if(readonly) {
			report(APIEvent::Type::SettingsReadOnly, APIEvent::Severity::Error);
			return false;
		}

		if(pin < 1 || pin > 2) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}

		auto cfg = getMutableStructurePointer<radgalaxy_settings_t>();
		if(cfg == nullptr) {
			report(APIEvent::Type::SettingNotAvaiableDevice, APIEvent::Severity::Error);
			return false;
		}

		const uint8_t dacValue = static_cast<uint8_t>(voltage);
		
		if(dacValue > 5) {
			report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return false;
		}

		if(pin == 1) {
			// Update low nibble of high byte (bits 8-11), preserve pin 2 value
			cfg->misc_io_initial_latch = (cfg->misc_io_initial_latch & 0xF0FF) | (static_cast<uint16_t>(dacValue) << 8);
		} else { // pin == 2
			// Update high nibble of high byte (bits 12-15), preserve pin 1 value
			cfg->misc_io_initial_latch = (cfg->misc_io_initial_latch & 0x0FFF) | (static_cast<uint16_t>(dacValue) << 12);
		}

		return true;
	}
};

}

#endif // __cplusplus

#endif