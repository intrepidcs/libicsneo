#ifndef __VIVIDCANSETTINGS_H_
#define __VIVIDCANSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint32_t ecu_id;

	CAN_SETTINGS can1;// 12 bytes
	SWCAN_SETTINGS swcan1;// 14 bytes
	CAN_SETTINGS lsftcan1;// 12 bytes

	uint16_t network_enables;
	uint16_t network_enabled_on_boot;

	uint16_t iso15765_separation_time_offset;

	uint16_t perf_en;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t can_switch_mode;
	uint16_t termination_enables;

	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t reserved : 30;
	} flags;
} vividcan_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class VividCANSettings : public IDeviceSettings {
public:
	VividCANSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(vividcan_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<vividcan_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->can1);
			case Network::NetID::LSFTCAN:
				return &(cfg->lsftcan1);
			default:
				return nullptr;
		}
	}
	const CAN_SETTINGS* getLSFTCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<vividcan_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::LSFTCAN:
				return &(cfg->lsftcan1);
			default:
				return nullptr;
		}
	}
	const SWCAN_SETTINGS* getSWCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<vividcan_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::SWCAN:
				return &(cfg->swcan1);
			default:
				return nullptr;
		}
	}

	virtual std::vector<TerminationGroup> getTerminationGroups() const override {
		return {
			{ Network(Network::NetID::HSCAN) }
		};
	}

	bool refresh(bool ignoreChecksum = false) override {
		// Because VividCAN uses a nonstandard 16-bit termination_enables
		// we need to keep the standard 64-bit values in memory and update
		// the structure when applying
		if(!IDeviceSettings::refresh(ignoreChecksum))
			return false;
		auto cfg = getStructurePointer<vividcan_settings_t>();
		if(cfg == nullptr)
			return false;
		activeTerminationEnables = queuedTerminationEnables = cfg->termination_enables;
		return true;
	}

	bool apply(bool permanent = true) override {
		auto cfg = getMutableStructurePointer<vividcan_settings_t>();
		if(cfg)
			cfg->termination_enables = uint16_t(queuedTerminationEnables & 0xFFFF);

		const bool success = IDeviceSettings::apply(permanent);
		if(success)
			activeTerminationEnables = cfg->termination_enables;
		return success;
	}

protected:
	ICSNEO_UNALIGNED(const uint64_t*) getTerminationEnables() const override {
		// Check the structure pointer even though we're not using it so
		// all of the other checks that go along with it are performed
		if(getStructurePointer<vividcan_settings_t>() == nullptr)
			return nullptr;
		return &activeTerminationEnables;
	}

	ICSNEO_UNALIGNED(uint64_t*) getMutableTerminationEnables() override {
		if(getMutableStructurePointer<vividcan_settings_t>() == nullptr)
			return nullptr;
		return &queuedTerminationEnables;
	}

private:
	uint64_t queuedTerminationEnables = 0;
	uint64_t activeTerminationEnables = 0;
};

}

#endif // __cplusplus

#endif