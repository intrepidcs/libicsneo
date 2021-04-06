#ifndef __VALUECAN4_4_2EL_SETTINGS_H_
#define __VALUECAN4_4_2EL_SETTINGS_H_

#include "icsneo/device/idevicesettings.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4settings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4_4_2ELSettings : public IDeviceSettings {
public:
	ValueCAN4_4_2ELSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(valuecan4_4_2el_settings_t)) {}
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
};

}

#endif // __cplusplus

#endif