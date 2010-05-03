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

add_subdirectory(unison) # Nothing really in here

SET(UNISON_BUFFER_LENGTH "1024" CACHE STRING
    "Audio buffer length. A work-around until runtime change is supported.")

#file(GLOB UNISON_SRCS *.cpp)
set(UNISON_SRCS
    CompositeProcessor.cpp
    JackBufferProvider.cpp
    JackEngine.cpp
    JackPort.cpp
    Lv2Plugin.cpp
    Lv2Port.cpp
    PluginManager.cpp
    PooledBufferProvider.cpp
    Port.cpp
    Processor.cpp
    Session.cpp
)

#TODO: Move this back into the unison/CMakeLists if there is a good way to
#      propagate a variable to the parent.
set(UNISON_MOC_HEADERS
    unison/JackEngine.h
)

qt4_wrap_cpp(UNISON_MOC_SRCS ${UNISON_MOC_HEADERS})

include_directories(${PRG_INCLUDE_DIR})

add_definitions(-D'UNISON_BUFFER_LENGTH=${UNISON_BUFFER_LENGTH}')
add_library(unison SHARED ${UNISON_MOC_SRCS} ${UNISON_SRCS})

target_link_libraries(unison
    ${JACK_LIBRARIES}
    ${QT_LIBRARIES}
    ${RDF_LIBRARY}
    ${SLV2_LIBRARY}
)
