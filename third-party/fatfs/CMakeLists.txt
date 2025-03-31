project(fatfs VERSION 0.14.1) # R0.14b

add_library(fatfs
	source/ff.c
	source/ffunicode.c
	source/ffsystem.c
)
target_include_directories(fatfs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)
