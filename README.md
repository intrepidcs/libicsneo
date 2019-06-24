# libicsneo
### The Intrepid Control Systems Open Cross-Platform Device Communication API

An open source solution to integrate Intrepid Control Systems vehicle networking hardware with your application.

[Read the Full Documentation](https://libicsneo.readthedocs.io/)

## Getting Started
There are two major ways to write a new application using libicsneo. You can use the C++ interface, which will be compiled with your project and statically linked, or you can use the C interface, which can be either statically or dynamically linked.
### Integration with CMake (Static Linking)
Integrating the library with your current CMake project is extremely easy.
1. Checkout the library (or add as a submodule) into a subdirectory of your project.
2. Within your `CMakeLists.txt` you can add the line `add_subdirectory("third-party/libicsneo")` to bring in the libicsneo targets. Replace `third-party` with any subdirectory you choose.
3. The libicsneo library include paths should automatically be added to your include path.
4. Link the library with your target by adding `target_link_libraries(libicsneocpp-example icsneocpp)` after your target, substituting `libicsneocpp-example` with your target application.

You can now include either the C++ API with `#include <icsneo/icsneocpp.h>` or the C API with `#include <icsneo/icsneoc.h>`

### DLL / SO / DYLIB Releases (Dynamic Linking)
It is also possible to use the precompiled binaries with runtime linking. It is not recommended or supported to attempt to use the C++ interface with dynamic linking due to the complexities of C++ compilers.
1. Add this repository's `/include` to your include path
2. Add `#define ICSNEOC_DYNAMICLOAD` to the top of your source file
2. Add `#import <icsneo/icsneoc.h>` below that line
3. Call `icsneo_init();` to import the library before using any other libicsneo functions.
4. Use the library as normal.
5. Call `icsneo_close();` to unload the library.

## Usage
### Using the C++ API
The C++ API is designed to be modern and easy to use. All library functions and classes are in the namespace `icsneo`. Most applications will start by calling `icsneo::FindAllDevices()`. This will return an `std::vector` of `std::shared_ptr<icsneo::Device>` objects. You will want to keep a copy of the `shared_ptr` to any devices you want to use, as allowing it to go out of scope will automatically close the device and free all memory associated with it.

Any time you get bus traffic from the API, you will receive it as an `std::shared_ptr<icsneo::Message>`. The message will be valid as long as the `shared_ptr` stays in scope. Checking the type of the message allows you to cast it accordingly and access extra data for certain protocols. For instance, casting an `icsneo::Message` to an `icsneo::CANMessage` allows you to access the arbitration ID.

A barebones example is provided. For a more complete example, check [intrepidcs/libicsneo-examples](https://github.com/intrepidcs/libicsneo-examples).
``` c++
std::vector<std::shared_ptr<icsneo::Device>> devices = icsneo::FindAllDevices();
std::cout << devices.size() << " found!" << std::endl;
for(auto& device : devices)
    std::cout << "Found " << device->describe() << std::endl; // "Found neoVI FIRE 2 CY2345"
std::shared_ptr<icsneo::Device> myDevice = devices[0];
if(!myDevice->open()) {
    // There was an error while attempting to open the device, print the error details
    for(auto& error : icsneo::getErrors())
        std::cout << error << std::endl;
}
myDevice->goOnline(); // Start receiving messages
myDevice->enableMessagePolling(); // Allow the use of myDevice->getMessages() later
// Alternatively, assign a callback for new messages
std::this_thread::wait_for(std::chrono::seconds(5));
std::vector<std::shared_ptr<icsneo::Message>> messages = myDevice->getMessages();
std::cout << "We got " << messages.size() << " messages!" << std::endl;
for(auto& msg : messages) {
    switch(msg->network.getType()) {
        case icsneo::Network::Type::CAN:
        case icsneo::Network::Type::SWCAN:
        case icsneo::Network::Type::LSFTCAN: {
            // A message of type CAN is guaranteed to be a CANMessage, so we can static cast safely
            auto canmsg = std::static_pointer_cast<icsneo::CANMessage>(msg);
            // canmsg->arbid is valid here
            // canmsg->data is an std::vector<uint8_t>, you can check .size() for the DLC of the message
            // canmsg->timestamp is the time recorded by the hardware in nanoseconds since (1/1/2007 12:00:00 GMT)
        }
        default:
            // Handle others
    }
}
myDevice->close();
```

### Using the C API
The C API is designed to be a robust and fault tolerant interface which allows easy integration with other languages as well as existing C applications. When calling `icsneo_findAllDevices()` you will provide a buffer of `neodevice_t` structures, which will be written with the found devices. These `neodevice_t` structures can be uses to interface with the API from then on. Once you call `icsneo_close()` with a device, that device and all associated memory will be freed. You will need to run `icsneo_findAllDevices()` again to reconnect.

Messages are passed in the form of `neomessage_t` structures when calling `icsneo_getMessages()`. These structures contain a `uint8_t*` to the payload data, and this pointer will be valid until the next call to `icsneo_getMessages()` or the device is closed.

A barebones example is provided. For a more complete example, check [intrepidcs/libicsneo-examples](https://github.com/intrepidcs/libicsneo-examples).
``` c
size_t deviceCount = 10; // Pre-set to the size of your buffer before the icsneo_findAllDevices() call
neodevice_t devices[10];
icsneo_findAllDevices(devices, &deviceCount);
printf("We found %ull devices\n", deviceCount);
for(size_t i = 0; i < deviceCount; i++) {
    neodevice_t* myDevice = &devices[i];
    char desc[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION];
    size_t sz = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
    icsneo_describeDevice(myDevice, desc, &sz);
    printf("Found %s\n", desc); // "Found neoVI FIRE 2 CY2345"
}

neodevice_t* myDevice = &devices[0];
if(!icsneo_openDevice(myDevice)) {
    neoevent_t error;
    if(icsneo_getLastError(&error))
        printf("Error! %s\n", error.description);
}
icsneo_goOnline(myDevice); // Start receiving messages
icsneo_enableMessagePolling(myDevice); // Allow the use of icsneo_getMessages() later
sleep(5);
neomessage_t messages[50];
size_t messageCount = 50;
icsneo_getMessages(myDevice, messages, &messageCount, 0 /* non-blocking */);
printf("We got %ull messages!\n", messageCount);
for(size_t i = 0; i < messageCount; i++) {
    if(messages[i].type == ICSNEO_NETWORK_TYPE_CAN) {
        // A message of type CAN should be interperated a neomessage_can_t, so we can cast safely
        neomessage_can_t* canmsg = (neomessage_can_t*)&messages[i];
        // canmsg->arbid is valid here
        // canmsg->data is an uint8_t*, you can check canmsg->length for the length of the payload
        // canmsg->timestamp is the time recorded by the hardware in nanoseconds since (1/1/2007 12:00:00 GMT)
    }
}
icsneo_closeDevice(myDevice);
```

## Building from Source
### Windows
Building will require Microsoft Visual Studio 2017 and CMake to be installed.
### macOS
Getting the dependencies is easiest with the Homebrew package manager. You will also need XCode installed. You can then install CMake, an up-to-date version of GCC or Clang, and `libusb-1.0`.
### Linux
The dependencies are as follows
 - CMake 3.2 or above
 - GCC 4.7 or above, 4.8+ recommended
 - `libusb-1.0-0-dev`
 - `build-essential` is recommended
 
If you'd like to be able to run programs that use this library without being root, consider using the included udev rules

```
$ sudo cp 99-intrepidcs.rules /etc/udev/rules.d/
```
