/*
 * Backend.h
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

#ifndef UNISON_BACKEND_H
#define UNISON_BACKEND_H

#include <prg/Uncopyable.h>

#include <QObject>
#include <unison/types.h>

namespace Unison {

class BackendPort;
class Processor;

/**
 * Backend encapsulates Audio-Interface compatibility.  There could
 * theoretically be multiple Interface classes (AsioBackend, for example),
 * but this requires us to implement missing features like connecting ports.
 * The primary functionality included is the processing entry-point and the
 * ability to register ports, query system ports, and make (external)
 * connections. */
class Backend : public QObject, PRG::Uncopyable
{
  Q_OBJECT

  public:
    Backend () :
      m_rootPatch(NULL)
    {};

    virtual ~Backend () {};

    /**
     * Returns the current buffer-length of the audio back-end */
    virtual nframes_t bufferLength () const = 0;

    /**
     * Returns the current sample-rate of the audio back-end */
    virtual nframes_t sampleRate () const = 0;

    /**
     * Returns whether or not the back-end is freewheeling.  Freewheeling is
     * when we process frames as quickly as possible with no regard to physical
     * time */
    virtual bool isFreewheeling () const = 0;

    /**
     * Register a port with Backend.
     * @returns the newly registered port */
    virtual BackendPort* registerPort (QString name, Unison::PortDirection direction) = 0;

    /**
     * Unregister a port.
     * FIXME: Consider disconnecting ports when unregistering */
    virtual void unregisterPort (BackendPort *) = 0;

    virtual void activate () = 0;
    virtual void deactivate () = 0;

    virtual int portCount () const = 0;
    virtual BackendPort* port (int index) const = 0;
    virtual BackendPort* port (QString name) const = 0;

    virtual int connect (const QString& source, const QString& dest) = 0;
    virtual int disconnect (const QString& source, const QString& dest) = 0;
    virtual int disconnect (BackendPort *) = 0;

    /**
     * Set the root-processor used by this backend.  I think ideally, we would
     * do this in the Engine. But, putting it in the Backend simplifies things
     * for now */
    void setRootProcessor (Processor *processor)
    {
      m_rootPatch = processor;
    }

    Processor *rootPatch () const
    {
      return m_rootPatch;
    }

  private:
    Processor *m_rootPatch;

};

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
