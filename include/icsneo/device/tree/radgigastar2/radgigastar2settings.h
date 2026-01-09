#ifndef __RADGIGASTAR2SETTINGS_H_
#define __RADGIGASTAR2SETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo
{

#endif

#pragma pack(push, 2)
	typedef struct
	{
		uint32_t ecu_id;
		uint16_t perf_en;
		struct
		{
			uint16_t hwComLatencyTestEn : 1;
			uint16_t disableUsbCheckOnBoot : 1;
			uint16_t reserved : 14;
		} flags;
		uint16_t network_enabled_on_boot;
		CAN_SETTINGS can1;
		CANFD_SETTINGS canfd1;
		CAN_SETTINGS can2;
		CANFD_SETTINGS canfd2;
		CAN_SETTINGS can3;
		CANFD_SETTINGS canfd3;
		CAN_SETTINGS can4;
		CANFD_SETTINGS canfd4;

		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
		uint16_t iso_parity_1;
		uint16_t iso_msg_termination_1;
		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_2;
		uint16_t iso_parity_2;
		uint16_t iso_msg_termination_2;
		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_3;
		uint16_t iso_parity_3;
		uint16_t iso_msg_termination_3;
		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_4;
		uint16_t iso_parity_4;
		uint16_t iso_msg_termination_4;
		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_5;
		uint16_t iso_parity_5;
		uint16_t iso_msg_termination_5;
		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_6;
		uint16_t iso_parity_6;
		uint16_t iso_msg_termination_6;
		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_7;
		uint16_t iso_parity_7;
		uint16_t iso_msg_termination_7;
		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_8;
		uint16_t iso_parity_8;
		uint16_t iso_msg_termination_8;
		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_9;
		uint16_t iso_parity_9;
		uint16_t iso_msg_termination_9;
		ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_10;
		uint16_t iso_parity_10;
		uint16_t iso_msg_termination_10;
		uint64_t network_enables;
		uint64_t network_enables_2;
		uint64_t termination_enables;
		TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
		RAD_REPORTING_SETTINGS reporting;
		int16_t iso15765_separation_time_offset;
		uint32_t pwr_man_timeout;
		uint16_t pwr_man_enable;
		RAD_GPTP_SETTINGS gPTP;
		STextAPISettings text_api;
		DISK_SETTINGS disk;
		LOGGER_SETTINGS logger;
		LIN_SETTINGS lin1;
		LIN_SETTINGS lin2;
		LIN_SETTINGS lin3;
		LIN_SETTINGS lin4;
		LIN_SETTINGS lin5;
		LIN_SETTINGS lin6;
		LIN_SETTINGS lin7;
		LIN_SETTINGS lin8;
		LIN_SETTINGS lin9;
		LIN_SETTINGS lin10;
		// TODO more LIN
		// Ethernet SFP
		ETHERNET_SETTINGS2 ethernet1;
		ETHERNET_SETTINGS2 ethernet2;
		// Ethernet General
		AE_GENERAL_SETTINGS aeGen;
		// 100/1000T1
		ETHERNET_SETTINGS2 ethT1;
		AE_SETTINGS ae_01;
		ETHERNET_SETTINGS2 ethT12;
		AE_SETTINGS ae_02;
		// 10T1S
		ETHERNET_SETTINGS2 ethT1s1;
		ETHERNET10T1S_SETTINGS t1s1;
		ETHERNET10T1S_SETTINGS_EXT t1s1Ext;
		// 10T1S
		ETHERNET_SETTINGS2 ethT1s2;
		ETHERNET10T1S_SETTINGS t1s2;
		ETHERNET10T1S_SETTINGS_EXT t1s2Ext;
		// 10T1S
		ETHERNET_SETTINGS2 ethT1s3;
		ETHERNET10T1S_SETTINGS t1s3;
		ETHERNET10T1S_SETTINGS_EXT t1s3Ext;
		// 10T1S
		ETHERNET_SETTINGS2 ethT1s4;
		ETHERNET10T1S_SETTINGS t1s4;
		ETHERNET10T1S_SETTINGS_EXT t1s4Ext;
		// 10T1S
		ETHERNET_SETTINGS2 ethT1s5;
		ETHERNET10T1S_SETTINGS t1s5;
		ETHERNET10T1S_SETTINGS_EXT t1s5Ext;
		// 10T1S
		ETHERNET_SETTINGS2 ethT1s6;
		ETHERNET10T1S_SETTINGS t1s6;
		ETHERNET10T1S_SETTINGS_EXT t1s6Ext;
		// 10T1S
		ETHERNET_SETTINGS2 ethT1s7;
		ETHERNET10T1S_SETTINGS t1s7;
		ETHERNET10T1S_SETTINGS_EXT t1s7Ext;
		// 10T1S
		ETHERNET_SETTINGS2 ethT1s8;
		ETHERNET10T1S_SETTINGS t1s8;
		ETHERNET10T1S_SETTINGS_EXT t1s8Ext;

		// SFP T1S
		ETHERNET10T1S_SETTINGS sfp_t1s_1;
		ETHERNET10T1S_SETTINGS_EXT sfp_t1s_ext_1;
		ETHERNET10T1S_SETTINGS sfp_t1s_2;
		ETHERNET10T1S_SETTINGS_EXT sfp_t1s_ext_2;

		LIN_SETTINGS lin11;
		LIN_SETTINGS lin12;
		LIN_SETTINGS lin13;
		LIN_SETTINGS lin14;
		LIN_SETTINGS lin15;
		LIN_SETTINGS lin16;

		// SFP T1L
		ETHERNET10T1L_SETTINGS sfp_t1l_1;
		ETHERNET10T1L_SETTINGS sfp_t1l_2;
	} radgigastar2_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

	static_assert(sizeof(radgigastar2_settings_t) == 2156, "RADGigastar2 settings size mismatch");

#include <iostream>

	class RADGigastar2Settings : public IDeviceSettings
	{
	public:
		RADGigastar2Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radgigastar2_settings_t)) {}
		const CAN_SETTINGS *getCANSettingsFor(Network net) const override
		{
			auto cfg = getStructurePointer<radgigastar2_settings_t>();
			if (cfg == nullptr)
				return nullptr;
			switch (net.getNetID())
			{
			case Network::NetID::DWCAN_01:
				return &(cfg->can1);
			case Network::NetID::DWCAN_02:
				return &(cfg->can2);
			case Network::NetID::DWCAN_03:
				return &(cfg->can3);
			case Network::NetID::DWCAN_04:
				return &(cfg->can4);
			default:
				return nullptr;
			}
		}
		const CANFD_SETTINGS *getCANFDSettingsFor(Network net) const override
		{
			auto cfg = getStructurePointer<radgigastar2_settings_t>();
			if (cfg == nullptr)
				return nullptr;
			switch (net.getNetID())
			{
			case Network::NetID::DWCAN_01:
				return &(cfg->canfd1);
			case Network::NetID::DWCAN_02:
				return &(cfg->canfd2);
			case Network::NetID::DWCAN_03:
				return &(cfg->canfd3);
			case Network::NetID::DWCAN_04:
				return &(cfg->canfd4);
			default:
				return nullptr;
			}
		}

		virtual std::vector<TerminationGroup> getTerminationGroups() const override
		{
			return {
				{Network(Network::NetID::DWCAN_01)},
				{Network(Network::NetID::DWCAN_02)},
				{Network(Network::NetID::DWCAN_03)},
				{Network(Network::NetID::DWCAN_04)}};
		}

		const LIN_SETTINGS *getLINSettingsFor(Network net) const override
		{
			auto cfg = getStructurePointer<radgigastar2_settings_t>();
			if (cfg == nullptr)
				return nullptr;
			switch (net.getNetID())
			{
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
			case Network::NetID::LIN_07:
				return &(cfg->lin7);
			case Network::NetID::LIN_08:
				return &(cfg->lin8);
			case Network::NetID::LIN_09:
				return &(cfg->lin9);
			case Network::NetID::LIN_10:
				return &(cfg->lin10);
			case Network::NetID::LIN_11:
				return &(cfg->lin11);
			case Network::NetID::LIN_12:
				return &(cfg->lin12);
			case Network::NetID::LIN_13:
				return &(cfg->lin13);
			case Network::NetID::LIN_14:
				return &(cfg->lin14);
			case Network::NetID::LIN_15:
				return &(cfg->lin15);
			case Network::NetID::LIN_16:
				return &(cfg->lin16);
			default:
				return nullptr;
			}
		}

		std::optional<bool> isT1SPLCAEnabledFor(Network net) const override {
			const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
			if(t1s == nullptr)
				return std::nullopt;
			
			return std::make_optional<bool>((t1s->flags & ETHERNET10T1S_SETTINGS_FLAG_ENABLE_PLCA) != 0);
		}

		bool setT1SPLCAFor(Network net, bool enable) override {
			ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
			if(t1s == nullptr)
				return false;
			
			if(enable)
				t1s->flags |= ETHERNET10T1S_SETTINGS_FLAG_ENABLE_PLCA;
			else
				t1s->flags &= ~ETHERNET10T1S_SETTINGS_FLAG_ENABLE_PLCA;
			
			return true;
		}

		std::optional<uint8_t> getT1SLocalIDFor(Network net) const override {
			const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
			if(t1s == nullptr)
				return std::nullopt;
			
			return std::make_optional(t1s->local_id);
		}

		bool setT1SLocalIDFor(Network net, uint8_t id) override {
			ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
			if(t1s == nullptr)
				return false;
			
			t1s->local_id = id;
			return true;
		}

		std::optional<uint8_t> getT1SMaxNodesFor(Network net) const override {
			const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
			if(t1s == nullptr)
				return std::nullopt;
			
			return std::make_optional(t1s->max_num_nodes);
		}

		bool setT1SMaxNodesFor(Network net, uint8_t nodes) override {
			ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
			if(t1s == nullptr)
				return false;
			
			t1s->max_num_nodes = nodes;
			return true;
		}

		std::optional<uint8_t> getT1STxOppTimerFor(Network net) const override {
			const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
			if(t1s == nullptr)
				return std::nullopt;
			
			return std::make_optional(t1s->to_timer);
		}

		bool setT1STxOppTimerFor(Network net, uint8_t timer) override {
			ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
			if(t1s == nullptr)
				return false;
			
			t1s->to_timer = timer;
			return true;
		}

		std::optional<uint8_t> getT1SMaxBurstFor(Network net) const override {
			const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
			if(t1s == nullptr)
				return std::nullopt;
			
			return std::make_optional(t1s->max_burst_count);
		}

		bool setT1SMaxBurstFor(Network net, uint8_t burst) override {
			ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
			if(t1s == nullptr)
				return false;
			
			t1s->max_burst_count = burst;
			return true;
		}

		std::optional<uint8_t> getT1SBurstTimerFor(Network net) const override {
			const ETHERNET10T1S_SETTINGS* t1s = getT1SSettingsFor(net);
			if(t1s == nullptr)
				return std::nullopt;
			
			return std::make_optional(t1s->burst_timer);
		}

		bool setT1SBurstTimerFor(Network net, uint8_t timer) override {
			ETHERNET10T1S_SETTINGS* t1s = getMutableT1SSettingsFor(net);
			if(t1s == nullptr)
				return false;
			
			t1s->burst_timer = timer;
			return true;
		}

	private:
		const ETHERNET10T1S_SETTINGS* getT1SSettingsFor(Network net) const {
			auto cfg = getStructurePointer<radgigastar2_settings_t>();
			if(cfg == nullptr)
				return nullptr;
			
			switch(net.getNetID()) {
				case Network::NetID::AE_03: return &(cfg->t1s1);
				case Network::NetID::AE_04: return &(cfg->t1s2);
				case Network::NetID::AE_05: return &(cfg->t1s3);
				case Network::NetID::AE_06: return &(cfg->t1s4);
				case Network::NetID::AE_07: return &(cfg->t1s5);
				case Network::NetID::AE_08: return &(cfg->t1s6);
				case Network::NetID::AE_09: return &(cfg->t1s7);
				case Network::NetID::AE_10: return &(cfg->t1s8);
				default:
					report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
					return nullptr;
			}
		}

		ETHERNET10T1S_SETTINGS* getMutableT1SSettingsFor(Network net) {
			auto cfg = getMutableStructurePointer<radgigastar2_settings_t>();
			if(cfg == nullptr)
				return nullptr;
			
			switch(net.getNetID()) {
				case Network::NetID::AE_03: return &(cfg->t1s1);
				case Network::NetID::AE_04: return &(cfg->t1s2);
				case Network::NetID::AE_05: return &(cfg->t1s3);
				case Network::NetID::AE_06: return &(cfg->t1s4);
				case Network::NetID::AE_07: return &(cfg->t1s5);
				case Network::NetID::AE_08: return &(cfg->t1s6);
				case Network::NetID::AE_09: return &(cfg->t1s7);
				case Network::NetID::AE_10: return &(cfg->t1s8);
				default:
					report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
					return nullptr;
			}
		}

	protected:
		ICSNEO_UNALIGNED(const uint64_t *)
		getTerminationEnables() const override
		{
			auto cfg = getStructurePointer<radgigastar2_settings_t>();
			if (cfg == nullptr)
				return nullptr;
			return &cfg->termination_enables;
		}
	};

	typedef struct
	{
		uint8_t unused[3];
		uint8_t ethernetActivationLineEnabled;
	} radgigastar2_status_t;
}

#endif // __cplusplus

#endif