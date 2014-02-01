#
# CMakeLists.txt - libvorbisfile extension CMake file
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

configure_file(OggVorbis.extinfo   ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_definitions(-DOGGVORBIS_EXTENSION)

set(OGGVORBIS_SRCS
    OggVorbisExtension.cpp
    OggVorbisBufferReader.cpp
)

set(OGGVORBIS_MOC_HEADERS
    OggVorbisExtension.hpp
    OggVorbisBufferReader.hpp
)

qt4_wrap_cpp(OGGVORBIS_MOC_SRCS ${OGGVORBIS_MOC_HEADERS})

include_directories(.. 
    ${COMMON_LIBS_INCLUDE_DIR}
    ${OGGVORBIS_INCLUDE_DIR}
)

add_library(OggVorbis SHARED ${OGGVORBIS_MOC_SRCS} 
    ${OGGVORBIS_SRCS})

target_link_libraries(OggVorbis
    ${QT_LIBRARIES}
    ${OGGVORBIS_LIBRARIES}
    aggregation
    extensionsystem
    Core
)

set_target_properties(OggVorbis PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${EXTENSIONS_RPATH}"
)

set(INSTALL_DIR lib/unison/extensions)
install(FILES   OggVorbis.extinfo  DESTINATION ${INSTALL_DIR})
install(TARGETS OggVorbis LIBRARY  DESTINATION ${INSTALL_DIR})
