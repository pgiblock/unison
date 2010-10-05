/*
 * Feature.cpp
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

#include "Feature.h"

#include <QSharedPointer>
#include <string.h>

namespace Lv2 {
  namespace Internal {

Feature::Feature (const QString& uri) :
  m_uri( uri )
{}


FeatureArray::FeatureArray (QList<Feature*> features) :
  m_features( features )
{
  m_array = new LV2_Feature*[features.count()+1];
  int i=0;
  foreach (const Feature* f, m_features) {
    m_array[i++] = f->lv2Feature();
  }
  m_array[i] = NULL;
}

FeatureArray::FeatureArray (const FeatureArray& rhs) :
  m_array( NULL),
  m_features( rhs.m_features )
{
  memcpy(m_array, rhs.m_array, sizeof(rhs.m_array));
}


FeatureArray::~FeatureArray ()
{
  int i=0;
  foreach (const Feature* f, m_features) {
    f->cleanup(m_array[i++]);
  }
  delete[] m_array;
}

void FeatureArray::initialize (const Lv2Plugin& plugin)
{
  int i=0;
  foreach (const Feature* f, m_features) {
    f->initialize(m_array[i++], plugin);
  }
}


Feature* FeatureSet::feature (const QString& uri) const
{
  foreach (Feature* f, m_features) {
    if (f->uri() == uri) {
      return f;
    }
  }
  return NULL;
}


QSharedPointer<FeatureArray> FeatureSet::featureArray () const
{
  return QSharedPointer<FeatureArray>( new FeatureArray(m_features) );
}


void FeatureSet::insert (Feature* feature)
{
  Feature *exists = this->feature(feature->uri());
  if (!exists) {
    // Causes a COW, so old FeatureArrays will still have the old (proper) lists
    m_features.append(feature);
  }
}

  } // Internal
} // Lv2

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
