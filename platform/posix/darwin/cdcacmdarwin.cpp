#include "icsneo/platform/cdcacm.h"
#include "icsneo/device/founddevice.h"
#include <mutex>
#include <vector>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>

using namespace icsneo;

/* The index into the TTY table starts at zero, but we want to keep zero
 * for an undefined handle, so add a constant.
 */
static constexpr const neodevice_handle_t HANDLE_OFFSET = 10;

static std::mutex ttyTableMutex;
static std::vector<std::string> ttyTable;

static neodevice_handle_t TTYToHandle(const std::string& tty) {
	std::lock_guard<std::mutex> lk(ttyTableMutex);
	for(size_t i = 0; i < ttyTable.size(); i++) {
		if(ttyTable[i] == tty)
			return neodevice_handle_t(i + HANDLE_OFFSET);
	}
	ttyTable.push_back(tty);
	return neodevice_handle_t(ttyTable.size() - 1 + HANDLE_OFFSET);
}

static std::string CFStringToString(CFStringRef cfString) {
	if(cfString == nullptr)
		return std::string();
	// As an optimization, we can try to directly read the CFString's CStringPtr
	// CoreFoundation will seemingly not lift a finger if the pointer is not readily available
	// so it will often return nullptr and we'll have to get the string "the hard way"
	const char* cstr = CFStringGetCStringPtr(cfString, kCFStringEncodingUTF8);
	if(cstr != nullptr)
		return std::string(cstr);
	// "The hard way"
	// CFStringGetLength returns the length in UTF-16 Code Points
	// CFStringGetCString will convert to UTF-8 for us so long as we give it enough space
	const int len = CFStringGetLength(cfString);
	if(len <= 0)
		return std::string();
	std::vector<char> utf8data;
	utf8data.resize(len * 4 + 1); // UTF-16 => UTF-8, 1 code point can become up to 4 bytes, plus NUL
	if(!CFStringGetCString(cfString, utf8data.data(), utf8data.size(), kCFStringEncodingUTF8))
		return std::string();
	return std::string(utf8data.data());
}

class CFReleaser {
public:
	CFReleaser(CFTypeRef obj) : toRelease(obj) {}
	~CFReleaser() {
		if(toRelease != nullptr)
			CFRelease(toRelease);
	}
	CFReleaser(CFReleaser const&) = delete;
	CFReleaser& operator=(CFReleaser const&) = delete;

private:
	CFTypeRef toRelease;
};

class IOReleaser {
public:
	IOReleaser(io_object_t obj) : toRelease(obj) {}
	~IOReleaser() {
		if(toRelease != 0)
			IOObjectRelease(toRelease);
	}
	IOReleaser(IOReleaser&& moved) : toRelease(moved.toRelease) {
		moved.toRelease = 0;
	}
	IOReleaser(IOReleaser const&) = delete;
	IOReleaser& operator=(IOReleaser const&) = delete;

private:
	io_object_t toRelease;
};

