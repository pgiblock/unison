/*
 * Feature.h
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

#ifndef UNISON_LV2_FEATURE_H
#define UNISON_LV2_FEATURE_H

#include <lv2/lv2.h>

#include <QList>
#include <QString>
#include <QSharedPointer>

namespace Lv2 {
  namespace Internal {

class Lv2Plugin;
class Lv2World;

/**
 * A host feature provided to an lv2 plugin.  This basically wraps the creation of a
 * LV2_Feature, and in some instances, will hold all the state as well.  Since we must
 * just pass an array of LV2_Feature to LV2's instantiate, there isn't a chance to use
 * real RIIA.  */
class Feature
{
  public:
    enum FeatureType {
      PLUGIN_FEATURE, ///< Feature is suitable for plugin and UI use
      UI_FEATURE      ///< Feature is only suitable for UI use
    };

    Feature (const QString& uri, FeatureType type);

    /**
     * The URI of the Feature.
     * @return The URI of this feature. */
    QString uri () const
    {
      return m_uri;
    }

    /**
     * Some features are only needed for UI
     * @return True if this feature is only for UIs */
    FeatureType type () const
    {
      return m_type;
    }

    /**
     * Fetch a pointer to the LV2_Feature needed for an additional Lv2Plugin.  The
     * returned value may or may not be shared.  The cleanup() function of this Feature
     * must be called with the LV2_Feature.
     * @return An LV2_Feature appropriate for a new Plugin  */
    virtual LV2_Feature* lv2Feature () = 0;

    /**
     * Initialize feature if a reference to LV2_Handle is needed.  Some LV2 Features
     * require a pointer back to the LV2_Handle or some data belonging to the handle.
     * However, we do not know this value until after the plugin has been constructed.
     * So, this is the 2nd-phase initialization for Features.
     * @param lv2Feature  LV2_Feature to intialize. the data field is generally changed
     * @param plugin      The plugin receiving this feature
     */
    virtual void initialize (LV2_Feature* lv2Feature, const Lv2Plugin& plugin) const = 0;

    /**
     * Cleanup the LV2_Feature. This may mean deleting it, or decrementing a refcount, or
     * any other number of things.  This is to support features that are shared as well as
     * ones constructed per-plugin.
     * @param lv2Feature  */
    virtual void cleanup (LV2_Feature* lv2Feature) const = 0;

  private:
    QString m_uri;
    FeatureType m_type;
};


/**
 * The array of features used by plugins.  This is basically the resource management for
 * LV2_Features.
 */
class FeatureArray
{
  public:
    /**
     * A FeatureArray is created with a list of features.  FeatureArray will obtain
     * LV2_Features from the Features and release them on destruction.
     * @param features The list of features to provide  */
    FeatureArray (QList<Feature*> features);

    /**
     * Copy constructor for FeatureArray.  The collections are copied, but the actual
     * Features and LV2_Features are shared by un-protected pointers.
     * @param rhs the other side  */
    FeatureArray (const FeatureArray& rhs);

    /**
     * Destruction of the FeatureArray guarantees that the LV2_Features are cleaned up by
     * the Feature they were created from. */
    ~FeatureArray ();

    /**
     * Initialized the Features in the FeatureArray.  This is event exists to accomodate
     * Features that need a reference to the Plugin's LV2_Handle.
     * @param plugin  The plugin receiving this feature  */
    void initialize (const Lv2Plugin& plugin);

    /**
     * The LV2_Features in an array for LV2 api.
     * @param  set to true if UI features should be returned as well
     * @return an array with the LV@_Features */
    const LV2_Feature* const * get (Feature::FeatureType type);

  private:
    LV2_Feature** m_array;
    QList<Feature*> m_features;
    bool m_uiFeatureCount;
};


/**
 * The set of Features that we know about.
 */
class FeatureSet
{
  Q_DISABLE_COPY(FeatureSet)
  public:
    FeatureSet () {};

    /**
     * Find the Feature matching the giving URI.
     * @param uri The URI to search for
     * @return The feature, if found.  Otherwise, NULL */
    Feature* feature (const QString& uri) const;

    /**
     * Get an array of supported features.
     * @return an array of features */
    QSharedPointer<FeatureArray> array () const;

  protected:
    /**
     * Add a new feature to our already awesome powers.  Input is ignored if a feature
     * with the given name is already registered.
     * @param feature The feature to add */
    void insert (Feature* feature);

    /**
     * Get a read-only list of features.  For Lv2World to clean-up features.
     * @return the list */
    const QList<Feature*> list() const
    {
      return m_features;
    }

  private:
    QList<Feature*> m_features; ///< Our features

  friend class Lv2World;
};


  } // Internal
} // Lv2

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
