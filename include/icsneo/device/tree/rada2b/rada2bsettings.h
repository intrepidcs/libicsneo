#ifndef __RADA2BSETTINGS_H_
#define __RADA2BSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"
#include "icsneo/communication/message/a2bmessage.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)



typedef struct
{
	uint8_t tdmMode;
	uint8_t upstreamChannelOffset;
	uint8_t downstreamChannelOffset;
	uint8_t nodeType;
	/* 
	 * bit0: 16-bit channel width
	 */
	uint8_t flags;
	uint8_t reserved[15];
} rada2b_monitor_settings_t;

typedef struct {
	uint16_t perf_en;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t disableUsbCheckOnBoot : 1;
		uint16_t : 14;
	} flags;
	uint16_t network_enabled_on_boot;
	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;
	LIN_SETTINGS lin1;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;
	uint16_t iso_msg_termination_1;
	uint64_t network_enables;
	uint64_t termination_enables;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	RAD_REPORTING_SETTINGS reporting;
	DISK_SETTINGS disk;
	LOGGER_SETTINGS logger;
	int16_t iso15765_separation_time_offset;
	rada2b_monitor_settings_t a2b_monitor;
	rada2b_monitor_settings_t a2b_node;
	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;
	ETHERNET_SETTINGS2 ethernet;
	RAD_GPTP_SETTINGS gPTP;
	uint64_t network_enables_5;
} rada2b_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(rada2b_settings_t) == 340, "RAD-A2B settings size mismatch");

#include <iostream>

class RADA2BSettings : public IDeviceSettings {
public:

	enum class NodeType : uint8_t {
		Monitor = 0,
		Master = 1,
		Subnode = 2
	};

	enum class TDMMode : uint8_t {
		TDM2 = 0,
		TDM4 = 1,
		TDM8 = 2,
		TDM12 = 3,
		TDM16 = 4,
		TDM20 = 5,
		TDM24 = 6,
		TDM32 = 7,
	};

	enum class ChannelSize : uint8_t {
		chSize32 = 0,
		chSize16 = 1
	};

	enum class RADA2BDevice : uint8_t {
		Monitor,
		Node
	};

	RADA2BSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(rada2b_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<rada2b_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->can1);
			case Network::NetID::DWCAN_02:
				return &(cfg->can2);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<rada2b_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::DWCAN_01:
				return &(cfg->canfd1);
			case Network::NetID::DWCAN_02:
				return &(cfg->canfd2);
			default:
				return nullptr;
		}
	}

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<rada2b_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LIN_01:
				return &(cfg->lin1);
			default:
				return nullptr;
		}
	}

	TDMMode getTDMMode(RADA2BDevice device) const {
		auto cfg = getStructurePointer<rada2b_settings_t>();
		auto &deviceSettings = device == RADA2BDevice::Monitor ? cfg->a2b_monitor : cfg->a2b_node;

		return static_cast<TDMMode>(deviceSettings.tdmMode);
	}

	uint8_t getNumChannels(RADA2BDevice device) const {
		return tdmModeToChannelNum(getTDMMode(device));
	}

	ChannelSize getChannelSize(RADA2BDevice device) const {
		auto cfg = getStructurePointer<rada2b_settings_t>();
		auto &deviceSettings = device == RADA2BDevice::Monitor ? cfg->a2b_monitor : cfg->a2b_node;

		return static_cast<ChannelSize>(deviceSettings.flags & a2bSettingsFlag16bit);
	}

	uint8_t getChannelOffset(RADA2BDevice device, A2BMessage::Direction dir) const {
		auto cfg = getStructurePointer<rada2b_settings_t>();
		auto &deviceSettings = device == RADA2BDevice::Monitor ? cfg->a2b_monitor : cfg->a2b_node;

		if(dir == A2BMessage::Direction::Upstream) {
			return deviceSettings.upstreamChannelOffset;
		}

		return deviceSettings.downstreamChannelOffset;
	}

	NodeType getNodeType(RADA2BDevice device) const {
		auto cfg = getStructurePointer<rada2b_settings_t>();
		auto &deviceSettings = device == RADA2BDevice::Monitor ? cfg->a2b_monitor : cfg->a2b_node;

		return static_cast<NodeType>(deviceSettings.nodeType);		
	}

	void setNodeType(RADA2BDevice device, NodeType newType) {
		auto cfg = getMutableStructurePointer<rada2b_settings_t>();
		auto &deviceSettings = device == RADA2BDevice::Monitor ? cfg->a2b_monitor : cfg->a2b_node;

		deviceSettings.nodeType = static_cast<uint8_t>(newType);
	}

	void setTDMMode(RADA2BDevice device, TDMMode newMode) {
		auto cfg = getMutableStructurePointer<rada2b_settings_t>();
		auto &deviceSettings = device == RADA2BDevice::Monitor ? cfg->a2b_monitor : cfg->a2b_node;

		deviceSettings.tdmMode = static_cast<uint8_t>(newMode);
	}

	void setChannelOffset(RADA2BDevice device, A2BMessage::Direction dir, uint8_t newOffset) {
		auto cfg = getMutableStructurePointer<rada2b_settings_t>();
		auto &deviceSettings = device == RADA2BDevice::Monitor ? cfg->a2b_monitor : cfg->a2b_node;

		if(dir == A2BMessage::Direction::Upstream) {
			deviceSettings.upstreamChannelOffset = newOffset;
		}
		else {
			deviceSettings.downstreamChannelOffset = newOffset;
		}
	}

	void setChannelSize(RADA2BDevice device, ChannelSize newChannelSize) {
		auto cfg = getMutableStructurePointer<rada2b_settings_t>();
		auto &deviceSettings = device == RADA2BDevice::Monitor ? cfg->a2b_monitor : cfg->a2b_node;

		if(newChannelSize == ChannelSize::chSize16) {
			deviceSettings.flags |= a2bSettingsFlag16bit;
		}
		else {
			deviceSettings.flags &= ~a2bSettingsFlag16bit;
		}
	}

	static uint8_t tdmModeToChannelNum(TDMMode mode) {

		switch(mode) {
			case TDMMode::TDM2:
				return 4;
			case TDMMode::TDM4:
				return 8;
			case TDMMode::TDM8:
				return 16;
			case TDMMode::TDM12:
				return 24;
			case TDMMode::TDM16:
				return 32;
			case TDMMode::TDM20:
				return 40;
			case TDMMode::TDM24:
				return 48;
			case TDMMode::TDM32:
				return 64;
			default:
				break;
		}
		
		return 0;
	}

	static constexpr uint8_t a2bSettingsFlag16bit = 0x01;
};

}

#endif // __cplusplus

#endif