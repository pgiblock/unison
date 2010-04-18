# - Try to find Slv2
# Once done this will define
#
#  SLV2_FOUND - system has Slv2
#  SLV2_INCLUDE_DIRS - the Slv2 include directory
#  SLV2_LIBRARIES - Link these to use Slv2
#  SLV2_DEFINITIONS - Compiler switches required for using Slv2
#
#  Copyright (c) 2010 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (SLV2_LIBRARIES AND SLV2_INCLUDE_DIRS)
  # in cache already
  set(SLV2_FOUND TRUE)
else (SLV2_LIBRARIES AND SLV2_INCLUDE_DIRS)
  include(FindLv2)
  include(FindRedlandRdf)

  if (RDF_FOUND AND LV2_FOUND)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_SLV2 slv2)
    endif (PKG_CONFIG_FOUND)

    find_path(SLV2_INCLUDE_DIR
      NAMES
        slv2/slv2.h
      PATHS
        ${_SLV2_INCLUDEDIR}
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
      PATH_SUFFIXES
        slv2
    )

    find_library(SLV2_LIBRARY
      NAMES
        slv2
      PATHS
        ${_SLV2_LIBDIR}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
    )

    set(SLV2_INCLUDE_DIRS
      ${SLV2_INCLUDE_DIR}
    )

    if (SLV2_LIBRARY)
      set(SLV2_LIBRARIES
          ${SLV2_LIBRARIES}
          ${SLV2_LIBRARY}
      )
    endif (SLV2_LIBRARY)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Slv2 DEFAULT_MSG SLV2_LIBRARIES SLV2_INCLUDE_DIRS)

    # show the SLV2_INCLUDE_DIRS and SLV2_LIBRARIES variables only in the advanced view
    mark_as_advanced(SLV2_INCLUDE_DIRS SLV2_LIBRARIES)
  endif (RDF_FOUND AND LV2_FOUND)

endif (SLV2_LIBRARIES AND SLV2_INCLUDE_DIRS)

