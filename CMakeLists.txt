cmake_minimum_required(VERSION 3.2)
project(libicsneo VERSION 0.2.0)

option(LIBICSNEO_BUILD_TESTS "Build all tests." OFF)
option(LIBICSNEO_BUILD_DOCS "Build documentation. Don't use in Visual Studio." OFF)
option(LIBICSNEO_BUILD_EXAMPLES "Build examples." ON)
option(LIBICSNEO_BUILD_ICSNEOC "Build dynamic C library" ON)
option(LIBICSNEO_BUILD_ICSNEOC_STATIC "Build static C library" ON)
option(LIBICSNEO_BUILD_ICSNEOLEGACY "Build icsnVC40 compatibility library" ON)
set(LIBICSNEO_NPCAP_INCLUDE_DIR "" CACHE STRING "Npcap include directory; set to build with Npcap")

set(CMAKE_CXX_STANDARD_REQUIRED 11)

include(GNUInstallDirs)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Enable Warnings
if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
else() #if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-switch -Wno-unknown-pragmas")
endif()

# doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
	set(ICSNEO_DOCS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/docs)
	if(NOT EXISTS "${DOXYGEN_OUT}")
		set(DOXYGEN_FOUND FALSE)
	endif()
endif()

if(LIBICSNEO_BUILD_DOCS)
	if(DOXYGEN_FOUND)
		message("Will build Doxygen based documentation")

		add_custom_target(libicsneo_doxygen
			COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMENT "Generating API documentation with Doxygen"
			VERBATIM
			DEPENDS icsneocpp icsneoc icsneolegacy)

		# sphinx
		find_package(Sphinx)
		if(SPHINX_EXECUTABLE)
			message("Will build Sphinx based documentation")

			set(SPHINX_OUT ${ICSNEO_DOCS_DIR}/build/sphinx)

			# configured documentation tools and intermediate build results
			set(SPHINX_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/_build)

			# Sphinx cache with pickled ReST documents
			set(SPHINX_CACHE_DIR ${CMAKE_CURRENT_BINARY_DIR}/_doctrees)

			# HTML output directory
			set(SPHINX_HTML_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc_sphinx)

			configure_file(
				"${ICSNEO_DOCS_DIR}/conf.py.template"
				"${ICSNEO_DOCS_DIR}/conf.py"
				@ONLY)

			add_custom_target(libicsneo_sphinx ALL
				${SPHINX_EXECUTABLE}
					-q -b html
					-c "${ICSNEO_DOCS_DIR}"
					-d "${SPHINX_CACHE_DIR}"
					"${ICSNEO_DOCS_DIR}"
					"${SPHINX_HTML_DIR}"
				COMMENT "Building HTML documentation with Sphinx"
				DEPENDS icsneocpp icsneoc icsneolegacy)
		endif()
	endif()
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(PLATFORM_SRC
		platform/windows/pcap.cpp
		platform/windows/registry.cpp
		platform/windows/vcp.cpp
		platform/windows/internal/pcapdll.cpp
	)
else() # Darwin or Linux
	set(PLATFORM_SRC
		platform/posix/ftdi.cpp
		platform/posix/pcap.cpp
		platform/posix/cdcacm.cpp
	)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		list(APPEND PLATFORM_SRC
			platform/posix/darwin/cdcacmdarwin.cpp
		)
	else() # Linux or other
		list(APPEND PLATFORM_SRC
			platform/posix/linux/cdcacmlinux.cpp
		)
		if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
			message(WARNING
				"There is no platform port defined for ${CMAKE_SYSTEM_NAME}!\n"
				"The Linux platform code will be used, as it will generally allow building, but some devices may not enumerate properly."
			)
		endif()
	endif()
endif()

