/*
 * RingBuffer.h
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

#ifndef UNISON_RING_BUFFER_H
#define UNISON_RING_BUFFER_H

#include <prg/Uncopyable.h>

#include <QObject>
#include <unison/types.h>

namespace Unison {


/**
 * A lock-free ringbuffer based off of the implementation written by
 * Paul Davis for JACK.
 *
 * The key attribute of a ringbuffer is that it can be safely accessed
 * by two threads simultaneously -- one reading from the buffer and
 * the other writing to it -- without using any synchronization or
 * mutual exclusion primitives.  For this to work correctly, there can
 * only be a single reader and a single writer thread.  Their
 * identities cannot be interchanged.
 */
template <typename T>
class RingBuffer
{

  public:
    /**
     * Contruct a ringbuffer, specifying the internal buffer size.
     * @param size  The size of the buffer to be allocated */
    RingBuffer (int size);

    /**
     * Destory the ringbuffer and allocated space. */
    virtual ~RingBuffer ();

    /**
     * Reset the read and write pointers, making an empty buffer.
     * This is not thread safe. */
    void reset ();

    /**
     * Get the size of the ringbuffer
     *
     * @param the ringbuffer size in bytes. */
    int capacity () const
    {
      return m_size;
    }

    /**
     * Return the number of bytes available for writing.
     *
     * @return the amount of free space (in bytes) available for writing. */
    int writeSpace () const;

    /**
     * Return the number of bytes available for reading.
     *
     * @return the number of bytes available to read. */
    int readSpace () const;

    /**
     * Read data from the ringbuffer.
     * Caller must check the return value and call this function again if
     * necessary, or use the read method which does this automatically.
     *
     * @param dest a pointer to a buffer where data read from the
     * ringbuffer will go.
     * @param cnt the number of bytes to read.
     *
     * @return the number of bytes read, which may range from 0 to cnt. */
    int readChunk (T *dest, int cnt);

    /**
     * Read data from the ringbuffer, wrapping when end of buffer is reached.
     * Like the readChunk() function, but will automatically wrap to the
     * beginning of the buffer if the end is reached.
     *
     * @param dest a pointer to a buffer where data read from the
     * ringbuffer will go.
     * @param cnt the number of bytes to read.
     *
     * @return true if the buffer contains enough data to fulfill the request */
    bool read (T *dest, int cnt);

    /**
     * Read from the ringbuffer without moving the read pointer.
     * A convenient way to inspect the data in a continous fashion.  The cost
     * is that the data must be copied into the destination buffer.
     * Caller must check the return value and call this function again if
     * necessary, or use the peek method which does this automatically.
     *
     * @param dest a pointer to a buffer where data read from the
     * ringbuffer will go.
     * @param cnt the number of bytes to read.
     * @return the number of bytes read, which may range from 0 to cnt. */
    int peekChunk (T *dest, int cnt);

    /**
     * Read data from the ringbuffer, wrapping when end of buffer is reached.
     * Like the peekChunk() function, but will automatically wrap to the
     * beginning of the buffer if the end is reached.
     *
     * @param dest a pointer to a buffer where data read from the
     * ringbuffer will go.
     * @param cnt the number of bytes to read.
     *
     * @return true if the buffer contains enough data to fulfill the request */
    bool peek (T *dest, int cnt);

    /* TODO: Non-copy peak (ala, jack_rb_get_read_vector() */

    /**
     * Advance the read pointer by the specified number of bytes.  The data
     * is not copied, and is lost forever.  This is useful to avoid a copy if
     * a reader determines that the next chunk of data can be ignored.
     *
     * @params cnt the number of bytes to advance
     * @return true if there was enough space, false if canceled due to
     *         too little space  */
    bool skip (int cnt);

    /**
     * Write data to the ringbuffer. Writes at most cnt bytes from src.
     *
     * @param src a pointer to a buffer where data written to the ringbuffer
     *            is read.
     * @param cnt the maximum number of bytes to write.
     *
     * @return the actual count of bytes copied */
    int write (const T *src, int cnt);

  protected:
    QAtomicInt m_writePtr;
    QAtomicInt m_readPtr;

    int m_size; ///< Size in bytes
    char *m_data; ///< The buffer contents
};


} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
