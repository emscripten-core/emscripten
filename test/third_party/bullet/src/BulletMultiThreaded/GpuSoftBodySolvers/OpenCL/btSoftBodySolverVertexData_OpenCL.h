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

#include "BulletMultiThreaded/GpuSoftBodySolvers/CPU/btSoftBodySolverData.h"
#include "btSoftBodySolverBuffer_OpenCL.h"

#ifndef BT_SOFT_BODY_SOLVER_VERTEX_DATA_OPENCL_H
#define BT_SOFT_BODY_SOLVER_VERTEX_DATA_OPENCL_H


class btSoftBodyVertexDataOpenCL : public btSoftBodyVertexData
{
protected:
	bool		m_onGPU;
	cl_command_queue	m_queue;

public:
	btOpenCLBuffer<int>									m_clClothIdentifier;
	btOpenCLBuffer<Vectormath::Aos::Point3>				m_clVertexPosition;
	btOpenCLBuffer<Vectormath::Aos::Point3>				m_clVertexPreviousPosition;
	btOpenCLBuffer<Vectormath::Aos::Vector3>				m_clVertexVelocity;
	btOpenCLBuffer<Vectormath::Aos::Vector3>				m_clVertexForceAccumulator;
	btOpenCLBuffer<Vectormath::Aos::Vector3>				m_clVertexNormal;
	btOpenCLBuffer<float>									m_clVertexInverseMass;
	btOpenCLBuffer<float>									m_clVertexArea;
	btOpenCLBuffer<int>									m_clVertexTriangleCount;
public:
	btSoftBodyVertexDataOpenCL( cl_command_queue queue,  cl_context ctx);

	virtual ~btSoftBodyVertexDataOpenCL();

	virtual bool onAccelerator();

	virtual bool moveToAccelerator();

	virtual bool moveFromAccelerator();
};


#endif // #ifndef BT_SOFT_BODY_SOLVER_VERTEX_DATA_OPENCL_H
