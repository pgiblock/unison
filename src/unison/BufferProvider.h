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

#include <QStack>
#include <QSharedPointer>

#include "prg/Uncopyable.h"
#include "unison/types.h"

namespace Unison {

class BufferProvider;
class SharedBufferPtr;

const nframes_t MAX_BUFFER_LENGTH = 4096;

class Buffer : PRG::Uncopyable {
public:
	Buffer (BufferProvider &provider) :
		m_provider(provider)
	{}

	virtual ~Buffer () {};

	virtual PortType type() = 0;

	virtual void* data() = 0;

	virtual const void* data() const = 0;

protected:
	BufferProvider & m_provider;

	friend class SharedBufferPtr;
};


class AudioBuffer : public Buffer {
public:
	AudioBuffer (BufferProvider &provider, nframes_t length) :
		Buffer(provider),
		m_length(length)
	{
		m_data = (float*)malloc(length * sizeof(float));
	}

	AudioBuffer (BufferProvider &provider, nframes_t length, void * data) :
		Buffer(provider),
		m_length(length)
	{
		m_data = (float*)data;
	}

	~AudioBuffer ()
	{
		free(m_data);
	}

	PortType type () {
		return AUDIO_PORT;
	}

	nframes_t length () {
		return m_length;
	}

	void setLength (nframes_t len)
	{
		m_length = len;
		free(m_data);
		m_data = (float*)malloc(len * sizeof(float));
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
	int m_length;
	float* m_data;
};



class ControlBuffer : public Buffer {
public:
	ControlBuffer (BufferProvider &provider) :
		Buffer(provider)
	{
		m_data = 0.0f;
	}

	~ControlBuffer()
	{}

	PortType type () {
		return CONTROL_PORT;
	}

	void* data()
	{
		return &m_data;
	}

	const void* data() const
	{
		return &m_data;
	}

protected:
	float m_data;
};


class BufferProvider {
public:
	virtual ~BufferProvider() {};
	virtual SharedBufferPtr aquire (PortType type, nframes_t nframes) = 0;
	virtual SharedBufferPtr zeroAudioBuffer () const = 0;

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
		m_audioBuffers(),
		m_controlBuffers(),
		m_zeroBuffer(NULL),
		m_periodLength(),
		m_next(0)
	{}

	~PooledBufferProvider() {
	}

	/*
	void hackInit (int poolSize) {
		m_buffers.resize(poolSize);
		for (int i=0; i<poolSize; ++i) {
			m_buffers.replace(i, new BufferImpl(*this, bufferLength()));
			for (int j=0; j<bufferLength(); ++j) {
				float * data = (float*)m_buffers[i]->data();
				data[j] = 0.0f;
			}
		}
	}*/


	SharedBufferPtr aquire (PortType type, nframes_t nframes) {
		//TODO assert(nframes == m_periodLength); (or whatever)
		QStack<Buffer*> * stack;
		switch (type) {
		case AUDIO_PORT:
			stack = &m_audioBuffers;
			break;
		case CONTROL_PORT:
			stack = &m_controlBuffers;
			break;
		default:
			// TODO: assert(false);
			return NULL;
		}

		if (!stack->isEmpty()) {
			return stack->pop();
		}

		//TODO ensure we are not in processing thread
		switch (type) {
		case AUDIO_PORT:
			std::cout << "New Audio Buffer " << nframes << " frames." << std::endl;
			return new AudioBuffer(*this, nframes);
		case CONTROL_PORT:
			std::cout << "New Control Buffer" << std::endl;
			return new ControlBuffer(*this);
		default:
			std::cout << "Couldn't create unknown port" << std::endl;
			// TODO: assert(false);
			return NULL;
		}
	}


	SharedBufferPtr zeroAudioBuffer () const {
		return m_zeroBuffer;
	}

	void setBufferLength (nframes_t nframes) {
		m_periodLength = nframes;

		m_zeroBuffer = aquire (AUDIO_PORT, nframes);
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
		switch (buf->type()) {
		case AUDIO_PORT:
			if (((AudioBuffer*)buf)->length() != bufferLength()) {
				// FIXME
				std::cout << "Deleting buffer, wrong size!" << std::endl;
				delete buf;
			}
			else {
				m_audioBuffers.push(buf);
			}
			break;

		case CONTROL_PORT:
			m_controlBuffers.push(buf);
			break;
		default:
			// TODO: Unhandled port!!
			break;
		}
	}

protected:
	// TODO: Use something RT-safe, instead of QStack
	QStack<Buffer*> m_audioBuffers;
	QStack<Buffer*> m_controlBuffers;
	SharedBufferPtr m_zeroBuffer;
	nframes_t m_periodLength;
	int m_next;
};


} // Unison


#endif
