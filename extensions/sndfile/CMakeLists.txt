#
# CMakeLists.txt - libsndfile extension CMake file
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

configure_file(SndFile.extinfo   ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_definitions(-DSNDFILE_EXTENSION)

set(SNDFILE_SRCS
    SndFileExtension.cpp
    SndFileBufferReader.cpp
)

set(SNDFILE_MOC_HEADERS
    SndFileExtension.hpp
    SndFileBufferReader.hpp
)

qt4_wrap_cpp(SNDFILE_MOC_SRCS ${SNDFILE_MOC_HEADERS})

include_directories(.. ${COMMON_LIBS_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIR})

add_library(SndFile SHARED ${SNDFILE_MOC_SRCS} ${SNDFILE_SRCS})

target_link_libraries(SndFile
    ${QT_LIBRARIES}
    ${SNDFILE_LIBRARIES}
    aggregation
    extensionsystem
    Core
)

set_target_properties(SndFile PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${EXTENSIONS_RPATH}"
)

set(INSTALL_DIR lib/unison/extensions)
install(FILES   SndFile.extinfo  DESTINATION ${INSTALL_DIR})
install(TARGETS SndFile LIBRARY  DESTINATION ${INSTALL_DIR})
