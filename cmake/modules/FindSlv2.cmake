# - Try to find Slv2
# Once done this will define
#
#  SLV2_FOUND - system has Slv2
#  SLV2_INCLUDE_DIRS - the Slv2 include directory
#  SLV2_LIBRARIES - Link these to use Slv2
#  SLV2_DEFINITIONS - Compiler switches required for using Slv2
#
#  Copyright (c) 2010 Paul Giblock <pgib@users.sourceforge.net>
#  Copyright (c) 2014 Lukas W <lukaswhl@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(RedlandRdf_FIND_REQUIRED ${Slv2_FIND_REQUIRED})
set(RedlandRdf_FIND_QUIETLY ${Slv2_FIND_QUIETLY})
find_package(RedlandRdf)

if (RDF_FOUND)
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(_SLV2 QUIET slv2)
  endif (PKG_CONFIG_FOUND)

  find_path(SLV2_INCLUDE_DIR
      NAMES slv2/slv2.h
      HINTS ${_SLV2_INCLUDEDIR}
      PATH_SUFFIXES slv2
  )

  find_library(SLV2_LIBRARY
      NAMES slv2
      HINTS ${_SLV2_LIBDIR}
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Slv2 DEFAULT_MSG SLV2_LIBRARY SLV2_INCLUDE_DIR)

  if (SLV2_FOUND)
    set(SLV2_INCLUDE_DIRS
        ${SLV2_INCLUDE_DIR}
        ${RDF_INCLUDE_DIRS}
    )
    set(SLV2_LIBRARIES
        ${SLV2_LIBRARY}
        ${RDF_INCLUDE}
    )
  endif (SLV2_FOUND)

  # show the SLV2_INCLUDE_DIR and SLV2_LIBRARY variables only in the advanced view
  mark_as_advanced(SLV2_INCLUDE_DIR SLV2_LIBRARY)
endif (RDF_FOUND)

# vim: tw=90 ts=8 sw=2 sts=2 et sta noai
