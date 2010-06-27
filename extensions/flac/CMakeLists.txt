#
# CMakeLists.txt - libFlac++ extension CMake file
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

configure_file(Flac.extinfo   ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_definitions(-DFlac_EXTENSION)

set(Flac_SRCS
    FlacExtension.cpp
    FlacBufferReader.cpp
)

set(Flac_MOC_HEADERS
    FlacExtension.h
    FlacBufferReader.h
)

qt4_wrap_cpp(Flac_MOC_SRCS ${Flac_MOC_HEADERS})

include_directories(.. ${COMMON_LIBS_INCLUDE_DIR})

add_library(Flac SHARED ${Flac_MOC_SRCS} ${Flac_SRCS})

target_link_libraries(Flac
    ${QT_LIBRARIES}
    aggregation
    extensionsystem
    Core
)

set_target_properties(Flac PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${EXTENSIONS_RPATH}"
)

set(INSTALL_DIR lib/unison/extensions)
install(FILES   Flac.extinfo  DESTINATION ${INSTALL_DIR})
install(TARGETS Flac LIBRARY  DESTINATION ${INSTALL_DIR})