void CDCACM::Find(std::vector<FoundDevice>& found) {
	CFMutableDictionaryRef ref = IOServiceMatching(kIOSerialBSDServiceValue);
	if(ref == nullptr)
		return;
	io_iterator_t matchingServices = 0;
	kern_return_t kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, ref, &matchingServices);
	if(KERN_SUCCESS != kernResult || matchingServices == 0)
		return;
	IOReleaser matchingServicesReleaser(matchingServices);

	io_object_t serialPort;
	while((serialPort = IOIteratorNext(matchingServices))) {
		IOReleaser serialPortReleaser(serialPort);

		// First get the parent device
		// We want to check that it has the right VID/PID
		
		// Find the parent structure that describes the USB device providing this port
		io_object_t parent = 0;
		io_object_t current = serialPort;
		io_object_t usb = 0;
		// Need to release every parent we find in the chain, but we should defer that until later
		std::vector<IOReleaser> releasers;
		while(IORegistryEntryGetParentEntry(current, kIOServicePlane, &parent) == KERN_SUCCESS) {
			releasers.emplace_back(parent);
			current = parent;
			// On old macOSes, IOUSBDevice is the type of the class we want
			// On newer macOSes, IOUSBDevice may further be subclassed as IOUSBHostDevice
			if(IOObjectConformsTo(parent, kIOUSBDeviceClassName)) {
				usb = parent;
				break;
			}
		}
		if(!usb) // Did not find a USB parent (maybe this is a Bluetooth modem or something)
			continue;

		// Get the VID
		CFTypeRef vendorProp = IORegistryEntryCreateCFProperty(usb, CFSTR("idVendor"), kCFAllocatorDefault, 0);
		if(vendorProp == nullptr)
			continue;
		CFReleaser vendorPropReleaser(vendorProp);
		if(CFGetTypeID(vendorProp) != CFNumberGetTypeID())
			continue;
		uint16_t vid = 0;
		if(!CFNumberGetValue(static_cast<CFNumberRef>(vendorProp), kCFNumberSInt16Type, &vid))
			continue;
		if(vid != INTREPID_USB_VENDOR_ID)
			continue;

		// Get the PID
		CFTypeRef productProp = IORegistryEntryCreateCFProperty(usb, CFSTR("idProduct"), kCFAllocatorDefault, 0);
		if(productProp == nullptr)
			continue;
		CFReleaser productPropReleaser(productProp);
		if(CFGetTypeID(productProp) != CFNumberGetTypeID())
			continue;

		// Read the PID directly into the FoundDevice structure
		FoundDevice device;
		if(!CFNumberGetValue(static_cast<CFNumberRef>(productProp), kCFNumberSInt16Type, &device.productId))
			continue;

		// Now, let's get the "call-out" device (/dev/cu.*)
		// We get the /dev/cu.* variant instead of the /dev/tty.* variant because the device will not assert DCD
		// Therefore, the open call on /dev/tty.* will hang, whereas /dev/cu.* will not
		// This `propertyValue` will need to be freed, that will happen in CFStringToString
		CFTypeRef calloutProp = IORegistryEntryCreateCFProperty(serialPort, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
		if(calloutProp == nullptr)
			continue;
		CFReleaser calloutPropReleaser(calloutProp);
		if(CFGetTypeID(calloutProp) != CFStringGetTypeID())
			continue;
		// We can static cast here because we have verified the type to be a CFString
		const std::string tty = CFStringToString(static_cast<CFStringRef>(calloutProp));
		if(tty.empty())
			continue;
		device.handle = TTYToHandle(tty);

		// Last but not least, get the serial number
		CFTypeRef serialProp = IORegistryEntryCreateCFProperty(usb, CFSTR("kUSBSerialNumberString"), kCFAllocatorDefault, 0);
		if(serialProp == nullptr)
			continue;
		CFReleaser serialPropReleaser(serialProp);
		if(CFGetTypeID(serialProp) != CFStringGetTypeID())
			continue;
		// We can static cast here because we have verified the type to be a CFString
		const std::string serial = CFStringToString(static_cast<CFStringRef>(serialProp));
		if(serial.empty())
			continue;
		device.serial[serial.copy(device.serial, sizeof(device.serial)-1)] = '\0';

		// Add a factory to make the driver
		device.makeDriver = [](const device_eventhandler_t& report, neodevice_t& device) {
			return std::unique_ptr<Driver>(new CDCACM(report, device));
		};

		found.push_back(device);
	}
}

std::string CDCACM::HandleToTTY(neodevice_handle_t handle) {
	std::lock_guard<std::mutex> lk(ttyTableMutex);
	const size_t index = size_t(handle - HANDLE_OFFSET);
	if(index >= ttyTable.size())
		return ""; // Not found, generic driver (cdcacm.cpp) will throw an error
	return ttyTable[index];
}