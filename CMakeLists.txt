cmake_minimum_required(VERSION 3.12)
project(libicsneo VERSION 0.3.0)

cmake_policy(SET CMP0074 NEW)

option(LIBICSNEO_BUILD_TESTS "Build all tests." OFF)
option(LIBICSNEO_BUILD_DOCS "Build documentation. Don't use in Visual Studio." OFF)
option(LIBICSNEO_BUILD_EXAMPLES "Build examples." ON)
option(LIBICSNEO_BUILD_ICSNEOC "Build dynamic C library" ON)
option(LIBICSNEO_BUILD_ICSNEOC_STATIC "Build static C library" ON)
option(LIBICSNEO_BUILD_ICSNEOLEGACY "Build icsnVC40 compatibility library" ON)
set(LIBICSNEO_NPCAP_INCLUDE_DIR "" CACHE STRING "Npcap include directory; set to build with Npcap")

# Device Drivers
# You almost certainly don't want firmio for your build,
# it is only relevant for communication between Linux and
# CoreMini from the onboard processor of the device.
option(LIBICSNEO_ENABLE_FIRMIO "Enable communication between Linux and CoreMini within the same device" OFF)
option(LIBICSNEO_ENABLE_RAW_ETHERNET "Enable devices which communicate over raw ethernet" ON)
option(LIBICSNEO_ENABLE_CDCACM "Enable devices which communicate over USB CDC ACM" ON)
option(LIBICSNEO_ENABLE_FTDI "Enable devices which communicate over USB FTDI2XX" ON)
option(LIBICSNEO_ENABLE_TCP "Enable devices which communicate over TCP" OFF)
option(LIBICSNEO_ENABLE_FTD3XX "Enable devices which communicate over USB FTD3XX" ON)

if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 17)
endif()

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
	# http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0618r0.html
	# Still supported until a suitable replacement is standardized
	add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
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

if(WIN32)
	set(PLATFORM_SRC
		platform/windows/registry.cpp
	)

	if(LIBICSNEO_ENABLE_RAW_ETHERNET)
		list(APPEND PLATFORM_SRC
			platform/windows/pcap.cpp
			platform/windows/internal/pcapdll.cpp
		)
	endif()

	if(LIBICSNEO_ENABLE_CDCACM OR LIBICSNEO_ENABLE_FTDI)
		list(APPEND PLATFORM_SRC
			platform/windows/vcp.cpp
		)
	endif()
else() # Darwin or Linux
	set(PLATFORM_SRC)

	if(LIBICSNEO_ENABLE_FIRMIO)
		list(APPEND PLATFORM_SRC
			platform/posix/firmio.cpp
		)
	endif()

	if(LIBICSNEO_ENABLE_RAW_ETHERNET)
		list(APPEND PLATFORM_SRC
			platform/posix/pcap.cpp
		)
	endif()

	if(LIBICSNEO_ENABLE_FTDI)
		list(APPEND PLATFORM_SRC
			platform/posix/ftdi.cpp
		)
	endif()

	if(LIBICSNEO_ENABLE_CDCACM)
		list(APPEND PLATFORM_SRC
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
					"There is no CDCACM platform port defined for ${CMAKE_SYSTEM_NAME}!\n"
					"The Linux platform code will be used, as it will generally allow building, but some devices may not enumerate properly."
				)
			endif()
		endif()
	endif()
endif()

