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
    icsneopy/communication/message/canerrormessage.cpp
    icsneopy/communication/message/ethernetmessage.cpp
    icsneopy/communication/message/linmessage.cpp
    icsneopy/communication/message/tc10statusmessage.cpp
    icsneopy/communication/message/mdiomessage.cpp
    icsneopy/communication/message/gptpstatusmessage.cpp
    icsneopy/communication/message/ethernetstatusmessage.cpp
    icsneopy/communication/message/callback/messagecallback.cpp
    icsneopy/communication/message/filter/messagefilter.cpp
    icsneopy/device/device.cpp
    icsneopy/icsneocpp.cpp
)
target_link_libraries(icsneopy PRIVATE icsneocpp)

install(TARGETS icsneopy LIBRARY DESTINATION icsneopy)

add_custom_command(TARGET icsneopy POST_BUILD COMMAND stubgen -m icsneopy -o .)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/icsneopy.pyi __init__.py py.typed DESTINATION icsneopy)
