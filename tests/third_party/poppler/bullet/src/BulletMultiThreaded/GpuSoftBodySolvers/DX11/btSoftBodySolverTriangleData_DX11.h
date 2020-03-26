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
#include "btSoftBodySolverBuffer_DX11.h"


#ifndef BT_SOFT_BODY_SOLVER_TRIANGLE_DATA_DX11_H
#define BT_SOFT_BODY_SOLVER_TRIANGLE_DATA_DX11_H

struct ID3D11Device;
struct ID3D11DeviceContext;

class btSoftBodyTriangleDataDX11 : public btSoftBodyTriangleData
{
public:
	bool				m_onGPU;
	ID3D11Device		*m_d3dDevice;
	ID3D11DeviceContext *m_d3dDeviceContext;

	btDX11Buffer<btSoftBodyTriangleData::TriangleNodeSet>							m_dx11VertexIndices;
	btDX11Buffer<float>									m_dx11Area;
	btDX11Buffer<Vectormath::Aos::Vector3>				m_dx11Normal;

	struct BatchPair
	{
		int start;
		int length;

		BatchPair() :
			start(0),
			length(0)
		{
		}

		BatchPair( int s, int l ) : 
			start( s ),
			length( l )
		{
		}
	};


	/**
	 * Link addressing information for each cloth.
	 * Allows link locations to be computed independently of data batching.
	 */
	btAlignedObjectArray< int >							m_triangleAddresses;

	/**
	 * Start and length values for computation batches over link data.
	 */
	btAlignedObjectArray< BatchPair >		m_batchStartLengths;

	//ID3D11Buffer*               readBackBuffer;

public:
	btSoftBodyTriangleDataDX11( ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dDeviceContext );

	virtual ~btSoftBodyTriangleDataDX11();


	/** Allocate enough space in all link-related arrays to fit numLinks links */
	virtual void createTriangles( int numTriangles );
	
	/** Insert the link described into the correct data structures assuming space has already been allocated by a call to createLinks */
	virtual void setTriangleAt( const btSoftBodyTriangleData::TriangleDescription &triangle, int triangleIndex );

	virtual bool onAccelerator();
	virtual bool moveToAccelerator();

	virtual bool moveFromAccelerator();
	/**
	 * Generate (and later update) the batching for the entire triangle set.
	 * This redoes a lot of work because it batches the entire set when each cloth is inserted.
	 * In theory we could delay it until just before we need the cloth.
	 * It's a one-off overhead, though, so that is a later optimisation.
	 */
	void generateBatches();
};



#endif // #ifndef BT_SOFT_BODY_SOLVER_TRIANGLE_DATA_DX11_H
