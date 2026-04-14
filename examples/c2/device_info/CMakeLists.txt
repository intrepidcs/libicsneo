add_executable(libicsneoc2-device-info-example src/main.c)
target_link_libraries(libicsneoc2-device-info-example icsneoc2-static)

if(WIN32)
	target_compile_definitions(libicsneoc2-device-info-example PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
