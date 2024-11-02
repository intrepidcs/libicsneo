cmake_minimum_required(VERSION 3.20)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(icsneopy
    icsneopy/api/event.cpp
    icsneopy/api/eventcallback.cpp
    icsneopy/api/eventmanager.cpp
    icsneopy/api/version.cpp
    icsneopy/device/devicetype.cpp
    icsneopy/communication/network.cpp
    icsneopy/communication/message/message.cpp
    icsneopy/communication/message/canmessage.cpp
    icsneopy/communication/message/ethernetmessage.cpp
    icsneopy/communication/message/tc10statusmessage.cpp
    icsneopy/communication/message/callback/messagecallback.cpp
    icsneopy/communication/message/filter/messagefilter.cpp
    icsneopy/device/device.cpp
    icsneopy/icsneocpp.cpp
)
target_link_libraries(icsneopy PRIVATE icsneocpp)

install(TARGETS icsneopy LIBRARY DESTINATION .)
