#ifndef __VALUECAN4_2_SETTINGS_H_
#define __VALUECAN4_2_SETTINGS_H_

#include "device/valuecan4/settings/include/valuecan4-1-2settings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4_2Settings : public ValueCAN4_1_2Settings {
public:
	ValueCAN4_2Settings(std::shared_ptr<Communication> com) : ValueCAN4_1_2Settings(com) {}
	CAN_SETTINGS* getCANSettingsFor(Network net) override {
		auto cfg = getStructurePointer<valuecan4_1_2_settings_t>();
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->can1);
			case Network::NetID::HSCAN2:
				return &(cfg->can2);
			default:
				return nullptr;
		}
	}
	// CANFD_SETTINGS* getCANFDSettingsFor(Network net) override { return nullptr; }
};

}

#endif // __cplusplus

#endif