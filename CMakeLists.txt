#
# CMakeLists.txt - The main CMake file
#
# Copyright (c) 2010-2011 Paul Giblock <pgib/at/users.sourceforge.net>
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
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

include(DefineCMakeDefaults)
include(DefineCompilerFlags)
include(MacroLogFeature)

# TODO: use these to ensure the user has at least one backend supported.
# Since JACK is the only backend right now, we simply require JACK
set(BACKENDS_AVAILABLE "")
set(BACKENDS_ENABLED "")

## Options ########

option(WANT_FLAC_EXTENSION "Include FLAC file support extension" OFF)
option(WANT_JACK_EXTENSION "Include JACK (Jack Audio Connection Kit) backend extension" ON)
option(WANT_LADSPA_EXTENSION "Include LADSPA plugin support extension" OFF)
option(WANT_LV2_EXTENSION "Include LV2 plugin support extension" OFF)
option(WANT_OGGVORBIS_EXTENSION "Include OGG/Vorbis file support extension" OFF)
option(WANT_SNDFILE_EXTENSION "Include libsndfile support (WAV,AIFF,SND,VOC,etc..) extension" OFF)

option(COMPILE_TESTS "Compile unit-tests" OFF)

## Find required dependencies ########

find_package(Qt4 4.6.0 COMPONENTS QtCore QtGui QtXml QtNetwork) # REQUIRED)
macro_log_feature(QT4_FOUND "Qt4"
    "Needed for cross-platform data, UI, XML, and Network support"
    "http://qt.nokia.com/"
    TRUE "" "")

#find_package(Raul)
#macro_log_feature(Raul_FOUND "Raul"
#    "A set of utilities used for audio related processing"
#    "http://drobilla.net/software/raul/"
#    TRUE "" "")

find_package(Ingen)
macro_log_feature(Ingen_FOUND "Ingen"
    "The DSP engine used by Unison"
    "http://drobilla.net/software/ingen"
    TRUE "" "")

## Find dependencies based on features selected ########

if(WANT_FLAC_EXTENSION)
  find_package(Flac)
  macro_log_feature(FLAC_FOUND "Flac"
    "Needed for the Flac audio file extension."
    "http://flac.sourceforge.net/"
    FALSE "" "")
  #find_package(Flac++)
  #macro_log_feature( FLAC++_FOUND "Flac++"
  #  "Needed for the Flac support extension."
  #  "http://flac.sourceforge.net/"
  #  FALSE "" "")
endif(WANT_FLAC_EXTENSION)

if(WANT_JACK_EXTENSION)
  find_package(Jack)
  # JACK is technically optional, but it is required since it is the only
  # backend implementation at the current time
  macro_log_feature(JACK_FOUND "Jack"
      "Needed for the Jack (Jack Audio Connection Kit) backend extension."
      "http://jackaudio.org/"
      TRUE "" "")
endif(WANT_JACK_EXTENSION)

if(WANT_LV2_EXTENSION)
  find_package(Slv2)
  macro_log_feature(SLV2_FOUND "SLV2"
      "Needed to discover, load, and use LV2 plugins."
      "http://drobilla.net/docs/slv2/"
      FALSE "" "")
endif(WANT_LV2_EXTENSION)

if(WANT_OGGVORBIS_EXTENSION)
  find_package(OggVorbis)
  macro_log_feature(OGGVORBIS_FOUND "Ogg Vorbis"
    "Needed for the Ogg Vorbis audio file extention."
    "http://www.vorbis.com/"
    FALSE "" "")
endif(WANT_OGGVORBIS_EXTENSION)

if(WANT_SNDFILE_EXTENSION)
  find_package(Sndfile)
  macro_log_feature(SNDFILE_FOUND "Sndfile"
    "Needed for the libsndfile extension which supports many audio types."
    "http://www.mega-nerd.com/libsndfile/"
    FALSE "" "")
endif(WANT_SNDFILE_EXTENSION)

## Now, enable features based on which deps are found ########

if(WANT_FLAC_EXTENSION AND FLAC_FOUND) # AND FLAC++_FOUND)
  set(BUILD_FLAC_EXTENSION 1)
endif(WANT_FLAC_EXTENSION AND FLAC_FOUND) # AND FLAC++_FOUND)

if(WANT_JACK_EXTENSION AND JACK_FOUND)
  set(BUILD_JACK_EXTENSION 1)
endif(WANT_JACK_EXTENSION AND JACK_FOUND)

if(WANT_LADSPA_EXTENSION)
  set(BUILD_LADSPA_EXTENSION 1)
endif(WANT_LADSPA_EXTENSION)

if(WANT_LV2_EXTENSION AND SLV2_FOUND)
  set(BUILD_LV2_EXTENSION 1)
endif(WANT_LV2_EXTENSION AND SLV2_FOUND)

if(WANT_OGGVORBIS_EXTENSION AND OGGVORBIS_FOUND)
  set(BUILD_OGGVORBIS_EXTENSION 1)
endif(WANT_OGGVORBIS_EXTENSION AND OGGVORBIS_FOUND)

if(WANT_SNDFILE_EXTENSION AND SNDFILE_FOUND)
  set(BUILD_SNDFILE_EXTENSION 1)
endif(WANT_SNDFILE_EXTENSION AND SNDFILE_FOUND)

## Doxygen ########

set(DOXYFILE_LATEX "NO")
include(UseDoxygen OPTIONAL)

configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in
               ${CMAKE_BINARY_DIR}/Doxyfile)

## Define some stuff ########

include(${QT_USE_FILE})

add_definitions(-DQT_NO_KEYWORDS)

# Qt translations feedback
if(EXISTS "${QT_TRANSLATIONS_DIR}")
  message("-- Found Qt translations in ${QT_TRANSLATIONS_DIR}")
  add_definitions(-D'QT_TRANSLATIONS_DIR="${QT_TRANSLATIONS_DIR}"')
endif(EXISTS "${QT_TRANSLATIONS_DIR}")

if(NOT WIN32)
  string(REPLACE "-DQT_DLL" "" QT_DEFINITIONS "${QT_DEFINITIONS}")
endif(NOT WIN32)

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# the RPATH to be used when installing TODO: Improve
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/unison" "${CMAKE_INSTALL_PREFIX}/lib")

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

#Directories
add_subdirectory(libs)
add_subdirectory(extensions)
add_subdirectory(plugins)
add_subdirectory(unisonstudio)

## Config Summary ########
macro_display_feature_log()

# vim: tw=90 ts=8 sw=2 sts=2 et sta noai
