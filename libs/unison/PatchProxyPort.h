/*
 * PatchProxyPort.h
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


#ifndef UNISON_PATCH_PROXY_PORT_H_
#define UNISON_PATCH_PROXY_PORT_H_

#include "Port.h"

namespace Unison {

  class Patch;

  namespace Internal {

/**
 * Internal class providing a proxy to a port internal to a Patch.  Patch provides support
 * for registerPorts.  This allows for Patch to expose a Port of a child as a Port of the
 * Patch.
 */
class PatchProxyPort : public Port
{
  protected:
    /**
     * Constructs a Port for a Patch.  Called by
     * Patch itself.
     * @param patch     The parent processor
     * @param port      The port to proxy* /
    PatchProxyPort (Patch* patch, Port* port) :
      m_port(port),
      m_patch(patch)
    {}

    ~PatchProxyPort ()
    {}

    QString name () const
    {
      return m_port->name();
    }

    PortType type () const
    {
      return m_port->type();
    }

    PortDirection direction () const
    {
      return m_port->direction();
    }

    float value () const
    {
      return m_port->value();
    }

    void setValue (float value)
    {
      m_port->setValue(value);
    }

    float defaultValue () const
    {
      return m_port->defaultValue();
    }

    bool isBounded () const
    {
      return m_port->isBounded();
    }

    float minimum () const
    {
      return m_port->minimum();
    }

    float maximum () const
    {
      return m_port->maximum();
    }

    bool isToggled () const
    {
      return m_port->isToggled();
    }

    const QSet<Node*  const> interfacedNodes () const
    {
      QSet<Node*  const> p;
      p.insert(m_patch);
      return p;
    }

    void connectToBuffer ()
    {
      // TODO: Might need more logic here
      m_port->connectToBuffer();
    }

  private:
    Port* m_port;       ///< The proxied port
    Patch* m_patch; ///< The processor owning this port

  friend class Patch;
};

  } // Internal

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
