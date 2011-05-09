# - Try to find Ingen
#
#  Ingen_FOUND                - system has Ingen
#  Ingen_INCLUDE_DIRS         - the Ingen interface include directories
#  Ingen_shared_INCLUDE_DIRS  - the Ingen shared include directories
#  Ingen_shared_LIBRARIES     - the Ingen shared libraries
#  Ingen_client_INCLUDE_DIRS  - the Ingen client include directories
#  Ingen_client_LIBRARIES     - the Ingen shared libraries
#  Ingen_server _INCLUDE_DIRS - the Ingen server  include directories
#  Ingen_server _LIBRARIES    - the Ingen shared libraries
#  Ingen_serialisation_INCLUDE_DIRS - the Ingen serialisation include directories
#  Ingen_serialisation_LIBRARIES - the Ingen shared libraries
#
#  Copyright (c) 2011 Paul Giblock <pgib@users.sourceforge.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(LibFindMacros)

libfind_package(Ingen Boost)
libfind_package(Ingen Glibmm)
libfind_package(Ingen LILV)
libfind_package(Ingen Raul)
libfind_package(Ingen Suil)

# Ingen API

find_path(Ingen_INCLUDE_DIR
  NAMES ingen/CommonInterface.hpp
  PATH_SUFFIXES ingen
)

set(Ingen_PROCESS_INCLUDES
  Ingen_INCLUDE_DIR
  Raul_INCLUDE_DIRS
)
set(Ingen_PROCESS_LIBS
  Raul_LIBRARIES
)
libfind_process(Ingen)

# Ingen Shared

find_library(Ingen_shared_LIBRARY
  NAMES ingen_shared
)

set(Ingen_shared_PROCESS_INCLUDES
  Ingen_INCLUDE_DIR
  Boost_INCLUDE_DIRS
  Glibmm_INCLUDE_DIRS
  Raul_INCLUDE_DIRS
)
set(Ingen_shared_PROCESS_LIBS
  Ingen_shared_LIBRARY
  Boost_LIBRARIES
  Glibmm_LIBRARIES
  Raul_LIBRARIES
)
libfind_process(Ingen_shared)

# Ingen Client

find_library(Ingen_client_LIBRARY
  NAMES ingen_client
)

set(Ingen_client_PROCESS_INCLUDES
  Ingen_INCLUDE_DIR
  # ${Ingen_INCLUDE_DIR}/shared # Headers reference shared without directory
  Boost_INCLUDE_DIRS
  Glibmm_INCLUDE_DIRS
  LILV_INCLUDE_DIRS
  Raul_INCLUDE_DIRS
  Sord_INCLUDE_DIRS
  Suil_INCLUDE_DIRS
)
set(Ingen_client_PROCESS_LIBS
  Ingen_client_LIBRARY
  # Ingen_shared_LIBRARY ??
  Boost_LIBRARIES
  Glibmm_LIBRARIES
  LILV_LIBRARIES
  Raul_LIBRARIES
  Sord_LIBRARIES
  Suil_LIBRARIES
)
libfind_process(Ingen_client)

# Ingen Server

find_library(Ingen_server_LIBRARY
  NAMES ingen_server
)

set(Ingen_server_PROCESS_INCLUDES
  Ingen_INCLUDE_DIR
  # ${Ingen_INCLUDE_DIR}/shared # Headers reference shared without directory
  Boost_INCLUDE_DIRS
  Glibmm_INCLUDE_DIRS
  Raul_INCLUDE_DIRS
)
set(Ingen_server_PROCESS_LIBS
  Ingen_server_LIBRARY
  # Ingen_shared_LIBRARY ??
  Boost_LIBRARIES
  Glibmm_LIBRARIES
  Raul_LIBRARIES
)
libfind_process(Ingen_server)

# Ingen Serialisation

find_library(Ingen_serialisation_LIBRARY
  NAMES ingen_serialisation
)

set(Ingen_serialisation_PROCESS_INCLUDES
  Ingen_INCLUDE_DIR
  # ${Ingen_INCLUDE_DIR}/shared # Headers reference shared without directory
  Glibmm_INCLUDE_DIRS
  Raul_INCLUDE_DIRS
  Sord_INCLUDE_DIRS
)
set(Ingen_serialisation_PROCESS_LIBS
  Ingen_serialisation_LIBRARY
  # Ingen_shared_LIBRARY ??
  Glibmm_LIBRARIES
  Raul_LIBRARIES
  Sord_LIBRARIES
)
libfind_process(Ingen_serialisation)

# vim: tw=90 ts=8 sw=2 sts=4 et sta
