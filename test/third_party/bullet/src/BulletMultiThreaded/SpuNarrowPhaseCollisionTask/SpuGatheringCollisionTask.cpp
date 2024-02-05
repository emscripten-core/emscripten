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

#include "SpuGatheringCollisionTask.h"

//#define DEBUG_SPU_COLLISION_DETECTION 1
#include "../SpuDoubleBuffer.h"

#include "../SpuCollisionTaskProcess.h"
#include "../SpuGatheringCollisionDispatcher.h" //for SPU_BATCHSIZE_BROADPHASE_PAIRS

#include "BulletCollision/BroadphaseCollision/btBroadphaseProxy.h"
#include "../SpuContactManifoldCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "SpuContactResult.h"
#include "BulletCollision/CollisionShapes/btOptimizedBvh.h"
#include "BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btConvexPointCloudShape.h"

#include "BulletCollision/CollisionShapes/btCapsuleShape.h"

#include "BulletCollision/CollisionShapes/btConvexShape.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"

#include "SpuMinkowskiPenetrationDepthSolver.h"
//#include "SpuEpaPenetrationDepthSolver.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h"


#include "boxBoxDistance.h"
#include "BulletMultiThreaded/vectormath2bullet.h"
#include "SpuCollisionShapes.h" //definition of SpuConvexPolyhedronVertexData
#include "BulletCollision/CollisionDispatch/btBoxBoxDetector.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h"
#include "BulletCollision/CollisionShapes/btTriangleShape.h"

#ifdef __SPU__
///Software caching from the IBM Cell SDK, it reduces 25% SPU time for our test cases
#ifndef USE_LIBSPE2
//#define USE_SOFTWARE_CACHE 1
#endif
#endif //__SPU__

int gSkippedCol = 0;
int gProcessedCol = 0;

////////////////////////////////////////////////
/// software caching
#if USE_SOFTWARE_CACHE
#include <spu_intrinsics.h>
#include <sys/spu_thread.h>
#include <sys/spu_event.h>
#include <stdint.h>
#define SPE_CACHE_NWAY   		4
//#define SPE_CACHE_NSETS 		32, 16
#define SPE_CACHE_NSETS 		8
//#define SPE_CACHELINE_SIZE 		512
#define SPE_CACHELINE_SIZE 		128
#define SPE_CACHE_SET_TAGID(set) 	15
///make sure that spe_cache.h is below those defines!
#include "../Extras/software_cache/cache/include/spe_cache.h"


int g_CacheMisses=0;
int g_CacheHits=0;

#if 0 // Added to allow cache misses and hits to be tracked, change this to 1 to restore unmodified version
#define spe_cache_read(ea)		_spe_cache_lookup_xfer_wait_(ea, 0, 1)
#else
#define spe_cache_read(ea)		\
({								\
    int set, idx, line, byte;					\
    _spe_cache_nway_lookup_(ea, set, idx);			\
								\
    if (btUnlikely(idx < 0)) {					\
        ++g_CacheMisses;                        \
	    idx = _spe_cache_miss_(ea, set, -1);			\
        spu_writech(22, SPE_CACHE_SET_TAGMASK(set));		\
        spu_mfcstat(MFC_TAG_UPDATE_ALL);			\
    } 								\
    else                            \
    {                               \
        ++g_CacheHits;              \
    }                               \
    line = _spe_cacheline_num_(set, idx);			\
    byte = _spe_cacheline_byte_offset_(ea);			\
    (void *) &spe_cache_mem[line + byte];			\
})

#endif

#endif // USE_SOFTWARE_CACHE

bool gUseEpa = false;

#ifdef USE_SN_TUNER
#include <LibSN_SPU.h>
#endif //USE_SN_TUNER

#if defined (__SPU__) && !defined (USE_LIBSPE2)
#include <spu_printf.h>
#elif defined (USE_LIBSPE2)
#define spu_printf(a)
#else
#define IGNORE_ALIGNMENT 1
#include <stdio.h>
#include <stdlib.h>
#define spu_printf printf

#endif

//int gNumConvexPoints0=0;

///Make sure no destructors are called on this memory
struct	CollisionTask_LocalStoreMemory
{
	///This CollisionTask_LocalStoreMemory is mainly used for the SPU version, using explicit DMA
	///Other platforms can use other memory programming models.

	ATTRIBUTE_ALIGNED16(btBroadphasePair	gBroadphasePairsBuffer[SPU_BATCHSIZE_BROADPHASE_PAIRS]);
	DoubleBuffer<unsigned char, MIDPHASE_WORKUNIT_PAGE_SIZE> g_workUnitTaskBuffers;
	ATTRIBUTE_ALIGNED16(char gSpuContactManifoldAlgoBuffer [sizeof(SpuContactManifoldCollisionAlgorithm)+16]);
	ATTRIBUTE_ALIGNED16(char gColObj0Buffer [sizeof(btCollisionObject)+16]);
	ATTRIBUTE_ALIGNED16(char gColObj1Buffer [sizeof(btCollisionObject)+16]);
	///we reserve 32bit integer indices, even though they might be 16bit
	ATTRIBUTE_ALIGNED16(int	spuIndices[16]);
	btPersistentManifold	gPersistentManifoldBuffer;
	CollisionShape_LocalStoreMemory gCollisionShapes[2];
	bvhMeshShape_LocalStoreMemory bvhShapeData;
	SpuConvexPolyhedronVertexData convexVertexData[2];
	CompoundShape_LocalStoreMemory compoundShapeData[2];
		
	///The following pointers might either point into this local store memory, or to the original/other memory locations.
	///See SpuFakeDma for implementation of cellDmaSmallGetReadOnly.
	btCollisionObject*	m_lsColObj0Ptr;
	btCollisionObject*	m_lsColObj1Ptr;
	btBroadphasePair* m_pairsPointer;
	btPersistentManifold*	m_lsManifoldPtr;
	SpuContactManifoldCollisionAlgorithm*	m_lsCollisionAlgorithmPtr;

	bool	needsDmaPutContactManifoldAlgo;

	btCollisionObject* getColObj0()
	{
		return m_lsColObj0Ptr;
	}
	btCollisionObject* getColObj1()
	{
		return m_lsColObj1Ptr;
	}


	btBroadphasePair* getBroadphasePairPtr()
	{
		return m_pairsPointer;
	}

	SpuContactManifoldCollisionAlgorithm*	getlocalCollisionAlgorithm()
	{
		return m_lsCollisionAlgorithmPtr;
	}
	
	btPersistentManifold*	getContactManifoldPtr()
	{
		return m_lsManifoldPtr;
	}
};


#if defined(__CELLOS_LV2__) || defined(USE_LIBSPE2) 

ATTRIBUTE_ALIGNED16(CollisionTask_LocalStoreMemory	gLocalStoreMemory);

void* createCollisionLocalStoreMemory()
{
	return &gLocalStoreMemory;
}
#else
void* createCollisionLocalStoreMemory()
{
        return new CollisionTask_LocalStoreMemory;
}

#endif

void	ProcessSpuConvexConvexCollision(SpuCollisionPairInput* wuInput, CollisionTask_LocalStoreMemory* lsMemPtr, SpuContactResult& spuContacts);


SIMD_FORCE_INLINE void small_cache_read(void* buffer, ppu_address_t ea, size_t size)
{
#if USE_SOFTWARE_CACHE
	// Check for alignment requirements. We need to make sure the entire request fits within one cache line,
	// so the first and last bytes should fall on the same cache line
	btAssert((ea & ~SPE_CACHELINE_MASK) == ((ea + size - 1) & ~SPE_CACHELINE_MASK));

	void* ls = spe_cache_read(ea);
	memcpy(buffer, ls, size);
#else
	stallingUnalignedDmaSmallGet(buffer,ea,size);
#endif
}

