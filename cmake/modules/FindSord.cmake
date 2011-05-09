# - Try to find Sord 
# Once done this will define
#
#  Sord_FOUND - system has Sord
#  Sord_INCLUDE_DIRS - the Sord include directory
#  Sord_LIBRARIES - Link these to use Sord
#  Sord_DEFINITIONS - Compiler switches required for using Sord
#
#  Copyright (c) 2011 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(LibFindMacros)

libfind_pkg_check_modules(Sord_PKGCONF sord-0)

find_path(Sord_INCLUDE_DIR
  NAMES sord/sord.h
  PATHS ${Sord_PKGCONF_INCLUDE_DIRS}
)

find_library(Sord_LIBRARY
  NAMES sord-0
  PATHS ${Sord_PKGCONF_LIBRARY_DIRS}
)

set(Sord_PROCESS_INCLUDES
  Sord_INCLUDE_DIR
)

set(Sord_PROCESS_LIBS
  Sord_LIBRARY
)

libfind_process(Sord)

# vim: tw=90 ts=8 sw=2 sts=4 et sta
