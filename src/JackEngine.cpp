/*
 * JackEngine.cpp
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

#include <jack/jack.h>

#include "unison/JackEngine.h"


namespace Unison {

	JackPort* JackEngine::registerPort (QString name,
			Port::Direction direction) {
		JackPortFlags flag;
		switch (direction) {
			case Port::INPUT:
				flag = JackPortIsOutput;
				break;
			case Port::OUTPUT:
			default:
				flag = JackPortIsInput;
				break;
		}

		jack_port_t* port = jack_port_register(
				client(), name.toLatin1(), JACK_DEFAULT_AUDIO_TYPE, flag, 0);
		if (port) {
			JackPort* myPort = new JackPort(*this, port);
			m_myPorts.append( myPort );
			return myPort;
		}
		return NULL;
	}

	uint32_t JackEngine::myPortCount () const {
		return m_myPorts.count();
	}

    JackPort* JackEngine::myPort (uint32_t index) const {
		return m_myPorts[index];
	}

    JackPort* JackEngine::myPort (QString name) const {
		return NULL; // TODO: implement
	}

} // Unison

// vim: et ts=8 sw=2 sts=2 noai
