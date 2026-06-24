add_executable(libicsneoc2-perf_test-example src/main.c)
target_link_libraries(libicsneoc2-perf_test-example icsneoc2-static)

if(WIN32)
	target_compile_definitions(libicsneoc2-perf_test-example PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
