/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://bulletphysics.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#ifndef BT_DOUBLE_BUFFER_H
#define BT_DOUBLE_BUFFER_H

#include "SpuFakeDma.h"
#include "LinearMath/btScalar.h"


///DoubleBuffer
template<class T, int size>
class DoubleBuffer
{
#if defined(__SPU__) || defined(USE_LIBSPE2)
	ATTRIBUTE_ALIGNED128( T m_buffer0[size] ) ;
	ATTRIBUTE_ALIGNED128( T m_buffer1[size] ) ;
#else
	T m_buffer0[size];
	T m_buffer1[size];
#endif
	
	T *m_frontBuffer;
	T *m_backBuffer;

	unsigned int m_dmaTag;
	bool m_dmaPending;
public:
	bool	isPending() const { return m_dmaPending;}
	DoubleBuffer();

	void init ();

	// dma get and put commands
	void backBufferDmaGet(uint64_t ea, unsigned int numBytes, unsigned int tag);
	void backBufferDmaPut(uint64_t ea, unsigned int numBytes, unsigned int tag);

	// gets pointer to a buffer
	T *getFront();
	T *getBack();

	// if back buffer dma was started, wait for it to complete
	// then move back to front and vice versa
	T *swapBuffers();
};

template<class T, int size>
DoubleBuffer<T,size>::DoubleBuffer()
{
	init ();
}

template<class T, int size>
void DoubleBuffer<T,size>::init()
{
	this->m_dmaPending = false;
	this->m_frontBuffer = &this->m_buffer0[0];
	this->m_backBuffer = &this->m_buffer1[0];
}

template<class T, int size>
void
DoubleBuffer<T,size>::backBufferDmaGet(uint64_t ea, unsigned int numBytes, unsigned int tag)
{
	m_dmaPending = true;
	m_dmaTag = tag;
	if (numBytes)
	{
		m_backBuffer = (T*)cellDmaLargeGetReadOnly(m_backBuffer, ea, numBytes, tag, 0, 0);
	}
}

template<class T, int size>
void
DoubleBuffer<T,size>::backBufferDmaPut(uint64_t ea, unsigned int numBytes, unsigned int tag)
{
	m_dmaPending = true;
	m_dmaTag = tag;
	cellDmaLargePut(m_backBuffer, ea, numBytes, tag, 0, 0);
}

template<class T, int size>
T *
DoubleBuffer<T,size>::getFront()
{
	return m_frontBuffer;
}

template<class T, int size>
T *
DoubleBuffer<T,size>::getBack()
{
	return m_backBuffer;
}

template<class T, int size>
T *
DoubleBuffer<T,size>::swapBuffers()
{
	if (m_dmaPending)
	{
		cellDmaWaitTagStatusAll(1<<m_dmaTag);
		m_dmaPending = false;
	}

	T *tmp = m_backBuffer;
	m_backBuffer = m_frontBuffer;
	m_frontBuffer = tmp;

	return m_frontBuffer;
}

#endif
