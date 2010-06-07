#
# CMakeLists.txt - Gui-Core Plugin CMake file
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

#set(LFO_CONTROLLER_SRCS
#    LfoController.c
#)

configure_file(GuiCore.extinfo   ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set(GUICORE_SRCS
    GuiCoreExtension.cpp
)

set(GUICORE_MOC_HEADERS
    GuiCoreExtension.h
)

qt4_wrap_cpp(GUICORE_MOC_SRCS ${GUICORE_MOC_HEADERS})

include_directories(${COMMON_LIBS_INCLUDE_DIR})

add_library(GuiCore SHARED ${GUICORE_MOC_SRCS} ${GUICORE_SRCS})

target_link_libraries(GuiCore
    ${QT_LIBRARIES}
    aggregation
    extensionsystem
)

set(INSTALL_DIR lib/unison/extensions)
install(FILES   GuiCore.extinfo  DESTINATION ${INSTALL_DIR})
install(TARGETS GuiCore    LIBRARY  DESTINATION ${INSTALL_DIR})
