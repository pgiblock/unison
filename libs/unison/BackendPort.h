/*
 * JackPort.h
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

#ifndef UNISON_BACKEND_PORT_H
#define UNISON_BACKEND_PORT_H

#include <unison/Port.h>
#include <unison/ProcessingContext.h>

namespace Unison {

class Backend;

class BackendPort : public Unison::Port
{
  public:

    Unison::Node* parent () const
    {
      return NULL;
    }

    // TODO: More functions, like whether it belongs to unison, another app, or
    // the system etc..  Probably need a sort of Id or Path to the port.
};

} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
