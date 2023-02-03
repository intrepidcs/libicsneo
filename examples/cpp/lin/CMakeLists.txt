cmake_minimum_required(VERSION 3.2)
project(libicsneocpp-lin VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED 11)

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

# Add libicsneo, usually a git submodule within your project works well
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third-party/libicsneo ${CMAKE_CURRENT_BINARY_DIR}/third-party/libicsneo)

add_executable(libicsneocpp-lin src/LINExample.cpp)
target_link_libraries(libicsneocpp-lin icsneocpp)