add_executable(libicsneoc2-reconnect-example src/main.c)
target_link_libraries(libicsneoc2-reconnect-example icsneoc2-static)

if(WIN32)
	target_compile_definitions(libicsneoc2-reconnect-example PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
