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


#include "SpuCollisionShapes.h"

///not supported on IBM SDK, until we fix the alignment of btVector3
#if defined (__CELLOS_LV2__) && defined (__SPU__)
#include <spu_intrinsics.h>
static inline vec_float4 vec_dot3( vec_float4 vec0, vec_float4 vec1 )
{
    vec_float4 result;
    result = spu_mul( vec0, vec1 );
    result = spu_madd( spu_rlqwbyte( vec0, 4 ), spu_rlqwbyte( vec1, 4 ), result );
    return spu_madd( spu_rlqwbyte( vec0, 8 ), spu_rlqwbyte( vec1, 8 ), result );
}
#endif //__SPU__


void computeAabb (btVector3& aabbMin, btVector3& aabbMax, btConvexInternalShape* convexShape, ppu_address_t convexShapePtr, int shapeType, const btTransform& xform)
{
	//calculate the aabb, given the types...
	switch (shapeType)
	{
	case CYLINDER_SHAPE_PROXYTYPE:
		/* fall through */
	case BOX_SHAPE_PROXYTYPE:
	{
		btScalar margin=convexShape->getMarginNV();
		btVector3 halfExtents = convexShape->getImplicitShapeDimensions();
		halfExtents += btVector3(margin,margin,margin);
		const btTransform& t = xform;
		btMatrix3x3 abs_b = t.getBasis().absolute();  
		btVector3 center = t.getOrigin();
		btVector3 extent = btVector3(abs_b[0].dot(halfExtents),abs_b[1].dot(halfExtents),abs_b[2].dot(halfExtents));
		
		aabbMin = center - extent;
		aabbMax = center + extent;
		break;
	}
	case CAPSULE_SHAPE_PROXYTYPE:
	{
		btScalar margin=convexShape->getMarginNV();
		btVector3 halfExtents = convexShape->getImplicitShapeDimensions();
		//add the radius to y-axis to get full height
		btScalar radius = halfExtents[0];
		halfExtents[1] += radius;
		halfExtents += btVector3(margin,margin,margin);
#if 0
		int capsuleUpAxis = convexShape->getUpAxis();
		btScalar halfHeight = convexShape->getHalfHeight();
		btScalar radius = convexShape->getRadius();
		halfExtents[capsuleUpAxis] = radius + halfHeight;
#endif
		const btTransform& t = xform;
		btMatrix3x3 abs_b = t.getBasis().absolute();  
		btVector3 center = t.getOrigin();
		btVector3 extent = btVector3(abs_b[0].dot(halfExtents),abs_b[1].dot(halfExtents),abs_b[2].dot(halfExtents));
		
		aabbMin = center - extent;
		aabbMax = center + extent;
		break;
	}
	case SPHERE_SHAPE_PROXYTYPE:
	{
		btScalar radius = convexShape->getImplicitShapeDimensions().getX();// * convexShape->getLocalScaling().getX();
		btScalar margin = radius + convexShape->getMarginNV();
		const btTransform& t = xform;
		const btVector3& center = t.getOrigin();
		btVector3 extent(margin,margin,margin);
		aabbMin = center - extent;
		aabbMax = center + extent;
		break;
	}
	case CONVEX_HULL_SHAPE_PROXYTYPE:
	{
		ATTRIBUTE_ALIGNED16(char convexHullShape0[sizeof(btConvexHullShape)]);
		cellDmaGet(&convexHullShape0, convexShapePtr  , sizeof(btConvexHullShape), DMA_TAG(1), 0, 0);
		cellDmaWaitTagStatusAll(DMA_MASK(1));
		btConvexHullShape* localPtr = (btConvexHullShape*)&convexHullShape0;
		const btTransform& t = xform;
		btScalar margin = convexShape->getMarginNV();
		localPtr->getNonvirtualAabb(t,aabbMin,aabbMax,margin);
		//spu_printf("SPU convex aabbMin=%f,%f,%f=\n",aabbMin.getX(),aabbMin.getY(),aabbMin.getZ());
		//spu_printf("SPU convex aabbMax=%f,%f,%f=\n",aabbMax.getX(),aabbMax.getY(),aabbMax.getZ());
		break;
	}
	default:
		{
	//	spu_printf("SPU: unsupported shapetype %d in AABB calculation\n");
		}
	};
}

