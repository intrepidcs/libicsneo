#ifndef __VALUECAN4_INDUSTRIAL_SETTINGS_H_
#define __VALUECAN4_INDUSTRIAL_SETTINGS_H_

#include "icsneo/device/idevicesettings.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4settings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4IndustrialSettings : public IDeviceSettings {
public:
	ValueCAN4IndustrialSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(valuecan4_industrial_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
                auto cfg = getStructurePointer<valuecan4_industrial_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->can1);
			case Network::NetID::HSCAN2:
				return &(cfg->can2);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
                auto cfg = getStructurePointer<valuecan4_industrial_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->canfd1);
			case Network::NetID::HSCAN2:
				return &(cfg->canfd2);
			default:
				return nullptr;
		}
	}

	virtual std::vector<TerminationGroup> getTerminationGroups() const override {
		return {
			{ Network(Network::NetID::HSCAN) },
			{ Network(Network::NetID::HSCAN2) }
		};
	}

protected:
	ICSNEO_UNALIGNED(const uint64_t*) getTerminationEnables() const override {
		auto cfg = getStructurePointer<valuecan4_industrial_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		return &cfg->termination_enables;
	}
};

}

#endif // __cplusplus

#endif
