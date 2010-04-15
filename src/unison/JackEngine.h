/*
 * JackEngine.h
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

#ifndef UNISON_JACK_ENGINE_H
#define UNISON_JACK_ENGINE_H

#include <QVarLengthArray>

#include "unison/JackPort.h"

namespace Unison
{

class JackEngine
{
  public:
    // TODO-NOW: This temporary constructor just makes JackEngine behave as a
    // wrapper around an seperately manage jack_client. This is good for
    // testing but sucks overall.
    JackEngine (jack_client_t* client) :
      m_client( client )
    {}

    jack_client_t* client () const
    {
      return m_client;
    }

    JackPort* registerPort (QString name, PortDirection direction);

    int myPortCount () const;
    JackPort* myPort (int index) const;
    JackPort* myPort (QString name) const;

  private:
    jack_client_t* m_client;
    QVarLengthArray<JackPort*> m_myPorts;
};

} // Unison

#endif

// vim: et ts=8 sw=2 sts=2 noai
