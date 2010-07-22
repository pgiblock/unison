/**
 * @file FxLine.cc
 * @brief An effects line.
 *
 * @author kizzobot (kb), kizzobot@gmail.com
 */

#include <QDebug>

#include "unison/FxLine.h"
#include "unison/Plugin.h"
#include "extensions/jack/JackEngine.h"
#include "extensions/core/PluginManager.h"
#include "extensions/core/Session.h"

using namespace Unison;

PluginEntry::PluginEntry(const PluginDescriptor& descriptor, Processor *proc):
  descriptor(descriptor), proc(proc)
{

}

FxLine::FxLine (Core::Session& session, QString name) :
  CompositeProcessor(),
  m_name(name),
  m_session(session)
{
  m_inPorts[0] = session.engine().registerPort(name + "/in 1", OUTPUT);
  m_inPorts[1] = session.engine().registerPort(name + "/in 2", OUTPUT);
  m_outPorts[0] = session.engine().registerPort(name + "/out 1", INPUT);
  m_outPorts[1] = session.engine().registerPort(name + "/out 2", INPUT);

  m_inPorts[0]->connect(m_outPorts[0]);
  m_inPorts[1]->connect(m_outPorts[1]);
}

FxLine::~FxLine ()
{
  m_session.engine().unregisterPort(m_inPorts[0]);
  m_session.engine().unregisterPort(m_inPorts[1]);
  m_session.engine().unregisterPort(m_outPorts[0]);
  m_session.engine().unregisterPort(m_outPorts[1]);
}

QString FxLine::name() const
{
  return m_name;
}

void
FxLine::addPlugin(const PluginDescriptor& descriptor, int pos)
{
  // Check for proper position value.
  if (pos >= plugins.length())
    return;

  // Create the plugin.
  Processor *proc = descriptor.createPlugin(48000);
  //add(proc);

  // Verify number of ports.
  if (proc->portCount() < 4)
    return;

  // Collect ports.
  QList<Port*> inPorts;
  QList<Port*> outPorts;
  for (int i = 0; i < proc->portCount(); i++)
  {
    Port* p = proc->port(i);

    if (p->type() == AUDIO_PORT)
    {
      switch (p->direction())
      {
      case INPUT:
        inPorts.append(p);
        break;
      case OUTPUT:
        outPorts.append(p);
        break;
      default:
        // Error.
        break;
      }
    }
  }
  if (inPorts.length() < 2)
    return;
  if (outPorts.length() < 2)
    return;

  if (plugins.length() == 0)
  {
    // If there are no plugins, we disconnect FxLine.in1 from FxLine.out1 and 
    // FxLine.in2 from FxLine.out2, and then reconnect FxLine.in1 to 
    // newPlugin.in1 and FxLine.in2 to newPlugin.in2; we also connect 
    // newPlugin.out1 to FxLine.out1 and newPlugin.out2 to FxLine.out2.

    m_inPorts[0]->disconnect(m_outPorts[0]);
    m_inPorts[1]->disconnect(m_outPorts[1]);

    m_inPorts[0]->connect(inPorts.at(0));
    m_inPorts[1]->connect(inPorts.at(1));

    outPorts.at(0)->connect(m_outPorts[0]);
    outPorts.at(1)->connect(m_outPorts[1]);
 
    return;
  }
  else if (pos == 0)
  {
    // At this point, there is at least one plugin already in the line, and we 
    // are trying to insert the new plugin at the beginning.
  }
  else if (pos < plugins.length())
  {
    // If this is reached, it means that we are inserting the plugin between 2 
    // others.
  }
  else if (pos == plugins.length())
  {
    // If this is reached, it means the new plugin will be appended to the end 
    // of the line.
  }
  else
  {
    // Something is wrong.
  }
}

void FxLine::addEffect()
{
  const char * uri = "http://calf.sourceforge.net/plugins/Phaser";
  PluginManager * man = PluginManager::instance();

  Processor * proc = man->descriptor(uri)->createPlugin(48000);
  add(proc);

  // TODO: support for port-groups
  // PortGroup { enum Type {STEREO, QUAD, FIVEPOINTONE}
  // or maybe a processor.portGroups() ?

  int inCnt=0, outCnt =0;
  for (int i=0; i<proc->portCount(); ++i) {
    Port* p = proc->port(i);
    if (p->type() == AUDIO_PORT) {
      switch (p->direction()) {
        case INPUT:
          qDebug() << "Connecting " << m_inPorts[inCnt]->name() << " to " << p->name();
          m_inPorts[inCnt++]->connect(p);
          break;
        case OUTPUT:
          qDebug() << "Connecting " << m_outPorts[outCnt]->name() << " to " << p->name();
          m_outPorts[outCnt++]->connect(p);
          break;
        default:
          //TODO: Programming error!
          break;
      }
    }
  }

  hackCompile(m_session.bufferProvider());
}

// vim: ts=8 sw=2 sts=2 et sta noai