if(LIBICSNEO_ENABLE_FTD3XX)
	if(NOT FTD3XX_ROOT) # allow system override
		include(FetchContent)
		if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(LIBICSNEO_FTD3XX_URL "https://github.com/intrepidcs/libftd3xx-repack/releases/download/23.28.0/libftd3xx-1.3.0.4-win-x64.zip")
			set(LIBICSNEO_FTD3XX_URL_HASH "SHA256=ee1289cdd5023de67275aaa1712e8e32e73e825a8392efb0a76f161e357fbdc9")
		elseif(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
			set(LIBICSNEO_FTD3XX_URL "https://github.com/intrepidcs/libftd3xx-repack/releases/download/23.28.0/libftd3xx-1.3.0.4-win-i686.zip")
			set(LIBICSNEO_FTD3XX_URL_HASH "SHA256=bcd31f5c3cb39ecb9b09db5ce722bd79de6e0f813130e7b8f2c86879a60d8ff0")
		elseif(APPLE AND CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(LIBICSNEO_FTD3XX_URL "https://github.com/intrepidcs/libftd3xx-repack/releases/download/23.28.0/libftd3xx-1.0.14-macos-universal2.zip")
			set(LIBICSNEO_FTD3XX_URL_HASH "SHA256=74c0d35f04242d0841532c6325eb2932b8f627e3c395382a15d9f39974d27a90")
		elseif(UNIX)
			if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
				if(CMAKE_SIZEOF_VOID_P EQUAL 8)
					set(LIBICSNEO_FTD3XX_URL "https://github.com/intrepidcs/libftd3xx-repack/releases/download/23.28.0/libftd3xx-1.0.14-linux-x64.zip")
					set(LIBICSNEO_FTD3XX_URL_HASH "SHA256=a9dc7eb6948c8977fbd79f6700bec6f882d3da5667aea8f2175b8d1f6f08e456")
				endif()
			elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm.*|aarch64")
				if(CMAKE_SIZEOF_VOID_P EQUAL 8)
					set(LIBICSNEO_FTD3XX_URL "https://github.com/intrepidcs/libftd3xx-repack/releases/download/23.28.0/libftd3xx-1.0.14-linux-aarch64.zip")
					set(LIBICSNEO_FTD3XX_URL_HASH "SHA256=da4b90ea1cbb905874cd159ad2ab8c1bdde65cc22b3aa55bf2b5fd85ca6efd22")
				elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
					set(LIBICSNEO_FTD3XX_URL "https://github.com/intrepidcs/libftd3xx-repack/releases/download/23.28.0/libftd3xx-1.0.14-linux-armhf.zip")
					set(LIBICSNEO_FTD3XX_URL_HASH "SHA256=d813008117422cae958f7c71a065cdac0d31dca3b24809d3ab5e13604a9c3fb1")
				endif()
			endif()
		endif()
		if(NOT LIBICSNEO_FTD3XX_URL)
			message(FATAL_ERROR "Unsupported platform for FTD3XX driver")
		endif()
		FetchContent_Declare(
			ftdi3xx
			URL ${LIBICSNEO_FTD3XX_URL}
			URL_HASH ${LIBICSNEO_FTD3XX_URL_HASH}
		)
		FetchContent_GetProperties(ftdi3xx)
		if(NOT ftdi3xx_POPULATED)
			FetchContent_Populate(ftdi3xx)
		endif()
		set(FTD3XX_ROOT "${ftdi3xx_SOURCE_DIR}")
	endif()
	find_package(FTD3XX REQUIRED)
	list(APPEND PLATFORM_SRC
		platform/ftd3xx.cpp
	)
endif()

if(LIBICSNEO_ENABLE_TCP)
	list(APPEND PLATFORM_SRC
		platform/tcp.cpp
	)
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
	communication/message/callback/streamoutput/a2bwavoutput.cpp
	communication/message/callback/streamoutput/a2bdecoder.cpp
	communication/message/neomessage.cpp
	communication/message/ethphymessage.cpp
	communication/message/linmessage.cpp
	communication/packet/flexraypacket.cpp
	communication/packet/canpacket.cpp
	communication/packet/a2bpacket.cpp
	communication/packet/ethernetpacket.cpp
	communication/packet/versionpacket.cpp
	communication/packet/iso9141packet.cpp
	communication/packet/ethphyregpacket.cpp
	communication/packet/logicaldiskinfopacket.cpp
	communication/packet/wivicommandpacket.cpp
	communication/packet/i2cpacket.cpp
	communication/packet/linpacket.cpp
	communication/packet/mdiopacket.cpp
	communication/packet/scriptstatuspacket.cpp
	communication/packet/componentversionpacket.cpp
	communication/packet/supportedfeaturespacket.cpp
	communication/packet/genericbinarystatuspacket.cpp
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
	device/neodevice.cpp
	disk/diskreaddriver.cpp
	disk/diskwritedriver.cpp
	disk/nulldiskdriver.cpp
	disk/neomemorydiskdriver.cpp
	disk/plasiondiskreaddriver.cpp
	disk/extextractordiskreaddriver.cpp
	disk/fat.cpp
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
if(LIBICSNEO_ENABLE_FIRMIO)
	target_compile_definitions(icsneocpp PRIVATE ICSNEO_ENABLE_FIRMIO)
endif()
if(LIBICSNEO_ENABLE_RAW_ETHERNET)
	target_compile_definitions(icsneocpp PRIVATE ICSNEO_ENABLE_RAW_ETHERNET)
endif()
if(LIBICSNEO_ENABLE_CDCACM)
	target_compile_definitions(icsneocpp PRIVATE ICSNEO_ENABLE_CDCACM)
endif()
if(LIBICSNEO_ENABLE_FTDI)
	target_compile_definitions(icsneocpp PRIVATE ICSNEO_ENABLE_FTDI)
endif()
if(LIBICSNEO_ENABLE_FTD3XX)
	target_compile_definitions(icsneocpp PRIVATE ICSNEO_ENABLE_FTD3XX)
	target_link_libraries(icsneocpp PRIVATE FTD3XX::FTD3XX)
endif()
if(LIBICSNEO_ENABLE_TCP)
	target_compile_definitions(icsneocpp PRIVATE ICSNEO_ENABLE_TCP)
	if(WIN32)
		target_link_libraries(icsneocpp PRIVATE ws2_32)
	endif()
endif()

# fatfs
add_subdirectory(third-party/fatfs)
set_property(TARGET fatfs PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(icsneocpp PRIVATE fatfs)

# libftdi
if(LIBICSNEO_ENABLE_FTDI)
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
		target_include_directories(icsneocpp PRIVATE ${LIBUSB_INCLUDE_DIR})

		find_package(Threads)
		set_property(TARGET ftdi1-static PROPERTY POSITION_INDEPENDENT_CODE ON)
		target_link_libraries(icsneocpp PUBLIC ftdi1-static)
		target_link_libraries(icsneocpp PUBLIC ${CMAKE_THREAD_LIBS_INIT})
	endif(NOT WIN32)
endif(LIBICSNEO_ENABLE_FTDI)

# pcap
if(LIBICSNEO_ENABLE_RAW_ETHERNET)
	if(WIN32)
		if(LIBICSNEO_NPCAP_INCLUDE_DIR STREQUAL "")
			target_include_directories(icsneocpp PUBLIC AFTER third-party/winpcap/include)
			add_definitions(-DWPCAP -DHAVE_REMOTE -DWIN32_LEAN_AND_MEAN)
		else()
			target_include_directories(icsneocpp PUBLIC AFTER ${LIBICSNEO_NPCAP_INCLUDE_DIR})
			add_definitions(-DNPCAP -DWIN32_LEAN_AND_MEAN)
		endif()
	else()
		find_package(PCAP REQUIRED)
		target_include_directories(icsneocpp PUBLIC ${PCAP_INCLUDE_DIR})
		target_link_libraries(icsneocpp PUBLIC ${PCAP_LIBRARY})
	endif(WIN32)
endif(LIBICSNEO_ENABLE_RAW_ETHERNET)

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

# googletest
if(LIBICSNEO_BUILD_TESTS)
	if(WIN32)
		set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	endif()

	if (NOT TARGET gtest)
		add_subdirectory(third-party/googletest-master)
	endif()

	if (CMAKE_VERSION VERSION_LESS 2.8.11)
		include_directories("${gtest_SOURCE_DIR}/include")
	endif()

	add_executable(libicsneo-tests
		test/main.cpp
		test/diskdriverreadtest.cpp
		test/diskdriverwritetest.cpp
		test/eventmanagertest.cpp
		test/ethernetpacketizertest.cpp
		test/i2cencoderdecodertest.cpp
		test/linencoderdecodertest.cpp
		test/a2bencoderdecodertest.cpp
		test/mdioencoderdecodertest.cpp
	)

	target_link_libraries(libicsneo-tests gtest gtest_main)
	target_link_libraries(libicsneo-tests icsneocpp)

	target_include_directories(libicsneo-tests PUBLIC ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

	enable_testing()

	add_test(NAME libicsneo-test-suite COMMAND libicsneo-tests)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)