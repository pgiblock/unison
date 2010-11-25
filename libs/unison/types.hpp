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


#ifndef UNISON_TYPES_HPP_
#define UNISON_TYPES_HPP_

#include <stdint.h>

namespace Unison {

typedef uint32_t nframes_t;
typedef uint32_t nticks_t;

/**
 * A float is a common abstraction for a sample.  We use float all throughout 
 * Unison - manipulating integer samples will cause aliasing etc..
 */
typedef float sample_t;

enum PortType
{
  AudioPort   = 1,  ///< Communicates by means of an audio buffer @sa AudioBuffer
  ControlPort = 2,  ///< Controls or is controled by a single value @sa ControlBuffer
  MidiPort    = 4,  ///< Port containing a queue of MIDI events
  UnknownPort = 0   ///< Invalid state
};

enum PortDirection
{
  Input  = 1,       ///< Port accepts input
  Output = 2        ///< Port generates output
};

} // Unison

#endif // UNISON_TYPES_H

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