if(LIBICSNEO_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

# Extensions
set(LIBICSNEO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
foreach(EXT_PATH ${LIBICSNEO_EXTENSION_DIRS})
	get_filename_component(EXT_DIR ${EXT_PATH} NAME)
	message("Adding extension " ${EXT_DIR})
	add_subdirectory(${EXT_PATH} ${CMAKE_CURRENT_BINARY_DIR}/${EXT_DIR})
endforeach()

set(SRC_FILES
	communication/message/flexray/control/flexraycontrolmessage.cpp
	communication/message/neomessage.cpp
	communication/packet/flexraypacket.cpp
	communication/packet/canpacket.cpp
	communication/packet/ethernetpacket.cpp
	communication/packet/versionpacket.cpp
	communication/packet/iso9141packet.cpp
	communication/decoder.cpp
	communication/encoder.cpp
	communication/ethernetpacketizer.cpp
	communication/packetizer.cpp
	communication/multichannelcommunication.cpp
	communication/communication.cpp
	communication/driver.cpp
	device/extensions/flexray/extension.cpp
	device/extensions/flexray/controller.cpp
	device/idevicesettings.cpp
	device/devicefinder.cpp
	device/device.cpp
	${PLATFORM_SRC}
)

# Generate build info header
execute_process(
	COMMAND git rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_BRANCH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND git describe --abbrev=6 --dirty --always --tags
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_DESCRIBE
	ERROR_VARIABLE GIT_DESCRIBE
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(${BUILD_METADATA})
	set(BUILD_METADATA_PLUS +${BUILD_METADATA})
endif()
if(NOT ${GIT_BRANCH} STREQUAL "master")
	set(BUILD_GIT_INFO "${GIT_BRANCH} @ ")
endif()
string(SUBSTRING GIT_DESCRIBE 0 1 GIT_DESCRIBE_FIRST)
if(NOT ${GIT_DESCRIBE_FIRST} STREQUAL "v")
	string(APPEND BUILD_GIT_INFO "${GIT_DESCRIBE}")
endif()

configure_file(api/icsneocpp/buildinfo.h.template ${CMAKE_CURRENT_BINARY_DIR}/generated/buildinfo.h)
configure_file(api/icsneoc/version.rc.template ${CMAKE_CURRENT_BINARY_DIR}/generated/icsneoc/version.rc)

foreach(EXTINC ${LIBICSNEO_EXTENSION_INCLUDES})
	message("Including " ${EXTINC})
	list(APPEND LIBICSNEO_EXT_CODE_INCS_LIST "#include \"${EXTINC}\"")
endforeach()
list(JOIN LIBICSNEO_EXT_CODE_INCS_LIST "\n" LIBICSNEO_EXT_CODE_INCS)
foreach(EXTCLASS ${LIBICSNEO_EXTENSION_CLASSES})
	list(APPEND LIBICSNEO_EXT_CODE_LIST "device->addExtension(std::make_shared<${EXTCLASS}>(*device))\;")
endforeach()
list(JOIN LIBICSNEO_EXT_CODE_LIST "\n\t" LIBICSNEO_EXT_CODE)
configure_file(include/icsneo/device/extensions/builtin.h.template ${CMAKE_CURRENT_BINARY_DIR}/generated/extensions/builtin.h)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

add_library(icsneocpp
	api/icsneocpp/icsneocpp.cpp
	api/icsneocpp/event.cpp
	api/icsneocpp/eventmanager.cpp
	api/icsneocpp/version.cpp
	${SRC_FILES}
)
message("Include paths " ${LIBICSNEO_EXTENSION_INCLUDE_PATHS})
target_include_directories(icsneocpp
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${LIBICSNEO_EXTENSION_INCLUDE_PATHS}
)
set_property(TARGET icsneocpp PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_features(icsneocpp PUBLIC cxx_auto_type cxx_constexpr cxx_lambdas cxx_nullptr cxx_range_for cxx_rvalue_references cxx_sizeof_member cxx_strong_enums)
message("Loaded extensions: " ${LIBICSNEO_EXTENSION_TARGETS})
target_link_libraries(icsneocpp PUBLIC ${LIBICSNEO_EXTENSION_TARGETS})

# libftdi
if(NOT WIN32)
	target_include_directories(icsneocpp PUBLIC third-party/libftdi/src)
	set(LIBFTDI_DOCUMENTATION OFF CACHE INTERNAL "")
	set(LIBFTDI_BUILD_TESTS OFF CACHE INTERNAL "")
	set(LIBFTDI_INSTALL OFF CACHE INTERNAL "")
	set(LIBFTDI_PYTHON_BINDINGS OFF CACHE INTERNAL "")
	set(LIBFTDI_LINK_PYTHON_LIBRARY OFF CACHE INTERNAL "")
	set(FTDIPP OFF CACHE INTERNAL "")
	set(FTDI_EEPROM OFF CACHE INTERNAL "")
	add_subdirectory(third-party/libftdi)
	include_directories(${LIBUSB_INCLUDE_DIR})
endif(NOT WIN32)

# winpcap
if(WIN32)
	if(LIBICSNEO_NPCAP_INCLUDE_DIR STREQUAL "")
		target_include_directories(icsneocpp PUBLIC AFTER third-party/winpcap/include)
		add_definitions(-DWPCAP -DHAVE_REMOTE -DWIN32_LEAN_AND_MEAN)
	else()
		target_include_directories(icsneocpp PUBLIC AFTER ${LIBICSNEO_NPCAP_INCLUDE_DIR})
		add_definitions(-DNPCAP -DWIN32_LEAN_AND_MEAN)
	endif()
endif(WIN32)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_link_libraries(icsneocpp PUBLIC "-framework CoreFoundation" "-framework IOKit")
endif()

if(LIBICSNEO_BUILD_ICSNEOC)
	add_library(icsneoc SHARED api/icsneoc/icsneoc.cpp ${CMAKE_CURRENT_BINARY_DIR}/generated/icsneoc/version.rc)
	target_include_directories(icsneoc
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:>
		PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	target_link_libraries(icsneoc PRIVATE icsneocpp)
	target_compile_features(icsneoc PRIVATE cxx_auto_type cxx_constexpr cxx_lambdas cxx_nullptr cxx_range_for cxx_rvalue_references cxx_sizeof_member cxx_strong_enums)
endif()

if(LIBICSNEO_BUILD_ICSNEOC_STATIC)
	add_library(icsneoc-static STATIC api/icsneoc/icsneoc.cpp)
	target_include_directories(icsneoc-static
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:>
		PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	target_link_libraries(icsneoc-static PUBLIC icsneocpp)
	target_compile_features(icsneoc-static PUBLIC cxx_auto_type cxx_constexpr cxx_lambdas cxx_nullptr cxx_range_for cxx_rvalue_references cxx_sizeof_member cxx_strong_enums)
endif()

if(LIBICSNEO_BUILD_ICSNEOLEGACY)
	add_library(icsneolegacy SHARED
		api/icsneolegacy/icsneolegacy.cpp
		api/icsneolegacy/icsneolegacyextra.cpp
		api/icsneoc/icsneoc.cpp
		platform/windows/icsneolegacy.def
	)
	target_include_directories(icsneolegacy
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:>
		PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	target_link_libraries(icsneolegacy PRIVATE icsneocpp)
	target_compile_features(icsneolegacy PRIVATE cxx_auto_type cxx_constexpr cxx_lambdas cxx_nullptr cxx_range_for cxx_rvalue_references cxx_sizeof_member cxx_strong_enums)
endif()

# libftdi
if(NOT WIN32)
	find_package(Threads)
	set_property(TARGET ftdi1-static PROPERTY POSITION_INDEPENDENT_CODE ON)
	target_link_libraries(icsneocpp PUBLIC ftdi1-static)
	target_link_libraries(icsneocpp PUBLIC ${CMAKE_THREAD_LIBS_INIT})
	find_package(PCAP REQUIRED)
	target_link_libraries(icsneocpp PUBLIC ${PCAP_LIBRARY})
endif()

# googletest
if(LIBICSNEO_BUILD_TESTS)
	if(WIN32)
		set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	endif()	

	add_subdirectory(third-party/googletest-master)
	
	if (CMAKE_VERSION VERSION_LESS 2.8.11)
		include_directories("${gtest_SOURCE_DIR}/include")
	endif()
	
	add_executable(runTests test/main.cpp test/eventmanagertest.cpp)
	
	target_link_libraries(runTests gtest gtest_main)
	target_link_libraries(runTests icsneocpp)
	
	target_include_directories(runTests PUBLIC ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
		
	enable_testing()
	
	add_test(NAME testSuite COMMAND runTests)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)