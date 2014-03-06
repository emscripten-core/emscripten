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

///The 3 following lines include the CPU implementation of the kernels, keep them in this order.
#include "BulletMultiThreaded/btGpuDefines.h"
#include "BulletMultiThreaded/btGpuUtilsSharedDefs.h"
#include "BulletMultiThreaded/btGpuUtilsSharedCode.h"



#include "LinearMath/btAlignedAllocator.h"
#include "LinearMath/btQuickprof.h"
#include "BulletCollision/BroadphaseCollision/btOverlappingPairCache.h"



#include "btGpuDefines.h"
#include "btGpuUtilsSharedDefs.h"

#include "btGpu3DGridBroadphaseSharedDefs.h"

#include "btGpu3DGridBroadphase.h"
#include <string.h> //for memset


#include <stdio.h>



static bt3DGridBroadphaseParams s3DGridBroadphaseParams;



btGpu3DGridBroadphase::btGpu3DGridBroadphase(	const btVector3& worldAabbMin,const btVector3& worldAabbMax, 
										int gridSizeX, int gridSizeY, int gridSizeZ, 
										int maxSmallProxies, int maxLargeProxies, int maxPairsPerBody,
										int maxBodiesPerCell,
										btScalar cellFactorAABB) :
	btSimpleBroadphase(maxSmallProxies,
//				     new (btAlignedAlloc(sizeof(btSortedOverlappingPairCache),16)) btSortedOverlappingPairCache),
				     new (btAlignedAlloc(sizeof(btHashedOverlappingPairCache),16)) btHashedOverlappingPairCache),
	m_bInitialized(false),
    m_numBodies(0)
{
	_initialize(worldAabbMin, worldAabbMax, gridSizeX, gridSizeY, gridSizeZ, 
				maxSmallProxies, maxLargeProxies, maxPairsPerBody,
				maxBodiesPerCell, cellFactorAABB);
}



btGpu3DGridBroadphase::btGpu3DGridBroadphase(	btOverlappingPairCache* overlappingPairCache,
										const btVector3& worldAabbMin,const btVector3& worldAabbMax, 
										int gridSizeX, int gridSizeY, int gridSizeZ, 
										int maxSmallProxies, int maxLargeProxies, int maxPairsPerBody,
										int maxBodiesPerCell,
										btScalar cellFactorAABB) :
	btSimpleBroadphase(maxSmallProxies, overlappingPairCache),
	m_bInitialized(false),
    m_numBodies(0)
{
	_initialize(worldAabbMin, worldAabbMax, gridSizeX, gridSizeY, gridSizeZ, 
				maxSmallProxies, maxLargeProxies, maxPairsPerBody,
				maxBodiesPerCell, cellFactorAABB);
}



btGpu3DGridBroadphase::~btGpu3DGridBroadphase()
{
	//btSimpleBroadphase will free memory of btSortedOverlappingPairCache, because m_ownsPairCache
	assert(m_bInitialized);
	_finalize();
}



