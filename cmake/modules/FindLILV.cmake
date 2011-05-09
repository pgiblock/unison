# - Try to find LILV
# Once done this will define
#
#  LILV_FOUND - system has LILV
#  LILV_INCLUDE_DIRS - the LILV include directory
#  LILV_LIBRARIES - Link these to use LILV
#  LILV_DEFINITIONS - Compiler switches required for using LILV
#
#  Copyright (c) 2011 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(LibFindMacros)

libfind_pkg_check_modules(LILV_PKGCONF lilv-0)

libfind_package(LILV Glib)
libfind_package(LILV Sord)

find_path(LILV_INCLUDE_DIR
  NAMES lilv/lilv.h
  PATHS ${LILV_PKGCONF_INCLUDE_DIRS}
)

find_library(LILV_LIBRARY
  NAMES lilv-0
  PATHS ${LILV_PKGCONF_LIBRARY_DIRS}
)

set(LILV_PROCESS_INCLUDES
  LILV_INCLUDE_DIR
  Glib_INCLUDE_DIRS
  Sord_INCLUDE_DIRS
)
set(LILV_PROCESS_LIBS
  LILV_LIBRARY
  Glib_LIBRARIES
  Sord_LIBRARIES
)
libfind_process(LILV)

# vim: tw=90 ts=8 sw=2 sts=4 et sta
