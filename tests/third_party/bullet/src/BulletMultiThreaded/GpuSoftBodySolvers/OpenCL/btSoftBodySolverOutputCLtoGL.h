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

#ifndef BT_SOFT_BODY_SOLVER_OUTPUT_CL_TO_GL_H
#define BT_SOFT_BODY_SOLVER_OUTPUT_CL_TO_GL_H

#include "btSoftBodySolver_OpenCL.h"

/** 
 * Class to manage movement of data from a solver to a given target.
 * This version is the CL to GL interop version.
 */
class btSoftBodySolverOutputCLtoGL : public btSoftBodySolverOutput
{
protected:
	cl_command_queue	m_cqCommandQue;
	cl_context			m_cxMainContext;
	CLFunctions			clFunctions;
	
	cl_kernel		outputToVertexArrayWithNormalsKernel;
	cl_kernel		outputToVertexArrayWithoutNormalsKernel;

	bool m_shadersInitialized;
	
	virtual bool checkInitialized();	
	virtual bool buildShaders();
	void releaseKernels();
public:
	btSoftBodySolverOutputCLtoGL(cl_command_queue cqCommandQue, cl_context cxMainContext) :
		m_cqCommandQue( cqCommandQue ),
		m_cxMainContext( cxMainContext ),
		clFunctions(cqCommandQue, cxMainContext),
		outputToVertexArrayWithNormalsKernel( 0 ),
		outputToVertexArrayWithoutNormalsKernel( 0 ),
		m_shadersInitialized( false )
	{
	}

	virtual ~btSoftBodySolverOutputCLtoGL()
	{
		releaseKernels();
	}

	/** Output current computed vertex data to the vertex buffers for all cloths in the solver. */
	virtual void copySoftBodyToVertexBuffer( const btSoftBody * const softBody, btVertexBufferDescriptor *vertexBuffer );
};



#endif // #ifndef BT_SOFT_BODY_SOLVER_OUTPUT_CL_TO_GL_H