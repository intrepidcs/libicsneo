# determine docdir
include(GNUInstallDirs)
if(NOT CMAKE_INSTALL_DOCDIR)
  if(WIN32)
    set(CMAKE_INSTALL_DOCDIR .)
  else(WIN32)
    set(CMAKE_INSTALL_DOCDIR ${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME})
  endif(WIN32)
endif(NOT CMAKE_INSTALL_DOCDIR)

option(FTDI_EEPROM "Build ftdi_eeprom" ON)

if ( FTDI_EEPROM )
  find_package ( Confuse )
  find_package ( Libintl )
else(FTDI_EEPROM)
  message(STATUS "ftdi_eeprom build is disabled")
endif ()


if ( CONFUSE_FOUND )
  message(STATUS "Building ftdi_eeprom")

  include_directories ( ${CONFUSE_INCLUDE_DIRS} )
  list ( APPEND libs ${CONFUSE_LIBRARIES} )

  if ( LIBINTL_FOUND )
    include_directories ( ${LIBINTL_INCLUDE_DIR} )
    list ( APPEND libs ${LIBINTL_LIBRARIES} )
  endif ()


  # Version defines
  set ( EEPROM_MAJOR_VERSION 0 )
  set ( EEPROM_MINOR_VERSION 17 )
  set ( EEPROM_VERSION_STRING ${EEPROM_MAJOR_VERSION}.${EEPROM_MINOR_VERSION} )

  include_directories ( BEFORE ${CMAKE_SOURCE_DIR}/src )
  include_directories ( BEFORE ${CMAKE_CURRENT_BINARY_DIR} )

  configure_file(
    ftdi_eeprom_version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/ftdi_eeprom_version.h
  )

  add_executable ( ftdi_eeprom main.c )
  target_link_libraries ( ftdi_eeprom ftdi1 ${CONFUSE_LIBRARIES} )
  if ( LIBINTL_FOUND )
    target_link_libraries ( ftdi_eeprom ${LIBINTL_LIBRARIES} )
  endif ()
  install ( TARGETS ftdi_eeprom DESTINATION bin )
  install ( FILES example.conf DESTINATION ${CMAKE_INSTALL_DOCDIR} )
else ()
  message ( STATUS "libConfuse not found, won't build ftdi_eeprom" )
endif ()
