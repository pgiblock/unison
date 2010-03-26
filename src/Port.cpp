/*
 * Port.cpp
 *
 * Copyright (c) 2010 Paul Giblock <pgib/at/users.sourceforge.net>
 *
 * This file is part of Unison - http://unison.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include "unison/Port.h"

namespace Unison {

  void Port::connect (Port* other) {
    // TODO: Check for existing connection and cycles!!!
    connectedPorts += other;
    other->connectedPorts += this;
  }

  void Port::disconnect (Port* other) {
    connectedPorts -= other;
    other->connectedPorts -= this;
  }

  bool Port::isConnected (Port* other) {
    return connectedPorts.contains( other );
  }

} // Unison

// vim: et ts=8 sw=2 sts=2 noai
