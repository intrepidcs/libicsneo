add_executable(libicsneoc2-read_messages-example src/main.c)
target_link_libraries(libicsneoc2-read_messages-example icsneoc2-static)

if(WIN32)
	target_compile_definitions(libicsneoc2-read_messages-example PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
