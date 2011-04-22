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

#ifndef BT_SOFT_BODY_SOLVER_OPENCL_SIMDAWARE_H
#define BT_SOFT_BODY_SOLVER_OPENCL_SIMDAWARE_H

#include "stddef.h" //for size_t
#include "vectormath/vmInclude.h"

#include "btSoftBodySolver_OpenCL.h"
#include "btSoftBodySolverBuffer_OpenCL.h"
#include "btSoftBodySolverLinkData_OpenCLSIMDAware.h"
#include "btSoftBodySolverVertexData_OpenCL.h"
#include "btSoftBodySolverTriangleData_OpenCL.h"





class btOpenCLSoftBodySolverSIMDAware : public btOpenCLSoftBodySolver
{
protected:
	

	btSoftBodyLinkDataOpenCLSIMDAware m_linkData;


	bool m_shadersInitialized;


	bool buildShaders();


	void updateConstants( float timeStep );

	float computeTriangleArea( 
		const Vectormath::Aos::Point3 &vertex0,
		const Vectormath::Aos::Point3 &vertex1,
		const Vectormath::Aos::Point3 &vertex2 );


	//////////////////////////////////////
	// Kernel dispatches
	void solveLinksForPosition( int startLink, int numLinks, float kst, float ti );
	
	void solveCollisionsAndUpdateVelocities( float isolverdt );
	// End kernel dispatches
	/////////////////////////////////////

public:
	btOpenCLSoftBodySolverSIMDAware(cl_command_queue queue,cl_context	ctx);

	virtual ~btOpenCLSoftBodySolverSIMDAware();

	virtual SolverTypes getSolverType() const
	{
		return CL_SIMD_SOLVER;
	}


	virtual btSoftBodyLinkData &getLinkData();


	virtual void optimize( btAlignedObjectArray< btSoftBody * > &softBodies , bool forceUpdate=false);

	virtual void solveConstraints( float solverdt );

}; // btOpenCLSoftBodySolverSIMDAware

#endif // #ifndef BT_SOFT_BODY_SOLVER_OPENCL_SIMDAWARE_H
