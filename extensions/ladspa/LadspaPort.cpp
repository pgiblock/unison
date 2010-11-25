/*
 * LadspaPort.cpp
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

#include "LadspaPort.h"

#include <unison/BufferProvider.h>

#include <QSet>

using namespace Unison;

namespace Ladspa {
  namespace Internal {

LadspaPort::LadspaPort (LadspaPlugin *plugin, uint32_t index) :
  Port(),
  m_plugin(plugin),
  m_index(index),
  m_value(0)
{
  Q_ASSERT(m_plugin);
  Q_ASSERT(m_index < pluginInfo()->PortCount);
  m_value = defaultValue();
}


LadspaPort::~LadspaPort ()
{
}


const LADSPA_Descriptor *LadspaPort::pluginInfo() const
{
  return m_plugin->ladspaDescriptor();
}


LADSPA_PortDescriptor LadspaPort::portDescriptor() const
{
  return pluginInfo()->PortDescriptors[m_index];
}


const LADSPA_PortRangeHint LadspaPort::portRangeHints() const
{
  return pluginInfo()->PortRangeHints[m_index];
}


PortType LadspaPort::type () const
{
  LADSPA_PortDescriptor desc = portDescriptor();
  if (LADSPA_IS_PORT_CONTROL(desc)) {
    return ControlPort;
  }
  if (LADSPA_IS_PORT_AUDIO(desc)) {
    return AudioPort;
  }
  return UnknownPort;
}


PortDirection LadspaPort::direction () const
{
  LADSPA_PortDescriptor desc = portDescriptor();
  if (LADSPA_IS_PORT_INPUT(desc)) {
    return Input;
  }
  if (LADSPA_IS_PORT_OUTPUT(desc)) {
    return Output;
  }
  // TODO: Maybe have an UNDEFINED direction?
  qFatal("Port `%s' is neither input or output", qPrintable(name()));
  return Input; // suppress no-return warning
}


QString LadspaPort::name () const
{
  return QString::fromAscii( pluginInfo()->PortNames[m_index] );
}


QString LadspaPort::id () const
{
  return QString("%1").arg(m_index);
}


float LadspaPort::value () const
{
  return m_value;
}


void LadspaPort::setValue (float value)
{
  m_value = value;
  updateBufferValue();
}


// This method is ripped from Paul Davis's Ardour.  The method is fully
// documented in the LADSPA specs, I just don't want to type it all out.
float LadspaPort::defaultValue () const
{
  const LADSPA_PortRangeHint prh = portRangeHints();
  float ret = 0.0f;
  bool boundsGiven = false;
  bool rateScaling = false;
  bool earlierHint = false;

  /* defaults - case 1 */

  if (LADSPA_IS_HINT_HAS_DEFAULT(prh.HintDescriptor)) {
    if (LADSPA_IS_HINT_DEFAULT_MINIMUM(prh.HintDescriptor)) {
      ret = prh.LowerBound;
      boundsGiven = true;
      rateScaling = true;
      earlierHint = true;
    }

    /* FIXME: add support for logarithmic defaults */

    else if (LADSPA_IS_HINT_DEFAULT_LOW(prh.HintDescriptor)) {
      ret = prh.LowerBound * 0.75f + prh.UpperBound * 0.25f;
      boundsGiven = true;
      rateScaling = true;
      earlierHint = true;
    }
    else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(prh.HintDescriptor)) {
      ret = prh.LowerBound * 0.50f + prh.UpperBound * 0.50f;
      boundsGiven = true;
      rateScaling = true;
      earlierHint = true;
    }
    else if (LADSPA_IS_HINT_DEFAULT_HIGH(prh.HintDescriptor)) {
      ret = prh.LowerBound * 0.25f + prh.UpperBound * 0.75f;
      boundsGiven = true;
      rateScaling = true;
      earlierHint = true;
    }
    else if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(prh.HintDescriptor)) {
      ret = prh.UpperBound;
      boundsGiven = true;
      rateScaling = true;
      earlierHint = true;
    }
    else if (LADSPA_IS_HINT_DEFAULT_0(prh.HintDescriptor)) {
      ret = 0.0f;
      earlierHint = true;
    }
    else if (LADSPA_IS_HINT_DEFAULT_1(prh.HintDescriptor)) {
      ret = 1.0f;
      earlierHint = true;
    }
    else if (LADSPA_IS_HINT_DEFAULT_100(prh.HintDescriptor)) {
      ret = 100.0f;
      earlierHint = true;
    }
    else if (LADSPA_IS_HINT_DEFAULT_440(prh.HintDescriptor)) {
      ret = 440.0f;
      earlierHint = true;
    }
    else {
      /* no hint found */
      ret = 0.0f;
    }
  }

  /* defaults - case 2 */
  else if (LADSPA_IS_HINT_BOUNDED_BELOW(prh.HintDescriptor) &&
      !LADSPA_IS_HINT_BOUNDED_ABOVE(prh.HintDescriptor)) {

    if (prh.LowerBound < 0) {
      ret = 0.0f;
    } else {
      ret = prh.LowerBound;
    }

    boundsGiven = true;
    rateScaling = true;
  }

  /* defaults - case 3 */
  else if (!LADSPA_IS_HINT_BOUNDED_BELOW(prh.HintDescriptor) &&
      LADSPA_IS_HINT_BOUNDED_ABOVE(prh.HintDescriptor)) {

    if (prh.UpperBound > 0) {
      ret = 0.0f;
    } else {
      ret = prh.UpperBound;
    }

    boundsGiven = true;
    rateScaling = true;
  }

  /* defaults - case 4 */
  else if (LADSPA_IS_HINT_BOUNDED_BELOW(prh.HintDescriptor) &&
      LADSPA_IS_HINT_BOUNDED_ABOVE(prh.HintDescriptor)) {

    if (prh.LowerBound < 0 && prh.UpperBound > 0) {
      ret = 0.0f;
    } else if (prh.LowerBound < 0 && prh.UpperBound < 0) {
      ret = prh.UpperBound;
    } else {
      ret = prh.LowerBound;
    }
    boundsGiven = true;	
    rateScaling = true;
  }

  /* defaults - case 5 */

  if (LADSPA_IS_HINT_SAMPLE_RATE(prh.HintDescriptor) && !earlierHint) {
    if (boundsGiven) {
      if (rateScaling) {
        ret *= m_plugin->sampleRate();
      }
    } else {
      ret = m_plugin->sampleRate();
    }
  }

  return ret;

}


