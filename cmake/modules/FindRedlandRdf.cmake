# - Try to find Rdf
# Once done this will define
#
#  RDF_FOUND - system has Rdf
#  RDF_INCLUDE_DIRS - the Rdf include directory
#  RDF_LIBRARIES - Link these to use Rdf
#  RDF_DEFINITIONS - Compiler switches required for using Rdf
#
#  Copyright (c) 2010 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (RDF_LIBRARIES AND RDF_INCLUDE_DIRS)
  # in cache already
  set(RDF_FOUND TRUE)
else (RDF_LIBRARIES AND RDF_INCLUDE_DIRS)
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(RDF redland)
  endif (PKG_CONFIG_FOUND)

  if (NOT RDF_INCLUDE_DIRS)
    find_path(RDF_INCLUDE_DIR
        NAMES rdf_init.h
    )

    set(RDF_INCLUDE_DIRS
        ${RDF_INCLUDE_DIR}
    )
  endif (NOT RDF_INCLUDE_DIRS)

  if (NOT RDF_LIBRARIES)
    find_library(RDF_LIBRARY
        NAMES rdf
    )

    if (RDF_LIBRARY)
      set(RDF_LIBRARIES
          ${RDF_LIBRARIES}
          ${RDF_LIBRARY}
      )
    endif (RDF_LIBRARY)
  endif (NOT RDF_LIBRARIES)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Rdf DEFAULT_MSG RDF_LIBRARIES RDF_INCLUDE_DIRS)

  # show the RDF_INCLUDE_DIRS and RDF_LIBRARIES variables only in the advanced view
  mark_as_advanced(RDF_INCLUDE_DIRS RDF_LIBRARIES)

endif (RDF_LIBRARIES AND RDF_INCLUDE_DIRS)

# vim: tw=90 ts=8 sw=2 sts=2 et sta noai
