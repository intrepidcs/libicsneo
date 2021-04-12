#ifndef __VALUECAN4_2EL_SETTINGS_H_
#define __VALUECAN4_2EL_SETTINGS_H_

#include "icsneo/device/idevicesettings.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4-4-2elsettings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4_2ELSettings : public ValueCAN4_4_2ELSettings {
public:
	ValueCAN4_2ELSettings(std::shared_ptr<Communication> com) : ValueCAN4_4_2ELSettings(com) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<valuecan4_4_2el_settings_t>();
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
		auto cfg = getStructurePointer<valuecan4_4_2el_settings_t>();
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
	const uint64_t* getTerminationEnables() const override {
		auto cfg = getStructurePointer<valuecan4_4_2el_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		return &cfg->termination_enables;
	}
};

}

#endif // __cplusplus

#endif