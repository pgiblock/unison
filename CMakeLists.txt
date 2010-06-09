#
# CMakeLists.txt - The main CMake file
#
# Copyright (c) 2010 Paul Giblock <pgib/at/users.sourceforge.net>
#
# This file is part of Unison - http://unison.sourceforge.net/
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program (see COPYING); if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA.
#


cmake_minimum_required(VERSION 2.6)

project(unison)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

include(DefineCMakeDefaults)
include(DefineCompilerFlags)

#include(CheckIncludeFiles)
#include(CheckLibraryExists)
include(FindPkgConfig)
include(FindJack)
include(FindSlv2)

find_package(Qt4 4.5.0 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED )
include(${QT_USE_FILE})

# Qt translations feedback
if(EXISTS "${QT_TRANSLATIONS_DIR}")
  message("-- Found Qt translations in ${QT_TRANSLATIONS_DIR}")
  add_definitions(-D'QT_TRANSLATIONS_DIR="${QT_TRANSLATIONS_DIR}"')
endif(EXISTS "${QT_TRANSLATIONS_DIR}")

if(NOT WIN32)
  string(REPLACE "-DQT_DLL" "" QT_DEFINITIONS "${QT_DEFINITIONS}")
endif(NOT WIN32)


if(JACK_FOUND)
  set(STATUS_JACK "OK")
else(JACK_FOUND)
  set(STATUS_JACK "not found: please install libjack1.9.4-dev (or similiar)")
endif(JACK_FOUND)


if(LV2_FOUND)
  set(STATUS_LV2 "OK")
else(LV2_FOUND)
  # TODO: Fallback to a distributed copy
  set(STATUS_LV2 "LV2 devel packages not found: need lv2core-devel "
                 "(or similar)")
endif(LV2_FOUND)


if(SLV2_FOUND)
  # TODO: Fallback to a distributed copy
  set(STATUS_SLV2 "SLV2 (Host) devel packages not found: need libslv2-dev, "
                  "slv2-devel (or similiar)")
else(SLV2_FOUND)
  set(STATUS_SLV2 "OK")
endif(SLV2_FOUND)


if(RDF_FOUND)
  # TODO: Fallback to a distributed copy
  set(STATUS_RDF "LV2 (RDF) devel packages not found : need librdf0-dev, "
                 "redland-devel or similiar")
else(RDF_FOUND)
  set(STATUS_RDF "OK")
endif(RDF_FOUND)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/unison")

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

#Internal location, TODO: Don't hardcode platform-specific paths!!
set(PRG_INCLUDE_DIR    ${CMAKE_SOURCE_DIR}/libs/prg)
set(UNISON_INCLUDE_DIR ) #${CMAKE_SOURCE_DIR}/libs/unison)
set(COMMON_LIBS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs)

#Directories
add_subdirectory(libs)
add_subdirectory(extensions)
add_subdirectory(plugins)
add_subdirectory(unisonstudio)