SIMD_FORCE_INLINE void small_cache_read_triple(	void* ls0, ppu_address_t ea0,
												void* ls1, ppu_address_t ea1,
												void* ls2, ppu_address_t ea2,
												size_t size)
{
		btAssert(size<16);
		ATTRIBUTE_ALIGNED16(char	tmpBuffer0[32]);
		ATTRIBUTE_ALIGNED16(char	tmpBuffer1[32]);
		ATTRIBUTE_ALIGNED16(char	tmpBuffer2[32]);

		uint32_t i;
		

		///make sure last 4 bits are the same, for cellDmaSmallGet
		char* localStore0 = (char*)ls0;
		uint32_t last4BitsOffset = ea0 & 0x0f;
		char* tmpTarget0 = tmpBuffer0 + last4BitsOffset;
#ifdef __SPU__
		cellDmaSmallGet(tmpTarget0,ea0,size,DMA_TAG(1),0,0);
#else
		tmpTarget0 = (char*)cellDmaSmallGetReadOnly(tmpTarget0,ea0,size,DMA_TAG(1),0,0);
#endif


		char* localStore1 = (char*)ls1;
		last4BitsOffset = ea1 & 0x0f;
		char* tmpTarget1 = tmpBuffer1 + last4BitsOffset;
#ifdef __SPU__
		cellDmaSmallGet(tmpTarget1,ea1,size,DMA_TAG(1),0,0);
#else
		tmpTarget1 = (char*)cellDmaSmallGetReadOnly(tmpTarget1,ea1,size,DMA_TAG(1),0,0);
#endif
		
		char* localStore2 = (char*)ls2;
		last4BitsOffset = ea2 & 0x0f;
		char* tmpTarget2 = tmpBuffer2 + last4BitsOffset;
#ifdef __SPU__
		cellDmaSmallGet(tmpTarget2,ea2,size,DMA_TAG(1),0,0);
#else
		tmpTarget2 = (char*)cellDmaSmallGetReadOnly(tmpTarget2,ea2,size,DMA_TAG(1),0,0);
#endif
		
		
		cellDmaWaitTagStatusAll( DMA_MASK(1) );

		//this is slowish, perhaps memcpy on SPU is smarter?
		for (i=0; btLikely( i<size );i++)
		{
			localStore0[i] = tmpTarget0[i];
			localStore1[i] = tmpTarget1[i];
			localStore2[i] = tmpTarget2[i];
		}

		
}




class spuNodeCallback : public btNodeOverlapCallback
{
	SpuCollisionPairInput* m_wuInput;
	SpuContactResult&		m_spuContacts;
	CollisionTask_LocalStoreMemory*	m_lsMemPtr;
	ATTRIBUTE_ALIGNED16(btTriangleShape)	m_tmpTriangleShape;

	ATTRIBUTE_ALIGNED16(btVector3	spuTriangleVertices[3]);
	ATTRIBUTE_ALIGNED16(btScalar	spuUnscaledVertex[4]);
	


public:
	spuNodeCallback(SpuCollisionPairInput* wuInput, CollisionTask_LocalStoreMemory*	lsMemPtr,SpuContactResult& spuContacts)
		:	m_wuInput(wuInput),
		m_spuContacts(spuContacts),
		m_lsMemPtr(lsMemPtr)
	{
	}

	virtual void processNode(int subPart, int triangleIndex)
	{
		///Create a triangle on the stack, call process collision, with GJK
		///DMA the vertices, can benefit from software caching

		//		spu_printf("processNode with triangleIndex %d\n",triangleIndex);

		if (m_lsMemPtr->bvhShapeData.gIndexMesh.m_indexType == PHY_SHORT)
		{
			unsigned short int* indexBasePtr = (unsigned short int*)(m_lsMemPtr->bvhShapeData.gIndexMesh.m_triangleIndexBase+triangleIndex*m_lsMemPtr->bvhShapeData.gIndexMesh.m_triangleIndexStride);
			ATTRIBUTE_ALIGNED16(unsigned short int tmpIndices[3]);

			small_cache_read_triple(&tmpIndices[0],(ppu_address_t)&indexBasePtr[0],
									&tmpIndices[1],(ppu_address_t)&indexBasePtr[1],
									&tmpIndices[2],(ppu_address_t)&indexBasePtr[2],
									sizeof(unsigned short int));

			m_lsMemPtr->spuIndices[0] = int(tmpIndices[0]);
			m_lsMemPtr->spuIndices[1] = int(tmpIndices[1]);
			m_lsMemPtr->spuIndices[2] = int(tmpIndices[2]);
		} else
		{
			unsigned int* indexBasePtr = (unsigned int*)(m_lsMemPtr->bvhShapeData.gIndexMesh.m_triangleIndexBase+triangleIndex*m_lsMemPtr->bvhShapeData.gIndexMesh.m_triangleIndexStride);

			small_cache_read_triple(&m_lsMemPtr->spuIndices[0],(ppu_address_t)&indexBasePtr[0],
								&m_lsMemPtr->spuIndices[1],(ppu_address_t)&indexBasePtr[1],
								&m_lsMemPtr->spuIndices[2],(ppu_address_t)&indexBasePtr[2],
								sizeof(int));
		}
		
		//		spu_printf("SPU index0=%d ,",spuIndices[0]);
		//		spu_printf("SPU index1=%d ,",spuIndices[1]);
		//		spu_printf("SPU index2=%d ,",spuIndices[2]);
		//		spu_printf("SPU: indexBasePtr=%llx\n",indexBasePtr);

		const btVector3& meshScaling = m_lsMemPtr->bvhShapeData.gTriangleMeshInterfacePtr->getScaling();
		for (int j=2;btLikely( j>=0 );j--)
		{
			int graphicsindex = m_lsMemPtr->spuIndices[j];

			//			spu_printf("SPU index=%d ,",graphicsindex);
			btScalar* graphicsbasePtr = (btScalar*)(m_lsMemPtr->bvhShapeData.gIndexMesh.m_vertexBase+graphicsindex*m_lsMemPtr->bvhShapeData.gIndexMesh.m_vertexStride);
			//			spu_printf("SPU graphicsbasePtr=%llx\n",graphicsbasePtr);


			///handle un-aligned vertices...

			//another DMA for each vertex
			small_cache_read_triple(&spuUnscaledVertex[0],(ppu_address_t)&graphicsbasePtr[0],
									&spuUnscaledVertex[1],(ppu_address_t)&graphicsbasePtr[1],
									&spuUnscaledVertex[2],(ppu_address_t)&graphicsbasePtr[2],
									sizeof(btScalar));
			
			m_tmpTriangleShape.getVertexPtr(j).setValue(spuUnscaledVertex[0]*meshScaling.getX(),
				spuUnscaledVertex[1]*meshScaling.getY(),
				spuUnscaledVertex[2]*meshScaling.getZ());

			//			spu_printf("SPU:triangle vertices:%f,%f,%f\n",spuTriangleVertices[j].x(),spuTriangleVertices[j].y(),spuTriangleVertices[j].z());
		}


		SpuCollisionPairInput triangleConcaveInput(*m_wuInput);
//		triangleConcaveInput.m_spuCollisionShapes[1] = &spuTriangleVertices[0];
		triangleConcaveInput.m_spuCollisionShapes[1] = &m_tmpTriangleShape;
		triangleConcaveInput.m_shapeType1 = TRIANGLE_SHAPE_PROXYTYPE;

		m_spuContacts.setShapeIdentifiersB(subPart,triangleIndex);

		//		m_spuContacts.flush();

		ProcessSpuConvexConvexCollision(&triangleConcaveInput, m_lsMemPtr,m_spuContacts);
		///this flush should be automatic
		//	m_spuContacts.flush();
	}

};



void btConvexPlaneCollideSingleContact (SpuCollisionPairInput* wuInput,CollisionTask_LocalStoreMemory* lsMemPtr,SpuContactResult&  spuContacts)
{
	
	btConvexShape* convexShape = (btConvexShape*) wuInput->m_spuCollisionShapes[0];
	btStaticPlaneShape* planeShape = (btStaticPlaneShape*) wuInput->m_spuCollisionShapes[1];

    bool hasCollision = false;
	const btVector3& planeNormal = planeShape->getPlaneNormal();
	const btScalar& planeConstant = planeShape->getPlaneConstant();
	
	
	btTransform convexWorldTransform = wuInput->m_worldTransform0;
	btTransform convexInPlaneTrans;
	convexInPlaneTrans= wuInput->m_worldTransform1.inverse() * convexWorldTransform;
	btTransform planeInConvex;
	planeInConvex= convexWorldTransform.inverse() * wuInput->m_worldTransform1;
	
	//btVector3 vtx = convexShape->localGetSupportVertexWithoutMarginNonVirtual(planeInConvex.getBasis()*-planeNormal);
	btVector3 vtx = convexShape->localGetSupportVertexNonVirtual(planeInConvex.getBasis()*-planeNormal);

	btVector3 vtxInPlane = convexInPlaneTrans(vtx);
	btScalar distance = (planeNormal.dot(vtxInPlane) - planeConstant);

	btVector3 vtxInPlaneProjected = vtxInPlane - distance*planeNormal;
	btVector3 vtxInPlaneWorld = wuInput->m_worldTransform1 * vtxInPlaneProjected;

	hasCollision = distance < lsMemPtr->getContactManifoldPtr()->getContactBreakingThreshold();
	//resultOut->setPersistentManifold(m_manifoldPtr);
	if (hasCollision)
	{
		/// report a contact. internally this will be kept persistent, and contact reduction is done
		btVector3 normalOnSurfaceB =wuInput->m_worldTransform1.getBasis() * planeNormal;
		btVector3 pOnB = vtxInPlaneWorld;
		spuContacts.addContactPoint(normalOnSurfaceB,pOnB,distance);
	}
}

