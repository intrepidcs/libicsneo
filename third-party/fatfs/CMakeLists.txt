project(fatfs VERSION 0.16.0) # R0.16

add_library(fatfs
	source/ff.c
	source/ffunicode.c
	source/ffsystem.c
)
target_include_directories(fatfs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)
