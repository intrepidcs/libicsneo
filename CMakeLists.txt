cmake_minimum_required(VERSION 3.2)
project(libicsneo VERSION 0.1.1)

option(test "Build all tests." ON) # Makes 'test' variable available

set(CMAKE_CXX_STANDARD 11)

include(GNUInstallDirs)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# macOS Homebrew Boost Fix
if(NOT MSVC)
	include_directories(AFTER /usr/local/include)
endif()

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
	set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/generated/Doxyfile)
	set(ICSNEO_DOCS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/docs)
	if(NOT EXISTS "${DOXYGEN_OUT}")
		set(DOXYGEN_FOUND FALSE)
	endif()
endif()

if(DOXYGEN_FOUND)
	message("Will build Doxygen based documentation")

	add_custom_target(libicsneo_doxygen ALL
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

if(WIN32)
	file(GLOB PLATFORM_SRC_EXTERNAL ${CMAKE_CURRENT_SOURCE_DIR}/platform/windows/*.cpp)
	file(GLOB PLATFORM_SRC_INTERNAL ${CMAKE_CURRENT_SOURCE_DIR}/platform/windows/internal/*.cpp)
	set(PLATFORM_SRC ${PLATFORM_SRC_EXTERNAL} ${PLATFORM_SRC_INTERNAL})
else()
	file(GLOB PLATFORM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/platform/posix/*.cpp)
endif()

set(COMMON_SRC
	communication/message/neomessage.cpp
	communication/packet/canpacket.cpp
	communication/packet/ethernetpacket.cpp
	communication/decoder.cpp
	communication/encoder.cpp
	communication/packetizer.cpp
	communication/multichannelcommunication.cpp
	communication/communication.cpp
	communication/icommunication.cpp
	device/idevicesettings.cpp
	device/devicefinder.cpp
	device/device.cpp
)

set(SRC_FILES ${COMMON_SRC} ${PLATFORM_SRC})

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
configure_file(api/icsneocpp/buildinfo.h.template ${CMAKE_CURRENT_BINARY_DIR}/generated/buildinfo.h)
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

add_library(icsneocpp
	api/icsneocpp/icsneocpp.cpp
	api/icsneocpp/event.cpp
	api/icsneocpp/eventmanager.cpp
	api/icsneocpp/version.cpp
	${SRC_FILES}
)
target_include_directories(icsneocpp
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
set_property(TARGET icsneocpp PROPERTY POSITION_INDEPENDENT_CODE ON)

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
endif(NOT WIN32)

# winpcap
if(WIN32)
	target_include_directories(icsneocpp PUBLIC AFTER third-party/winpcap/include)
	add_definitions(-DWPCAP -DHAVE_REMOTE -DWIN32_LEAN_AND_MEAN)
endif(WIN32)

add_library(icsneoc SHARED api/icsneoc/icsneoc.cpp)
target_include_directories(icsneoc
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(icsneoc PRIVATE icsneocpp)

add_library(icsneoc-static STATIC api/icsneoc/icsneoc.cpp)
target_include_directories(icsneoc-static
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(icsneoc-static PUBLIC icsneocpp)

add_library(icsneolegacy SHARED
	api/icsneolegacy/icsneolegacy.cpp
	api/icsneolegacy/icsneolegacyextra.cpp
	api/icsneoc/icsneoc.cpp
)
target_include_directories(icsneolegacy
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(icsneolegacy PRIVATE icsneocpp)

target_compile_features(icsneocpp PUBLIC cxx_auto_type cxx_constexpr cxx_lambdas cxx_nullptr cxx_range_for cxx_rvalue_references cxx_sizeof_member cxx_strong_enums)
target_compile_features(icsneoc PRIVATE cxx_auto_type cxx_constexpr cxx_lambdas cxx_nullptr cxx_range_for cxx_rvalue_references cxx_sizeof_member cxx_strong_enums)
target_compile_features(icsneoc-static PUBLIC cxx_auto_type cxx_constexpr cxx_lambdas cxx_nullptr cxx_range_for cxx_rvalue_references cxx_sizeof_member cxx_strong_enums)
target_compile_features(icsneolegacy PRIVATE cxx_auto_type cxx_constexpr cxx_lambdas cxx_nullptr cxx_range_for cxx_rvalue_references cxx_sizeof_member cxx_strong_enums)

# libftdi
if(NOT WIN32)
	find_package(Threads)
	set_property(TARGET ftdi1-static PROPERTY POSITION_INDEPENDENT_CODE ON)
	target_link_libraries(icsneocpp PUBLIC ftdi1-static)
	target_link_libraries(icsneocpp PUBLIC ${CMAKE_THREAD_LIBS_INIT})
	find_package(PCAP REQUIRED)
	target_link_libraries(icsneocpp PUBLIC ${PCAP_LIBRARY})
endif()

# googletest, hopefully cross-platform and won't have to split into 2 places in CMakeLists
if(test)
	if(WIN32)
		set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	endif()	

	add_subdirectory(third-party/googletest-master)
	
	if (CMAKE_VERSION VERSION_LESS 2.8.11)
		include_directories("${gtest_SOURCE_DIR}/include")
	endif()
	
	add_executable(runTests test/main.cpp)
	
	target_link_libraries(runTests gtest gtest_main)
	target_link_libraries(runTests icsneocpp)
	
	target_include_directories(runTests PUBLIC ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
		
	enable_testing()
	
	add_test(NAME plshelpme COMMAND runTests)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)