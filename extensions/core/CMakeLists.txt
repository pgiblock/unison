#
# CMakeLists.txt - Core Plugin CMake file
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

configure_file(Core.extinfo   ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_definitions(-DCORE_EXTENSION)

set(CORE_SRCS
    CoreExtension.cpp
    Engine.cpp
    PluginManager.cpp
    # Might belongs in new extension:
    FxLine.cpp
    # Silly demo
    StupidSamplerDemo.cpp
)

set(CORE_MOC_HEADERS
    CoreExtension.h
    Engine.h
    IBackendProvider.h
    IPluginProvider.h
    ISampleBufferReader.h
)

qt4_wrap_cpp(CORE_MOC_SRCS ${CORE_MOC_HEADERS})

include_directories(
    ${COMMON_LIBS_INCLUDE_DIR}
    ${PRG_INCLUDE_DIR}
    )

add_library(Core SHARED ${CORE_MOC_SRCS} ${CORE_SRCS})

target_link_libraries(Core
    ${QT_LIBRARIES}
    aggregation
    extensionsystem
    unison
)

set(INSTALL_DIR lib/unison/extensions)
install(FILES   Core.extinfo DESTINATION ${INSTALL_DIR})
install(TARGETS Core    LIBRARY DESTINATION ${INSTALL_DIR})
