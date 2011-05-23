#
# CMakeLists.txt - Main unisonstudio executable CMake file
#
# Copyright (c) 2010-2011 Paul Giblock <pgib/at/users.sourceforge.net>
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

set(UNISONSTUDIO_SRCS
  main.cpp
)

set(UNISONSTUDIO_MOC_HEADERS
)

qt4_wrap_cpp(UNISONSTUDIO_MOC_SRCS ${UNISONSTUDIO_MOC_HEADERS})

# Extension system stuff
set(AGGREGATION_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/libs/aggregation)
set(EXTSYSTEM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/libs/extensionsystem)
set(EXTSYSTEM_INCLUDE_DIRS ${AGGREGATION_INCLUDE_DIR} ${EXTSYSTEM_INCLUDE_DIR})
set(EXTSYSTEM_LIBRARIES extensionsystem)

include_directories(
  ${CMAKE_SOURCE_DIR}/extensions
  ${EXTSYSTEM_INCLUDE_DIRS}
  ${Ingen_client_qt_INCLUDE_DIRS}
  ${IngenConfig_INCLUDE_DIR}
  ${QT_INCLUDE_DIRS}
)

add_executable(unisonstudio
  ${UNISONSTUDIO_SRCS}
  ${UNISONSTUDIO_MOC_SRCS}
)

target_link_libraries(unisonstudio
  ${EXTSYSTEM_LIBRARIES}
  ${Ingen_client_qt_LIBRARIES}
  ${QT_LIBRARIES}
)

install(TARGETS unisonstudio RUNTIME
        DESTINATION bin/)


# vim: tw=90 ts=8 sw=2 sts=4 et sta