void dmaBvhShapeData (bvhMeshShape_LocalStoreMemory* bvhMeshShape, btBvhTriangleMeshShape* triMeshShape)
{
	register int dmaSize;
	register ppu_address_t	dmaPpuAddress2;

	dmaSize = sizeof(btTriangleIndexVertexArray);
	dmaPpuAddress2 = reinterpret_cast<ppu_address_t>(triMeshShape->getMeshInterface());
	//	spu_printf("trimeshShape->getMeshInterface() == %llx\n",dmaPpuAddress2);
#ifdef __SPU__
	cellDmaGet(&bvhMeshShape->gTriangleMeshInterfaceStorage, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);
	bvhMeshShape->gTriangleMeshInterfacePtr = &bvhMeshShape->gTriangleMeshInterfaceStorage;
#else
	bvhMeshShape->gTriangleMeshInterfacePtr = (btTriangleIndexVertexArray*)cellDmaGetReadOnly(&bvhMeshShape->gTriangleMeshInterfaceStorage, dmaPpuAddress2  , dmaSize, DMA_TAG(1), 0, 0);
#endif

	//cellDmaWaitTagStatusAll(DMA_MASK(1));
	
	///now DMA over the BVH
	
	dmaSize = sizeof(btOptimizedBvh);
	dmaPpuAddress2 = reinterpret_cast<ppu_address_t>(triMeshShape->getOptimizedBvh());
	//spu_printf("trimeshShape->getOptimizedBvh() == %llx\n",dmaPpuAddress2);
	cellDmaGet(&bvhMeshShape->gOptimizedBvh, dmaPpuAddress2  , dmaSize, DMA_TAG(2), 0, 0);
	//cellDmaWaitTagStatusAll(DMA_MASK(2));
	cellDmaWaitTagStatusAll(DMA_MASK(1) | DMA_MASK(2));
}

void dmaBvhIndexedMesh (btIndexedMesh* IndexMesh, IndexedMeshArray& indexArray, int index, uint32_t dmaTag)
{		
	cellDmaGet(IndexMesh, (ppu_address_t)&indexArray[index]  , sizeof(btIndexedMesh), DMA_TAG(dmaTag), 0, 0);
	
}

void dmaBvhSubTreeHeaders (btBvhSubtreeInfo* subTreeHeaders, ppu_address_t subTreePtr, int batchSize, uint32_t dmaTag)
{
	cellDmaGet(subTreeHeaders, subTreePtr, batchSize * sizeof(btBvhSubtreeInfo), DMA_TAG(dmaTag), 0, 0);
}

void dmaBvhSubTreeNodes (btQuantizedBvhNode* nodes, const btBvhSubtreeInfo& subtree, QuantizedNodeArray&	nodeArray, int dmaTag)
{
	cellDmaGet(nodes, reinterpret_cast<ppu_address_t>(&nodeArray[subtree.m_rootNodeIndex]) , subtree.m_subtreeSize* sizeof(btQuantizedBvhNode), DMA_TAG(2), 0, 0);
}

///getShapeTypeSize could easily be optimized, but it is not likely a bottleneck
int		getShapeTypeSize(int shapeType)
{


	switch (shapeType)
	{
	case CYLINDER_SHAPE_PROXYTYPE:
		{
			int shapeSize = sizeof(btCylinderShape);
			btAssert(shapeSize < MAX_SHAPE_SIZE);
			return shapeSize;
		}
	case BOX_SHAPE_PROXYTYPE:
		{
			int shapeSize = sizeof(btBoxShape);
			btAssert(shapeSize < MAX_SHAPE_SIZE);
			return shapeSize;
		}
	case SPHERE_SHAPE_PROXYTYPE:
		{
			int shapeSize = sizeof(btSphereShape);
			btAssert(shapeSize < MAX_SHAPE_SIZE);
			return shapeSize;
		}
	case TRIANGLE_MESH_SHAPE_PROXYTYPE:
		{
			int shapeSize = sizeof(btBvhTriangleMeshShape);
			btAssert(shapeSize < MAX_SHAPE_SIZE);
			return shapeSize;
		}
	case CAPSULE_SHAPE_PROXYTYPE:
		{
			int shapeSize = sizeof(btCapsuleShape);
			btAssert(shapeSize < MAX_SHAPE_SIZE);
			return shapeSize;
		}

	case CONVEX_HULL_SHAPE_PROXYTYPE:
		{
			int shapeSize = sizeof(btConvexHullShape);
			btAssert(shapeSize < MAX_SHAPE_SIZE);
			return shapeSize;
		}

	case COMPOUND_SHAPE_PROXYTYPE:
		{
			int shapeSize = sizeof(btCompoundShape);
			btAssert(shapeSize < MAX_SHAPE_SIZE);
			return shapeSize;
		}
	case STATIC_PLANE_PROXYTYPE:
		{
			int shapeSize = sizeof(btStaticPlaneShape);
			btAssert(shapeSize < MAX_SHAPE_SIZE);
			return shapeSize;
		}

	default:
		btAssert(0);
		//unsupported shapetype, please add here
		return 0;
	}
}

