# - Try to find Jack
# Once done this will define
#
#  JACK_FOUND - system has Jack
#  JACK_INCLUDE_DIRS - the Jack include directory
#  JACK_LIBRARIES - Link these to use Jack
#  JACK_DEFINITIONS - Compiler switches required for using Jack
#
#  Copyright (c) 2010 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (JACK_LIBRARIES AND JACK_INCLUDE_DIRS)
  # in cache already
  set(JACK_FOUND TRUE)
else (JACK_LIBRARIES AND JACK_INCLUDE_DIRS)
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(_JACK jack)
  endif (PKG_CONFIG_FOUND)

  find_path(JACK_INCLUDE_DIR
    NAMES
      jack/jack.h
    HINTS
      ${_JACK_INCLUDEDIR}
    PATH_SUFFIXES
      jack
  )

  find_library(JACK_LIBRARY
    NAMES
      jack
    HINTS
      ${_JACK_LIBDIR}
  )

  set(JACK_INCLUDE_DIRS
      ${JACK_INCLUDE_DIR}
  )

  if (JACK_LIBRARY)
    set(JACK_LIBRARIES
        ${JACK_LIBRARIES}
        ${JACK_LIBRARY}
    )
  endif (JACK_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Jack DEFAULT_MSG JACK_LIBRARIES JACK_INCLUDE_DIRS)

  # show the JACK_INCLUDE_DIRS and JACK_LIBRARIES variables only in the advanced view
  mark_as_advanced(JACK_INCLUDE_DIRS JACK_LIBRARIES)

endif (JACK_LIBRARIES AND JACK_INCLUDE_DIRS)

# vim: tw=90 ts=8 sw=2 sts=2 et sta noai
