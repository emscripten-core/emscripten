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

#ifndef BT_ACCELERATED_SOFT_BODY_LINK_DATA_DX11_SIMDAWARE_H
#define BT_ACCELERATED_SOFT_BODY_LINK_DATA_DX11_SIMDAWARE_H

struct ID3D11Device;
struct ID3D11DeviceContext;


class btSoftBodyLinkDataDX11SIMDAware : public btSoftBodyLinkData
{
public:
	bool				m_onGPU;
	ID3D11Device		*m_d3dDevice;
	ID3D11DeviceContext *m_d3dDeviceContext;

	const int m_wavefrontSize;
	const int m_linksPerWorkItem;
	const int m_maxLinksPerWavefront;
	int m_maxBatchesWithinWave;
	int m_maxVerticesWithinWave;
	int m_numWavefronts;

	int m_maxVertex;

	struct NumBatchesVerticesPair
	{
		int numBatches;
		int numVertices;
	};

	// Array storing number of links in each wavefront
	btAlignedObjectArray<int>									m_linksPerWavefront;
	btAlignedObjectArray<NumBatchesVerticesPair>				m_numBatchesAndVerticesWithinWaves;
	btDX11Buffer< NumBatchesVerticesPair >						m_dx11NumBatchesAndVerticesWithinWaves;

	// All arrays here will contain batches of m_maxLinksPerWavefront links
	// ordered by wavefront.
	// with either global vertex pairs or local vertex pairs
	btAlignedObjectArray< int >									m_wavefrontVerticesGlobalAddresses; // List of global vertices per wavefront
	btDX11Buffer<int>											m_dx11WavefrontVerticesGlobalAddresses;
	btAlignedObjectArray< LinkNodePair >						m_linkVerticesLocalAddresses; // Vertex pair for the link
	btDX11Buffer<LinkNodePair>									m_dx11LinkVerticesLocalAddresses;
	btDX11Buffer<float>											m_dx11LinkStrength;
	btDX11Buffer<float>											m_dx11LinksMassLSC;
	btDX11Buffer<float>											m_dx11LinksRestLengthSquared;
	btDX11Buffer<float>											m_dx11LinksRestLength;
	btDX11Buffer<float>											m_dx11LinksMaterialLinearStiffnessCoefficient;

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
	btAlignedObjectArray< int >							m_linkAddresses;

	/**
	 * Start and length values for computation batches over link data.
	 */
	btAlignedObjectArray< BatchPair >		m_wavefrontBatchStartLengths;


	//ID3D11Buffer*               readBackBuffer;
	
	btSoftBodyLinkDataDX11SIMDAware( ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dDeviceContext );

	virtual ~btSoftBodyLinkDataDX11SIMDAware();

	/** Allocate enough space in all link-related arrays to fit numLinks links */
	virtual void createLinks( int numLinks );
	
	/** Insert the link described into the correct data structures assuming space has already been allocated by a call to createLinks */
	virtual void setLinkAt( const LinkDescription &link, int linkIndex );

	virtual bool onAccelerator();

	virtual bool moveToAccelerator();

	virtual bool moveFromAccelerator();

	/**
	 * Generate (and later update) the batching for the entire link set.
	 * This redoes a lot of work because it batches the entire set when each cloth is inserted.
	 * In theory we could delay it until just before we need the cloth.
	 * It's a one-off overhead, though, so that is a later optimisation.
	 */
	void generateBatches();

	int getMaxVerticesPerWavefront()
	{
		return m_maxVerticesWithinWave;
	}

	int getWavefrontSize()
	{
		return m_wavefrontSize;
	}

	int getLinksPerWorkItem()
	{
		return m_linksPerWorkItem;
	}

	int getMaxLinksPerWavefront()
	{
		return m_maxLinksPerWavefront;
	}

	int getMaxBatchesPerWavefront()
	{
		return m_maxBatchesWithinWave;
	}

	int getNumWavefronts()
	{
		return m_numWavefronts;
	}

	NumBatchesVerticesPair getNumBatchesAndVerticesWithinWavefront( int wavefront )
	{
		return m_numBatchesAndVerticesWithinWaves[wavefront];
	}

	int getVertexGlobalAddresses( int vertexIndex )
	{
		return m_wavefrontVerticesGlobalAddresses[vertexIndex];
	}

	/**
	 * Get post-batching local addresses of the vertex pair for a link assuming all vertices used by a wavefront are loaded locally.
	 */
	LinkNodePair getVertexPairLocalAddresses( int linkIndex )
	{
		return m_linkVerticesLocalAddresses[linkIndex];
	}

};


#endif // #ifndef BT_ACCELERATED_SOFT_BODY_LINK_DATA_DX11_SIMDAWARE_H
