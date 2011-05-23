/*
 * JackDriverProvider.hpp
 *
 * Copyright (c) 2010-2011 Paul Giblock <pgib/at/users.sourceforge.net>
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

#ifndef UNISON_JACK_DRIVERPROVIDER_H
#define UNISON_JACK_DRIVERPROVIDER_H

#include <core/IDriverProvider.hpp>

#include <QObject>

namespace Jack {
  namespace Internal {

class JackDriverProvider : public Core::IDriverProvider
{
  Q_OBJECT
  public:
    JackDriverProvider (QObject* parent = 0) :
      Core::IDriverProvider(parent)
    {}

    ~JackDriverProvider ()
    {}

    QString displayName () const
    {
      return "jack";
    }

    bool loadDriver ();
};

  } // Internal
} // Jack

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
