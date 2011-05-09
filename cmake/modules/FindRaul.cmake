# - Try to find Raul
# Once done this will define
#
#  Raul_FOUND - system has Raul
#  Raul_INCLUDE_DIRS - the Raul include directory
#  Raul_LIBRARIES - Link these to use Raul
#  Raul_DEFINITIONS - Compiler switches required for using Raul
#
#  Copyright (c) 2011 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(LibFindMacros)

libfind_package(Raul Boost)
libfind_package(Raul Glib)
libfind_pkg_check_modules(Raul_PKGCONF raul)

find_path(Raul_INCLUDE_DIR
  NAMES raul/Array.hpp
  PATHS ${Raul_PKGCONF_INCLUDE_DIRS}
)

find_library(Raul_LIBRARY
  NAMES raul
  PATHS ${Raul_PKGCONF_LIBRARY_DIRS}
)

set(Raul_PROCESS_INCLUDES
  Raul_INCLUDE_DIR
  Boost_INCLUDE_DIRS
  Glib_INCLUDE_DIRS
)

set(Raul_PROCESS_LIBS
  Raul_LIBRARY
  Boost_LIBRARIES
  Glib_LIBRARIES
)

libfind_process(Raul)

# vim: tw=90 ts=8 sw=2 sts=4 et sta
