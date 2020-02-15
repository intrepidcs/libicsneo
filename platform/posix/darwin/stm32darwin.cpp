#include "icsneo/platform/stm32.h"
#include <mutex>
#include <vector>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
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

std::vector<neodevice_t> STM32::FindByProduct(int product) {
	std::vector<neodevice_t> found;

	CFMutableDictionaryRef ref = IOServiceMatching(kIOSerialBSDServiceValue);
	if(ref == nullptr)
		return found;
	io_iterator_t matchingServices = 0;
	kern_return_t kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, ref, &matchingServices);
	if(KERN_SUCCESS != kernResult || matchingServices == 0)
		return found;
	IOReleaser matchingServicesReleaser(matchingServices);

	io_object_t serialPort;
	while((serialPort = IOIteratorNext(matchingServices))) {
		IOReleaser serialPortReleaser(serialPort);
		neodevice_t device;

		// First get the parent device
		// We want to check that it has the right VID/PID
		
		// Find the parent structure that describes the USB device providing this port
		io_object_t parent = 0;
		io_object_t current = serialPort;
		io_object_t usb = 0;
		// Need to release every parent we find in the chain, but we should defer that until later
		std::vector<IOReleaser> releasers;
		const std::string usbClass = "IOUSBDevice";
		while(IORegistryEntryGetParentEntry(current, kIOServicePlane, &parent) == KERN_SUCCESS) {
			releasers.emplace_back(parent);
			current = parent;
			io_name_t className;
			// io_name_t does not need to be freed, it's just a stack char[128]
			if(IOObjectGetClass(parent, className) != KERN_SUCCESS)
				continue;
			if(std::string(className) == usbClass) {
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
		uint16_t pid = 0;
		if(!CFNumberGetValue(static_cast<CFNumberRef>(productProp), kCFNumberSInt16Type, &pid))
			continue;
		if(pid != product)
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

		found.push_back(device);
	}

	return found;
}

std::string STM32::HandleToTTY(neodevice_handle_t handle) {
	std::lock_guard<std::mutex> lk(ttyTableMutex);
	const size_t index = size_t(handle - HANDLE_OFFSET);
	if(index >= ttyTable.size())
		return ""; // Not found, generic driver (stm32.cpp) will throw an error
	return ttyTable[index];
}