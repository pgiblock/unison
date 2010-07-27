/*
 * JackBackend.h
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

#ifndef UNISON_JACK_BACKEND_H
#define UNISON_JACK_BACKEND_H

#include "JackPort.h"

#include <unison/Backend.h>
#include <core/IBackendProvider.h>
#include <jack/jack.h>
#include <QObject>
#include <QVarLengthArray>

namespace Jack {
  namespace Internal {

class JackBackendProvider : public Core::IBackendProvider
{
  Q_OBJECT
  public:
    JackBackendProvider (QObject *parent = 0) :
      Core::IBackendProvider(parent)
    {};

    ~JackBackendProvider ()
    {};

    QString displayName ()
    {
      return "jack";
    };

    Unison::Backend * createBackend();
};



/**
 * JackBackend encapsulates JACK compatibility.  There could theoretically be
 * multiple Backend classes (AsioBackend, for example), but this requires
 * us to implement missing features like connecting ports.  Therefore, right
 * now we are only targeting Jack, with the knowledge that this class may be
 * generalized.  The primary functionality included is the processing
 * entry-point and the ability to register ports, query system ports, and make
 * (external) connections. */
class JackBackend : public Unison::Backend
{
  Q_OBJECT

  public:
    JackBackend ();
    virtual ~JackBackend ();

    /**
     * @returns the underlying Jack client. */
    jack_client_t* client () const
    {
      return m_client;
    }

    /**
     * FIXME: BufferLength change support is currently lacking. */
    Unison::nframes_t bufferLength () const;
    Unison::nframes_t sampleRate () const;
    bool isFreewheeling () const;

    /**
     * Register a port with Jack.
     * @returns the newly registered port */
    JackPort* registerPort (QString name, Unison::PortDirection direction);

    /**
     * Unregister a port with Jack.
     * FIXME: Consider disconnecting ports when unregistering */
    void unregisterPort (JackPort *);
    void unregisterPort (Unison::BackendPort *);

    void activate ();
    void deactivate ();

    int portCount () const;
    JackPort* port (int index) const;
    JackPort* port (QString name) const;

    int connect (const QString& source, const QString& dest);
    int disconnect (const QString& source, const QString& dest);
    int disconnect (Unison::BackendPort *);

  private:
    void initClient();

    static void shutdown (void* backend);
    static int bufferSizeCb (Unison::nframes_t nframes, void* backend);
    static void freewheelCb (int starting, void* backend);
    static int graphOrderCb (void* backend);
    static int processCb (Unison::nframes_t nframes, void* backend);
    static int sampleRateCb (Unison::nframes_t nframes, void* backend);
    static int syncCb (jack_transport_state_t, jack_position_t*, void* eng);
    static void threadInitCb (void* backend);
    static void timebaseCb (jack_transport_state_t, Unison::nframes_t, jack_position_t*, int, void*);
    static int xrunCb (void* backend);

    jack_client_t* m_client;
    QVarLengthArray<JackPort*> m_myPorts;
    Unison::nframes_t m_bufferLength;
    Unison::nframes_t m_sampleRate;
    bool m_freewheeling;
    bool m_running;
};

  } // Internal
} // Jack

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
