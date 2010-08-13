#
# CMakeLists.txt - LADSPA Plugin CMake file
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

configure_file(Ladspa.extinfo   ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_definitions(-DLADSPA_EXTENSION)

set(UNISON_BUFFER_LENGTH "1024" CACHE STRING
    "Audio buffer length. A work-around until runtime change is supported.")


set(LADSPA_SRCS
    LadspaExtension.cpp
    #LadspaPlugin.cpp
    LadspaPluginProvider.cpp
    #LadspaPort.cpp
)

set(LADSPA_MOC_HEADERS
    LadspaExtension.h
    LadspaPluginProvider.h
)

qt4_wrap_cpp(LADSPA_MOC_SRCS ${LADSPA_MOC_HEADERS})

include_directories(.. ${COMMON_LIBS_INCLUDE_DIR} ${PRG_INCLUDE_DIR})

add_library(Ladspa SHARED ${LADSPA_MOC_SRCS} ${LADSPA_SRCS})

target_link_libraries(Ladspa
    ${QT_LIBRARIES}
    aggregation
    extensionsystem
    unison
    Core
)

set_target_properties(Ladspa PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${EXTENSIONS_RPATH}"
)

set(INSTALL_DIR lib/unison/extensions)
install(FILES   Ladspa.extinfo  DESTINATION ${INSTALL_DIR})
install(TARGETS Ladspa LIBRARY  DESTINATION ${INSTALL_DIR})
