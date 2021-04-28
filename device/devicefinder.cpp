#include "icsneo/device/devicefinder.h"
#include "icsneo/platform/devices.h"
#include "generated/extensions/builtin.h"

using namespace icsneo;

static bool supportedDevicesCached = false;
static std::vector<DeviceType> supportedDevices = {

	#ifdef __ETHERBADGE_H_
	EtherBADGE::DEVICE_TYPE,
	#endif

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

	#ifdef __RADGIGALOG_ETH_H_
	RADGigalogETH::DEVICE_TYPE,
	#endif

	#ifdef __RADGIGALOG_USB_H_
	RADGigalogUSB::DEVICE_TYPE,
	#endif

	#ifdef __RADGIGASTAR_ETH_H_
	RADGigastarETH::DEVICE_TYPE,
	#endif

	#ifdef __RADGIGASTAR_USB_H_
	RADGigastarUSB::DEVICE_TYPE,
	#endif

	#ifdef __RADMOON2_H_
	RADMoon2::DEVICE_TYPE,
	#endif

	#ifdef __RADMOONDUO_H_
	RADMoonDuo::DEVICE_TYPE,
	#endif

	#ifdef __RADPLUTOUSB_H_
	RADPlutoUSB::DEVICE_TYPE,
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

	#ifdef __VALUECAN4_2EL_ETH_H_
	ValueCAN4_2EL_ETH::DEVICE_TYPE,
	#endif

	#ifdef __VALUECAN4_2EL_USB_H_
	ValueCAN4_2EL_USB::DEVICE_TYPE,
	#endif

	#ifdef __VALUECAN4_4_H_
	ValueCAN4_4::DEVICE_TYPE,
	#endif

	#ifdef __VALUECAN4INDUSTRIAL_ETH_H_
	ValueCAN4IndustrialETH::DEVICE_TYPE,
	#endif

	#ifdef __VALUECAN4INDUSTRIAL_USB_H_
	ValueCAN4IndustrialUSB::DEVICE_TYPE,
	#endif

	#ifdef __VIVIDCAN_H_
	VividCAN::DEVICE_TYPE,
	#endif

};

std::vector<std::shared_ptr<Device>> DeviceFinder::FindAll() {
	std::vector<std::shared_ptr<Device>> foundDevices;
	std::vector<std::vector<std::shared_ptr<Device>>> findResults;

#if defined(LIBICSNEO_HAVE_PCAP) && LIBICSNEO_HAVE_PCAP == 1
	auto pcapDevices = PCAP::FindAll();
#endif

	#ifdef __ETHERBADGE_H_
	findResults.push_back(EtherBADGE::Find());
	#endif
	
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
	findResults.push_back(NeoVIFIRE2ETH::Find(pcapDevices));
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
	findResults.push_back(RADGalaxy::Find(pcapDevices));
	#endif

	#ifdef __RADGIGALOG_ETH_H_
	findResults.push_back(RADGigalogETH::Find(pcapDevices));
	#endif

	#ifdef __RADGIGALOG_USB_H_
	findResults.push_back(RADGigalogUSB::Find());
	#endif

	#ifdef __RADGIGASTAR_ETH_H_
	findResults.push_back(RADGigastarETH::Find(pcapDevices));
	#endif

	#ifdef __RADGIGASTAR_USB_H_
	findResults.push_back(RADGigastarUSB::Find());
	#endif

	#ifdef __RADMOON2_H_
	findResults.push_back(RADMoon2::Find());
	#endif

	#ifdef __RADMOONDUO_H_
	findResults.push_back(RADMoonDuo::Find());
	#endif

	#ifdef __RADPLUTOUSB_H_
	findResults.push_back(RADPlutoUSB::Find());
	#endif

	#ifdef __RADSTAR2ETH_H_
	findResults.push_back(RADStar2ETH::Find(pcapDevices));
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

	#ifdef __VALUECAN4_2EL_ETH_H_
	findResults.push_back(ValueCAN4_2EL_ETH::Find(pcapDevices));
	#endif

	#ifdef __VALUECAN4_2EL_USB_H_
	findResults.push_back(ValueCAN4_2EL_USB::Find());
	#endif

	#ifdef __VALUECAN4_4_H_
	findResults.push_back(ValueCAN4_4::Find());
	#endif

	#ifdef __VALUECAN4INDUSTRIAL_ETH_H_
	findResults.push_back(ValueCAN4IndustrialETH::Find(pcapDevices));
	#endif

	#ifdef __VALUECAN4INDUSTRIAL_USB_H_
	findResults.push_back(ValueCAN4IndustrialUSB::Find());
	#endif

	#ifdef __VIVIDCAN_H_
	findResults.push_back(VividCAN::Find());
	#endif

	for(auto& results : findResults) {
		if(results.size())
			foundDevices.insert(foundDevices.end(), std::make_move_iterator(results.begin()), std::make_move_iterator(results.end()));
	}

	for(auto& device : foundDevices) {
		AddBuiltInExtensionsTo(device);
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