void btGpu3DGridBroadphase::_initialize(	const btVector3& worldAabbMin,const btVector3& worldAabbMax, 
										int gridSizeX, int gridSizeY, int gridSizeZ, 
										int maxSmallProxies, int maxLargeProxies, int maxPairsPerBody,
										int maxBodiesPerCell,
										btScalar cellFactorAABB)
{
	// set various paramerers
	m_ownsPairCache = true;
	m_params.m_gridSizeX = gridSizeX;
	m_params.m_gridSizeY = gridSizeY;
	m_params.m_gridSizeZ = gridSizeZ;
	m_params.m_numCells = m_params.m_gridSizeX * m_params.m_gridSizeY * m_params.m_gridSizeZ;
	btVector3 w_org = worldAabbMin;
	m_params.m_worldOriginX = w_org.getX();
	m_params.m_worldOriginY = w_org.getY();
	m_params.m_worldOriginZ = w_org.getZ();
	btVector3 w_size = worldAabbMax - worldAabbMin;
	m_params.m_cellSizeX = w_size.getX() / m_params.m_gridSizeX;
	m_params.m_cellSizeY = w_size.getY() / m_params.m_gridSizeY;
	m_params.m_cellSizeZ = w_size.getZ() / m_params.m_gridSizeZ;
	m_maxRadius = btMin(btMin(m_params.m_cellSizeX, m_params.m_cellSizeY), m_params.m_cellSizeZ);
	m_maxRadius *= btScalar(0.5f);
	m_params.m_numBodies = m_numBodies;
	m_params.m_maxBodiesPerCell = maxBodiesPerCell;

	m_numLargeHandles = 0;						
	m_maxLargeHandles = maxLargeProxies;

	m_maxPairsPerBody = maxPairsPerBody;

	m_cellFactorAABB = cellFactorAABB;

	m_LastLargeHandleIndex = -1;

    assert(!m_bInitialized);
    // allocate host storage
    m_hBodiesHash = new unsigned int[m_maxHandles * 2];
    memset(m_hBodiesHash, 0x00, m_maxHandles*2*sizeof(unsigned int));

    m_hCellStart = new unsigned int[m_params.m_numCells];
    memset(m_hCellStart, 0x00, m_params.m_numCells * sizeof(unsigned int));

	m_hPairBuffStartCurr = new unsigned int[m_maxHandles * 2 + 2];
	// --------------- for now, init with m_maxPairsPerBody for each body
	m_hPairBuffStartCurr[0] = 0;
	m_hPairBuffStartCurr[1] = 0;
	for(int i = 1; i <= m_maxHandles; i++) 
	{
		m_hPairBuffStartCurr[i * 2] = m_hPairBuffStartCurr[(i-1) * 2] + m_maxPairsPerBody;
		m_hPairBuffStartCurr[i * 2 + 1] = 0;
	}
	//----------------
	unsigned int numAABB = m_maxHandles + m_maxLargeHandles;
	m_hAABB = new bt3DGrid3F1U[numAABB * 2]; // AABB Min & Max

	m_hPairBuff = new unsigned int[m_maxHandles * m_maxPairsPerBody];
	memset(m_hPairBuff, 0x00, m_maxHandles * m_maxPairsPerBody * sizeof(unsigned int)); // needed?

	m_hPairScan = new unsigned int[m_maxHandles + 1];

	m_hPairOut = new unsigned int[m_maxHandles * m_maxPairsPerBody];

// large proxies

	// allocate handles buffer and put all handles on free list
	m_pLargeHandlesRawPtr = btAlignedAlloc(sizeof(btSimpleBroadphaseProxy) * m_maxLargeHandles, 16);
	m_pLargeHandles = new(m_pLargeHandlesRawPtr) btSimpleBroadphaseProxy[m_maxLargeHandles];
	m_firstFreeLargeHandle = 0;
	{
		for (int i = m_firstFreeLargeHandle; i < m_maxLargeHandles; i++)
		{
			m_pLargeHandles[i].SetNextFree(i + 1);
			m_pLargeHandles[i].m_uniqueId = m_maxHandles+2+i;
		}
		m_pLargeHandles[m_maxLargeHandles - 1].SetNextFree(0);
	}

// debug data
	m_numPairsAdded = 0;
	m_numOverflows = 0;

    m_bInitialized = true;
}



void btGpu3DGridBroadphase::_finalize()
{
    assert(m_bInitialized);
    delete [] m_hBodiesHash;
    delete [] m_hCellStart;
    delete [] m_hPairBuffStartCurr;
    delete [] m_hAABB;
	delete [] m_hPairBuff;
	delete [] m_hPairScan;
	delete [] m_hPairOut;
	btAlignedFree(m_pLargeHandlesRawPtr);
	m_bInitialized = false;
}



void btGpu3DGridBroadphase::calculateOverlappingPairs(btDispatcher* dispatcher)
{
	if(m_numHandles <= 0)
	{
		BT_PROFILE("addLarge2LargePairsToCache");
		addLarge2LargePairsToCache(dispatcher);
		return;
	}
	// update constants
	setParameters(&m_params);
	// prepare AABB array
	prepareAABB();
	// calculate hash
	calcHashAABB();
	// sort bodies based on hash
	sortHash();
	// find start of each cell
	findCellStart();
	// findOverlappingPairs (small/small)
	findOverlappingPairs();
	// findOverlappingPairs (small/large)
	findPairsLarge();
	// add pairs to CPU cache
	computePairCacheChanges();
	scanOverlappingPairBuff();
	squeezeOverlappingPairBuff();
	addPairsToCache(dispatcher);
	// find and add large/large pairs to CPU cache
	addLarge2LargePairsToCache(dispatcher);
	return;
}



