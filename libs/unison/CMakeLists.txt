#
# CMakeLists.txt - The sources CMake file
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

set(UNISON_SRCS
    CompositeProcessor.cpp
    PooledBufferProvider.cpp
    Port.cpp
    Processor.cpp
)

set(UNISON_INCLUDES
    AudioBuffer.h
    Buffer.h
    BufferProvider.h
    CompositeProcessor.h
    ControlBuffer.h
    Node.h
    Plugin.h
    PooledBufferProvider.h
    Port.h
    ProcessingContext.h
    Processor.h
    types.h
)

qt4_wrap_cpp(UNISON_MOC_SRCS ${UNISON_MOC_HEADERS})

include_directories(.. ${PRG_INCLUDE_DIR})

add_definitions(-D'UNISON_BUFFER_LENGTH=${UNISON_BUFFER_LENGTH}')
add_library(unison SHARED ${UNISON_MOC_SRCS} ${UNISON_SRCS})

target_link_libraries(unison
    ${QT_LIBRARIES}
)

install(TARGETS unison LIBRARY
        DESTINATION lib/unison)

install(FILES ${UNISON_INCLUDES}
        DESTINATION include/unison/)
