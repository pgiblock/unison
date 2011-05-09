# - Try to find Suil 
# Once done this will define
#
#  Suil_FOUND - system has Suil
#  Suil_INCLUDE_DIRS - the Suil include directory
#  Suil_LIBRARIES - Link these to use Suil
#  Suil_DEFINITIONS - Compiler switches required for using Suil
#
#  Copyright (c) 2011 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(LibFindMacros)

libfind_pkg_check_modules(Suil_PKGCONF suil-0)

find_path(Suil_INCLUDE_DIR
  NAMES suil/suil.h
  PATHS ${Suil_PKGCONF_INCLUDE_DIRS}
)

# No such thing
find_library(Suil_LIBRARY
  NAMES suil-0
  PATHS ${Suil_PKGCONF_LIBRARY_DIRS}
)

set(Suil_PROCESS_INCLUDES
  Suil_INCLUDE_DIR
)
set(Suil_PROCESS_LIBS
  Suil_LIBRARY
)
libfind_process(Suil)

# vim: tw=90 ts=8 sw=2 sts=4 et sta
