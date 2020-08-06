cmake_minimum_required(VERSION 3.2)
project(icsneocsharp VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 11)

include(GNUInstallDirs)

# Add an include directory like so if desired
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third-party/libicsneo ${CMAKE_CURRENT_BINARY_DIR}/third-party/libicsneo)

add_library(icsneocsharp SHARED ${CMAKE_CURRENT_SOURCE_DIR}/csharp_wrap.c)
target_link_libraries(icsneocsharp icsneoc)