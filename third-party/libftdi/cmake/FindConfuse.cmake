# libConfuse is a configuration file parser library
# available at http://www.nongnu.org/confuse/
#
# The module defines the following variables:
#  CONFUSE_FOUND - the system has Confuse
#  CONFUSE_INCLUDE_DIR - where to find confuse.h
#  CONFUSE_INCLUDE_DIRS - confuse includes
#  CONFUSE_LIBRARY - where to find the Confuse library
#  CONFUSE_LIBRARIES - aditional libraries
#  CONFUSE_ROOT_DIR - root dir (ex. /usr/local)

#=============================================================================
# Copyright 2010-2013, Julien Schueller
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met: 
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution. 
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies, 
# either expressed or implied, of the FreeBSD Project.
#=============================================================================


find_path ( CONFUSE_INCLUDE_DIR
  NAMES confuse.h
)

set ( CONFUSE_INCLUDE_DIRS ${CONFUSE_INCLUDE_DIR} )

find_library ( CONFUSE_LIBRARY
  NAMES confuse
)

set ( CONFUSE_LIBRARIES ${CONFUSE_LIBRARY} )


# try to guess root dir from include dir
if ( CONFUSE_INCLUDE_DIR )
  string ( REGEX REPLACE "(.*)/include.*" "\\1" CONFUSE_ROOT_DIR ${CONFUSE_INCLUDE_DIR} )
# try to guess root dir from library dir
elseif ( CONFUSE_LIBRARY )
  string ( REGEX REPLACE "(.*)/lib[/|32|64].*" "\\1" CONFUSE_ROOT_DIR ${CONFUSE_LIBRARY} )
endif ()


# handle the QUIETLY and REQUIRED arguments
include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Confuse DEFAULT_MSG CONFUSE_LIBRARY CONFUSE_INCLUDE_DIR  )

mark_as_advanced (
  CONFUSE_LIBRARY 
  CONFUSE_LIBRARIES
  CONFUSE_INCLUDE_DIR
  CONFUSE_INCLUDE_DIRS
  CONFUSE_ROOT_DIR
)