void	ProcessConvexPlaneSpuCollision(SpuCollisionPairInput* wuInput, CollisionTask_LocalStoreMemory* lsMemPtr, SpuContactResult& spuContacts)
{

		register	int dmaSize = 0;
		register ppu_address_t	dmaPpuAddress2;
		btPersistentManifold* manifold = (btPersistentManifold*)wuInput->m_persistentManifoldPtr;

		///DMA in the vertices for convex shapes
		ATTRIBUTE_ALIGNED16(char convexHullShape0[sizeof(btConvexHullShape)]);
		ATTRIBUTE_ALIGNED16(char convexHullShape1[sizeof(btConvexHullShape)]);

		if ( btLikely( wuInput->m_shapeType0== CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			//	spu_printf("SPU: DMA btConvexHullShape\n");
			
			dmaSize = sizeof(btConvexHullShape);
			dmaPpuAddress2 = wuInput->m_collisionShapes[0];

			cellDmaGet(&convexHullShape0, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);
			//cellDmaWaitTagStatusAll(DMA_MASK(1));
		}

		if ( btLikely( wuInput->m_shapeType1 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			//	spu_printf("SPU: DMA btConvexHullShape\n");
			dmaSize = sizeof(btConvexHullShape);
			dmaPpuAddress2 = wuInput->m_collisionShapes[1];
			cellDmaGet(&convexHullShape1, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);
			//cellDmaWaitTagStatusAll(DMA_MASK(1));
		}
		
		if ( btLikely( wuInput->m_shapeType0 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{		
			cellDmaWaitTagStatusAll(DMA_MASK(1));
			dmaConvexVertexData (&lsMemPtr->convexVertexData[0], (btConvexHullShape*)&convexHullShape0);
			lsMemPtr->convexVertexData[0].gSpuConvexShapePtr = wuInput->m_spuCollisionShapes[0];
		}

			
		if ( btLikely( wuInput->m_shapeType1 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			cellDmaWaitTagStatusAll(DMA_MASK(1));
			dmaConvexVertexData (&lsMemPtr->convexVertexData[1], (btConvexHullShape*)&convexHullShape1);
			lsMemPtr->convexVertexData[1].gSpuConvexShapePtr = wuInput->m_spuCollisionShapes[1];
		}

		
		btConvexPointCloudShape cpc0,cpc1;

		if ( btLikely( wuInput->m_shapeType0 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			cellDmaWaitTagStatusAll(DMA_MASK(2));
			lsMemPtr->convexVertexData[0].gConvexPoints = &lsMemPtr->convexVertexData[0].g_convexPointBuffer[0];
			btConvexHullShape* ch = (btConvexHullShape*)wuInput->m_spuCollisionShapes[0];
			const btVector3& localScaling = ch->getLocalScalingNV();
			cpc0.setPoints(lsMemPtr->convexVertexData[0].gConvexPoints,lsMemPtr->convexVertexData[0].gNumConvexPoints,false,localScaling);
			wuInput->m_spuCollisionShapes[0] = &cpc0;
		}

		if ( btLikely( wuInput->m_shapeType1 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			cellDmaWaitTagStatusAll(DMA_MASK(2));		
			lsMemPtr->convexVertexData[1].gConvexPoints = &lsMemPtr->convexVertexData[1].g_convexPointBuffer[0];
			btConvexHullShape* ch = (btConvexHullShape*)wuInput->m_spuCollisionShapes[1];
			const btVector3& localScaling = ch->getLocalScalingNV();
			cpc1.setPoints(lsMemPtr->convexVertexData[1].gConvexPoints,lsMemPtr->convexVertexData[1].gNumConvexPoints,false,localScaling);
			wuInput->m_spuCollisionShapes[1] = &cpc1;

		}


//		const btConvexShape* shape0Ptr = (const btConvexShape*)wuInput->m_spuCollisionShapes[0];
//		const btConvexShape* shape1Ptr = (const btConvexShape*)wuInput->m_spuCollisionShapes[1];
//		int shapeType0 = wuInput->m_shapeType0;
//		int shapeType1 = wuInput->m_shapeType1;
		float marginA = wuInput->m_collisionMargin0;
		float marginB = wuInput->m_collisionMargin1;

		SpuClosestPointInput	cpInput;
		cpInput.m_convexVertexData[0] = &lsMemPtr->convexVertexData[0];
		cpInput.m_convexVertexData[1] = &lsMemPtr->convexVertexData[1];
		cpInput.m_transformA = wuInput->m_worldTransform0;
		cpInput.m_transformB = wuInput->m_worldTransform1;
		float sumMargin = (marginA+marginB+lsMemPtr->getContactManifoldPtr()->getContactBreakingThreshold());
		cpInput.m_maximumDistanceSquared = sumMargin * sumMargin;

		ppu_address_t manifoldAddress = (ppu_address_t)manifold;

		btPersistentManifold* spuManifold=lsMemPtr->getContactManifoldPtr();
		//spuContacts.setContactInfo(spuManifold,manifoldAddress,wuInput->m_worldTransform0,wuInput->m_worldTransform1,wuInput->m_isSwapped);
		spuContacts.setContactInfo(spuManifold,manifoldAddress,lsMemPtr->getColObj0()->getWorldTransform(),
			lsMemPtr->getColObj1()->getWorldTransform(),
			lsMemPtr->getColObj0()->getRestitution(),lsMemPtr->getColObj1()->getRestitution(),
			lsMemPtr->getColObj0()->getFriction(),lsMemPtr->getColObj1()->getFriction(),
			wuInput->m_isSwapped);


		btConvexPlaneCollideSingleContact(wuInput,lsMemPtr,spuContacts);


		
	
}




////////////////////////
/// Convex versus Concave triangle mesh collision detection (handles concave triangle mesh versus sphere, box, cylinder, triangle, cone, convex polyhedron etc)
///////////////////
void	ProcessConvexConcaveSpuCollision(SpuCollisionPairInput* wuInput, CollisionTask_LocalStoreMemory* lsMemPtr, SpuContactResult& spuContacts)
{
	//order: first collision shape is convex, second concave. m_isSwapped is true, if the original order was opposite
	
	btBvhTriangleMeshShape*	trimeshShape = (btBvhTriangleMeshShape*)wuInput->m_spuCollisionShapes[1];
	//need the mesh interface, for access to triangle vertices
	dmaBvhShapeData (&lsMemPtr->bvhShapeData, trimeshShape);

	btVector3 aabbMin(-1,-400,-1);
	btVector3 aabbMax(1,400,1);


	//recalc aabbs
	btTransform convexInTriangleSpace;
	convexInTriangleSpace = wuInput->m_worldTransform1.inverse() * wuInput->m_worldTransform0;
	btConvexInternalShape* convexShape = (btConvexInternalShape*)wuInput->m_spuCollisionShapes[0];

	computeAabb (aabbMin, aabbMax, convexShape, wuInput->m_collisionShapes[0], wuInput->m_shapeType0, convexInTriangleSpace);


	//CollisionShape* triangleShape = static_cast<btCollisionShape*>(triBody->m_collisionShape);
	//convexShape->getAabb(convexInTriangleSpace,m_aabbMin,m_aabbMax);

	//	btScalar extraMargin = collisionMarginTriangle;
	//	btVector3 extra(extraMargin,extraMargin,extraMargin);
	//	aabbMax += extra;
	//	aabbMin -= extra;

	///quantize query AABB
	unsigned short int quantizedQueryAabbMin[3];
	unsigned short int quantizedQueryAabbMax[3];
	lsMemPtr->bvhShapeData.getOptimizedBvh()->quantizeWithClamp(quantizedQueryAabbMin,aabbMin,0);
	lsMemPtr->bvhShapeData.getOptimizedBvh()->quantizeWithClamp(quantizedQueryAabbMax,aabbMax,1);

	QuantizedNodeArray&	nodeArray = lsMemPtr->bvhShapeData.getOptimizedBvh()->getQuantizedNodeArray();
	//spu_printf("SPU: numNodes = %d\n",nodeArray.size());

	BvhSubtreeInfoArray& subTrees = lsMemPtr->bvhShapeData.getOptimizedBvh()->getSubtreeInfoArray();


	spuNodeCallback	nodeCallback(wuInput,lsMemPtr,spuContacts);
	IndexedMeshArray&	indexArray = lsMemPtr->bvhShapeData.gTriangleMeshInterfacePtr->getIndexedMeshArray();
	//spu_printf("SPU:indexArray.size() = %d\n",indexArray.size());

	//	spu_printf("SPU: numSubTrees = %d\n",subTrees.size());
	//not likely to happen
	if (subTrees.size() && indexArray.size() == 1)
	{
		///DMA in the index info
		dmaBvhIndexedMesh (&lsMemPtr->bvhShapeData.gIndexMesh, indexArray, 0 /* index into indexArray */, 1 /* dmaTag */);
		cellDmaWaitTagStatusAll(DMA_MASK(1));
		
		//display the headers
		int numBatch = subTrees.size();
		for (int i=0;i<numBatch;)
		{
			//@todo- can reorder DMA transfers for less stall
			int remaining = subTrees.size() - i;
			int nextBatch = remaining < MAX_SPU_SUBTREE_HEADERS ? remaining : MAX_SPU_SUBTREE_HEADERS;
			
			dmaBvhSubTreeHeaders (&lsMemPtr->bvhShapeData.gSubtreeHeaders[0], (ppu_address_t)(&subTrees[i]), nextBatch, 1);
			cellDmaWaitTagStatusAll(DMA_MASK(1));
			

			//			spu_printf("nextBatch = %d\n",nextBatch);

			for (int j=0;j<nextBatch;j++)
			{
				const btBvhSubtreeInfo& subtree = lsMemPtr->bvhShapeData.gSubtreeHeaders[j];

				unsigned int overlap = spuTestQuantizedAabbAgainstQuantizedAabb(quantizedQueryAabbMin,quantizedQueryAabbMax,subtree.m_quantizedAabbMin,subtree.m_quantizedAabbMax);
				if (overlap)
				{
					btAssert(subtree.m_subtreeSize);

					//dma the actual nodes of this subtree
					dmaBvhSubTreeNodes (&lsMemPtr->bvhShapeData.gSubtreeNodes[0], subtree, nodeArray, 2);
					cellDmaWaitTagStatusAll(DMA_MASK(2));

					/* Walk this subtree */
					spuWalkStacklessQuantizedTree(&nodeCallback,quantizedQueryAabbMin,quantizedQueryAabbMax,
						&lsMemPtr->bvhShapeData.gSubtreeNodes[0],
						0,
						subtree.m_subtreeSize);
				}
				//				spu_printf("subtreeSize = %d\n",gSubtreeHeaders[j].m_subtreeSize);
			}

			//	unsigned short int	m_quantizedAabbMin[3];
			//	unsigned short int	m_quantizedAabbMax[3];
			//	int			m_rootNodeIndex;
			//	int			m_subtreeSize;
			i+=nextBatch;
		}

		//pre-fetch first tree, then loop and double buffer
	}

}


int stats[11]={0,0,0,0,0,0,0,0,0,0,0};
int degenerateStats[11]={0,0,0,0,0,0,0,0,0,0,0};


////////////////////////
/// Convex versus Convex collision detection (handles collision between sphere, box, cylinder, triangle, cone, convex polyhedron etc)
///////////////////
void	ProcessSpuConvexConvexCollision(SpuCollisionPairInput* wuInput, CollisionTask_LocalStoreMemory* lsMemPtr, SpuContactResult& spuContacts)
{
	register int dmaSize;
	register ppu_address_t	dmaPpuAddress2;
	
#ifdef DEBUG_SPU_COLLISION_DETECTION
	//spu_printf("SPU: ProcessSpuConvexConvexCollision\n");
#endif //DEBUG_SPU_COLLISION_DETECTION
	//CollisionShape* shape0 = (CollisionShape*)wuInput->m_collisionShapes[0];
	//CollisionShape* shape1 = (CollisionShape*)wuInput->m_collisionShapes[1];
	btPersistentManifold* manifold = (btPersistentManifold*)wuInput->m_persistentManifoldPtr;

	bool genericGjk = true;

	if (genericGjk)
	{
		//try generic GJK

		
		
		//SpuConvexPenetrationDepthSolver* penetrationSolver=0;
		btVoronoiSimplexSolver simplexSolver;
		btGjkEpaPenetrationDepthSolver	epaPenetrationSolver2;
		
		btConvexPenetrationDepthSolver* penetrationSolver = &epaPenetrationSolver2;
		
		//SpuMinkowskiPenetrationDepthSolver	minkowskiPenetrationSolver;
#ifdef ENABLE_EPA
		if (gUseEpa)
		{
			penetrationSolver = &epaPenetrationSolver2;
		} else
#endif
		{
			//penetrationSolver = &minkowskiPenetrationSolver;
		}


		///DMA in the vertices for convex shapes
		ATTRIBUTE_ALIGNED16(char convexHullShape0[sizeof(btConvexHullShape)]);
		ATTRIBUTE_ALIGNED16(char convexHullShape1[sizeof(btConvexHullShape)]);

		if ( btLikely( wuInput->m_shapeType0== CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			//	spu_printf("SPU: DMA btConvexHullShape\n");
			
			dmaSize = sizeof(btConvexHullShape);
			dmaPpuAddress2 = wuInput->m_collisionShapes[0];

			cellDmaGet(&convexHullShape0, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);
			//cellDmaWaitTagStatusAll(DMA_MASK(1));
		}

		if ( btLikely( wuInput->m_shapeType1 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			//	spu_printf("SPU: DMA btConvexHullShape\n");
			dmaSize = sizeof(btConvexHullShape);
			dmaPpuAddress2 = wuInput->m_collisionShapes[1];
			cellDmaGet(&convexHullShape1, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);
			//cellDmaWaitTagStatusAll(DMA_MASK(1));
		}
		
		if ( btLikely( wuInput->m_shapeType0 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{		
			cellDmaWaitTagStatusAll(DMA_MASK(1));
			dmaConvexVertexData (&lsMemPtr->convexVertexData[0], (btConvexHullShape*)&convexHullShape0);
			lsMemPtr->convexVertexData[0].gSpuConvexShapePtr = wuInput->m_spuCollisionShapes[0];
		}

			
		if ( btLikely( wuInput->m_shapeType1 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			cellDmaWaitTagStatusAll(DMA_MASK(1));
			dmaConvexVertexData (&lsMemPtr->convexVertexData[1], (btConvexHullShape*)&convexHullShape1);
			lsMemPtr->convexVertexData[1].gSpuConvexShapePtr = wuInput->m_spuCollisionShapes[1];
		}

		
		btConvexPointCloudShape cpc0,cpc1;

		if ( btLikely( wuInput->m_shapeType0 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			cellDmaWaitTagStatusAll(DMA_MASK(2));
			lsMemPtr->convexVertexData[0].gConvexPoints = &lsMemPtr->convexVertexData[0].g_convexPointBuffer[0];
			btConvexHullShape* ch = (btConvexHullShape*)wuInput->m_spuCollisionShapes[0];
			const btVector3& localScaling = ch->getLocalScalingNV();
			cpc0.setPoints(lsMemPtr->convexVertexData[0].gConvexPoints,lsMemPtr->convexVertexData[0].gNumConvexPoints,false,localScaling);
			wuInput->m_spuCollisionShapes[0] = &cpc0;
		}

		if ( btLikely( wuInput->m_shapeType1 == CONVEX_HULL_SHAPE_PROXYTYPE ) )
		{
			cellDmaWaitTagStatusAll(DMA_MASK(2));		
			lsMemPtr->convexVertexData[1].gConvexPoints = &lsMemPtr->convexVertexData[1].g_convexPointBuffer[0];
			btConvexHullShape* ch = (btConvexHullShape*)wuInput->m_spuCollisionShapes[1];
			const btVector3& localScaling = ch->getLocalScalingNV();
			cpc1.setPoints(lsMemPtr->convexVertexData[1].gConvexPoints,lsMemPtr->convexVertexData[1].gNumConvexPoints,false,localScaling);
			wuInput->m_spuCollisionShapes[1] = &cpc1;

		}


		const btConvexShape* shape0Ptr = (const btConvexShape*)wuInput->m_spuCollisionShapes[0];
		const btConvexShape* shape1Ptr = (const btConvexShape*)wuInput->m_spuCollisionShapes[1];
		int shapeType0 = wuInput->m_shapeType0;
		int shapeType1 = wuInput->m_shapeType1;
		float marginA = wuInput->m_collisionMargin0;
		float marginB = wuInput->m_collisionMargin1;

		SpuClosestPointInput	cpInput;
		cpInput.m_convexVertexData[0] = &lsMemPtr->convexVertexData[0];
		cpInput.m_convexVertexData[1] = &lsMemPtr->convexVertexData[1];
		cpInput.m_transformA = wuInput->m_worldTransform0;
		cpInput.m_transformB = wuInput->m_worldTransform1;
		float sumMargin = (marginA+marginB+lsMemPtr->getContactManifoldPtr()->getContactBreakingThreshold());
		cpInput.m_maximumDistanceSquared = sumMargin * sumMargin;

		ppu_address_t manifoldAddress = (ppu_address_t)manifold;

		btPersistentManifold* spuManifold=lsMemPtr->getContactManifoldPtr();
		//spuContacts.setContactInfo(spuManifold,manifoldAddress,wuInput->m_worldTransform0,wuInput->m_worldTransform1,wuInput->m_isSwapped);
		spuContacts.setContactInfo(spuManifold,manifoldAddress,lsMemPtr->getColObj0()->getWorldTransform(),
			lsMemPtr->getColObj1()->getWorldTransform(),
			lsMemPtr->getColObj0()->getRestitution(),lsMemPtr->getColObj1()->getRestitution(),
			lsMemPtr->getColObj0()->getFriction(),lsMemPtr->getColObj1()->getFriction(),
			wuInput->m_isSwapped);

		{
			btGjkPairDetector gjk(shape0Ptr,shape1Ptr,shapeType0,shapeType1,marginA,marginB,&simplexSolver,penetrationSolver);//&vsSolver,penetrationSolver);
			gjk.getClosestPoints(cpInput,spuContacts,0);//,debugDraw);
			
			stats[gjk.m_lastUsedMethod]++;
			degenerateStats[gjk.m_degenerateSimplex]++;

#ifdef USE_SEPDISTANCE_UTIL			
			btScalar sepDist = gjk.getCachedSeparatingDistance()+spuManifold->getContactBreakingThreshold();
			lsMemPtr->getlocalCollisionAlgorithm()->m_sepDistance.initSeparatingDistance(gjk.getCachedSeparatingAxis(),sepDist,wuInput->m_worldTransform0,wuInput->m_worldTransform1);
			lsMemPtr->needsDmaPutContactManifoldAlgo = true;
#endif //USE_SEPDISTANCE_UTIL

		}

	}


}


template<typename T> void DoSwap(T& a, T& b)
{
	char tmp[sizeof(T)];
	memcpy(tmp, &a, sizeof(T));
	memcpy(&a, &b, sizeof(T));
	memcpy(&b, tmp, sizeof(T));
}

SIMD_FORCE_INLINE void	dmaAndSetupCollisionObjects(SpuCollisionPairInput& collisionPairInput, CollisionTask_LocalStoreMemory& lsMem)
{
	register int dmaSize;
	register ppu_address_t	dmaPpuAddress2;
		
	dmaSize = sizeof(btCollisionObject);//btTransform);
	dmaPpuAddress2 = /*collisionPairInput.m_isSwapped ? (ppu_address_t)lsMem.gProxyPtr1->m_clientObject :*/ (ppu_address_t)lsMem.getlocalCollisionAlgorithm()->getCollisionObject0();
	lsMem.m_lsColObj0Ptr = (btCollisionObject*)cellDmaGetReadOnly(&lsMem.gColObj0Buffer, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);		

	dmaSize = sizeof(btCollisionObject);//btTransform);
	dmaPpuAddress2 = /*collisionPairInput.m_isSwapped ? (ppu_address_t)lsMem.gProxyPtr0->m_clientObject :*/ (ppu_address_t)lsMem.getlocalCollisionAlgorithm()->getCollisionObject1();
	lsMem.m_lsColObj1Ptr = (btCollisionObject*)cellDmaGetReadOnly(&lsMem.gColObj1Buffer, dmaPpuAddress2  , dmaSize, DMA_TAG(2), 0, 0);		
	
	cellDmaWaitTagStatusAll(DMA_MASK(1) | DMA_MASK(2));

	btCollisionObject* ob0 = lsMem.getColObj0();
	btCollisionObject* ob1 = lsMem.getColObj1();

	collisionPairInput.m_worldTransform0 = ob0->getWorldTransform();
	collisionPairInput.m_worldTransform1 = ob1->getWorldTransform();
}



void	handleCollisionPair(SpuCollisionPairInput& collisionPairInput, CollisionTask_LocalStoreMemory& lsMem,
							SpuContactResult &spuContacts,
							ppu_address_t collisionShape0Ptr, void* collisionShape0Loc,
							ppu_address_t collisionShape1Ptr, void* collisionShape1Loc, bool dmaShapes = true)
{
	
	if (btBroadphaseProxy::isConvex(collisionPairInput.m_shapeType0) 
		&& btBroadphaseProxy::isConvex(collisionPairInput.m_shapeType1))
	{
		if (dmaShapes)
		{
			dmaCollisionShape (collisionShape0Loc, collisionShape0Ptr, 1, collisionPairInput.m_shapeType0);
			dmaCollisionShape (collisionShape1Loc, collisionShape1Ptr, 2, collisionPairInput.m_shapeType1);
			cellDmaWaitTagStatusAll(DMA_MASK(1) | DMA_MASK(2));
		}

		btConvexInternalShape* spuConvexShape0 = (btConvexInternalShape*)collisionShape0Loc;
		btConvexInternalShape* spuConvexShape1 = (btConvexInternalShape*)collisionShape1Loc;

		btVector3 dim0 = spuConvexShape0->getImplicitShapeDimensions();
		btVector3 dim1 = spuConvexShape1->getImplicitShapeDimensions();

		collisionPairInput.m_primitiveDimensions0 = dim0;
		collisionPairInput.m_primitiveDimensions1 = dim1;
		collisionPairInput.m_collisionShapes[0] = collisionShape0Ptr;
		collisionPairInput.m_collisionShapes[1] = collisionShape1Ptr;
		collisionPairInput.m_spuCollisionShapes[0] = spuConvexShape0;
		collisionPairInput.m_spuCollisionShapes[1] = spuConvexShape1;
		ProcessSpuConvexConvexCollision(&collisionPairInput,&lsMem,spuContacts);
	} 
	else if (btBroadphaseProxy::isCompound(collisionPairInput.m_shapeType0) && 
			btBroadphaseProxy::isCompound(collisionPairInput.m_shapeType1))
	{
		//snPause();

		dmaCollisionShape (collisionShape0Loc, collisionShape0Ptr, 1, collisionPairInput.m_shapeType0);
		dmaCollisionShape (collisionShape1Loc, collisionShape1Ptr, 2, collisionPairInput.m_shapeType1);
		cellDmaWaitTagStatusAll(DMA_MASK(1) | DMA_MASK(2));

		// Both are compounds, do N^2 CD for now
		///@todo: add some AABB-based pruning (probably not -> slower)
	
		btCompoundShape* spuCompoundShape0 = (btCompoundShape*)collisionShape0Loc;
		btCompoundShape* spuCompoundShape1 = (btCompoundShape*)collisionShape1Loc;

		dmaCompoundShapeInfo (&lsMem.compoundShapeData[0], spuCompoundShape0, 1);
		dmaCompoundShapeInfo (&lsMem.compoundShapeData[1], spuCompoundShape1, 2);
		cellDmaWaitTagStatusAll(DMA_MASK(1) | DMA_MASK(2));
		

		dmaCompoundSubShapes (&lsMem.compoundShapeData[0], spuCompoundShape0, 1);
		cellDmaWaitTagStatusAll(DMA_MASK(1));
		dmaCompoundSubShapes (&lsMem.compoundShapeData[1], spuCompoundShape1, 1);
		cellDmaWaitTagStatusAll(DMA_MASK(1));

		int childShapeCount0 = spuCompoundShape0->getNumChildShapes();
		btAssert(childShapeCount0< MAX_SPU_COMPOUND_SUBSHAPES);
		int childShapeCount1 = spuCompoundShape1->getNumChildShapes();
		btAssert(childShapeCount1< MAX_SPU_COMPOUND_SUBSHAPES);

		// Start the N^2
		for (int i = 0; i < childShapeCount0; ++i)
		{
			btCompoundShapeChild& childShape0 = lsMem.compoundShapeData[0].gSubshapes[i];
			btAssert(!btBroadphaseProxy::isCompound(childShape0.m_childShapeType));

			for (int j = 0; j < childShapeCount1; ++j)
			{
				btCompoundShapeChild& childShape1 = lsMem.compoundShapeData[1].gSubshapes[j];
				btAssert(!btBroadphaseProxy::isCompound(childShape1.m_childShapeType));


				/* Create a new collision pair input struct using the two child shapes */
				SpuCollisionPairInput cinput (collisionPairInput);

				cinput.m_worldTransform0 = collisionPairInput.m_worldTransform0 * childShape0.m_transform;
				cinput.m_shapeType0 = childShape0.m_childShapeType;
				cinput.m_collisionMargin0 = childShape0.m_childMargin;

				cinput.m_worldTransform1 = collisionPairInput.m_worldTransform1 * childShape1.m_transform;
				cinput.m_shapeType1 = childShape1.m_childShapeType;
				cinput.m_collisionMargin1 = childShape1.m_childMargin;
				/* Recursively call handleCollisionPair () with new collision pair input */
				
				handleCollisionPair(cinput, lsMem, spuContacts,			
					(ppu_address_t)childShape0.m_childShape, lsMem.compoundShapeData[0].gSubshapeShape[i], 
					(ppu_address_t)childShape1.m_childShape, lsMem.compoundShapeData[1].gSubshapeShape[j], false);
			}
		}
	}
	else if (btBroadphaseProxy::isCompound(collisionPairInput.m_shapeType0) )
	{
		//snPause();
		
		dmaCollisionShape (collisionShape0Loc, collisionShape0Ptr, 1, collisionPairInput.m_shapeType0);
		dmaCollisionShape (collisionShape1Loc, collisionShape1Ptr, 2, collisionPairInput.m_shapeType1);
		cellDmaWaitTagStatusAll(DMA_MASK(1) | DMA_MASK(2));

		// object 0 compound, object 1 non-compound
		btCompoundShape* spuCompoundShape = (btCompoundShape*)collisionShape0Loc;
		dmaCompoundShapeInfo (&lsMem.compoundShapeData[0], spuCompoundShape, 1);
		cellDmaWaitTagStatusAll(DMA_MASK(1));

		int childShapeCount = spuCompoundShape->getNumChildShapes();
		btAssert(childShapeCount< MAX_SPU_COMPOUND_SUBSHAPES);

		for (int i = 0; i < childShapeCount; ++i)
		{
			btCompoundShapeChild& childShape = lsMem.compoundShapeData[0].gSubshapes[i];
			btAssert(!btBroadphaseProxy::isCompound(childShape.m_childShapeType));
			// Dma the child shape
			dmaCollisionShape (&lsMem.compoundShapeData[0].gSubshapeShape[i], (ppu_address_t)childShape.m_childShape, 1, childShape.m_childShapeType);
			cellDmaWaitTagStatusAll(DMA_MASK(1));
			
			SpuCollisionPairInput cinput (collisionPairInput);
			cinput.m_worldTransform0 = collisionPairInput.m_worldTransform0 * childShape.m_transform;
			cinput.m_shapeType0 = childShape.m_childShapeType;
			cinput.m_collisionMargin0 = childShape.m_childMargin;

			handleCollisionPair(cinput, lsMem, spuContacts,			
				(ppu_address_t)childShape.m_childShape, lsMem.compoundShapeData[0].gSubshapeShape[i], 
				collisionShape1Ptr, collisionShape1Loc, false);
		}
	}
	else if (btBroadphaseProxy::isCompound(collisionPairInput.m_shapeType1) )
	{
		//snPause();
		
		dmaCollisionShape (collisionShape0Loc, collisionShape0Ptr, 1, collisionPairInput.m_shapeType0);
		dmaCollisionShape (collisionShape1Loc, collisionShape1Ptr, 2, collisionPairInput.m_shapeType1);
		cellDmaWaitTagStatusAll(DMA_MASK(1) | DMA_MASK(2));
		// object 0 non-compound, object 1 compound
		btCompoundShape* spuCompoundShape = (btCompoundShape*)collisionShape1Loc;
		dmaCompoundShapeInfo (&lsMem.compoundShapeData[0], spuCompoundShape, 1);
		cellDmaWaitTagStatusAll(DMA_MASK(1));
		
		int childShapeCount = spuCompoundShape->getNumChildShapes();
		btAssert(childShapeCount< MAX_SPU_COMPOUND_SUBSHAPES);


		for (int i = 0; i < childShapeCount; ++i)
		{
			btCompoundShapeChild& childShape = lsMem.compoundShapeData[0].gSubshapes[i];
			btAssert(!btBroadphaseProxy::isCompound(childShape.m_childShapeType));
			// Dma the child shape
			dmaCollisionShape (&lsMem.compoundShapeData[0].gSubshapeShape[i], (ppu_address_t)childShape.m_childShape, 1, childShape.m_childShapeType);
			cellDmaWaitTagStatusAll(DMA_MASK(1));

			SpuCollisionPairInput cinput (collisionPairInput);
			cinput.m_worldTransform1 = collisionPairInput.m_worldTransform1 * childShape.m_transform;
			cinput.m_shapeType1 = childShape.m_childShapeType;
			cinput.m_collisionMargin1 = childShape.m_childMargin;
			handleCollisionPair(cinput, lsMem, spuContacts,
				collisionShape0Ptr, collisionShape0Loc, 
				(ppu_address_t)childShape.m_childShape, lsMem.compoundShapeData[0].gSubshapeShape[i], false);
		}
		
	}
	else
	{
		//a non-convex shape is involved									
		bool handleConvexConcave = false;

		//snPause();

		if (btBroadphaseProxy::isConcave(collisionPairInput.m_shapeType0) &&
			btBroadphaseProxy::isConvex(collisionPairInput.m_shapeType1))
		{
			// Swap stuff
			DoSwap(collisionShape0Ptr, collisionShape1Ptr);
			DoSwap(collisionShape0Loc, collisionShape1Loc);
			DoSwap(collisionPairInput.m_shapeType0, collisionPairInput.m_shapeType1);
			DoSwap(collisionPairInput.m_worldTransform0, collisionPairInput.m_worldTransform1);
			DoSwap(collisionPairInput.m_collisionMargin0, collisionPairInput.m_collisionMargin1);
			
			collisionPairInput.m_isSwapped = true;
		}
		
		if (btBroadphaseProxy::isConvex(collisionPairInput.m_shapeType0)&&
			btBroadphaseProxy::isConcave(collisionPairInput.m_shapeType1))
		{
			handleConvexConcave = true;
		}
		if (handleConvexConcave)
		{
			if (dmaShapes)
			{
				dmaCollisionShape (collisionShape0Loc, collisionShape0Ptr, 1, collisionPairInput.m_shapeType0);
				dmaCollisionShape (collisionShape1Loc, collisionShape1Ptr, 2, collisionPairInput.m_shapeType1);
				cellDmaWaitTagStatusAll(DMA_MASK(1) | DMA_MASK(2));
			}
			
			if (collisionPairInput.m_shapeType1 == STATIC_PLANE_PROXYTYPE)
			{
				btConvexInternalShape* spuConvexShape0 = (btConvexInternalShape*)collisionShape0Loc;
				btStaticPlaneShape* planeShape= (btStaticPlaneShape*)collisionShape1Loc;

				btVector3 dim0 = spuConvexShape0->getImplicitShapeDimensions();
				collisionPairInput.m_primitiveDimensions0 = dim0;
				collisionPairInput.m_collisionShapes[0] = collisionShape0Ptr;
				collisionPairInput.m_collisionShapes[1] = collisionShape1Ptr;
				collisionPairInput.m_spuCollisionShapes[0] = spuConvexShape0;
				collisionPairInput.m_spuCollisionShapes[1] = planeShape;

				ProcessConvexPlaneSpuCollision(&collisionPairInput,&lsMem,spuContacts);
			} else
			{
				btConvexInternalShape* spuConvexShape0 = (btConvexInternalShape*)collisionShape0Loc;
				btBvhTriangleMeshShape* trimeshShape = (btBvhTriangleMeshShape*)collisionShape1Loc;

				btVector3 dim0 = spuConvexShape0->getImplicitShapeDimensions();
				collisionPairInput.m_primitiveDimensions0 = dim0;
				collisionPairInput.m_collisionShapes[0] = collisionShape0Ptr;
				collisionPairInput.m_collisionShapes[1] = collisionShape1Ptr;
				collisionPairInput.m_spuCollisionShapes[0] = spuConvexShape0;
				collisionPairInput.m_spuCollisionShapes[1] = trimeshShape;

				ProcessConvexConcaveSpuCollision(&collisionPairInput,&lsMem,spuContacts);
			}
		}

	}
	
	spuContacts.flush();

}


void	processCollisionTask(void* userPtr, void* lsMemPtr)
{

	SpuGatherAndProcessPairsTaskDesc* taskDescPtr = (SpuGatherAndProcessPairsTaskDesc*)userPtr;
	SpuGatherAndProcessPairsTaskDesc& taskDesc = *taskDescPtr;
	CollisionTask_LocalStoreMemory*	colMemPtr = (CollisionTask_LocalStoreMemory*)lsMemPtr;
	CollisionTask_LocalStoreMemory& lsMem = *(colMemPtr);

	gUseEpa = taskDesc.m_useEpa;

	//	spu_printf("taskDescPtr=%llx\n",taskDescPtr);

	SpuContactResult spuContacts;

	////////////////////

	ppu_address_t dmaInPtr = taskDesc.m_inPairPtr;
	unsigned int numPages = taskDesc.numPages;
	unsigned int numOnLastPage = taskDesc.numOnLastPage;

	// prefetch first set of inputs and wait
	lsMem.g_workUnitTaskBuffers.init();

	unsigned int nextNumOnPage = (numPages > 1)? MIDPHASE_NUM_WORKUNITS_PER_PAGE : numOnLastPage;
	lsMem.g_workUnitTaskBuffers.backBufferDmaGet(dmaInPtr, nextNumOnPage*sizeof(SpuGatherAndProcessWorkUnitInput), DMA_TAG(3));
	dmaInPtr += MIDPHASE_WORKUNIT_PAGE_SIZE;

	
	register unsigned char *inputPtr;
	register unsigned int numOnPage;
	register unsigned int j;
	SpuGatherAndProcessWorkUnitInput* wuInputs;	
	register int dmaSize;
	register ppu_address_t	dmaPpuAddress;
	register ppu_address_t	dmaPpuAddress2;

	int numPairs;
	register int p;
	SpuCollisionPairInput collisionPairInput;
	
	for (unsigned int i = 0; btLikely(i < numPages); i++)
	{

		// wait for back buffer dma and swap buffers
		inputPtr = lsMem.g_workUnitTaskBuffers.swapBuffers();

		// number on current page is number prefetched last iteration
		numOnPage = nextNumOnPage;


		// prefetch next set of inputs
#if MIDPHASE_NUM_WORKUNIT_PAGES > 2
		if ( btLikely( i < numPages-1 ) )
#else
		if ( btUnlikely( i < numPages-1 ) )
#endif
		{
			nextNumOnPage = (i == numPages-2)? numOnLastPage : MIDPHASE_NUM_WORKUNITS_PER_PAGE;
			lsMem.g_workUnitTaskBuffers.backBufferDmaGet(dmaInPtr, nextNumOnPage*sizeof(SpuGatherAndProcessWorkUnitInput), DMA_TAG(3));
			dmaInPtr += MIDPHASE_WORKUNIT_PAGE_SIZE;
		}

		wuInputs = reinterpret_cast<SpuGatherAndProcessWorkUnitInput *>(inputPtr);
		
		
		for (j = 0; btLikely( j < numOnPage ); j++)
		{
#ifdef DEBUG_SPU_COLLISION_DETECTION
		//	printMidphaseInput(&wuInputs[j]);
#endif //DEBUG_SPU_COLLISION_DETECTION


			numPairs = wuInputs[j].m_endIndex - wuInputs[j].m_startIndex;
			
			if ( btLikely( numPairs ) )
			{
					dmaSize = numPairs*sizeof(btBroadphasePair);
					dmaPpuAddress = wuInputs[j].m_pairArrayPtr+wuInputs[j].m_startIndex * sizeof(btBroadphasePair);
					lsMem.m_pairsPointer = (btBroadphasePair*)cellDmaGetReadOnly(&lsMem.gBroadphasePairsBuffer, dmaPpuAddress  , dmaSize, DMA_TAG(1), 0, 0);
					cellDmaWaitTagStatusAll(DMA_MASK(1));
				

				for (p=0;p<numPairs;p++)
				{

					//for each broadphase pair, do something

					btBroadphasePair& pair = lsMem.getBroadphasePairPtr()[p];
#ifdef DEBUG_SPU_COLLISION_DETECTION
					spu_printf("pair->m_userInfo = %d\n",pair.m_userInfo);
					spu_printf("pair->m_algorithm = %d\n",pair.m_algorithm);
					spu_printf("pair->m_pProxy0 = %d\n",pair.m_pProxy0);
					spu_printf("pair->m_pProxy1 = %d\n",pair.m_pProxy1);
#endif //DEBUG_SPU_COLLISION_DETECTION

					if (pair.m_internalTmpValue == 2 && pair.m_algorithm && pair.m_pProxy0 && pair.m_pProxy1)
					{
						dmaSize = sizeof(SpuContactManifoldCollisionAlgorithm);
						dmaPpuAddress2 = (ppu_address_t)pair.m_algorithm;
						lsMem.m_lsCollisionAlgorithmPtr = (SpuContactManifoldCollisionAlgorithm*)cellDmaGetReadOnly(&lsMem.gSpuContactManifoldAlgoBuffer, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);

						cellDmaWaitTagStatusAll(DMA_MASK(1));

						lsMem.needsDmaPutContactManifoldAlgo = false;

						collisionPairInput.m_persistentManifoldPtr = (ppu_address_t) lsMem.getlocalCollisionAlgorithm()->getContactManifoldPtr();
						collisionPairInput.m_isSwapped = false;

						if (1)
						{

							///can wait on the combined DMA_MASK, or dma on the same tag


#ifdef DEBUG_SPU_COLLISION_DETECTION
					//		spu_printf("SPU collisionPairInput->m_shapeType0 = %d\n",collisionPairInput->m_shapeType0);
					//		spu_printf("SPU collisionPairInput->m_shapeType1 = %d\n",collisionPairInput->m_shapeType1);
#endif //DEBUG_SPU_COLLISION_DETECTION

							
							dmaSize = sizeof(btPersistentManifold);

							dmaPpuAddress2 = collisionPairInput.m_persistentManifoldPtr;
							lsMem.m_lsManifoldPtr = (btPersistentManifold*)cellDmaGetReadOnly(&lsMem.gPersistentManifoldBuffer, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);

							collisionPairInput.m_shapeType0 = lsMem.getlocalCollisionAlgorithm()->getShapeType0();
							collisionPairInput.m_shapeType1 = lsMem.getlocalCollisionAlgorithm()->getShapeType1();
							collisionPairInput.m_collisionMargin0 = lsMem.getlocalCollisionAlgorithm()->getCollisionMargin0();
							collisionPairInput.m_collisionMargin1 = lsMem.getlocalCollisionAlgorithm()->getCollisionMargin1();
							
							
							
							//??cellDmaWaitTagStatusAll(DMA_MASK(1));
							

							if (1)
							{
								//snPause();

								// Get the collision objects
								dmaAndSetupCollisionObjects(collisionPairInput, lsMem);

								if (lsMem.getColObj0()->isActive() || lsMem.getColObj1()->isActive())
								{

									lsMem.needsDmaPutContactManifoldAlgo = true;
#ifdef USE_SEPDISTANCE_UTIL
									lsMem.getlocalCollisionAlgorithm()->m_sepDistance.updateSeparatingDistance(collisionPairInput.m_worldTransform0,collisionPairInput.m_worldTransform1);
#endif //USE_SEPDISTANCE_UTIL
							
#define USE_DEDICATED_BOX_BOX 1
#ifdef USE_DEDICATED_BOX_BOX
									bool boxbox = ((lsMem.getlocalCollisionAlgorithm()->getShapeType0()==BOX_SHAPE_PROXYTYPE)&&
										(lsMem.getlocalCollisionAlgorithm()->getShapeType1()==BOX_SHAPE_PROXYTYPE));
									if (boxbox)
									{
										//spu_printf("boxbox dist = %f\n",distance);
										btPersistentManifold* spuManifold=lsMem.getContactManifoldPtr();
										btPersistentManifold* manifold = (btPersistentManifold*)collisionPairInput.m_persistentManifoldPtr;
										ppu_address_t manifoldAddress = (ppu_address_t)manifold;

										spuContacts.setContactInfo(spuManifold,manifoldAddress,lsMem.getColObj0()->getWorldTransform(),
											lsMem.getColObj1()->getWorldTransform(),
											lsMem.getColObj0()->getRestitution(),lsMem.getColObj1()->getRestitution(),
											lsMem.getColObj0()->getFriction(),lsMem.getColObj1()->getFriction(),
											collisionPairInput.m_isSwapped);

						
									//float distance=0.f;
									btVector3 normalInB;


									if (//!gUseEpa &&
#ifdef USE_SEPDISTANCE_UTIL
										lsMem.getlocalCollisionAlgorithm()->m_sepDistance.getConservativeSeparatingDistance()<=0.f
#else
										1
#endif											
										)
										{
//#define USE_PE_BOX_BOX 1
#ifdef USE_PE_BOX_BOX
											{

												//getCollisionMargin0
												btScalar margin0 = lsMem.getlocalCollisionAlgorithm()->getCollisionMargin0();
												btScalar margin1 = lsMem.getlocalCollisionAlgorithm()->getCollisionMargin1();
												btVector3 shapeDim0 = lsMem.getlocalCollisionAlgorithm()->getShapeDimensions0()+btVector3(margin0,margin0,margin0);
												btVector3 shapeDim1 = lsMem.getlocalCollisionAlgorithm()->getShapeDimensions1()+btVector3(margin1,margin1,margin1);
/*
												//Box boxA(shapeDim0.getX(),shapeDim0.getY(),shapeDim0.getZ());
												vmVector3 vmPos0 = getVmVector3(collisionPairInput.m_worldTransform0.getOrigin());
												vmVector3 vmPos1 = getVmVector3(collisionPairInput.m_worldTransform1.getOrigin());
												vmMatrix3 vmMatrix0 = getVmMatrix3(collisionPairInput.m_worldTransform0.getBasis());
												vmMatrix3 vmMatrix1 = getVmMatrix3(collisionPairInput.m_worldTransform1.getBasis());

												vmTransform3 transformA(vmMatrix0,vmPos0);
												Box boxB(shapeDim1.getX(),shapeDim1.getY(),shapeDim1.getZ());
												vmTransform3 transformB(vmMatrix1,vmPos1);
												BoxPoint resultClosestBoxPointA;
												BoxPoint resultClosestBoxPointB;
												vmVector3 resultNormal;
												*/

#ifdef USE_SEPDISTANCE_UTIL
												float distanceThreshold = FLT_MAX
#else
												//float distanceThreshold = 0.f;
#endif


												vmVector3 n;
												Box boxA;
												vmVector3 hA(shapeDim0.getX(),shapeDim0.getY(),shapeDim0.getZ());
												vmVector3 hB(shapeDim1.getX(),shapeDim1.getY(),shapeDim1.getZ());
												boxA.mHalf= hA;
												vmTransform3 trA;
												trA.setTranslation(getVmVector3(collisionPairInput.m_worldTransform0.getOrigin()));
												trA.setUpper3x3(getVmMatrix3(collisionPairInput.m_worldTransform0.getBasis()));
												Box boxB;
												boxB.mHalf = hB;
												vmTransform3 trB;
												trB.setTranslation(getVmVector3(collisionPairInput.m_worldTransform1.getOrigin()));
												trB.setUpper3x3(getVmMatrix3(collisionPairInput.m_worldTransform1.getBasis()));
												
												float distanceThreshold = spuManifold->getContactBreakingThreshold();//0.001f;


												BoxPoint ptA,ptB;
												float dist = boxBoxDistance(n, ptA, ptB,
														   boxA, trA, boxB,	   trB,
															distanceThreshold );


//												float distance = boxBoxDistance(resultNormal,resultClosestBoxPointA,resultClosestBoxPointB,  boxA, transformA, boxB,transformB,distanceThreshold);
												
												normalInB = -getBtVector3(n);//resultNormal);

												//if(dist < distanceThreshold)//spuManifold->getContactBreakingThreshold())
												if(dist < spuManifold->getContactBreakingThreshold())
												{
													btVector3 pointOnB = collisionPairInput.m_worldTransform1(getBtVector3(ptB.localPoint));

													spuContacts.addContactPoint(
														normalInB,
														pointOnB,
														dist);
												}
											} 
#else									
											{

												btScalar margin0 = lsMem.getlocalCollisionAlgorithm()->getCollisionMargin0();
												btScalar margin1 = lsMem.getlocalCollisionAlgorithm()->getCollisionMargin1();
												btVector3 shapeDim0 = lsMem.getlocalCollisionAlgorithm()->getShapeDimensions0()+btVector3(margin0,margin0,margin0);
												btVector3 shapeDim1 = lsMem.getlocalCollisionAlgorithm()->getShapeDimensions1()+btVector3(margin1,margin1,margin1);


												btBoxShape box0(shapeDim0);
												btBoxShape box1(shapeDim1);

												struct SpuBridgeContactCollector : public btDiscreteCollisionDetectorInterface::Result
												{
													SpuContactResult&	m_spuContacts;

													virtual void setShapeIdentifiersA(int partId0,int index0)
													{
														m_spuContacts.setShapeIdentifiersA(partId0,index0);
													}
													virtual void setShapeIdentifiersB(int partId1,int index1)
													{
														m_spuContacts.setShapeIdentifiersB(partId1,index1);
													}
													virtual void addContactPoint(const btVector3& normalOnBInWorld,const btVector3& pointInWorld,btScalar depth)
													{
														m_spuContacts.addContactPoint(normalOnBInWorld,pointInWorld,depth);
													}

													SpuBridgeContactCollector(SpuContactResult& spuContacts)
														:m_spuContacts(spuContacts)
													{

													}
												};
												
												SpuBridgeContactCollector  bridgeOutput(spuContacts);

												btDiscreteCollisionDetectorInterface::ClosestPointInput input;
												input.m_maximumDistanceSquared = BT_LARGE_FLOAT;
												input.m_transformA = collisionPairInput.m_worldTransform0;
												input.m_transformB = collisionPairInput.m_worldTransform1;

												btBoxBoxDetector detector(&box0,&box1);
												
												detector.getClosestPoints(input,bridgeOutput,0);

											}
#endif //USE_PE_BOX_BOX
											
											lsMem.needsDmaPutContactManifoldAlgo = true;
#ifdef USE_SEPDISTANCE_UTIL
											btScalar sepDist2 = distance+spuManifold->getContactBreakingThreshold();
											lsMem.getlocalCollisionAlgorithm()->m_sepDistance.initSeparatingDistance(normalInB,sepDist2,collisionPairInput.m_worldTransform0,collisionPairInput.m_worldTransform1);
#endif //USE_SEPDISTANCE_UTIL
											gProcessedCol++;
										} else
										{
											gSkippedCol++;
										}

										spuContacts.flush();
											

									} else
#endif //USE_DEDICATED_BOX_BOX
									{
										if (
#ifdef USE_SEPDISTANCE_UTIL
											lsMem.getlocalCollisionAlgorithm()->m_sepDistance.getConservativeSeparatingDistance()<=0.f
#else
											1
#endif //USE_SEPDISTANCE_UTIL
											)
										{
											handleCollisionPair(collisionPairInput, lsMem, spuContacts,
												(ppu_address_t)lsMem.getColObj0()->getRootCollisionShape(), &lsMem.gCollisionShapes[0].collisionShape,
												(ppu_address_t)lsMem.getColObj1()->getRootCollisionShape(), &lsMem.gCollisionShapes[1].collisionShape);
										} else
										{
												//spu_printf("boxbox dist = %f\n",distance);
											btPersistentManifold* spuManifold=lsMem.getContactManifoldPtr();
											btPersistentManifold* manifold = (btPersistentManifold*)collisionPairInput.m_persistentManifoldPtr;
											ppu_address_t manifoldAddress = (ppu_address_t)manifold;

											spuContacts.setContactInfo(spuManifold,manifoldAddress,lsMem.getColObj0()->getWorldTransform(),
												lsMem.getColObj1()->getWorldTransform(),
												lsMem.getColObj0()->getRestitution(),lsMem.getColObj1()->getRestitution(),
												lsMem.getColObj0()->getFriction(),lsMem.getColObj1()->getFriction(),
												collisionPairInput.m_isSwapped);

											spuContacts.flush();
										}
									}
								
								}

							}
						}

#ifdef USE_SEPDISTANCE_UTIL
#if defined (__SPU__) || defined (USE_LIBSPE2)
						if (lsMem.needsDmaPutContactManifoldAlgo)
						{
							dmaSize = sizeof(SpuContactManifoldCollisionAlgorithm);
							dmaPpuAddress2 = (ppu_address_t)pair.m_algorithm;
							cellDmaLargePut(&lsMem.gSpuContactManifoldAlgoBuffer, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);
							cellDmaWaitTagStatusAll(DMA_MASK(1));
						}
#endif
#endif //#ifdef USE_SEPDISTANCE_UTIL

					}
				}
			}
		} //end for (j = 0; j < numOnPage; j++)

	}//	for 



	return;
}


