/*
Bullet Continuous Collision Detection and Physics Library, http://bulletphysics.org
Copyright (C) 2006, 2009 Sony Computer Entertainment Inc. 

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

//----------------------------------------------------------------------------------------

#ifndef BTGPU3DGRIDBROADPHASE_H
#define BTGPU3DGRIDBROADPHASE_H

//----------------------------------------------------------------------------------------

#include "BulletCollision/BroadphaseCollision/btSimpleBroadphase.h"

#include "btGpu3DGridBroadphaseSharedTypes.h"

//----------------------------------------------------------------------------------------

///The btGpu3DGridBroadphase uses GPU-style code compiled for CPU to compute overlapping pairs

class btGpu3DGridBroadphase : public btSimpleBroadphase
{
protected:
	bool			m_bInitialized;
    unsigned int	m_numBodies;
    unsigned int	m_numCells;
	unsigned int	m_maxPairsPerBody;
	btScalar		m_cellFactorAABB;
    unsigned int	m_maxBodiesPerCell;
	bt3DGridBroadphaseParams m_params;
	btScalar		m_maxRadius;
	// CPU data
    unsigned int*	m_hBodiesHash;
    unsigned int*	m_hCellStart;
	unsigned int*	m_hPairBuffStartCurr;
	bt3DGrid3F1U*		m_hAABB;
	unsigned int*	m_hPairBuff;
	unsigned int*	m_hPairScan;
	unsigned int*	m_hPairOut;
// large proxies
	int		m_numLargeHandles;						
	int		m_maxLargeHandles;						
	int		m_LastLargeHandleIndex;							
	btSimpleBroadphaseProxy* m_pLargeHandles;
	void* m_pLargeHandlesRawPtr;
	int		m_firstFreeLargeHandle;
	int allocLargeHandle()
	{
		btAssert(m_numLargeHandles < m_maxLargeHandles);
		int freeLargeHandle = m_firstFreeLargeHandle;
		m_firstFreeLargeHandle = m_pLargeHandles[freeLargeHandle].GetNextFree();
		m_numLargeHandles++;
		if(freeLargeHandle > m_LastLargeHandleIndex)
		{
			m_LastLargeHandleIndex = freeLargeHandle;
		}
		return freeLargeHandle;
	}
	void freeLargeHandle(btSimpleBroadphaseProxy* proxy)
	{
		int handle = int(proxy - m_pLargeHandles);
		btAssert((handle >= 0) && (handle < m_maxHandles));
		if(handle == m_LastLargeHandleIndex)
		{
			m_LastLargeHandleIndex--;
		}
		proxy->SetNextFree(m_firstFreeLargeHandle);
		m_firstFreeLargeHandle = handle;
		proxy->m_clientObject = 0;
		m_numLargeHandles--;
	}
	bool isLargeProxy(const btVector3& aabbMin,  const btVector3& aabbMax);
	bool isLargeProxy(btBroadphaseProxy* proxy);
// debug
	unsigned int	m_numPairsAdded;
	unsigned int	m_numPairsRemoved;
	unsigned int	m_numOverflows;
// 
public:
	btGpu3DGridBroadphase(const btVector3& worldAabbMin,const btVector3& worldAabbMax, 
					   int gridSizeX, int gridSizeY, int gridSizeZ, 
					   int maxSmallProxies, int maxLargeProxies, int maxPairsPerBody,
					   int maxBodiesPerCell = 8,
					   btScalar cellFactorAABB = btScalar(1.0f));
	btGpu3DGridBroadphase(	btOverlappingPairCache* overlappingPairCache,
						const btVector3& worldAabbMin,const btVector3& worldAabbMax, 
						int gridSizeX, int gridSizeY, int gridSizeZ, 
						int maxSmallProxies, int maxLargeProxies, int maxPairsPerBody,
						int maxBodiesPerCell = 8,
						btScalar cellFactorAABB = btScalar(1.0f));
	virtual ~btGpu3DGridBroadphase();
	virtual void	calculateOverlappingPairs(btDispatcher* dispatcher);

	virtual btBroadphaseProxy*	createProxy(const btVector3& aabbMin,  const btVector3& aabbMax,int shapeType,void* userPtr ,short int collisionFilterGroup,short int collisionFilterMask, btDispatcher* dispatcher,void* multiSapProxy);
	virtual void	destroyProxy(btBroadphaseProxy* proxy,btDispatcher* dispatcher);
	virtual void	rayTest(const btVector3& rayFrom,const btVector3& rayTo, btBroadphaseRayCallback& rayCallback);
	virtual void	resetPool(btDispatcher* dispatcher);

protected:
	void _initialize(	const btVector3& worldAabbMin,const btVector3& worldAabbMax, 
						int gridSizeX, int gridSizeY, int gridSizeZ, 
						int maxSmallProxies, int maxLargeProxies, int maxPairsPerBody,
						int maxBodiesPerCell = 8,
						btScalar cellFactorAABB = btScalar(1.0f));
	void _finalize();
	void addPairsToCache(btDispatcher* dispatcher);
	void addLarge2LargePairsToCache(btDispatcher* dispatcher);

// overrides for CPU version
	virtual void setParameters(bt3DGridBroadphaseParams* hostParams);
	virtual void prepareAABB();
	virtual void calcHashAABB();
	virtual void sortHash();	
	virtual void findCellStart();
	virtual void findOverlappingPairs();
	virtual void findPairsLarge();
	virtual void computePairCacheChanges();
	virtual void scanOverlappingPairBuff();
	virtual void squeezeOverlappingPairBuff();
};

//----------------------------------------------------------------------------------------

#endif //BTGPU3DGRIDBROADPHASE_H

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
