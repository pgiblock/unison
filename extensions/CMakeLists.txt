#
# CMakeLists.txt - CMake for extensions
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

set(EXTENSIONS_RPATH ${CMAKE_INSTALL_PREFIX}/lib/unison/extensions)

add_subdirectory(core)
add_subdirectory(guicore)

if(BUILD_JACK_EXTENSION)
  add_subdirectory(jack)
endif(BUILD_JACK_EXTENSION)

if(BUILD_ALSA_EXTENSION)
  add_subdirectory(alsa)
endif(BUILD_ALSA_EXTENSION)

if(BUILD_FLAC_EXTENSION)
  add_subdirectory(flac)
endif(BUILD_FLAC_EXTENSION)

if(BUILD_LADSPA_EXTENSION)
  add_subdirectory(ladspa)
endif(BUILD_LADSPA_EXTENSION)

if(BUILD_LV2_EXTENSION)
  add_subdirectory(lv2)
endif(BUILD_LV2_EXTENSION)

if(BUILD_OGGVORBIS_EXTENSION)
  add_subdirectory(oggvorbis)
endif(BUILD_OGGVORBIS_EXTENSION)

if(BUILD_SNDFILE_EXTENSION)
  add_subdirectory(sndfile)
endif(BUILD_SNDFILE_EXTENSION)

# vim: tw=90 ts=8 sw=2 sts=2 et sta noai
