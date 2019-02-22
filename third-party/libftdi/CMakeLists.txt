# Project
project(libftdi1)
set(MAJOR_VERSION 1)
set(MINOR_VERSION 4)
set(PACKAGE libftdi1)
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION})
set(VERSION ${VERSION_STRING})
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# CMake
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
   set(CMAKE_BUILD_TYPE     RelWithDebInfo)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")
set(CMAKE_COLOR_MAKEFILE ON)
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

add_definitions(-Wall)

# Debug build
message("-- Build type: ${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
   add_definitions(-DDEBUG)
endif(${CMAKE_BUILD_TYPE} STREQUAL Debug)

# find libusb
find_package ( USB1 REQUIRED )
include_directories ( ${LIBUSB_INCLUDE_DIR} )

# Find Boost (optional package)
find_package(Boost)

# Set components
set(CPACK_COMPONENTS_ALL sharedlibs staticlibs headers)
set(CPACK_COMPONENT_SHAREDLIBS_DISPLAY_NAME "Shared libraries")
set(CPACK_COMPONENT_STATICLIBS_DISPLAY_NAME "Static libraries")
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")

set(CPACK_COMPONENT_SHAREDLIBS_DESCRIPTION
"Shared library for general use.")
set(CPACK_COMPONENT_STATICLIBS_DESCRIPTION
"Static library, good if you want to embed libftdi1 in your application.")
set(CPACK_COMPONENT_HEADERS_DESCRIPTION
"C/C++ header files.")

set(CPACK_COMPONENT_SHAREDLIBS_GROUP "Development")
set(CPACK_COMPONENT_STATICLIBS_GROUP "Development")
set(CPACK_COMPONENT_HEADERS_GROUP    "Development")

option ( LIBFTDI_STATICLIBS "Build static libraries" ON )

# guess LIB_SUFFIX, don't take debian multiarch into account 
if ( NOT DEFINED LIB_SUFFIX )
  if( CMAKE_SYSTEM_NAME MATCHES "Linux"
      AND NOT CMAKE_CROSSCOMPILING
      AND NOT EXISTS "/etc/debian_version"
      AND NOT EXISTS "/etc/arch-release" )
    if ( "${CMAKE_SIZEOF_VOID_P}" EQUAL "8" )
      set ( LIB_SUFFIX 64 )
    endif ()
  endif ()
endif ()

if(NOT APPLE)
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      SET(PACK_ARCH "")
    else(CMAKE_SIZEOF_VOID_P EQUAL 8)
      SET(PACK_ARCH .x86_64)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
else(NOT APPLE)
  SET(PACK_ARCH "")
endif(NOT APPLE)

# Package information
set(CPACK_PACKAGE_VERSION              ${VERSION_STRING})
set(CPACK_PACKAGE_CONTACT              "Intra2net AG <libftdi@developer.intra2net.com>")
set(CPACK_PACKAGE_DESCRIPTION          "libftdi1 library.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY  ${CPACK_PACKAGE_DESCRIPTION}
                                       )
# Package settings
if ( UNIX )
  set(CPACK_GENERATOR                    "DEB;RPM")
  set(CPACK_CMAKE_GENERATOR              "Unix Makefiles")
  set(CPACK_PACKAGE_NAME                 ${PROJECT_NAME})
  set(CPACK_PACKAGE_FILE_NAME            ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}${PACK_ARCH})
endif ()

if ( WIN32 )
  set ( CPACK_GENERATOR                    "NSIS" )
  set ( CPACK_CMAKE_GENERATOR              "MinGW Makefiles" )
  set ( CPACK_PACKAGE_NAME                 "${PROJECT_NAME}" )
  set ( CPACK_PACKAGE_VENDOR               "" )
  set ( CPACK_PACKAGE_INSTALL_DIRECTORY    "libftdi1" )
  set ( CPACK_PACKAGE_FILE_NAME            "${PROJECT_NAME}-${VERSION_STRING}-win32")
  set ( CPACK_NSIS_DISPLAY_NAME            "libftdi1" )
  set ( CPACK_NSIS_MODIFY_PATH             ON )
endif ()

set(CPACK_RESOURCE_FILE_LICENSE        ${PROJECT_SOURCE_DIR}/LICENSE)

set(CPACK_SOURCE_GENERATOR             TGZ)
set(CPACK_SOURCE_IGNORE_FILES          "\\\\.git;~$;build/")
set(CPACK_SOURCE_PACKAGE_FILE_NAME     ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})

# Subdirectories
if ( UNIX )
  set ( CPACK_SET_DESTDIR ON )
endif ()

# "make dist" target
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${VERSION_STRING})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
            | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Tests
option ( LIBFTDI_BUILD_TESTS "Build unit tests with Boost Unit Test framework" ON )

# Documentation
option ( LIBFTDI_DOCUMENTATION "Generate API documentation with Doxygen" ON )


find_package ( Doxygen )
if ( LIBFTDI_DOCUMENTATION AND DOXYGEN_FOUND )

   # Find doxy config
   message(STATUS "Doxygen found.")

   # Copy doxy.config.in
   set(top_srcdir ${PROJECT_SOURCE_DIR})
   configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile )
   configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.xml.in ${CMAKE_BINARY_DIR}/Doxyfile.xml )

   # Run doxygen
   add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/doc/html/index.html
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
    DEPENDS ${c_headers};${c_sources};${cpp_sources};${cpp_headers}
   )

   add_custom_target(docs ALL DEPENDS ${CMAKE_BINARY_DIR}/doc/html/index.html)

   message(STATUS "Generating API documentation with Doxygen")
