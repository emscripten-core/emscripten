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

#include "btMultiSapBroadphase.h"

#include "btSimpleBroadphase.h"
#include "LinearMath/btAabbUtil2.h"
#include "btQuantizedBvh.h"

///	btSapBroadphaseArray	m_sapBroadphases;

///	btOverlappingPairCache*	m_overlappingPairs;
extern int gOverlappingPairs;

/*
class btMultiSapSortedOverlappingPairCache : public btSortedOverlappingPairCache
{
public:

	virtual btBroadphasePair*	addOverlappingPair(btBroadphaseProxy* proxy0,btBroadphaseProxy* proxy1)
	{
		return btSortedOverlappingPairCache::addOverlappingPair((btBroadphaseProxy*)proxy0->m_multiSapParentProxy,(btBroadphaseProxy*)proxy1->m_multiSapParentProxy);
	}
};

*/

btMultiSapBroadphase::btMultiSapBroadphase(int /*maxProxies*/,btOverlappingPairCache* pairCache)
:m_overlappingPairs(pairCache),
m_optimizedAabbTree(0),
m_ownsPairCache(false),
m_invalidPair(0)
{
	if (!m_overlappingPairs)
	{
		m_ownsPairCache = true;
		void* mem = btAlignedAlloc(sizeof(btSortedOverlappingPairCache),16);
		m_overlappingPairs = new (mem)btSortedOverlappingPairCache();
	}

	struct btMultiSapOverlapFilterCallback : public btOverlapFilterCallback
	{
		virtual ~btMultiSapOverlapFilterCallback()
		{}
		// return true when pairs need collision
		virtual bool	needBroadphaseCollision(btBroadphaseProxy* childProxy0,btBroadphaseProxy* childProxy1) const
		{
			btBroadphaseProxy* multiProxy0 = (btBroadphaseProxy*)childProxy0->m_multiSapParentProxy;
			btBroadphaseProxy* multiProxy1 = (btBroadphaseProxy*)childProxy1->m_multiSapParentProxy;
			
			bool collides = (multiProxy0->m_collisionFilterGroup & multiProxy1->m_collisionFilterMask) != 0;
			collides = collides && (multiProxy1->m_collisionFilterGroup & multiProxy0->m_collisionFilterMask);
	
			return collides;
		}
	};

	void* mem = btAlignedAlloc(sizeof(btMultiSapOverlapFilterCallback),16);
	m_filterCallback = new (mem)btMultiSapOverlapFilterCallback();

	m_overlappingPairs->setOverlapFilterCallback(m_filterCallback);
//	mem = btAlignedAlloc(sizeof(btSimpleBroadphase),16);
//	m_simpleBroadphase = new (mem) btSimpleBroadphase(maxProxies,m_overlappingPairs);
}

btMultiSapBroadphase::~btMultiSapBroadphase()
{
	if (m_ownsPairCache)
	{
		m_overlappingPairs->~btOverlappingPairCache();
		btAlignedFree(m_overlappingPairs);
	}
}


void	btMultiSapBroadphase::buildTree(const btVector3& bvhAabbMin,const btVector3& bvhAabbMax)
{
	m_optimizedAabbTree = new btQuantizedBvh();
	m_optimizedAabbTree->setQuantizationValues(bvhAabbMin,bvhAabbMax);
	QuantizedNodeArray&	nodes = m_optimizedAabbTree->getLeafNodeArray();
	for (int i=0;i<m_sapBroadphases.size();i++)
	{
		btQuantizedBvhNode node;
		btVector3 aabbMin,aabbMax;
		m_sapBroadphases[i]->getBroadphaseAabb(aabbMin,aabbMax);
		m_optimizedAabbTree->quantize(&node.m_quantizedAabbMin[0],aabbMin,0);
		m_optimizedAabbTree->quantize(&node.m_quantizedAabbMax[0],aabbMax,1);
		int partId = 0;
		node.m_escapeIndexOrTriangleIndex = (partId<<(31-MAX_NUM_PARTS_IN_BITS)) | i;
		nodes.push_back(node);
	}
	m_optimizedAabbTree->buildInternal();
}

