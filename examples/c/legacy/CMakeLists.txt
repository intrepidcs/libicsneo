cmake_minimum_required(VERSION 3.2)
project(libicsneoc-legacy-lin-example VERSION 0.2.0)

add_executable(libicsneoc-legacy-lin-example lin/main.c)
add_executable(libicsneoc-legacy-device-settings-example deviceSettings/main.c)
target_link_libraries(libicsneoc-legacy-lin-example icsneolegacy)
target_link_libraries(libicsneoc-legacy-device-settings-example icsneolegacy)