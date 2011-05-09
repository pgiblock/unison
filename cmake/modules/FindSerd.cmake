# - Try to find Serd 
# Once done this will define
#
#  Serd_FOUND - system has Serd
#  Serd_INCLUDE_DIRS - the Serd include directory
#  Serd_LIBRARIES - Link these to use Serd
#  Serd_DEFINITIONS - Compiler switches required for using Serd
#
#  Copyright (c) 2011 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(LibFindMacros)

libfind_pkg_check_modules(Serd_PKGCONF serd-0)

find_path(Serd_INCLUDE_DIR
  NAMES serd/serd.h
  PATHS ${Serd_PKGCONF_INCLUDE_DIRS}
)

find_library(Serd_LIBRARY
  NAMES serd-0
  PATHS ${Serd_PKGCONF_LIBRARY_DIRS}
)

set(Serd_PROCESS_INCLUDES
  Serd_INCLUDE_DIR
)

set(Serd_PROCESS_LIBS
  Serd_LIBRARY
)

libfind_process(Serd)

# vim: tw=90 ts=8 sw=2 sts=4 et sta
