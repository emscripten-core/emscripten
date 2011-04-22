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

#include "vectormath/vmInclude.h"
#include "btSoftBodySolver_DX11.h"
#include "btSoftBodySolverVertexBuffer_DX11.h"
#include "btSoftBodySolverLinkData_DX11SIMDAware.h"
#include "btSoftBodySolverVertexData_DX11.h"
#include "btSoftBodySolverTriangleData_DX11.h"


#ifndef BT_SOFT_BODY_DX11_SOLVER_SIMDAWARE_H
#define BT_SOFT_BODY_DX11_SOLVER_SIMDAWARE_H

class btDX11SIMDAwareSoftBodySolver : public btDX11SoftBodySolver
{
protected:
	struct SolvePositionsFromLinksKernelCB
	{		
		int startWave;
		int numWaves;
		float kst;
		float ti;
	};


	/** Link data for all cloths. Note that this will be sorted batch-wise for efficient computation and m_linkAddresses will maintain the addressing. */
	btSoftBodyLinkDataDX11SIMDAware m_linkData;
		
	/** Variable to define whether we need to update solver constants on the next iteration */
	bool m_updateSolverConstants;

	
	virtual bool buildShaders();

	void updateConstants( float timeStep );


	//////////////////////////////////////
	// Kernel dispatches
	

	void solveLinksForPosition( int startLink, int numLinks, float kst, float ti );

	// End kernel dispatches
	/////////////////////////////////////



public:
	btDX11SIMDAwareSoftBodySolver(ID3D11Device * dx11Device, ID3D11DeviceContext* dx11Context, DXFunctions::CompileFromMemoryFunc dx11CompileFromMemory = &D3DX11CompileFromMemory);

	virtual ~btDX11SIMDAwareSoftBodySolver();

	virtual btSoftBodyLinkData &getLinkData();

	virtual void optimize( btAlignedObjectArray< btSoftBody * > &softBodies , bool forceUpdate=false);

	virtual void solveConstraints( float solverdt );
	
	virtual SolverTypes getSolverType() const
	{
		return DX_SIMD_SOLVER;
	}
	
};

#endif // #ifndef BT_SOFT_BODY_DX11_SOLVER_SIMDAWARE_H

