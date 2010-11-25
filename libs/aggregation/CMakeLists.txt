#
# CMakeLists.txt - Aggregation CMake file
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

set(AGGREGATION_SRCS
    aggregate.cpp
)

set(AGGREGATION_HEADERS
    Aggregate.hpp
    aggregate.hpp
    aggregation_global.hpp
)

set(AGGREGATION_MOC_HEADERS
    aggregate.hpp
)

qt4_wrap_cpp(AGGREGATION_MOC_SRCS ${AGGREGATION_MOC_HEADERS})

add_library(aggregation SHARED ${AGGREGATION_MOC_SRCS} ${AGGREGATION_SRCS})

target_link_libraries(aggregation
    ${QT_LIBRARIES}
)

install(TARGETS aggregation LIBRARY
        DESTINATION lib/unison)

install(FILES ${AGGREGATION_HEADERS}
        DESTINATION include/unison/aggregation)
