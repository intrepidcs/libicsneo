cmake_minimum_required(VERSION 3.20)

set(PYBIND11_FINDPYTHON ON)

if(PYBIND11_ROOT)
    find_package(pybind11 CONFIG REQUIRED)
else()
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v3.0.1
    )
    FetchContent_MakeAvailable(pybind11)
endif()

pybind11_add_module(icsneopy
    icsneopy/api/event.cpp
    icsneopy/api/eventcallback.cpp
    icsneopy/api/eventmanager.cpp
    icsneopy/api/version.cpp
    icsneopy/device/devicetype.cpp
    icsneopy/communication/network.cpp
    icsneopy/communication/io.cpp
    icsneopy/communication/livedata.cpp
    icsneopy/communication/message/message.cpp
    icsneopy/communication/message/canmessage.cpp
    icsneopy/communication/message/canerrormessage.cpp
    icsneopy/communication/message/ethernetmessage.cpp
    icsneopy/communication/message/linmessage.cpp
    icsneopy/communication/message/tc10statusmessage.cpp
    icsneopy/communication/message/mdiomessage.cpp
    icsneopy/communication/message/gptpstatusmessage.cpp
    icsneopy/communication/message/ethernetstatusmessage.cpp
    icsneopy/communication/message/spimessage.cpp
    icsneopy/communication/message/scriptstatusmessage.cpp
    icsneopy/communication/message/ethphymessage.cpp
    icsneopy/communication/message/livedatamessage.cpp
    icsneopy/communication/message/callback/messagecallback.cpp
    icsneopy/communication/message/filter/messagefilter.cpp
    icsneopy/core/macseccfg.cpp
    icsneopy/flexray/flexray.cpp
    icsneopy/disk/diskdriver.cpp
    icsneopy/device/chipid.cpp
    icsneopy/device/versionreport.cpp
    icsneopy/device/device.cpp
    icsneopy/device/extensions/deviceextension.cpp
    icsneopy/device/idevicesettings.cpp
    icsneopy/icsneocpp.cpp
)
target_link_libraries(icsneopy PRIVATE icsneocpp)

install(TARGETS icsneopy LIBRARY DESTINATION icsneopy)

find_program(STUBGEN_EXE stubgen)
if(STUBGEN_EXE)
    add_custom_command(TARGET icsneopy POST_BUILD COMMAND stubgen -v -p icsneopy -o .)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/icsneopy.pyi py.typed DESTINATION icsneopy)
endif()

install(FILES __init__.py DESTINATION icsneopy)
