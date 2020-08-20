find_package(PkgConfig)
pkg_check_modules(LibUSB QUIET libusb)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUSB
    REQUIRED_VARS
        LibUSB_INCLUDE_DIRS
        LibUSB_LIBRARIES
)

if(LibUSB_FOUND AND NOT TARGET LibUSB::LibUSB)
    add_library(LibUSB::LibUSB INTERFACE IMPORTED)
    set_target_properties(LibUSB::LibUSB PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LibUSB_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${LibUSB_LIBRARIES}"
    )
endif()