btBroadphaseProxy*	btMultiSapBroadphase::createProxy(  const btVector3& aabbMin,  const btVector3& aabbMax,int shapeType,void* userPtr, short int collisionFilterGroup,short int collisionFilterMask, btDispatcher* dispatcher,void* /*ignoreMe*/)
{
	//void* ignoreMe -> we could think of recursive multi-sap, if someone is interested

	void* mem = btAlignedAlloc(sizeof(btMultiSapProxy),16);
	btMultiSapProxy* proxy = new (mem)btMultiSapProxy(aabbMin,  aabbMax,shapeType,userPtr, collisionFilterGroup,collisionFilterMask);
	m_multiSapProxies.push_back(proxy);

	///this should deal with inserting/removal into child broadphases
	setAabb(proxy,aabbMin,aabbMax,dispatcher);
	return proxy;
}

void	btMultiSapBroadphase::destroyProxy(btBroadphaseProxy* /*proxy*/,btDispatcher* /*dispatcher*/)
{
	///not yet
	btAssert(0);

}


void	btMultiSapBroadphase::addToChildBroadphase(btMultiSapProxy* parentMultiSapProxy, btBroadphaseProxy* childProxy, btBroadphaseInterface*	childBroadphase)
{
	void* mem = btAlignedAlloc(sizeof(btBridgeProxy),16);
	btBridgeProxy* bridgeProxyRef = new(mem) btBridgeProxy;
	bridgeProxyRef->m_childProxy = childProxy;
	bridgeProxyRef->m_childBroadphase = childBroadphase;
	parentMultiSapProxy->m_bridgeProxies.push_back(bridgeProxyRef);
}


bool boxIsContainedWithinBox(const btVector3& amin,const btVector3& amax,const btVector3& bmin,const btVector3& bmax);
bool boxIsContainedWithinBox(const btVector3& amin,const btVector3& amax,const btVector3& bmin,const btVector3& bmax)
{
return
amin.getX() >= bmin.getX() && amax.getX() <= bmax.getX() &&
amin.getY() >= bmin.getY() && amax.getY() <= bmax.getY() &&
amin.getZ() >= bmin.getZ() && amax.getZ() <= bmax.getZ();
}






void	btMultiSapBroadphase::getAabb(btBroadphaseProxy* proxy,btVector3& aabbMin, btVector3& aabbMax ) const
{
	btMultiSapProxy* multiProxy = static_cast<btMultiSapProxy*>(proxy);
	aabbMin = multiProxy->m_aabbMin;
	aabbMax = multiProxy->m_aabbMax;
}

void	btMultiSapBroadphase::rayTest(const btVector3& rayFrom,const btVector3& rayTo, btBroadphaseRayCallback& rayCallback, const btVector3& aabbMin,const btVector3& aabbMax)
{
	for (int i=0;i<m_multiSapProxies.size();i++)
	{
		rayCallback.process(m_multiSapProxies[i]);
	}
}


//#include <stdio.h>

