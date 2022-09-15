#include "icsneo/device/devicefinder.h"
#include "icsneo/platform/devices.h"
#include "icsneo/device/founddevice.h"
#include "generated/extensions/builtin.h"

#ifdef ICSNEO_ENABLE_FIRMIO
#include "icsneo/platform/firmio.h"
#endif

#ifdef ICSNEO_ENABLE_RAW_ETHERNET
#include "icsneo/platform/pcap.h"
#endif

#ifdef ICSNEO_ENABLE_CDCACM
#include "icsneo/platform/cdcacm.h"
#endif

#ifdef ICSNEO_ENABLE_FTDI
#include "icsneo/platform/ftdi.h"
#endif

using namespace icsneo;

template<typename T>
static void makeIfSerialMatches(const FoundDevice& dev, std::vector<std::shared_ptr<Device>>& into) {
	// Relies on the subclass to have a `static constexpr const char* SERIAL_START = "XX"`
	// and also a public constructor `T(const FoundDevice& dev)`
	// Use macro ICSNEO_FINDABLE_DEVICE() to create these
	if(dev.serial[0] == T::SERIAL_START[0] && dev.serial[1] == T::SERIAL_START[1])
		into.push_back(std::make_shared<T>(dev));
}

template<typename T>
static void makeIfPIDMatches(const FoundDevice& dev, std::vector<std::shared_ptr<Device>>& into) {
	// Relies on the subclass to have a `static constexpr uint16_t PRODUCT_ID = 0x1111`
	// and also a public constructor `T(const FoundDevice& dev)`
	// Use macro ICSNEO_FINDABLE_DEVICE_BY_PID() to create these
	if(dev.productId == T::PRODUCT_ID)
		into.push_back(std::make_shared<T>(dev));
}

std::vector<std::shared_ptr<Device>> DeviceFinder::FindAll() {
	static std::vector<FoundDevice> newDriverFoundDevices;
	newDriverFoundDevices.clear();

	#ifdef ICSNEO_ENABLE_FIRMIO
	FirmIO::Find(newDriverFoundDevices);
	#endif

	#ifdef ICSNEO_ENABLE_RAW_ETHERNET
	PCAP::Find(newDriverFoundDevices);
	#endif

	#ifdef ICSNEO_ENABLE_CDCACM
	CDCACM::Find(newDriverFoundDevices);
	#endif

	#ifdef ICSNEO_ENABLE_FTDI
	FTDI::Find(newDriverFoundDevices);
	#endif

	// Weak because we don't want to keep devices open if they go out of scope elsewhere
	static std::vector<std::weak_ptr<Device>> foundDevices;

	// Remove Devices that have dropped out of scope or are no longer present
	for (auto it = foundDevices.begin(); it != foundDevices.end(); ) {
		if (const auto device = it->lock()) {
			if (std::none_of(newDriverFoundDevices.begin(), newDriverFoundDevices.end(),
					[&](const auto& driverDevice) {
						return std::string(driverDevice.serial) == device->getSerial();
					}
				)) {
				it = foundDevices.erase(it); // Device not found by drivers but pointer has a >0 use_count, error?
			} else {
				++it; // Valid weak pointer and device found by drivers
			}
		} else {
			it = foundDevices.erase(it); // Weak pointer has a zero use_count
		}
	}

	// Remove existing driver devices so we only create new ones
	for (auto it = newDriverFoundDevices.begin(); it != newDriverFoundDevices.end(); ) {
		if (std::any_of(foundDevices.begin(), foundDevices.end(),
				[&](const auto& weakDevice) {
					const auto device = weakDevice.lock();
					return device && std::string(it->serial) == device->getSerial();
				}
			)) {
			it = newDriverFoundDevices.erase(it);
		} else {
			++it;
		}
	}

	std::vector<std::shared_ptr<Device>> newFoundDevices;
	newFoundDevices.reserve(newDriverFoundDevices.size());
	// Offer found devices to each of the subclasses
	for (const FoundDevice& dev : newDriverFoundDevices) {
		#ifdef __ETHERBADGE_H_
		makeIfSerialMatches<EtherBADGE>(dev, newFoundDevices);
		#endif

		#ifdef __NEOOBD2PRO_H_
		makeIfSerialMatches<NeoOBD2PRO>(dev, newFoundDevices);
		#endif

		#ifdef __NEOOBD2SIM_H_
		makeIfSerialMatches<NeoOBD2SIM>(dev, newFoundDevices);
		#endif

		#ifdef __NEOVIFIRE_H_
		makeIfPIDMatches<NeoVIFIRE>(dev, newFoundDevices);
		#endif

		#ifdef __NEOVIFIRE2_H_
		makeIfSerialMatches<NeoVIFIRE2>(dev, newFoundDevices);
		#endif

		#ifdef __NEOVIRED2_H_
		makeIfSerialMatches<NeoVIRED2>(dev, newFoundDevices);
		#endif

		#ifdef __NEOVIION_H_
		makeIfPIDMatches<NeoVIION>(dev, newFoundDevices);
		#endif

		#ifdef __NEOVIPLASMA_H_
		makeIfPIDMatches<NeoVIPLASMA>(dev, newFoundDevices);
		#endif

		#ifdef __RADA2B_H_
		makeIfSerialMatches<RADA2B>(dev, newFoundDevices);
		#endif

		#ifdef __RADEPSILON_H_
		makeIfSerialMatches<RADEpsilon>(dev, newFoundDevices);
		#endif

		#ifdef __RADGALAXY_H_
		makeIfSerialMatches<RADGalaxy>(dev, newFoundDevices);
		#endif

		#ifdef __RADMARS_H_
		makeIfSerialMatches<RADMars>(dev, newFoundDevices);
		#endif

		#ifdef __RADGIGASTAR_H_
		makeIfSerialMatches<RADGigastar>(dev, newFoundDevices);
		#endif

		#ifdef __RADJUPITER_H_
		makeIfSerialMatches<RADJupiter>(dev, newFoundDevices);
		#endif

		#ifdef __RADMOON2_H_
		makeIfSerialMatches<RADMoon2>(dev, newFoundDevices);
		#endif

		#ifdef __RADMOONDUO_H_
		makeIfSerialMatches<RADMoonDuo>(dev, newFoundDevices);
		#endif

		#ifdef __RADPLUTO_H_
		makeIfSerialMatches<RADPluto>(dev, newFoundDevices);
		#endif

		#ifdef __RADSTAR2_H_
		makeIfSerialMatches<RADStar2>(dev, newFoundDevices);
		#endif

		#ifdef __RADSUPERMOON_H_
		makeIfSerialMatches<RADSupermoon>(dev, newFoundDevices);
		#endif

		#ifdef __VALUECAN3_H_
		makeIfPIDMatches<ValueCAN3>(dev, newFoundDevices);
		#endif

		#ifdef __VALUECAN4_1_H_
		makeIfSerialMatches<ValueCAN4_1>(dev, newFoundDevices);
		#endif

		#ifdef __VALUECAN4_2_H_
		makeIfSerialMatches<ValueCAN4_2>(dev, newFoundDevices);
		#endif

		#ifdef __VALUECAN4_2EL_H_
		makeIfSerialMatches<ValueCAN4_2EL>(dev, newFoundDevices);
		#endif

		#ifdef __VALUECAN4_4_H_
		makeIfSerialMatches<ValueCAN4_4>(dev, newFoundDevices);
		#endif

		#ifdef __VALUECAN4INDUSTRIAL_H_
		makeIfSerialMatches<ValueCAN4Industrial>(dev, newFoundDevices);
		#endif

		#ifdef __VIVIDCAN_H_
		makeIfSerialMatches<VividCAN>(dev, newFoundDevices);
		#endif
	}

	for(auto& device : newFoundDevices) {
		AddBuiltInExtensionsTo(device);
	}

	// Grab a weak pointer from the new devices
	foundDevices.insert(foundDevices.end(), newFoundDevices.begin(), newFoundDevices.end());

	// Upgrade to shared for the return
	return std::vector<std::shared_ptr<Device>>(foundDevices.begin(), foundDevices.end());
}

