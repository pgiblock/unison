/*
 * Uncopyable.h
 *
 * Copyright (c) 2010 Paul Giblock <pgib/at/users.sourceforge.net>
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

#ifndef PRG_UNCOPYABLE_H
#define PRG_UNCOPYABLE_H

/**
 * A class with private copy semantics to prevent any derived classes from
 * being copied.  This implementation is from Scott Meyer's Effective C++.
 * The same class exists in boost, but has a stupid name, 'noncopyable'.
 */
namespace PRG {

class Uncopyable
{
  protected: // Can't create an Uncopyable
    Uncopyable () {}
    ~Uncopyable () {}
  private:   // Can't copy derived classes.
    Uncopyable ( const Uncopyable& );
    const Uncopyable& operator=( const Uncopyable& );
};

} // PRG

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
