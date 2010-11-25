/*
 * Node.cpp
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

#include "Node.hpp"
#include "Patch.hpp"

namespace Unison {

Patch* Node::parentPatch () const
{
  Node* n = parent();

  // While non-patch parents exist
  while (n != NULL) {
    // Is this a patch?
    Patch* p = dynamic_cast<Patch*>(n);
    if (p) {
      return p;
    }
    n = n->parent();
  }

  return NULL;
}

} // Unison

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
