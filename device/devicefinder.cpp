#include "device/include/devicefinder.h"
#include "platform/include/devices.h"

using namespace icsneo;

std::vector<std::shared_ptr<Device>> DeviceFinder::FindAll() {
	std::vector<std::shared_ptr<Device>> foundDevices;
	std::vector<std::vector<std::shared_ptr<Device>>> findResults;
	
	#ifdef __NEOOBD2PRO_H_
	findResults.push_back(NeoOBD2PRO::Find());
	#endif

	#ifdef __NEOOBD2SIM_H_
	findResults.push_back(NeoOBD2SIM::Find());
	#endif

	#ifdef __NEOVIFIRE_H_
	findResults.push_back(NeoVIFIRE::Find());
	#endif

	#ifdef __NEOVIFIRE2ETH_H_
	findResults.push_back(NeoVIFIRE2ETH::Find());
	#endif

	#ifdef __NEOVIFIRE2USB_H_
	findResults.push_back(NeoVIFIRE2USB::Find());
	#endif

	#ifdef __NEOVIION_H_
	findResults.push_back(NeoVIION::Find());
	#endif

	#ifdef __NEOVIPLASMA_H_
	findResults.push_back(NeoVIPLASMA::Find());
	#endif

	#ifdef __RADGALAXY_H_
	findResults.push_back(RADGalaxy::Find());
	#endif

	#ifdef __RADSTAR2_H_
	findResults.push_back(RADStar2::Find());
	#endif

	#ifdef __RADSUPERMOON_H_
	findResults.push_back(RADSupermoon::Find());
	#endif

	#ifdef __VALUECAN3_H_
	findResults.push_back(ValueCAN3::Find());
	#endif

	#ifdef __VALUECAN4_H_
	findResults.push_back(ValueCAN4::Find());
	#endif

	#ifdef __VIVIDCAN_H_
	findResults.push_back(VividCAN::Find());
	#endif

	for(auto& results : findResults) {
		if(results.size())
			foundDevices.insert(foundDevices.end(), std::make_move_iterator(results.begin()), std::make_move_iterator(results.end()));
	}

	return foundDevices;
}