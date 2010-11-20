/*
 * Commander.h
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


#ifndef UNISON_COMMANDER_H_
#define UNISON_COMMANDER_H_

#include "RingBuffer.h"

#include <QtCore/QSemaphore>
#include <QtCore/QMutex>

namespace Unison {

  class Command;
  class ProcessingContext;

  namespace Internal {

/**
 * Commander is a centralized place for components to queue commands that require
 * synchronization with the processing thread.
 * TODO: Really, this class should NOT be a singleton, it should either be passed into the
 * client, or part of Engine or Backend.  This is just a temporary location until we find
 * a place for it.
 */
class Commander
{
  Q_DISABLE_COPY(Commander)
  public:
    /**
     * Initialize the static Commander instance
     */
    static void initialize();

    /**
     * Get the static Commander instance.
     * @return the static Commander instnace
     */
    static Commander* instance()
    {
      Q_ASSERT(m_instance);
      return m_instance;
    }

    /**
     * Push a command for later execution.  The Command's preExecute function will be
     * called from a normal context. Later, the Command's execute will fire at the
     * beginning of the processing thread's process function.  postExecute will be called
     * eventually.  Finally, the  Command is deleted.  This function will block until the
     * Command is postExecuted if the Command has isBlocking=true.
     * @param command The command to enqueue
     */
    void push (Command* command);

    /**
     * Releases the block (counting semaphore).  This function must only be called from
     * blocking Commands within the postExecute method.  The Command must call release
     * only AFTER it is safe for another Command to begin execution.
     */
    inline void release ()
    {
      m_blockWait.release();
    }

    /**
     * Process Commands needing to be executed during this processing pass.  MUST be
     * called in realtime. MUST NOT be called while the graph is actually process()ing.
     * This function will probably only ever be called by Backend's processing logic.
     * @param context The current processing context
     */
    void process (ProcessingContext& context);

  protected:
    /**
     * Construct a Commander, must use the static initialize() function instead
     */
    Commander ();

  private:
    enum {
      COMMAND_BUFFER_LENGTH = 1024,   ///< How big is our buffer?
      COMMANDS_PER_PROCESS =  8       ///< How many commands to process per period
    };

    static Commander* m_instance;   ///< The instance
    QMutex m_writeLock;             ///< Protect us from affinity issues
    QSemaphore m_blockWait;         ///< Blocking for blocking Commands
    RingBuffer<Command*> m_buffer; ///< Storage for queued Commands
};

  } // Internal
} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
