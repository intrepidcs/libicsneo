#ifndef __VALUECAN4_4_SETTINGS_H_
#define __VALUECAN4_4_SETTINGS_H_

#include "icsneo/device/idevicesettings.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4-4-2elsettings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4_4Settings : public ValueCAN4_4_2ELSettings {
public:
	ValueCAN4_4Settings(std::shared_ptr<Communication> com) : ValueCAN4_4_2ELSettings(com) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<valuecan4_4_2el_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::_icsneo_netid_t::icsneo_netid_hscan:
				return &(cfg->can1);
			case Network::_icsneo_netid_t::HSCAN2:
				return &(cfg->can2);
			case Network::_icsneo_netid_t::HSCAN3:
				return &(cfg->can3);
			case Network::_icsneo_netid_t::HSCAN4:
				return &(cfg->can4);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<valuecan4_4_2el_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::_icsneo_netid_t::icsneo_netid_hscan:
				return &(cfg->canfd1);
			case Network::_icsneo_netid_t::HSCAN2:
				return &(cfg->canfd2);
			case Network::_icsneo_netid_t::HSCAN3:
				return &(cfg->canfd3);
			case Network::_icsneo_netid_t::HSCAN4:
				return &(cfg->canfd4);
			default:
				return nullptr;
		}
	}

	virtual std::vector<TerminationGroup> getTerminationGroups() const override {
		return {
			{
				Network(Network::_icsneo_netid_t::icsneo_netid_hscan),
				Network(Network::_icsneo_netid_t::HSCAN3)
			},
			{
				Network(Network::_icsneo_netid_t::HSCAN2),
				Network(Network::_icsneo_netid_t::HSCAN4)
			}
		};
	}

	const LIN_SETTINGS* getLINSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<valuecan4_4_2el_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::_icsneo_netid_t::icsneo_netid_lin:
				return &(cfg->lin1);
			default:
				return nullptr;
		}
	}

protected:
	ICSNEO_UNALIGNED(const uint64_t*) getTerminationEnables() const override {
		auto cfg = getStructurePointer<valuecan4_4_2el_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		return &cfg->termination_enables;
	}
};

}

#endif // __cplusplus

#endif