void	btMultiSapBroadphase::setAabb(btBroadphaseProxy* proxy,const btVector3& aabbMin,const btVector3& aabbMax, btDispatcher* dispatcher)
{
	btMultiSapProxy* multiProxy = static_cast<btMultiSapProxy*>(proxy);
	multiProxy->m_aabbMin = aabbMin;
	multiProxy->m_aabbMax = aabbMax;
	
	
//	bool fullyContained = false;
//	bool alreadyInSimple = false;
	


	
	struct MyNodeOverlapCallback : public btNodeOverlapCallback
	{
		btMultiSapBroadphase*	m_multiSap;
		btMultiSapProxy*		m_multiProxy;
		btDispatcher*			m_dispatcher;

		MyNodeOverlapCallback(btMultiSapBroadphase* multiSap,btMultiSapProxy* multiProxy,btDispatcher* dispatcher)
			:m_multiSap(multiSap),
			m_multiProxy(multiProxy),
			m_dispatcher(dispatcher)
		{

		}

		virtual void processNode(int /*nodeSubPart*/, int broadphaseIndex)
		{
			btBroadphaseInterface* childBroadphase = m_multiSap->getBroadphaseArray()[broadphaseIndex];

			int containingBroadphaseIndex = -1;
			//already found?
			for (int i=0;i<m_multiProxy->m_bridgeProxies.size();i++)
			{

				if (m_multiProxy->m_bridgeProxies[i]->m_childBroadphase == childBroadphase)
				{
					containingBroadphaseIndex = i;
					break;
				}
			}
			if (containingBroadphaseIndex<0)
			{
				//add it
				btBroadphaseProxy* childProxy = childBroadphase->createProxy(m_multiProxy->m_aabbMin,m_multiProxy->m_aabbMax,m_multiProxy->m_shapeType,m_multiProxy->m_clientObject,m_multiProxy->m_collisionFilterGroup,m_multiProxy->m_collisionFilterMask, m_dispatcher,m_multiProxy);
				m_multiSap->addToChildBroadphase(m_multiProxy,childProxy,childBroadphase);

			}
		}
	};

	MyNodeOverlapCallback	myNodeCallback(this,multiProxy,dispatcher);



	
	if (m_optimizedAabbTree)
		m_optimizedAabbTree->reportAabbOverlappingNodex(&myNodeCallback,aabbMin,aabbMax);

	int i;

	for ( i=0;i<multiProxy->m_bridgeProxies.size();i++)
	{
		btVector3 worldAabbMin,worldAabbMax;
		multiProxy->m_bridgeProxies[i]->m_childBroadphase->getBroadphaseAabb(worldAabbMin,worldAabbMax);
		bool overlapsBroadphase = TestAabbAgainstAabb2(worldAabbMin,worldAabbMax,multiProxy->m_aabbMin,multiProxy->m_aabbMax);
		if (!overlapsBroadphase)
		{
			//remove it now
			btBridgeProxy* bridgeProxy = multiProxy->m_bridgeProxies[i];

			btBroadphaseProxy* childProxy = bridgeProxy->m_childProxy;
			bridgeProxy->m_childBroadphase->destroyProxy(childProxy,dispatcher);
			
			multiProxy->m_bridgeProxies.swap( i,multiProxy->m_bridgeProxies.size()-1);
			multiProxy->m_bridgeProxies.pop_back();

		}
	}


	/*

	if (1)
	{

		//find broadphase that contain this multiProxy
		int numChildBroadphases = getBroadphaseArray().size();
		for (int i=0;i<numChildBroadphases;i++)
		{
			btBroadphaseInterface* childBroadphase = getBroadphaseArray()[i];
			btVector3 worldAabbMin,worldAabbMax;
			childBroadphase->getBroadphaseAabb(worldAabbMin,worldAabbMax);
			bool overlapsBroadphase = TestAabbAgainstAabb2(worldAabbMin,worldAabbMax,multiProxy->m_aabbMin,multiProxy->m_aabbMax);
			
		//	fullyContained = fullyContained || boxIsContainedWithinBox(worldAabbMin,worldAabbMax,multiProxy->m_aabbMin,multiProxy->m_aabbMax);
			int containingBroadphaseIndex = -1;
			
			//if already contains this
			
			for (int i=0;i<multiProxy->m_bridgeProxies.size();i++)
			{
				if (multiProxy->m_bridgeProxies[i]->m_childBroadphase == childBroadphase)
				{
					containingBroadphaseIndex = i;
				}
				alreadyInSimple = alreadyInSimple || (multiProxy->m_bridgeProxies[i]->m_childBroadphase == m_simpleBroadphase);
			}

			if (overlapsBroadphase)
			{
				if (containingBroadphaseIndex<0)
				{
					btBroadphaseProxy* childProxy = childBroadphase->createProxy(aabbMin,aabbMax,multiProxy->m_shapeType,multiProxy->m_clientObject,multiProxy->m_collisionFilterGroup,multiProxy->m_collisionFilterMask, dispatcher);
					childProxy->m_multiSapParentProxy = multiProxy;
					addToChildBroadphase(multiProxy,childProxy,childBroadphase);
				}
			} else
			{
				if (containingBroadphaseIndex>=0)
				{
					//remove
					btBridgeProxy* bridgeProxy = multiProxy->m_bridgeProxies[containingBroadphaseIndex];

					btBroadphaseProxy* childProxy = bridgeProxy->m_childProxy;
					bridgeProxy->m_childBroadphase->destroyProxy(childProxy,dispatcher);
					
					multiProxy->m_bridgeProxies.swap( containingBroadphaseIndex,multiProxy->m_bridgeProxies.size()-1);
					multiProxy->m_bridgeProxies.pop_back();
				}
			}
		}


		///If we are in no other child broadphase, stick the proxy in the global 'simple' broadphase (brute force)
		///hopefully we don't end up with many entries here (can assert/provide feedback on stats)
		if (0)//!multiProxy->m_bridgeProxies.size())
		{
			///we don't pass the userPtr but our multisap proxy. We need to patch this, before processing an actual collision
			///this is needed to be able to calculate the aabb overlap
			btBroadphaseProxy* childProxy = m_simpleBroadphase->createProxy(aabbMin,aabbMax,multiProxy->m_shapeType,multiProxy->m_clientObject,multiProxy->m_collisionFilterGroup,multiProxy->m_collisionFilterMask, dispatcher);
			childProxy->m_multiSapParentProxy = multiProxy;
			addToChildBroadphase(multiProxy,childProxy,m_simpleBroadphase);
		}
	}

	if (!multiProxy->m_bridgeProxies.size())
	{
		///we don't pass the userPtr but our multisap proxy. We need to patch this, before processing an actual collision
		///this is needed to be able to calculate the aabb overlap
		btBroadphaseProxy* childProxy = m_simpleBroadphase->createProxy(aabbMin,aabbMax,multiProxy->m_shapeType,multiProxy->m_clientObject,multiProxy->m_collisionFilterGroup,multiProxy->m_collisionFilterMask, dispatcher);
		childProxy->m_multiSapParentProxy = multiProxy;
		addToChildBroadphase(multiProxy,childProxy,m_simpleBroadphase);
	}
*/


	//update
	for ( i=0;i<multiProxy->m_bridgeProxies.size();i++)
	{
		btBridgeProxy* bridgeProxyRef = multiProxy->m_bridgeProxies[i];
		bridgeProxyRef->m_childBroadphase->setAabb(bridgeProxyRef->m_childProxy,aabbMin,aabbMax,dispatcher);
	}

}
bool stopUpdating=false;



