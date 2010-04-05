/*
 * BufferProvider.h
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

#ifndef BUFFER_PROVIDER_H
#define BUFFER_PROVIDER_H

#include <iostream>
#include <malloc.h>

#include <QVector>
#include <QSharedPointer>

#include "prg/Uncopyable.h"
#include "unison/types.h"

namespace Unison {

class BufferProvider;
class SharedBufferPtr;

const nframes_t MAX_BUFFER_LENGTH = 4096;

class Buffer : PRG::Uncopyable {
public:
	Buffer (BufferProvider &provider, nframes_t length) :
		m_provider(provider),
		m_length(length)
	{}

	virtual ~Buffer () {};

	nframes_t length () {
		return m_length;
	}

	virtual void setLength (nframes_t len) {
		m_length = len;
	}

	virtual void* data() = 0;

	virtual const void* data() const = 0;

protected:
	BufferProvider & m_provider;
	nframes_t m_length;

	friend class SharedBufferPtr;
};


/** TODO: individual impl isn't really needed anymore */
class BufferImpl : public Buffer {
public:
	BufferImpl (BufferProvider &provider, nframes_t length) :
		Buffer(provider, length)
	{
		m_data = malloc(length * sizeof(float));
	}

	BufferImpl (BufferProvider &provider, nframes_t length, void * data) :
		Buffer(provider, length)
	{
		m_data = data;
	}

	~BufferImpl ()
	{
		free(m_data);
	}

	void setLength (nframes_t len)
	{
		Buffer::setLength(len);
		free(m_data);
		m_data = malloc(len * sizeof(float));
	}

	void* data()
	{
		return m_data;
	}

	const void* data() const
	{
		return m_data;
	}

protected:
	void* m_data;
};



class AudioBuffer : public Buffer {

};



class BufferProvider {
public:
	virtual ~BufferProvider() {};
	virtual SharedBufferPtr aquire (nframes_t nframes) = 0;
	virtual SharedBufferPtr zeroBuffer () const = 0;

protected:
	virtual void release (Buffer * buf) = 0;
	friend class SharedBufferPtr;
};


class SharedBufferPtr : public QSharedPointer<Buffer> {
public:
	SharedBufferPtr () :
		QSharedPointer<Buffer>()
	{}

	SharedBufferPtr (Buffer * buf) :
		QSharedPointer<Buffer>( buf, SharedBufferPtr::deleter )
	{}

protected:
	static void deleter (Buffer * buf) {
		buf->m_provider.release(buf);
	}
};



class PooledBufferProvider : public BufferProvider {
public:
	PooledBufferProvider () :
		m_buffers(),
		m_zeroBuffer(NULL),
		m_periodLength(),
		m_next(0)
	{}

	~PooledBufferProvider() {
	}

	void hackInit (int poolSize) {
		m_buffers.resize(poolSize);
		for (int i=0; i<poolSize; ++i) {
			m_buffers.replace(i, new BufferImpl(*this, bufferLength()));
			for (int j=0; j<bufferLength(); ++j) {
				float * data = (float*)m_buffers[i]->data();
				data[j] = 0.0f;
			}
		}

		m_zeroBuffer = aquire (bufferLength());
	}


	SharedBufferPtr aquire (nframes_t nframes) {
		//std::cout << "Aquiring buffer" << std::endl;
		//TODO assert(nframes == m_periodLength);
		return m_buffers[m_next++];
	}

	SharedBufferPtr zeroBuffer () const {
		return m_zeroBuffer;
	}

	void setBufferLength (nframes_t nframes) {
		m_periodLength = nframes;
	/*
		m_zeroBuffer = aquire (nframes);
		for (int i=0; i<nframes; ++i) {
			// TODO: Initialize buffer to silent!!
		}
	*/
	}

	nframes_t bufferLength () {
		return m_periodLength;
	}

	/*
	void resize (int size) {
		int oldSize = m_buffers.size();
		if (size > oldSize) {
			m_buffers.resize(size);
			for (int i=oldSize; i<size; ++i) {
				m_buffers[i] = new Buffer(
		}
	}*/

protected:
	void release (Buffer * buf) {
		if (buf->length() != bufferLength()) {
			// FIXME
			std::cout << "Deleting buffer, wrong size!" << std::endl;
			delete buf;
		}
		else {
			//std::cout << "Repooling buffer" << std::endl;
			m_buffers[--m_next] = buf;
		}
	}

protected:
	QVector<Buffer*> m_buffers;
	SharedBufferPtr m_zeroBuffer;
	nframes_t m_periodLength;
	int m_next;
};


} // Unison


#endif
