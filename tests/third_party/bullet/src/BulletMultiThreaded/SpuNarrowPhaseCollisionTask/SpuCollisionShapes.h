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
#ifndef __SPU_COLLISION_SHAPES_H
#define __SPU_COLLISION_SHAPES_H

#include "../SpuDoubleBuffer.h"

#include "BulletCollision/BroadphaseCollision/btBroadphaseProxy.h"
#include "BulletCollision/CollisionShapes/btConvexInternalShape.h"
#include "BulletCollision/CollisionShapes/btCylinderShape.h"
#include "BulletCollision/CollisionShapes/btStaticPlaneShape.h"

#include "BulletCollision/CollisionShapes/btOptimizedBvh.h"
#include "BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"

#include "BulletCollision/CollisionShapes/btCapsuleShape.h"

#include "BulletCollision/CollisionShapes/btConvexShape.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"

#define MAX_NUM_SPU_CONVEX_POINTS 128 //@fallback to PPU if a btConvexHullShape has more than MAX_NUM_SPU_CONVEX_POINTS points
#define MAX_SPU_COMPOUND_SUBSHAPES 16 //@fallback on PPU if compound has more than MAX_SPU_COMPOUND_SUBSHAPES child shapes
#define MAX_SHAPE_SIZE 256 //@todo: assert on this

ATTRIBUTE_ALIGNED16(struct)	SpuConvexPolyhedronVertexData
{
	void*	gSpuConvexShapePtr;
	btVector3* gConvexPoints;
	int gNumConvexPoints;
	int unused;
	ATTRIBUTE_ALIGNED16(btVector3 g_convexPointBuffer[MAX_NUM_SPU_CONVEX_POINTS]);
};



ATTRIBUTE_ALIGNED16(struct) CollisionShape_LocalStoreMemory
{
	ATTRIBUTE_ALIGNED16(char collisionShape[MAX_SHAPE_SIZE]);
};

ATTRIBUTE_ALIGNED16(struct) CompoundShape_LocalStoreMemory
{
	// Compound data

	ATTRIBUTE_ALIGNED16(btCompoundShapeChild gSubshapes[MAX_SPU_COMPOUND_SUBSHAPES]);
	ATTRIBUTE_ALIGNED16(char gSubshapeShape[MAX_SPU_COMPOUND_SUBSHAPES][MAX_SHAPE_SIZE]);
};

ATTRIBUTE_ALIGNED16(struct) bvhMeshShape_LocalStoreMemory
{
	//ATTRIBUTE_ALIGNED16(btOptimizedBvh	gOptimizedBvh);
	ATTRIBUTE_ALIGNED16(char gOptimizedBvh[sizeof(btOptimizedBvh)+16]);
	btOptimizedBvh*	getOptimizedBvh()
	{
		return (btOptimizedBvh*) gOptimizedBvh;
	}

	ATTRIBUTE_ALIGNED16(btTriangleIndexVertexArray	gTriangleMeshInterfaceStorage);
	btTriangleIndexVertexArray*	gTriangleMeshInterfacePtr;
	///only a single mesh part for now, we can add support for multiple parts, but quantized trees don't support this at the moment 
	ATTRIBUTE_ALIGNED16(btIndexedMesh	gIndexMesh);
	#define MAX_SPU_SUBTREE_HEADERS 32
	//1024
	ATTRIBUTE_ALIGNED16(btBvhSubtreeInfo	gSubtreeHeaders[MAX_SPU_SUBTREE_HEADERS]);
	ATTRIBUTE_ALIGNED16(btQuantizedBvhNode	gSubtreeNodes[MAX_SUBTREE_SIZE_IN_BYTES/sizeof(btQuantizedBvhNode)]);
};


void computeAabb (btVector3& aabbMin, btVector3& aabbMax, btConvexInternalShape* convexShape, ppu_address_t convexShapePtr, int shapeType, const btTransform& xform);
void dmaBvhShapeData (bvhMeshShape_LocalStoreMemory* bvhMeshShape, btBvhTriangleMeshShape* triMeshShape);
void dmaBvhIndexedMesh (btIndexedMesh* IndexMesh, IndexedMeshArray& indexArray, int index, uint32_t dmaTag);
void dmaBvhSubTreeHeaders (btBvhSubtreeInfo* subTreeHeaders, ppu_address_t subTreePtr, int batchSize, uint32_t dmaTag);
void dmaBvhSubTreeNodes (btQuantizedBvhNode* nodes, const btBvhSubtreeInfo& subtree, QuantizedNodeArray&	nodeArray, int dmaTag);

int  getShapeTypeSize(int shapeType);
void dmaConvexVertexData (SpuConvexPolyhedronVertexData* convexVertexData, btConvexHullShape* convexShapeSPU);
void dmaCollisionShape (void* collisionShapeLocation, ppu_address_t collisionShapePtr, uint32_t dmaTag, int shapeType);
void dmaCompoundShapeInfo (CompoundShape_LocalStoreMemory* compoundShapeLocation, btCompoundShape* spuCompoundShape, uint32_t dmaTag);
void dmaCompoundSubShapes (CompoundShape_LocalStoreMemory* compoundShapeLocation, btCompoundShape* spuCompoundShape, uint32_t dmaTag);


#define USE_BRANCHFREE_TEST 1
#ifdef USE_BRANCHFREE_TEST
SIMD_FORCE_INLINE unsigned int spuTestQuantizedAabbAgainstQuantizedAabb(unsigned short int* aabbMin1,unsigned short int* aabbMax1,const unsigned short int* aabbMin2,const unsigned short int* aabbMax2)
{		
#if defined(__CELLOS_LV2__) && defined (__SPU__)
	vec_ushort8 vecMin = {aabbMin1[0],aabbMin2[0],aabbMin1[2],aabbMin2[2],aabbMin1[1],aabbMin2[1],0,0};
	vec_ushort8 vecMax = {aabbMax2[0],aabbMax1[0],aabbMax2[2],aabbMax1[2],aabbMax2[1],aabbMax1[1],0,0};
	vec_ushort8 isGt = spu_cmpgt(vecMin,vecMax);
	return spu_extract(spu_gather(isGt),0)==0;

#else
	return btSelect((unsigned)((aabbMin1[0] <= aabbMax2[0]) & (aabbMax1[0] >= aabbMin2[0])
		& (aabbMin1[2] <= aabbMax2[2]) & (aabbMax1[2] >= aabbMin2[2])
		& (aabbMin1[1] <= aabbMax2[1]) & (aabbMax1[1] >= aabbMin2[1])),
		1, 0);
#endif
}
#else

SIMD_FORCE_INLINE unsigned int spuTestQuantizedAabbAgainstQuantizedAabb(const unsigned short int* aabbMin1,const unsigned short int* aabbMax1,const unsigned short int* aabbMin2,const unsigned short int*  aabbMax2)
{
	unsigned int overlap = 1;
	overlap = (aabbMin1[0] > aabbMax2[0] || aabbMax1[0] < aabbMin2[0]) ? 0 : overlap;
	overlap = (aabbMin1[2] > aabbMax2[2] || aabbMax1[2] < aabbMin2[2]) ? 0 : overlap;
	overlap = (aabbMin1[1] > aabbMax2[1] || aabbMax1[1] < aabbMin2[1]) ? 0 : overlap;
	return overlap;
}
#endif

void	spuWalkStacklessQuantizedTree(btNodeOverlapCallback* nodeCallback,unsigned short int* quantizedQueryAabbMin,unsigned short int* quantizedQueryAabbMax,const btQuantizedBvhNode* rootNode,int startNodeIndex,int endNodeIndex);

#endif
