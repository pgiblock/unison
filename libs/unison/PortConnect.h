/*
 * PortConnect.h
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


#ifndef UNISON_PORT_CONNECT_H_
#define UNISON_PORT_CONNECT_H_

#include "Command.h"
#include "Patch.h"

namespace Unison {
  
  namespace Internal {

class PortConnect : public Command
{
  public:
    PortConnect (Port *port1, Port *port2);
    void preExecute ();
    void execute (ProcessingContext &context);
    void postExecute ();
  private:
    Port *m_producer, *m_consumer;
    Patch *m_patch;
    QList<Patch::CompiledProcessor> *m_compiled;
};

  } // Internal
} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