bool LadspaPort::isBounded () const
{
  // XXX: well, we only have a single isBounded, but ladspa considers lower
  // and upper seperately.  We can enhance our Plugin::isBounded function,
  // but for now, only bound if LADSPA tells us both sides should be bounded
  const int BoundedBoth = (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE);
  return (portRangeHints().HintDescriptor & BoundedBoth) == BoundedBoth;
}


float LadspaPort::minimum () const
{
  if (LADSPA_IS_HINT_LOGARITHMIC(portRangeHints().HintDescriptor)) {
    return portRangeHints().LowerBound;
  }
  else {
    return portRangeHints().LowerBound * m_plugin->sampleRate();
  }
}


float LadspaPort::maximum () const
{
  if (LADSPA_IS_HINT_LOGARITHMIC(portRangeHints().HintDescriptor)) {
    return portRangeHints().UpperBound;
  }
  else {
    return portRangeHints().UpperBound * m_plugin->sampleRate();
  }
}


bool LadspaPort::isToggled () const
{
  return LADSPA_IS_HINT_TOGGLED(portRangeHints().HintDescriptor);
}


Node* LadspaPort::parent () const
{
  return m_plugin;
}


const QSet<Node* const> LadspaPort::interfacedNodes () const
{
  QSet<Node* const> p;
  p.insert( m_plugin );
  return p;
}


void LadspaPort::connectToBuffer ()
{
  pluginInfo()->connect_port(m_plugin->ladspaHandle(),
      m_index, (LADSPA_Data*)buffer()->data());
}


  } // Internal
} // Ladspa

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
