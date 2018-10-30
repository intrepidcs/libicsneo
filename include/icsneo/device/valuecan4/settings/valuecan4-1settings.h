#ifndef __VALUECAN4_1_SETTINGS_H_
#define __VALUECAN4_1_SETTINGS_H_

#include "icsneo/device/valuecan4/settings/valuecan4-1-2settings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4_1Settings : public ValueCAN4_1_2Settings {
public:
	ValueCAN4_1Settings(std::shared_ptr<Communication> com) : ValueCAN4_1_2Settings(com) {}
	CAN_SETTINGS* getCANSettingsFor(Network net) override {
		auto cfg = getStructurePointer<valuecan4_1_2_settings_t>();
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->can1);
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif