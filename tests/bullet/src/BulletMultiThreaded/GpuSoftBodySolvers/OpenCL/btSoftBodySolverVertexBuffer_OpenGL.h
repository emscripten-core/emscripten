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

#ifndef BT_SOFT_BODY_SOLVER_VERTEX_BUFFER_OPENGL_H
#define BT_SOFT_BODY_SOLVER_VERTEX_BUFFER_OPENGL_H 


#include "BulletSoftBody/btSoftBodySolverVertexBuffer.h"
#ifdef USE_MINICL
	#include "MiniCL/cl.h"
#else //USE_MINICL
	#ifdef __APPLE__
		#include <OpenCL/OpenCL.h>
	#else
		#include <CL/cl.h>
		#include <CL/cl_gl.h>
	#endif //__APPLE__
#endif//USE_MINICL


#ifdef _WIN32//for glut.h
#include <windows.h>
#endif

//think different
#if defined(__APPLE__) && !defined (VMDMESA)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else


#ifdef _WINDOWS
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/glut.h>
#endif //_WINDOWS
#endif //APPLE



class btOpenGLInteropVertexBufferDescriptor : public btVertexBufferDescriptor
{
protected:
	/** OpenCL context */
	cl_context			m_context;

	/** OpenCL command queue */
	cl_command_queue	m_commandQueue;
	
	/** OpenCL interop buffer */
	cl_mem m_buffer;

	/** VBO in GL that is the basis of the interop buffer */
	GLuint m_openGLVBO;


public:
	/**
	 * context is the OpenCL context this interop buffer will work in.
	 * queue is the command queue that kernels and data movement will be enqueued into.
	 * openGLVBO is the OpenGL vertex buffer data will be copied into.
	 * vertexOffset is the offset in floats to the first vertex.
	 * vertexStride is the stride in floats between vertices.
	 */
	btOpenGLInteropVertexBufferDescriptor( cl_command_queue cqCommandQue, cl_context context, GLuint openGLVBO, int vertexOffset, int vertexStride )
	{
#ifndef USE_MINICL
		cl_int ciErrNum = CL_SUCCESS;
		m_context = context;
		m_commandQueue = cqCommandQue;
		
		m_vertexOffset = vertexOffset;
		m_vertexStride = vertexStride;

		m_openGLVBO = openGLVBO;
		
		m_buffer = clCreateFromGLBuffer(m_context, CL_MEM_WRITE_ONLY, openGLVBO, &ciErrNum);
		if( ciErrNum != CL_SUCCESS )
		{
			btAssert( 0 &&  "clEnqueueAcquireGLObjects(copySoftBodyToVertexBuffer)");
		}

		m_hasVertexPositions = true;
#else
		btAssert(0);//MiniCL shouldn't get here
#endif
	}

	/**
	 * context is the OpenCL context this interop buffer will work in.
	 * queue is the command queue that kernels and data movement will be enqueued into.
	 * openGLVBO is the OpenGL vertex buffer data will be copied into.
	 * vertexOffset is the offset in floats to the first vertex.
	 * vertexStride is the stride in floats between vertices.
	 * normalOffset is the offset in floats to the first normal.
	 * normalStride is the stride in floats between normals.
	 */
	btOpenGLInteropVertexBufferDescriptor( cl_command_queue cqCommandQue, cl_context context, GLuint openGLVBO, int vertexOffset, int vertexStride, int normalOffset, int normalStride )
	{
#ifndef USE_MINICL
		cl_int ciErrNum = CL_SUCCESS;
		m_context = context;
		m_commandQueue = cqCommandQue;
		
		m_openGLVBO = openGLVBO;
		
		m_buffer = clCreateFromGLBuffer(m_context, CL_MEM_WRITE_ONLY, openGLVBO, &ciErrNum);
		if( ciErrNum != CL_SUCCESS )
		{
			btAssert( 0 &&  "clEnqueueAcquireGLObjects(copySoftBodyToVertexBuffer)");
		}

		m_vertexOffset = vertexOffset;
		m_vertexStride = vertexStride;
		m_hasVertexPositions = true;

		m_normalOffset = normalOffset;
		m_normalStride = normalStride;
		m_hasNormals = true;
#else
		btAssert(0);
#endif //USE_MINICL
		
	}

	virtual ~btOpenGLInteropVertexBufferDescriptor()
	{
		clReleaseMemObject( m_buffer );
	}

	/**
	 * Return the type of the vertex buffer descriptor.
	 */
	virtual BufferTypes getBufferType() const
	{
		return OPENGL_BUFFER;
	}

	virtual cl_context getContext() const
	{
		return m_context;
	}

	virtual cl_mem getBuffer() const
	{
		return m_buffer;
	}	
};

#endif // #ifndef BT_SOFT_BODY_SOLVER_VERTEX_BUFFER_OPENGL_H