class btMultiSapBroadphasePairSortPredicate
{
	public:

		bool operator() ( const btBroadphasePair& a1, const btBroadphasePair& b1 )
		{
				btMultiSapBroadphase::btMultiSapProxy* aProxy0 = a1.m_pProxy0 ? (btMultiSapBroadphase::btMultiSapProxy*)a1.m_pProxy0->m_multiSapParentProxy : 0;
				btMultiSapBroadphase::btMultiSapProxy* aProxy1 = a1.m_pProxy1 ? (btMultiSapBroadphase::btMultiSapProxy*)a1.m_pProxy1->m_multiSapParentProxy : 0;
				btMultiSapBroadphase::btMultiSapProxy* bProxy0 = b1.m_pProxy0 ? (btMultiSapBroadphase::btMultiSapProxy*)b1.m_pProxy0->m_multiSapParentProxy : 0;
				btMultiSapBroadphase::btMultiSapProxy* bProxy1 = b1.m_pProxy1 ? (btMultiSapBroadphase::btMultiSapProxy*)b1.m_pProxy1->m_multiSapParentProxy : 0;

				 return aProxy0 > bProxy0 || 
					(aProxy0 == bProxy0 && aProxy1 > bProxy1) ||
					(aProxy0 == bProxy0 && aProxy1 == bProxy1 && a1.m_algorithm > b1.m_algorithm); 
		}
};


        ///calculateOverlappingPairs is optional: incremental algorithms (sweep and prune) might do it during the set aabb
