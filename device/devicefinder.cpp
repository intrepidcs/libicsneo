#include "icsneo/device/devicefinder.h"
#include "icsneo/platform/devices.h"

using namespace icsneo;

static bool supportedDevicesCached = false;
static std::vector<DeviceType> supportedDevices = {

	#ifdef __NEOOBD2PRO_H_
	NeoOBD2PRO::DEVICE_TYPE,
	#endif

	#ifdef __NEOOBD2SIM_H_
	NeoOBD2SIM::DEVICE_TYPE,
	#endif

	#ifdef __NEOVIFIRE_H_
	NeoVIFIRE::DEVICE_TYPE,
	#endif

	#ifdef __NEOVIFIRE2ETH_H_
	NeoVIFIRE2ETH::DEVICE_TYPE,
	#endif

	#ifdef __NEOVIFIRE2USB_H_
	NeoVIFIRE2USB::DEVICE_TYPE,
	#endif

	#ifdef __NEOVIION_H_
	NeoVIION::DEVICE_TYPE,
	#endif

	#ifdef __NEOVIPLASMA_H_
	NeoVIPLASMA::DEVICE_TYPE,
	#endif

	#ifdef __RADGALAXY_H_
	RADGalaxy::DEVICE_TYPE,
	#endif

	#ifdef __RADSTAR2ETH_H_
	RADStar2ETH::DEVICE_TYPE,
	#endif

	#ifdef __RADSTAR2USB_H_
	RADStar2USB::DEVICE_TYPE,
	#endif

	#ifdef __RADSUPERMOON_H_
	RADSupermoon::DEVICE_TYPE,
	#endif

	#ifdef __VALUECAN3_H_
	ValueCAN3::DEVICE_TYPE,
	#endif

	#ifdef __VALUECAN4_1_H_
	ValueCAN4_1::DEVICE_TYPE,
	#endif

	#ifdef __VALUECAN4_2_H_
	ValueCAN4_2::DEVICE_TYPE,
	#endif

	#ifdef __VALUECAN4_2EL_H_
	ValueCAN4_2EL::DEVICE_TYPE,
	#endif

	#ifdef __VALUECAN4_4_H_
	ValueCAN4_4::DEVICE_TYPE,
	#endif

	#ifdef __VIVIDCAN_H_
	VividCAN::DEVICE_TYPE,
	#endif

};

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

	#ifdef __RADSTAR2ETH_H_
	findResults.push_back(RADStar2ETH::Find());
	#endif

	#ifdef __RADSTAR2USB_H_
	findResults.push_back(RADStar2USB::Find());
	#endif

	#ifdef __RADSUPERMOON_H_
	findResults.push_back(RADSupermoon::Find());
	#endif

	#ifdef __VALUECAN3_H_
	findResults.push_back(ValueCAN3::Find());
	#endif

	#ifdef __VALUECAN4_1_H_
	findResults.push_back(ValueCAN4_1::Find());
	#endif

	#ifdef __VALUECAN4_2_H_
	findResults.push_back(ValueCAN4_2::Find());
	#endif

	#ifdef __VALUECAN4_2EL_H_
	findResults.push_back(ValueCAN4_2EL::Find());
	#endif

	#ifdef __VALUECAN4_4_H_
	findResults.push_back(ValueCAN4_4::Find());
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

const std::vector<DeviceType>& DeviceFinder::GetSupportedDevices() {
	if(!supportedDevicesCached) {
		supportedDevices.erase(std::unique(supportedDevices.begin(), supportedDevices.end()), supportedDevices.end());
		supportedDevicesCached = true;
	}
	return supportedDevices;
}