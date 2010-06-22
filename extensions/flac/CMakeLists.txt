#
# CMakeLists.txt - libflac++ extension CMake file
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

configure_file(FLACPP.extinfo   ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_definitions(-DFLACPP_EXTENSION)

set(FLACPP_SRCS
    FLACPPExtension.cpp
    FLACPPBufferReader.cpp
)

set(FLACPP_MOC_HEADERS
    FLACPPExtension.h
    FLACPPBufferReader.h
)

qt4_wrap_cpp(FLACPP_MOC_SRCS ${FLACPP_MOC_HEADERS})

include_directories(.. ${COMMON_LIBS_INCLUDE_DIR})

add_library(FLACPP SHARED ${FLACPP_MOC_SRCS} ${FLACPP_SRCS})

target_link_libraries(FLACPP
    ${QT_LIBRARIES}
    aggregation
    extensionsystem
    Core
)

set_target_properties(FLACPP PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${EXTENSIONS_RPATH}"
)

set(INSTALL_DIR lib/unison/extensions)
install(FILES   FLACPP.extinfo  DESTINATION ${INSTALL_DIR})
install(TARGETS FLACPP LIBRARY  DESTINATION ${INSTALL_DIR})