void btGpu3DGridBroadphase::addPairsToCache(btDispatcher* dispatcher)
{
	m_numPairsAdded = 0;
	m_numPairsRemoved = 0;
	for(int i = 0; i < m_numHandles; i++) 
	{
		unsigned int num = m_hPairScan[i+1] - m_hPairScan[i];
		if(!num)
		{
			continue;
		}
		unsigned int* pInp = m_hPairOut + m_hPairScan[i];
		unsigned int index0 = m_hAABB[i * 2].uw;
		btSimpleBroadphaseProxy* proxy0 = &m_pHandles[index0];
		for(unsigned int j = 0; j < num; j++)
		{
			unsigned int indx1_s = pInp[j];
			unsigned int index1 = indx1_s & (~BT_3DGRID_PAIR_ANY_FLG);
			btSimpleBroadphaseProxy* proxy1;
			if(index1 < (unsigned int)m_maxHandles)
			{
				proxy1 = &m_pHandles[index1];
			}
			else
			{
				index1 -= m_maxHandles;
				btAssert((index1 >= 0) && (index1 < (unsigned int)m_maxLargeHandles));
				proxy1 = &m_pLargeHandles[index1];
			}
			if(indx1_s & BT_3DGRID_PAIR_NEW_FLG)
			{
				m_pairCache->addOverlappingPair(proxy0,proxy1);
				m_numPairsAdded++;
			}
			else
			{
				m_pairCache->removeOverlappingPair(proxy0,proxy1,dispatcher);
				m_numPairsRemoved++;
			}
		}
	}
}



btBroadphaseProxy* btGpu3DGridBroadphase::createProxy(  const btVector3& aabbMin,  const btVector3& aabbMax,int shapeType,void* userPtr ,short int collisionFilterGroup,short int collisionFilterMask, btDispatcher* dispatcher,void* multiSapProxy)
{
	btBroadphaseProxy*  proxy;
	bool bIsLarge = isLargeProxy(aabbMin, aabbMax);
	if(bIsLarge)
	{
		if (m_numLargeHandles >= m_maxLargeHandles)
		{
			///you have to increase the cell size, so 'large' proxies become 'small' proxies (fitting a cell)
			btAssert(0);
			return 0; //should never happen, but don't let the game crash ;-)
		}
		btAssert((aabbMin[0]<= aabbMax[0]) && (aabbMin[1]<= aabbMax[1]) && (aabbMin[2]<= aabbMax[2]));
		int newHandleIndex = allocLargeHandle();
		proxy = new (&m_pLargeHandles[newHandleIndex])btSimpleBroadphaseProxy(aabbMin,aabbMax,shapeType,userPtr,collisionFilterGroup,collisionFilterMask,multiSapProxy);
	}
	else
	{
		proxy = btSimpleBroadphase::createProxy(aabbMin, aabbMax, shapeType, userPtr, collisionFilterGroup, collisionFilterMask, dispatcher, multiSapProxy);
	}
	return proxy;
}



void btGpu3DGridBroadphase::destroyProxy(btBroadphaseProxy* proxy, btDispatcher* dispatcher)
{
	bool bIsLarge = isLargeProxy(proxy);
	if(bIsLarge)
	{
		
		btSimpleBroadphaseProxy* proxy0 = static_cast<btSimpleBroadphaseProxy*>(proxy);
		freeLargeHandle(proxy0);
		m_pairCache->removeOverlappingPairsContainingProxy(proxy,dispatcher);
	}
	else
	{
		btSimpleBroadphase::destroyProxy(proxy, dispatcher);
	}
	return;
}



void btGpu3DGridBroadphase::resetPool(btDispatcher* dispatcher)
{
	m_hPairBuffStartCurr[0] = 0;
	m_hPairBuffStartCurr[1] = 0;
	for(int i = 1; i <= m_maxHandles; i++) 
	{
		m_hPairBuffStartCurr[i * 2] = m_hPairBuffStartCurr[(i-1) * 2] + m_maxPairsPerBody;
		m_hPairBuffStartCurr[i * 2 + 1] = 0;
	}
}



