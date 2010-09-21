/*
 * BackendPort.h
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

#ifndef UNISON_BACKEND_PORT_H_
#define UNISON_BACKEND_PORT_H_

#include "Port.h"

namespace Unison {

  class Backend;

/**
 * BackendPort is a Port that provides an interface for other Ports to utilize the
 * Backend.  This is accomplished by implementing the Port interface.  A However, a
 * BackendPort utilizes resources of a Backend.  Therefore, we must register a BackendPort
 * from the Backend in order to create one.
 */
class BackendPort : public Port
{
  public:

    /**
     * Backend ports do not participate in the graph's hierachial stucture.
     * Although, it may make sense to do so at some point in the future.
     */
    Node* parent () const
    {
      return NULL;
    }

    // TODO: More functions, like whether it belongs to unison, another app, or
    // the system etc..  Probably need a sort of Id or Path to the port.
};

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
