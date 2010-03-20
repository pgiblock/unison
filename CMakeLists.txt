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


CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(unison)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
INCLUDE(FindPkgConfig)

FILE(GLOB_RECURSE unison_includes ${CMAKE_SOURCE_DIR}/*.h)
FILE(GLOB_RECURSE unison_srcs     ${CMAKE_SOURCE_DIR}/*.cpp)


# check for Qt4
SET(QT_MIN_VERSION "4.5.0")
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTNETWORK 1)
SET(QT_USE_QTXML 1)
EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_TRANSLATIONS" OUTPUT_VARIABLE QT_TRANSLATIONS_DIR)
IF(EXISTS "${QT_TRANSLATIONS_DIR}")
	MESSAGE("-- Found Qt translations in ${QT_TRANSLATIONS_DIR}")
	ADD_DEFINITIONS(-D'QT_TRANSLATIONS_DIR="${QT_TRANSLATIONS_DIR}"')
ENDIF(EXISTS "${QT_TRANSLATIONS_DIR}")
IF(NOT WIN32)
	STRING(REPLACE "-DQT_DLL" "" QT_DEFINITIONS "${QT_DEFINITIONS}")
ENDIF(NOT WIN32)
INCLUDE(${QT_USE_FILE})


# check for JACK (Relying on >=1.9.4 as I've only tested with JACK2)
PKG_CHECK_MODULES(JACK jack>=1.9.4)
IF(JACK_FOUND)
	SET(STATUS_JACK "OK")
ELSE(JACK_FOUND)
	SET(STATUS_JACK "not found: please install libjack1.9.4-dev (or similiar)")
ENDIF(JACK_FOUND)


# check for LV2 (and transitively, RDF too)
CHECK_INCLUDE_FILES(lv2.h HAVE_LV2_H)
# compatibility for slv2 <= 0.6.2
CHECK_INCLUDE_FILES(slv2/scalepoints.h HAVE_SLV2_SCALEPOINTS_H)
CHECK_LIBRARY_EXISTS(slv2 slv2_world_new "" HAVE_LIBSLV2)

# MESSAGE("SLV2 Exists    : '${LMMS_HAVE_SLV2}'")
# MESSAGE("SLV2 Library   : '${SLV2_LIBRARY}'")   #RIGHT
# MESSAGE("SLV2 Libraries : '${SLV2_LIBRARIES}'") # WRONG

CHECK_LIBRARY_EXISTS(rdf librdf_new_world "" HAVE_LIBRDF)


IF(HAVE_LV2_H)
    SET(STATUS_LV2 "OK")
ELSE(HAVE_LV2_H)
    # Maybe we should include a fallback copy of lv2 and slv2.
    SET(STATUS_LV2 "LV2 devel packages not found: need lv2core-devel "
        "(or similar)")
ENDIF(HAVE_LV2_H)

IF(HAVE_LIBSLV2)
    SET(STATUS_SLV2 "SLV2 (Host) devel packages not found: need libslv2-dev, "
        "slv2-devel (or similiar)")
ELSE(HAVE_LIBSLV2)
    SET(STATUS_SLV2 "OK")
ENDIF(HAVE_LIBSLV2)

IF(HAVE_LIBRDF)
    SET(STATUS_RDF "LV2 (RDF) devel packages not found : need librdf0-dev, redland-devel or similiar")
ELSE(HAVE_LIBRDF)
    SET(STATUS_RDF "OK")
ENDIF(HAVE_LIBRDF)

IF("${STATUS_LV2}" STREQUAL "OK")
    FIND_LIBRARY(SLV2_LIBRARY NAMES slv2 PATH /usr/lib /usr/local/lib)
    FIND_LIBRARY(RDF_LIBRARY NAMES rdf PATH /usr/lib /usr/local/lib)
ENDIF("${STATUS_LV2}" STREQUAL "OK")



# Compiler flags
SET(CMAKE_C_FLAGS "-g -Wall -Wextra -ansi -ftree-vectorize ${CMAKE_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "-g -fno-exceptions -Wall -Wextra -ansi -ftree-vectorize ${CMAKE_CXX_FLAGS}")

#Directories
INCLUDE_DIRECTORIES(src)
ADD_SUBDIRECTORY(src)

#Pre-processing
QT4_AUTOMOC(${unison_srcs})

#linking
LINK_LIBRARIES(${JACK_LIBRARY} ${QT_LIBRARIES} ${RDF_LIBRARY} ${SLV2_LIBRARY})

#Output
ADD_EXECUTABLE(unisonstudio ${unison_srcs} ${unison_includes})


