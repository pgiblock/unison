/*
 * FastRandom.h
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

#ifndef UNISON_FAST_RANDOM_H_
#define UNISON_FAST_RANDOM_H_

namespace Unison {

/**
 * An implementation of the pseudo-random number generator presented in
 * POSIX.1-2001.  It looks like this should be sufficiently fast. */
class FastRandom
{
  public:

    FastRandom () : n(0)
    {}

    FastRandom (const FastRandom& other) : n(other.n)
    {}

    /**
     * Seed our random number generator. Use time, or another RNG. */
    void seed (unsigned s)
    {
      n = s;
    }

    /**
     * Gets a random int in [0, 32767]
     */
    inline unsigned nextInt ()
    {
      return (unsigned)(next()/65536) % 32768;
    }

    /**
     * Gets a random float in [0, 1.0)
     */
    inline float nextFloat ()
    {
      return next() * 2.328306e-10;
    }

    /**
     * Gets a random double in [0, 1.0)
     */
    inline double nextDouble ()
    {
      return next() * 2.328306e-10;
    }

  private:

    inline unsigned next ()
    {
      return (n = n * 1103515245 + 12345);
    }

    unsigned n;
};

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
