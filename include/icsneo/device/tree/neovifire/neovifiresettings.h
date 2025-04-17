#ifndef __NEOVIFIRESETTINGS_H_
#define __NEOVIFIRESETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint16_t netId;
	uint8_t zero0;
	uint8_t Config;
} SNeoMostGatewaySettings;

typedef struct {
	CAN_SETTINGS can1;
	CAN_SETTINGS can2;
	CAN_SETTINGS can3;
	CAN_SETTINGS can4;

	SWCAN_SETTINGS swcan;
	CAN_SETTINGS lsftcan;

	LIN_SETTINGS lin1;
	LIN_SETTINGS lin2;
	LIN_SETTINGS lin3;
	LIN_SETTINGS lin4;

	uint16_t cgi_enable_reserved;
	uint16_t cgi_baud;
	uint16_t cgi_tx_ifs_bit_times;
	uint16_t cgi_rx_ifs_bit_times;
	uint16_t cgi_chksum_enable;

	uint16_t network_enables;
	uint16_t network_enabled_on_boot;

	uint32_t pwm_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t misc_io_initial_ddr;
	uint16_t misc_io_initial_latch;

	uint16_t misc_io_analog_enable;
	uint16_t misc_io_report_period;
	uint16_t misc_io_on_report_events;
	uint16_t ain_sample_period;
	uint16_t ain_threshold;

	uint16_t iso15765_separation_time_offset;

	uint16_t iso9141_kwp_enable_reserved;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings;

	uint16_t perf_en;

	/* ISO9141: iso_parity 0 - no parity, 1 - event, 2 - odd; iso_msg_termination 0 - use inner frame time, 1 - GME CIM-SCL */

	uint16_t iso_parity;
	uint16_t iso_msg_termination;
	uint16_t iso_tester_pullup_enable;

	uint16_t network_enables_2;

	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_2;
	uint16_t iso_parity_2;
	uint16_t iso_msg_termination_2;

	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_3;
	uint16_t iso_parity_3;
	uint16_t iso_msg_termination_3;

	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_4;
	uint16_t iso_parity_4;
	uint16_t iso_msg_termination_4;

	uint16_t fast_init_network_enables_1;
	uint16_t fast_init_network_enables_2;

	UART_SETTINGS uart;
	UART_SETTINGS uart2;

	STextAPISettings text_api;

	SNeoMostGatewaySettings neoMostGateway;

	uint16_t vnetBits;
} neovifire_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class NeoVIFIRESettings : public IDeviceSettings {
public:
	NeoVIFIRESettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(neovifire_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire_settings_t>();
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
			case Network::NetID::LSFTCAN_01:
				return &(cfg->lsftcan);
			default:
				return nullptr;
		}
	}
	const CAN_SETTINGS* getLSFTCANSettingsFor(Network net) const override { return getCANSettingsFor(net); }
	const SWCAN_SETTINGS* getSWCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::SWCAN_01:
				return &(cfg->swcan);
			default:
				return nullptr;
		}
	}
	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<neovifire_settings_t>();
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
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif