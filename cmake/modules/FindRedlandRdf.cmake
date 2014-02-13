# - Try to find Rdf
# Once done this will define
#
#  RDF_FOUND - system has Rdf
#  RDF_INCLUDE_DIRS - the Rdf include directory
#  RDF_LIBRARIES - Link these to use Rdf
#  RDF_DEFINITIONS - Compiler switches required for using Rdf
#
#  Copyright (c) 2010 Paul Giblock <pgib@users.sourceforge.net>
#  Copyright (c) 2014 Lukas W <lukaswhl@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

find_package(PkgConfig)
# XXX: Redland depends on raptor and rasqal. Their include dirs are provided
# by pkg-config. When pkg-config is not available, building might fail.
# Additional FindRaptor.cmake and FindRasqal.cmake scripts may be needed?
if (PKG_CONFIG_FOUND)
  pkg_check_modules(PC_RDF redland)
endif (PKG_CONFIG_FOUND)

find_path(RDF_INCLUDE_DIR
    HINTS ${PC_RDF_INCLUDEDIR}
    NAMES rdf_init.h
)

find_library(RDF_LIBRARY
    HINTS ${PC_RDF_LIBDIR}
    NAMES rdf
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Rdf DEFAULT_MSG RDF_LIBRARY RDF_INCLUDE_DIR)

set(RDF_INCLUDE_DIRS
    ${PC_RDF_INCLUDE_DIRS}
    ${RDF_INCLUDE_DIR}
)

set(RDF_LIBRARIES
    ${RDF_LIBRARIES}
    ${RDF_LIBRARY}
)

# show the RDF_INCLUDE_DIR and RDF_LIBRARY variables only in the advanced view
mark_as_advanced(RDF_INCLUDE_DIR RDF_LIBRARY)

# vim: tw=90 ts=8 sw=2 sts=2 et sta noai