else(LIBFTDI_DOCUMENTATION AND DOXYGEN_FOUND)
   message(STATUS "Not generating API documentation")
endif(LIBFTDI_DOCUMENTATION AND DOXYGEN_FOUND)

add_subdirectory(src)
add_subdirectory(ftdipp)
add_subdirectory(python)
add_subdirectory(ftdi_eeprom)
add_subdirectory(packages)
add_subdirectory(test)

# PkgConfig
set(prefix      ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX}/bin)
set(includedir  ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})

if(${UNIX})
   set(libdir      ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
endif(${UNIX})
if(${WIN32})
   set(libdir      ${CMAKE_INSTALL_PREFIX}/bin)
endif(${WIN32})

configure_file(${PROJECT_SOURCE_DIR}/libftdi1.spec.in ${CMAKE_BINARY_DIR}/libftdi1.spec @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/libftdi1.pc.in ${CMAKE_BINARY_DIR}/libftdi1.pc @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/libftdipp1.pc.in ${CMAKE_BINARY_DIR}/libftdipp1.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libftdi1.pc ${CMAKE_BINARY_DIR}/libftdipp1.pc
        DESTINATION lib${LIB_SUFFIX}/pkgconfig)

if (UNIX OR MINGW)
  configure_file ( libftdi1-config.in ${CMAKE_CURRENT_BINARY_DIR}/libftdi1-config @ONLY )
  install ( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/libftdi1-config
            DESTINATION bin )
endif ()

# config script install path
if ( NOT DEFINED LIBFTDI_CMAKE_CONFIG_DIR )
  set ( LIBFTDI_CMAKE_CONFIG_DIR lib${LIB_SUFFIX}/cmake/libftdi1 )
endif ()

set ( LIBFTDI_INCLUDE_DIR ${includedir} )
set ( LIBFTDI_INCLUDE_DIRS ${LIBFTDI_INCLUDE_DIR} )
set ( LIBFTDI_LIBRARY ftdi1 )
set ( LIBFTDI_LIBRARIES ${LIBFTDI_LIBRARY} )
list ( APPEND LIBFTDI_LIBRARIES ${LIBUSB_LIBRARIES} )
set ( LIBFTDI_STATIC_LIBRARY ftdi1.a )
set ( LIBFTDI_STATIC_LIBRARIES ${LIBFTDI_STATIC_LIBRARY} )
list ( APPEND LIBFTDI_STATIC_LIBRARIES ${LIBUSB_LIBRARIES} )
if (FTDI_BUILD_CPP)
  set ( LIBFTDIPP_LIBRARY ftdipp1 )
  set ( LIBFTDIPP_LIBRARIES ${LIBFTDIPP_LIBRARY} )
  list ( APPEND LIBFTDIPP_LIBRARIES ${LIBUSB_LIBRARIES} )
endif ()
set ( LIBFTDI_LIBRARY_DIRS ${libdir} )
set ( LIBFTDI_ROOT_DIR ${prefix} )
set ( LIBFTDI_VERSION_STRING ${VERSION_STRING} )
set ( LIBFTDI_VERSION_MAJOR ${MAJOR_VERSION} )
set ( LIBFTDI_VERSION_MINOR ${MINOR_VERSION} )

set ( LIBFTDI_USE_FILE ${CMAKE_INSTALL_PREFIX}/${LIBFTDI_CMAKE_CONFIG_DIR}/UseLibFTDI1.cmake )

if(CMAKE_VERSION VERSION_LESS 2.8.8) 
  configure_file ( cmake/LibFTDI1Config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/LibFTDI1Config.cmake @ONLY )
  configure_file ( cmake/LibFTDI1ConfigVersion.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/LibFTDI1ConfigVersion.cmake @ONLY )
else ()
  include(CMakePackageConfigHelpers)

  configure_package_config_file (
    cmake/LibFTDI1Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LibFTDI1Config.cmake
    INSTALL_DESTINATION ${LIBFTDI_CMAKE_CONFIG_DIR}
    PATH_VARS
      LIBFTDI_USE_FILE
      LIBFTDI_ROOT_DIR
      LIBFTDI_INCLUDE_DIR
      LIBFTDI_INCLUDE_DIRS
      LIBFTDI_LIBRARY_DIRS
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )
  write_basic_package_version_file (
    LibFTDI1ConfigVersion.cmake 
    VERSION ${LIBFTDI_VERSION_STRING} 
    COMPATIBILITY AnyNewerVersion
  )
endif ()

option ( LIBFTDI_INSTALL "Generate install step" ON )

if (LIBFTDI_INSTALL)
  install ( FILES 
            ${CMAKE_CURRENT_BINARY_DIR}/LibFTDI1Config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/LibFTDI1ConfigVersion.cmake
            cmake/UseLibFTDI1.cmake

          DESTINATION ${LIBFTDI_CMAKE_CONFIG_DIR}
        )
endif (LIBFTDI_INSTALL)




include(CPack)
