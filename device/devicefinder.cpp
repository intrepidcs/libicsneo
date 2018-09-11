#include "device/include/devicefinder.h"
#include "device/neovifire/include/neovifire.h"
#include "device/neovifire2/include/neovifire2.h"
#include "device/plasion/include/neoviion.h"
#include "device/plasion/include/neoviplasma.h"
#include "device/radstar2/include/radstar2.h"
#include "device/radsupermoon/include/radsupermoon.h"
#include "device/valuecan3/include/valuecan3.h"
#include "device/valuecan4/include/valuecan4.h"
#include "device/vividcan/include/vividcan.h"

using namespace icsneo;

std::vector<std::shared_ptr<Device>> DeviceFinder::FindAll() {
	std::vector<std::shared_ptr<Device>> foundDevices;
	std::vector<std::vector<std::shared_ptr<Device>>> findResults;
	
	findResults.push_back(NeoVIFIRE::Find());
	findResults.push_back(NeoVIFIRE2::Find());
	findResults.push_back(NeoVIION::Find());
	findResults.push_back(NeoVIPLASMA::Find());
	findResults.push_back(RADStar2::Find());
	findResults.push_back(RADSupermoon::Find());
	findResults.push_back(ValueCAN3::Find());
	findResults.push_back(ValueCAN4::Find());
	findResults.push_back(VividCAN::Find());

	for(auto& results : findResults) {
		if(results.size())
			foundDevices.insert(foundDevices.end(), std::make_move_iterator(results.begin()), std::make_move_iterator(results.end()));
	}

	return foundDevices;
}