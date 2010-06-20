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

#include "prg/Uncopyable.h"
#include "extensions/jack/JackPort.h"

namespace Unison
{

class Session;

/**
 * JackEngine encapsulates JACK compatibility.  There could theoretically be
 * multiple Engine classes (AsioEngine, for example), but this requires us to
 * implement missing features like connecting ports.  Therefore, right now we
 * are only targeting Jack, with the knowledge that this class may be
 * generalized.  The primary functionality included is the processing
 * entry-point and the ability to register ports, query system ports, and
 * make (external) connections. */
class JackEngine : public QObject, PRG::Uncopyable
{
  Q_OBJECT

  public:
    JackEngine ();
    virtual ~JackEngine ();

    /**
     * Set the session of this engine,  generally called by Session's ctor.
     * JackEngine's behavior is undefined if a Session is not assigned. */
    void setSession (Session * session);

    /**
     * Remove the session of this engine.  JackEngine is in an undefined state
     * until a Session is added. */
    void removeSession ();

    /**
     * @returns the underlying Jack client. */
    jack_client_t* client () const
    {
      return m_client;
    }

    /**
     * FIXME: BufferLength change support is currently lacking. */
    nframes_t bufferLength () const;
    nframes_t sampleRate () const;
    bool isFreewheeling () const;

    /**
     * Register a port with Jack.
     * @returns the newly registered port */
    JackPort* registerPort (QString name, PortDirection direction);

    /**
     * Unregister a port with Jack.
     * FIXME: Consider disconnecting ports when unregistering */
    void unregisterPort (JackPort *);

    void activate ();
    void deactivate ();

    int portCount () const;
    JackPort* port (int index) const;
    JackPort* port (QString name) const;

    int connect (const QString& source, const QString& dest);
    int disconnect (const QString& source, const QString& dest);
    int disconnect (Port *);

  private:
    void initClient();

    static void shutdown (void* engine);
    static int bufferSizeCb (nframes_t nframes, void* engine);
    static void freewheelCb (int starting, void* engine);
    static int graphOrderCb (void* engine);
    static int processCb (nframes_t nframes, void* engine);
    static int sampleRateCb (nframes_t nframes, void* engine);
    static int syncCb (jack_transport_state_t, jack_position_t*, void* eng);
    static void threadInitCb (void* engine);
    static void timebaseCb (jack_transport_state_t, nframes_t, jack_position_t*, int, void*);
    static int xrunCb (void* engine);

    Session* m_session;
    jack_client_t* m_client;
    QVarLengthArray<JackPort*> m_myPorts;
    nframes_t m_bufferLength;
    nframes_t m_sampleRate;
    bool m_freewheeling;
    bool m_running;
};

} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
