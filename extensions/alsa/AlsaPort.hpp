/*
 * AlsaPort.hpp
 *
 * Copyright (c) 2010 Andrew Kelley <superjoe30+unison@gmail.com>
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

#ifndef UNISON_ALSA_PORT_H
#define UNISON_ALSA_PORT_H

#include <unison/BackendPort.hpp>
#include <unison/ProcessingContext.hpp>

namespace Unison {
  class BufferProvider;
}

namespace Alsa {
  namespace Internal {

class AlsaBackend;

class AlsaPort : public Unison::BackendPort
{
  public:
    AlsaPort(AlsaBackend& backend, const QString &name, Unison::PortDirection direction);

    bool registerPort();

    bool isRegistered() const
    {
      return true;
    }

    Unison::Node* parent() const;

    AlsaBackend& backend() const
    {
      return m_backend;
    }

    QString id() const
    {
      return m_id;
    }

    QString name() const
    {
      return m_id;
    }

    Unison::PortDirection direction() const
    {
      return m_direction;
    }

    Unison::PortType type() const
    {
      return Unison::AudioPort;
    }

    float value() const
    {
      return 0.0f;
    }

    void setValue(float)
    {
    }

    float defaultValue() const
    {
      return 0.0f;
    }

    bool isBounded() const
    {
      return false;
    }

    float minimum() const
    {
      return 0.0f;
    }

    float maximum() const
    {
      return 0.0f;
    }

    bool isToggled() const
    {
      return false;
    }

    const QSet<Unison::Node* const> interfacedNodes() const;

    void connectToBuffer();

    void activate(Unison::BufferProvider& bp);

  private:
    AlsaBackend& m_backend;
    QString m_id;
    Unison::PortDirection m_direction;

};

  } // Internal
} // Alsa

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
