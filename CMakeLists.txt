cmake_minimum_required(VERSION 3.2)
project(icsneonext VERSION 0.1.0)

include(GNUInstallDirs)

include_directories(${CMAKE_SOURCE_DIR})

# Enable Warnings
if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
else() #if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-unknown-pragmas -Wno-zero-length-array -pedantic")
endif()

# libftdi
if(NOT WIN32)
	include_directories(${CMAKE_SOURCE_DIR} third-party/libftdi/src third-party/libftdi/ftdipp)
	add_subdirectory(third-party/libftdi)
endif(NOT WIN32)

if(WIN32)
	file(GLOB PLATFORM_SRC ${CMAKE_SOURCE_DIR}/platform/windows/*.cpp)
else()
	file(GLOB PLATFORM_SRC ${CMAKE_SOURCE_DIR}/platform/linux/*.cpp)
endif()

set(COMMON_SRC
	communication/messagedecoder.cpp
	communication/multichannelcommunication.cpp
	communication/communication.cpp
	communication/icommunication.cpp
	device/devicefinder.cpp
	device/device.cpp
)

set(SRC_FILES ${COMMON_SRC} ${PLATFORM_SRC})

add_library(icsneocpp
	api/icsneocpp/icsneocpp.cpp
	${SRC_FILES}
)

add_library(icsneoc SHARED
	api/icsneoc/icsneoc.cpp
	api/icsneocpp/icsneocpp.cpp
	${SRC_FILES}
)

# libftdi
if(NOT WIN32)
	find_package(Threads)
	set_property(TARGET ftdi1-static PROPERTY POSITION_INDEPENDENT_CODE ON)
	set_property(TARGET ftdipp1-static PROPERTY POSITION_INDEPENDENT_CODE ON)
	target_link_libraries(icsneocpp ftdipp1-static)
	target_link_libraries(icsneoc ftdipp1-static)
	target_link_libraries(icsneocpp ftdi1-static)
	target_link_libraries(icsneoc ftdi1-static)
	target_link_libraries(icsneocpp ${CMAKE_THREAD_LIBS_INIT})
	target_link_libraries(icsneoc ${CMAKE_THREAD_LIBS_INIT})
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
