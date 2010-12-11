#
# CMakeLists.txt - ALSA Plugin CMake file
#
# Copyright (c) 2010 Andrew Kelley <superjoe30+unison@gmail.com>
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

configure_file(Alsa.extinfo   ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_definitions(-DALSA_EXTENSION)

set(ALSA_SRCS
  AlsaBackend.cpp
  AlsaExtension.cpp
  AlsaPort.cpp
)

set(ALSA_MOC_HEADERS
  AlsaBackend.hpp
  AlsaExtension.hpp
  AlsaPort.hpp
)

qt4_wrap_cpp(ALSA_MOC_SRCS ${ALSA_MOC_HEADERS})

include_directories(.. ${COMMON_LIBS_INCLUDE_DIR} ${PRG_INCLUDE_DIR})

add_library(Alsa SHARED ${ALSA_MOC_SRCS} ${ALSA_SRCS})

target_link_libraries(Alsa
  ${QT_LIBRARIES}
  ${ALSA_LIBRARIES}
  aggregation
  extensionsystem
  unison
  Core
)

set_target_properties(Alsa PROPERTIES
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${EXTENSIONS_RPATH}"
)

set(INSTALL_DIR lib/unison/extensions)
install(FILES   Alsa.extinfo  DESTINATION ${INSTALL_DIR})
install(TARGETS Alsa    LIBRARY  DESTINATION ${INSTALL_DIR})

# vim: tw=90 ts=8 sw=2 sts=2 et sta noai
