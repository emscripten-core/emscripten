/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_SOFT_BODY_SOLVER_BUFFER_OPENCL_H
#define BT_SOFT_BODY_SOLVER_BUFFER_OPENCL_H

// OpenCL support

#ifdef USE_MINICL
	#include "MiniCL/cl.h"
#else //USE_MINICL
	#ifdef __APPLE__
		#include <OpenCL/OpenCL.h>
	#else
		#include <CL/cl.h>
	#endif //__APPLE__
#endif//USE_MINICL

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

template <typename ElementType> class btOpenCLBuffer
{
public:

	cl_command_queue	m_cqCommandQue;
	cl_context			m_clContext;
	cl_mem				m_buffer;



	btAlignedObjectArray< ElementType > * m_CPUBuffer;
	
	int  m_gpuSize;
	bool m_onGPU;
	bool m_readOnlyOnGPU;
	bool m_allocated;


	bool createBuffer( cl_mem* preexistingBuffer = 0)
	{

		cl_int err;
		 

		if( preexistingBuffer )
		{
			m_buffer = *preexistingBuffer;
		} 
		else {

			cl_mem_flags flags= m_readOnlyOnGPU ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE;

			size_t size = m_CPUBuffer->size() * sizeof(ElementType);
			// At a minimum the buffer must exist
			if( size == 0 )
				size = sizeof(ElementType);
			m_buffer = clCreateBuffer(m_clContext, flags, size, 0, &err);
			if( err != CL_SUCCESS )
			{
				btAssert( "Buffer::Buffer(m_buffer)");
			}
		}

		m_gpuSize = m_CPUBuffer->size();

		return true;
	}

public:
	btOpenCLBuffer( cl_command_queue	commandQue,cl_context ctx, btAlignedObjectArray< ElementType >* CPUBuffer, bool readOnly)
		:m_cqCommandQue(commandQue),
		m_clContext(ctx),
		m_buffer(0),
		m_CPUBuffer(CPUBuffer),
		m_gpuSize(0),
		m_onGPU(false),
		m_readOnlyOnGPU(readOnly),
		m_allocated(false)
	{
	}

	~btOpenCLBuffer()
	{
		clReleaseMemObject(m_buffer);
	}


	bool moveToGPU()
	{


		cl_int err;

		if( (m_CPUBuffer->size() != m_gpuSize) )
		{
			m_onGPU = false;
		}

		if( !m_allocated && m_CPUBuffer->size() == 0  )
		{
			// If it isn't on the GPU and yet there is no data on the CPU side this may cause a problem with some kernels.
			// We should create *something* on the device side
			if (!createBuffer()) {
				return false;
			}
			m_allocated = true;
		}

		if( !m_onGPU && m_CPUBuffer->size() > 0 )
		{
			if (!m_allocated || (m_CPUBuffer->size() != m_gpuSize)) {
				if (!createBuffer()) {
					return false;
				}
				m_allocated = true;
			}
			
			size_t size = m_CPUBuffer->size() * sizeof(ElementType);
			err = clEnqueueWriteBuffer(m_cqCommandQue,m_buffer,
				CL_FALSE,
				0,
				size, 
				&((*m_CPUBuffer)[0]),0,0,0);
			if( err != CL_SUCCESS )
			{
				btAssert( "CommandQueue::enqueueWriteBuffer(m_buffer)" );
			}

			m_onGPU = true;
		}

		return true;

	}

	bool moveFromGPU()
	{

		cl_int err;

		if (m_CPUBuffer->size() > 0) {
			if (m_onGPU && !m_readOnlyOnGPU) {
				size_t size = m_CPUBuffer->size() * sizeof(ElementType);
				err = clEnqueueReadBuffer(m_cqCommandQue,
					m_buffer,
					CL_TRUE,
					0,
					size,
					&((*m_CPUBuffer)[0]),0,0,0);

				if( err != CL_SUCCESS )
				{
					btAssert( "CommandQueue::enqueueReadBuffer(m_buffer)" );
				}

				m_onGPU = false;
			}
		}

		return true;
	}

	bool copyFromGPU()
	{

		cl_int err;
		size_t size = m_CPUBuffer->size() * sizeof(ElementType);

		if (m_CPUBuffer->size() > 0) {
			if (m_onGPU && !m_readOnlyOnGPU) {
				err = clEnqueueReadBuffer(m_cqCommandQue,
					m_buffer,
					CL_TRUE,
					0,size, 
					&((*m_CPUBuffer)[0]),0,0,0);

				if( err != CL_SUCCESS )
				{
					btAssert( "CommandQueue::enqueueReadBuffer(m_buffer)");
				}

			}
		}

		return true;
	}

	virtual void changedOnCPU()
	{
		m_onGPU = false;
	}
}; // class btOpenCLBuffer


#endif // #ifndef BT_SOFT_BODY_SOLVER_BUFFER_OPENCL_H