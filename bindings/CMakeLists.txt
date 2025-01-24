if(LIBICSNEO_ENABLE_BINDINGS_PYTHON)
	add_subdirectory(python)
endif()

if(LIBICSNEO_ENABLE_BINDINGS_RUST)
	include(FetchContent)

	FetchContent_Declare(
		Corrosion
		GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
		GIT_TAG v0.5 # Optionally specify a commit hash, version tag or branch here
	)
	FetchContent_MakeAvailable(Corrosion)

	corrosion_import_crate(MANIFEST_PATH ${CMAKE_SOURCE_DIR}/bindings/rust/icsneoc2rs/Cargo.toml)
else()
	message(STATUS "Not building rust bindings")
endif()