const std::vector<DeviceType>& DeviceFinder::GetSupportedDevices() {
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

		#ifdef __NEOVIRED2_H_
		NeoVIRED2::DEVICE_TYPE,
		#endif

		#ifdef __NEOVIFIRE_H_
		NeoVIFIRE::DEVICE_TYPE,
		#endif

		#ifdef __NEOVIFIRE2_H_
		NeoVIFIRE2::DEVICE_TYPE,
		#endif

		#ifdef __NEOVIION_H_
		NeoVIION::DEVICE_TYPE,
		#endif

		#ifdef __NEOVIPLASMA_H_
		NeoVIPLASMA::DEVICE_TYPE,
		#endif

		#ifdef __RADA2B_H_
		RADA2B::DEVICE_TYPE,
		#endif

		#ifdef __RADEPSILON_H_
		RADEpsilon::DEVICE_TYPE,
		#endif

		#ifdef __RADGALAXY_H_
		RADGalaxy::DEVICE_TYPE,
		#endif

		#ifdef __RADMARS_H_
		RADMars::DEVICE_TYPE,
		#endif

		#ifdef __RADGIGASTAR_H_
		RADGigastar::DEVICE_TYPE,
		#endif

		#ifdef __RADMOON2_H_
		RADMoon2::DEVICE_TYPE,
		#endif

		#ifdef __RADMOONDUO_H_
		RADMoonDuo::DEVICE_TYPE,
		#endif

		#ifdef __RADPLUTO_H_
		RADPluto::DEVICE_TYPE,
		#endif

		#ifdef __RADSTAR2_H_
		RADStar2::DEVICE_TYPE,
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

		#ifdef __VALUECAN4INDUSTRIAL_H_
		ValueCAN4Industrial::DEVICE_TYPE,
		#endif

		#ifdef __VIVIDCAN_H_
		VividCAN::DEVICE_TYPE,
		#endif

	};

	return supportedDevices;
}
