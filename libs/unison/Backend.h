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

#ifndef UNISON_BACKEND_H_
#define UNISON_BACKEND_H_

#include "types.h"

#include <QtCore/QObject>

namespace Unison {

  class BackendPort;
  class Patch;

/**
 * Backend encapsulates Audio-Interface compatibility.  There could theoretically be
 * multiple Interface classes (AsioBackend, for example), but this requires us to
 * implement missing features like connecting ports.  The primary functionality included
 * is the processing entry-point and the ability to register ports, query system ports,
 * and make (external) connections.
 */
class Backend : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(Backend)

  public:
    Backend () :
      m_rootPatch(NULL)
    {};

    virtual ~Backend () {};

    /**
     * @returns the current buffer-length of the audio Backend
     */
    virtual nframes_t bufferLength () const = 0;

    /**
     * Returns the current sample-rate of the audio Backend
     */
    virtual nframes_t sampleRate () const = 0;

    /**
     * Whether or not the Backend is freewheeling.  Freewheeling is when we process frames
     * as quickly as possible with no regard to physical time
     * @returns @c true if we are freewheeling
     */
    virtual bool isFreewheeling () const = 0;

    /**
     * Register a port with Backend.
     * @param name the name (non-qualified id) of the port
     * @param direction the direction of the port
     * @returns the newly registered port
     */
    virtual BackendPort* registerPort (const QString& name, PortDirection direction) = 0;

    /**
     * Unregister a port.
     * FIXME: Consider disconnecting ports when unregistering
     */
    virtual void unregisterPort (BackendPort*) = 0;

    /**
     * Activate the backend - This should cause the backend to complete any
     * pending initialization and to begin running.  That is, the root-
     * processor will start to process().
     * @returns false if the backend could not be activated */
    virtual bool activate () = 0;

    /**
     * Deactive the backend - The backend should stop calling process() within
     * a reasonable amount of time.  The backend is free to do any cleanup, but
     * must not be destroyed.
     * @returns true if the backend is deactivated (regardless of whether the
     *          backend was running or not before calling this function */
    virtual bool deactivate () = 0;

    /**
     * The total number of Ports, for this particular Backend, in the system
     * @return the total number of ports
     */
    virtual int portCount () const = 0;

    /**
     * @returns the Port at @p index
     */
    virtual BackendPort* port (int index) const = 0;

    /**
     * @returns the Port named @p name
     */
    virtual BackendPort* port (const QString& name) const = 0;

    /**
     * Create an intra-Backend connection between two ports.  A backend must support
     * connections. At least, it must support connecting two ports that have been
     * registered.  There is no requirement that non-Unison ports exist in the Backend.
     * However, most Backends will have some ports not created by Unison, this allows us
     * to actually input or output somewhere.  Examples are: hardware ports, other
     * programs, storage, etc..
     * @param source The port to behave as input
     * @param source The port to behave as output
     */
    virtual int connect (const QString& source, const QString& dest) = 0;

    /**
     * Disconnect two ports.  The order should probably not matter.
     * @param source one port to disconnect
     * @param source theport to disconnect from
     */
    virtual int disconnect (const QString& source, const QString& dest) = 0;

    /**
     * Disconect all ports from @p port
     */
    virtual int disconnect (BackendPort* port) = 0;

    /**
     * Set the root-processor used by this backend.  I think ideally, we would do this in
     * the Engine. But, putting it in the Backend simplifies things for now
     */
    void setRootProcessor (Patch* processor)
    {
      m_rootPatch = processor;
    }

    Patch* rootPatch () const
    {
      return m_rootPatch;
    }

  private:
    Patch* m_rootPatch;   ///< Pointer to the root patch/processor

};

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
