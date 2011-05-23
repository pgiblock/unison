/*
 * IDriverProvider.hpp
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

#ifndef UNISON_IDRIVER_PROVIDER_H
#define UNISON_IDRIVER_PROVIDER_H

#include "Core_global.hpp"

#include <QObject>

namespace Core {

/**
 * Enumerates and creates a backend.  Used to describe which backends are
 * available, so that engine can pick the right one during startup. */
class CORE_EXPORT IDriverProvider : public QObject
{
  Q_OBJECT
  public:
    IDriverProvider (QObject* parent = 0) : QObject(parent) {}
    virtual ~IDriverProvider () {}

    virtual QString displayName () const = 0;
    virtual bool loadDriver () = 0;
};

} // Core


#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
