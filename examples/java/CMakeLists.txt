cmake_minimum_required(VERSION 3.2)
project(icsneojava VERSION 0.2.0)

find_package(Java REQUIRED)
find_package(JNI REQUIRED)

include(UseJava)
include_directories(${JNI_INCLUDE_DIRS})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third-party/libicsneo ${CMAKE_CURRENT_BINARY_DIR}/third-party/libicsneo)

add_library(icsneojava SHARED ${CMAKE_CURRENT_SOURCE_DIR}/java_wrap.c)
target_link_libraries(icsneojava icsneoc)