bool btGpu3DGridBroadphase::isLargeProxy(const btVector3& aabbMin,  const btVector3& aabbMax)
{
	btVector3 diag = aabbMax - aabbMin;
	
	///use the bounding sphere radius of this bounding box, to include rotation
	btScalar radius = diag.length() * btScalar(0.5f);
	radius *= m_cellFactorAABB; // user-defined factor

	return (radius > m_maxRadius);
}



bool btGpu3DGridBroadphase::isLargeProxy(btBroadphaseProxy* proxy)
{
	return (proxy->getUid() >= (m_maxHandles+2));
}



void btGpu3DGridBroadphase::addLarge2LargePairsToCache(btDispatcher* dispatcher)
{
	int i,j;
	if (m_numLargeHandles <= 0)
	{
		return;
	}
	int new_largest_index = -1;
	for(i = 0; i <= m_LastLargeHandleIndex; i++)
	{
		btSimpleBroadphaseProxy* proxy0 = &m_pLargeHandles[i];
		if(!proxy0->m_clientObject)
		{
			continue;
		}
		new_largest_index = i;
		for(j = i + 1; j <= m_LastLargeHandleIndex; j++)
		{
			btSimpleBroadphaseProxy* proxy1 = &m_pLargeHandles[j];
			if(!proxy1->m_clientObject)
			{
				continue;
			}
			btAssert(proxy0 != proxy1);
			btSimpleBroadphaseProxy* p0 = getSimpleProxyFromProxy(proxy0);
			btSimpleBroadphaseProxy* p1 = getSimpleProxyFromProxy(proxy1);
			if(aabbOverlap(p0,p1))
			{
				if (!m_pairCache->findPair(proxy0,proxy1))
				{
					m_pairCache->addOverlappingPair(proxy0,proxy1);
				}
			} 
			else
			{
				if(m_pairCache->findPair(proxy0,proxy1))
				{
					m_pairCache->removeOverlappingPair(proxy0,proxy1,dispatcher);
				}
			}
		}
	}
	m_LastLargeHandleIndex = new_largest_index;
	return;
}



void btGpu3DGridBroadphase::rayTest(const btVector3& rayFrom,const btVector3& rayTo, btBroadphaseRayCallback& rayCallback)
{
	btSimpleBroadphase::rayTest(rayFrom, rayTo, rayCallback);
	for (int i=0; i <= m_LastLargeHandleIndex; i++)
	{
		btSimpleBroadphaseProxy* proxy = &m_pLargeHandles[i];
		if(!proxy->m_clientObject)
		{
			continue;
		}
		rayCallback.process(proxy);
	}
}



//
// overrides for CPU version
//



void btGpu3DGridBroadphase::prepareAABB()
{
	BT_PROFILE("prepareAABB");
	bt3DGrid3F1U* pBB = m_hAABB;
	int i;
	int new_largest_index = -1;
	unsigned int num_small = 0;
	for(i = 0; i <= m_LastHandleIndex; i++) 
	{
		btSimpleBroadphaseProxy* proxy0 = &m_pHandles[i];
		if(!proxy0->m_clientObject)
		{
			continue;
		}
		new_largest_index = i;
		pBB->fx = proxy0->m_aabbMin.getX();
		pBB->fy = proxy0->m_aabbMin.getY();
		pBB->fz = proxy0->m_aabbMin.getZ();
		pBB->uw = i;
		pBB++;
		pBB->fx = proxy0->m_aabbMax.getX();
		pBB->fy = proxy0->m_aabbMax.getY();
		pBB->fz = proxy0->m_aabbMax.getZ();
		pBB->uw = num_small;
		pBB++;
		num_small++;
	}
	m_LastHandleIndex = new_largest_index;
	new_largest_index = -1;
	unsigned int num_large = 0;
	for(i = 0; i <= m_LastLargeHandleIndex; i++) 
	{
		btSimpleBroadphaseProxy* proxy0 = &m_pLargeHandles[i];
		if(!proxy0->m_clientObject)
		{
			continue;
		}
		new_largest_index = i;
		pBB->fx = proxy0->m_aabbMin.getX();
		pBB->fy = proxy0->m_aabbMin.getY();
		pBB->fz = proxy0->m_aabbMin.getZ();
		pBB->uw = i + m_maxHandles;
		pBB++;
		pBB->fx = proxy0->m_aabbMax.getX();
		pBB->fy = proxy0->m_aabbMax.getY();
		pBB->fz = proxy0->m_aabbMax.getZ();
		pBB->uw = num_large + m_maxHandles;
		pBB++;
		num_large++;
	}
	m_LastLargeHandleIndex = new_largest_index;
	// paranoid checks
	btAssert(num_small == m_numHandles);
	btAssert(num_large == m_numLargeHandles);
	return;
}



