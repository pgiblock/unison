/*
 * types.h - Common types used in Unison
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


#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

namespace Unison {

typedef uint32_t nframes_t;
typedef uint32_t nticks_t;

enum PortType { AUDIO_PORT=1, CONTROL_PORT=2, MIDI_PORT=4, UNKNOWN_PORT=0 };
enum PortDirection { INPUT=1, OUTPUT=2 };

} // Unison

#endif // TYPES_H

