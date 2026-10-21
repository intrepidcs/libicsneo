project(miniz VERSION 3.1.2)

add_library(miniz STATIC
	miniz.c
)
target_include_directories(miniz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(MSVC)
	target_compile_options(miniz PRIVATE /wd4127)
endif()
