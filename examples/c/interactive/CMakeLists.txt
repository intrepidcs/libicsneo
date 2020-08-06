cmake_minimum_required(VERSION 3.2)
project(libicsneoc-interactive-example VERSION 0.2.0)

include(GNUInstallDirs)

# Include libicsneo's include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include)

if(UNIX)
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
endif()

add_executable(libicsneoc-interactive-example src/main.c)
if(UNIX)
	target_link_libraries(libicsneoc-interactive-example ${CMAKE_DL_LIBS})
endif()