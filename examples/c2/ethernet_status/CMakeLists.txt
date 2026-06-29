add_executable(libicsneoc2-ethernet-status-example src/main.c)
target_link_libraries(libicsneoc2-ethernet-status-example icsneoc2-static)

if(WIN32)
	target_compile_definitions(libicsneoc2-ethernet-status-example PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