void dmaConvexVertexData (SpuConvexPolyhedronVertexData* convexVertexData, btConvexHullShape* convexShapeSPU)
{
	convexVertexData->gNumConvexPoints = convexShapeSPU->getNumPoints();
	if (convexVertexData->gNumConvexPoints>MAX_NUM_SPU_CONVEX_POINTS)
	{
		btAssert(0);
	//	spu_printf("SPU: Error: MAX_NUM_SPU_CONVEX_POINTS(%d) exceeded: %d\n",MAX_NUM_SPU_CONVEX_POINTS,convexVertexData->gNumConvexPoints);
		return;
	}
			
	register int dmaSize = convexVertexData->gNumConvexPoints*sizeof(btVector3);
	ppu_address_t pointsPPU = (ppu_address_t) convexShapeSPU->getUnscaledPoints();
	cellDmaGet(&convexVertexData->g_convexPointBuffer[0], pointsPPU  , dmaSize, DMA_TAG(2), 0, 0);
}

void dmaCollisionShape (void* collisionShapeLocation, ppu_address_t collisionShapePtr, uint32_t dmaTag, int shapeType)
{
	register int dmaSize = getShapeTypeSize(shapeType);
	cellDmaGet(collisionShapeLocation, collisionShapePtr  , dmaSize, DMA_TAG(dmaTag), 0, 0);
	//cellDmaGetReadOnly(collisionShapeLocation, collisionShapePtr  , dmaSize, DMA_TAG(dmaTag), 0, 0);
	//cellDmaWaitTagStatusAll(DMA_MASK(dmaTag));
}

void dmaCompoundShapeInfo (CompoundShape_LocalStoreMemory* compoundShapeLocation, btCompoundShape* spuCompoundShape, uint32_t dmaTag)
{
	register int dmaSize;
	register	ppu_address_t	dmaPpuAddress2;
	int childShapeCount = spuCompoundShape->getNumChildShapes();
	dmaSize = childShapeCount * sizeof(btCompoundShapeChild);
	dmaPpuAddress2 = (ppu_address_t)spuCompoundShape->getChildList();
	cellDmaGet(&compoundShapeLocation->gSubshapes[0], dmaPpuAddress2, dmaSize, DMA_TAG(dmaTag), 0, 0);
}

void dmaCompoundSubShapes (CompoundShape_LocalStoreMemory* compoundShapeLocation, btCompoundShape* spuCompoundShape, uint32_t dmaTag)
{
	int childShapeCount = spuCompoundShape->getNumChildShapes();
	int i;
	// DMA all the subshapes 
	for ( i = 0; i < childShapeCount; ++i)
	{
		btCompoundShapeChild& childShape = compoundShapeLocation->gSubshapes[i];
		dmaCollisionShape (&compoundShapeLocation->gSubshapeShape[i],(ppu_address_t)childShape.m_childShape, dmaTag, childShape.m_childShapeType);
	}
}


void	spuWalkStacklessQuantizedTree(btNodeOverlapCallback* nodeCallback,unsigned short int* quantizedQueryAabbMin,unsigned short int* quantizedQueryAabbMax,const btQuantizedBvhNode* rootNode,int startNodeIndex,int endNodeIndex)
{

	int curIndex = startNodeIndex;
	int walkIterations = 0;
#ifdef BT_DEBUG
	int subTreeSize = endNodeIndex - startNodeIndex;
#endif

	int escapeIndex;

	unsigned int aabbOverlap, isLeafNode;

	while (curIndex < endNodeIndex)
	{
		//catch bugs in tree data
		btAssert (walkIterations < subTreeSize);

		walkIterations++;
		aabbOverlap = spuTestQuantizedAabbAgainstQuantizedAabb(quantizedQueryAabbMin,quantizedQueryAabbMax,rootNode->m_quantizedAabbMin,rootNode->m_quantizedAabbMax);
		isLeafNode = rootNode->isLeafNode();

		if (isLeafNode && aabbOverlap)
		{
			//printf("overlap with node %d\n",rootNode->getTriangleIndex());
			nodeCallback->processNode(0,rootNode->getTriangleIndex());
			//			spu_printf("SPU: overlap detected with triangleIndex:%d\n",rootNode->getTriangleIndex());
		} 

		if (aabbOverlap || isLeafNode)
		{
			rootNode++;
			curIndex++;
		} else
		{
			escapeIndex = rootNode->getEscapeIndex();
			rootNode += escapeIndex;
			curIndex += escapeIndex;
		}
	}

}