void btGpu3DGridBroadphase::setParameters(bt3DGridBroadphaseParams* hostParams)
{
	s3DGridBroadphaseParams = *hostParams;
	return;
}



void btGpu3DGridBroadphase::calcHashAABB()
{
	BT_PROFILE("bt3DGrid_calcHashAABB");
	btGpu_calcHashAABB(m_hAABB, m_hBodiesHash, m_numHandles);
	return;
}



void btGpu3DGridBroadphase::sortHash()
{
	class bt3DGridHashKey
	{
	public:
	   unsigned int hash;
	   unsigned int index;
	   void quickSort(bt3DGridHashKey* pData, int lo, int hi)
	   {
			int i=lo, j=hi;
			bt3DGridHashKey x = pData[(lo+hi)/2];
			do
			{    
				while(pData[i].hash > x.hash) i++; 
				while(x.hash > pData[j].hash) j--;
				if(i <= j)
				{
					bt3DGridHashKey t = pData[i];
					pData[i] = pData[j];
					pData[j] = t;
					i++; j--;
				}
			} while(i <= j);
			if(lo < j) pData->quickSort(pData, lo, j);
			if(i < hi) pData->quickSort(pData, i, hi);
	   }
	};
	BT_PROFILE("bt3DGrid_sortHash");
	bt3DGridHashKey* pHash = (bt3DGridHashKey*)m_hBodiesHash;
	pHash->quickSort(pHash, 0, m_numHandles - 1);
	return;
}



void btGpu3DGridBroadphase::findCellStart()
{
	BT_PROFILE("bt3DGrid_findCellStart");
	btGpu_findCellStart(m_hBodiesHash, m_hCellStart, m_numHandles, m_params.m_numCells);
	return;
}



void btGpu3DGridBroadphase::findOverlappingPairs()
{
	BT_PROFILE("bt3DGrid_findOverlappingPairs");
	btGpu_findOverlappingPairs(m_hAABB, m_hBodiesHash, m_hCellStart, m_hPairBuff, m_hPairBuffStartCurr, m_numHandles);
	return;
}



void btGpu3DGridBroadphase::findPairsLarge()
{
	BT_PROFILE("bt3DGrid_findPairsLarge");
	btGpu_findPairsLarge(m_hAABB, m_hBodiesHash, m_hCellStart, m_hPairBuff, m_hPairBuffStartCurr,	m_numHandles, m_numLargeHandles);
	return;
}



void btGpu3DGridBroadphase::computePairCacheChanges()
{
	BT_PROFILE("bt3DGrid_computePairCacheChanges");
	btGpu_computePairCacheChanges(m_hPairBuff, m_hPairBuffStartCurr, m_hPairScan, m_hAABB, m_numHandles);
	return;
}



void btGpu3DGridBroadphase::scanOverlappingPairBuff()
{
	BT_PROFILE("bt3DGrid_scanOverlappingPairBuff");
	m_hPairScan[0] = 0;
	for(int i = 1; i <= m_numHandles; i++) 
	{
		unsigned int delta = m_hPairScan[i];
		m_hPairScan[i] = m_hPairScan[i-1] + delta;
	}
	return;
}



void btGpu3DGridBroadphase::squeezeOverlappingPairBuff()
{
	BT_PROFILE("bt3DGrid_squeezeOverlappingPairBuff");
	btGpu_squeezeOverlappingPairBuff(m_hPairBuff, m_hPairBuffStartCurr, m_hPairScan, m_hPairOut, m_hAABB, m_numHandles);
	return;
}



#include "btGpu3DGridBroadphaseSharedCode.h"


