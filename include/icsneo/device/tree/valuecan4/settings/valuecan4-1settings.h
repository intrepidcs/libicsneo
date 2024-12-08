#ifndef __VALUECAN4_1_SETTINGS_H_
#define __VALUECAN4_1_SETTINGS_H_

#include "icsneo/device/tree/valuecan4/settings/valuecan4-1-2settings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4_1Settings : public ValueCAN4_1_2Settings {
public:
	ValueCAN4_1Settings(std::shared_ptr<Communication> com) : ValueCAN4_1_2Settings(com) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<valuecan4_1_2_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case _icsneo_netid_t::icsneo_netid_hscan:
				return &(cfg->can1);
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif