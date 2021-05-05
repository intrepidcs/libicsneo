#ifndef __VALUECAN4_2_SETTINGS_H_
#define __VALUECAN4_2_SETTINGS_H_

#include "icsneo/device/tree/valuecan4/settings/valuecan4-1-2settings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4_2Settings : public ValueCAN4_1_2Settings {
public:
	ValueCAN4_2Settings(std::shared_ptr<Communication> com) : ValueCAN4_1_2Settings(com) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<valuecan4_1_2_settings_t>();
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
		auto cfg = getStructurePointer<valuecan4_1_2_settings_t>();
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
		auto cfg = getStructurePointer<valuecan4_1_2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		return &cfg->termination_enables;
	}
};

}

#endif // __cplusplus

#endif