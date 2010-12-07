/*
 * PostExecuter.hpp
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


#ifndef UNISON_POST_EXECUTER_HPP_
#define UNISON_POST_EXECUTER_HPP_

#include "RingBuffer.hpp"

#include <QThread>

namespace Unison {

  class Command;

  namespace Internal {

/**
 * PostExecuter is responsible for post-executing and deleting finished Commands.
 * There is only one client (the Commander), so, our RingBuffer implementation is
 * enough to guarantee exclusion. */
class PostExecuter : public QThread
{
  Q_OBJECT
  Q_DISABLE_COPY(PostExecuter)

  public:
    PostExecuter();
    virtual ~PostExecuter();

    /**
     * Push an executed command onto the queue.  It will be post-executed and
     * deleted shortly.
     * @param command The command to post-execute */
    void push (Command* command);

  protected:
    virtual void run();

    /**
     * Stops the thread execution. If successful, one more iteration will run,
     * purging the queue of all remaining Commands.  Can be restarted with start()
     * @returns true if the thread properly joined cleanly. */
    bool stop();

    /**
     * Post execute and destroy the given Command immediately.
     * @param command The command to post-execute, now. */
    void postExecute (Command* command);

  private:
    enum {
      COMMAND_BUFFER_LENGTH = 1024, ///< How big is our buffer?
      IDLE_TIMEOUT = 250            ///< How long to idle, in msec
    };

    RingBuffer<Command*> m_buffer;  ///< Storage for queued Commands
    bool m_done;                    ///< Flag to kill loop
};


  } // Internal
} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
