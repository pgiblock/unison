#
# CMakeLists.txt - QIngen CMake file
#
# Copyright (c) 2011 Paul Giblock <pgib/at/users.sourceforge.net>
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

set(QINGEN_SRCS
  src/World.cpp
)

set(QINGEN_MOC_HEADERS
  qingen/QObjectClientInterface.hpp
)

set(QINGEN_INCLUDES
  qingen/QObjectClientInterface.hpp
  qingen/World.hpp
)

qt4_wrap_cpp(QINGEN_MOC_SRCS ${QINGEN_MOC_HEADERS})

include_directories(
  ${Ingen_module_INCLUDE_DIRS}
  ${Ingen_shared_INCLUDE_DIRS}
  ${Raul_INCLUDE_DIRS}
  qingen
)

add_library(qingen SHARED ${QINGEN_SRCS} ${QINGEN_MOC_SRCS})

target_link_libraries(qingen
  ${QT_LIBRARIES}
  ${Ingen_module_LIBRARIES}
  ${Ingen_shared_LIBRARIES}
  ${Raul_LIBRARIES}
)

install(TARGETS qingen LIBRARY      DESTINATION lib/)
install(FILES   ${QINGEN_INCLUDES}  DESTINATION include/qingen/)

# vim: tw=90 ts=8 sw=2 sts=4 et sta