void    btMultiSapBroadphase::calculateOverlappingPairs(btDispatcher* dispatcher)
{

//	m_simpleBroadphase->calculateOverlappingPairs(dispatcher);

	if (!stopUpdating && getOverlappingPairCache()->hasDeferredRemoval())
	{
	
		btBroadphasePairArray&	overlappingPairArray = getOverlappingPairCache()->getOverlappingPairArray();

	//	quicksort(overlappingPairArray,0,overlappingPairArray.size());

		overlappingPairArray.quickSort(btMultiSapBroadphasePairSortPredicate());

		//perform a sort, to find duplicates and to sort 'invalid' pairs to the end
	//	overlappingPairArray.heapSort(btMultiSapBroadphasePairSortPredicate());

		overlappingPairArray.resize(overlappingPairArray.size() - m_invalidPair);
		m_invalidPair = 0;

		
		int i;

		btBroadphasePair previousPair;
		previousPair.m_pProxy0 = 0;
		previousPair.m_pProxy1 = 0;
		previousPair.m_algorithm = 0;
		
		
		for (i=0;i<overlappingPairArray.size();i++)
		{
		
			btBroadphasePair& pair = overlappingPairArray[i];

			btMultiSapProxy* aProxy0 = pair.m_pProxy0 ? (btMultiSapProxy*)pair.m_pProxy0->m_multiSapParentProxy : 0;
			btMultiSapProxy* aProxy1 = pair.m_pProxy1 ? (btMultiSapProxy*)pair.m_pProxy1->m_multiSapParentProxy : 0;
			btMultiSapProxy* bProxy0 = previousPair.m_pProxy0 ? (btMultiSapProxy*)previousPair.m_pProxy0->m_multiSapParentProxy : 0;
			btMultiSapProxy* bProxy1 = previousPair.m_pProxy1 ? (btMultiSapProxy*)previousPair.m_pProxy1->m_multiSapParentProxy : 0;

			bool isDuplicate = (aProxy0 == bProxy0) && (aProxy1 == bProxy1);
			
			previousPair = pair;

			bool needsRemoval = false;

			if (!isDuplicate)
			{
				bool hasOverlap = testAabbOverlap(pair.m_pProxy0,pair.m_pProxy1);

				if (hasOverlap)
				{
					needsRemoval = false;//callback->processOverlap(pair);
				} else
				{
					needsRemoval = true;
				}
			} else
			{
				//remove duplicate
				needsRemoval = true;
				//should have no algorithm
				btAssert(!pair.m_algorithm);
			}
			
			if (needsRemoval)
			{
				getOverlappingPairCache()->cleanOverlappingPair(pair,dispatcher);

		//		m_overlappingPairArray.swap(i,m_overlappingPairArray.size()-1);
		//		m_overlappingPairArray.pop_back();
				pair.m_pProxy0 = 0;
				pair.m_pProxy1 = 0;
				m_invalidPair++;
				gOverlappingPairs--;
			} 
			
		}

	///if you don't like to skip the invalid pairs in the array, execute following code:
	#define CLEAN_INVALID_PAIRS 1
	#ifdef CLEAN_INVALID_PAIRS

		//perform a sort, to sort 'invalid' pairs to the end
		//overlappingPairArray.heapSort(btMultiSapBroadphasePairSortPredicate());
		overlappingPairArray.quickSort(btMultiSapBroadphasePairSortPredicate());

		overlappingPairArray.resize(overlappingPairArray.size() - m_invalidPair);
		m_invalidPair = 0;
	#endif//CLEAN_INVALID_PAIRS
		
		//printf("overlappingPairArray.size()=%d\n",overlappingPairArray.size());
	}


}


bool	btMultiSapBroadphase::testAabbOverlap(btBroadphaseProxy* childProxy0,btBroadphaseProxy* childProxy1)
{
	btMultiSapProxy* multiSapProxy0 = (btMultiSapProxy*)childProxy0->m_multiSapParentProxy;
		btMultiSapProxy* multiSapProxy1 = (btMultiSapProxy*)childProxy1->m_multiSapParentProxy;

		return	TestAabbAgainstAabb2(multiSapProxy0->m_aabbMin,multiSapProxy0->m_aabbMax,
			multiSapProxy1->m_aabbMin,multiSapProxy1->m_aabbMax);
		
}


void	btMultiSapBroadphase::printStats()
{
/*	printf("---------------------------------\n");
	
		printf("btMultiSapBroadphase.h\n");
		printf("numHandles = %d\n",m_multiSapProxies.size());
			//find broadphase that contain this multiProxy
		int numChildBroadphases = getBroadphaseArray().size();
		for (int i=0;i<numChildBroadphases;i++)
		{

			btBroadphaseInterface* childBroadphase = getBroadphaseArray()[i];
			childBroadphase->printStats();

		}
		*/

}

void btMultiSapBroadphase::resetPool(btDispatcher* dispatcher)
{
	// not yet
}
