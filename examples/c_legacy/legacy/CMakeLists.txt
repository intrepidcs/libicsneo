add_executable(libicsneoc-legacy-lin-example lin/main.c)
add_executable(libicsneoc-legacy-device-settings-example deviceSettings/main.c)
target_link_libraries(libicsneoc-legacy-lin-example icsneolegacy)
target_link_libraries(libicsneoc-legacy-device-settings-example icsneolegacy)