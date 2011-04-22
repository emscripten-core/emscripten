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

#include "SpuContactResult.h"

//#define DEBUG_SPU_COLLISION_DETECTION 1

#ifdef DEBUG_SPU_COLLISION_DETECTION
#ifndef __SPU__
#include <stdio.h>
#define spu_printf printf
#endif
#endif //DEBUG_SPU_COLLISION_DETECTION

SpuContactResult::SpuContactResult()
{
	m_manifoldAddress = 0;
	m_spuManifold = NULL;
	m_RequiresWriteBack = false;
}

 SpuContactResult::~SpuContactResult()
{
	g_manifoldDmaExport.swapBuffers();
}

 	///User can override this material combiner by implementing gContactAddedCallback and setting body0->m_collisionFlags |= btCollisionObject::customMaterialCallback;
inline btScalar	calculateCombinedFriction(btScalar friction0,btScalar friction1)
{
	btScalar friction = friction0*friction1;

	const btScalar MAX_FRICTION  = btScalar(10.);

	if (friction < -MAX_FRICTION)
		friction = -MAX_FRICTION;
	if (friction > MAX_FRICTION)
		friction = MAX_FRICTION;
	return friction;

}

inline btScalar	calculateCombinedRestitution(btScalar restitution0,btScalar restitution1)
{
	return restitution0*restitution1;
}



 void	SpuContactResult::setContactInfo(btPersistentManifold* spuManifold, ppu_address_t	manifoldAddress,const btTransform& worldTrans0,const btTransform& worldTrans1, btScalar restitution0,btScalar restitution1, btScalar friction0,btScalar friction1, bool isSwapped)
 {
	//spu_printf("SpuContactResult::setContactInfo ManifoldAddress: %lu\n", manifoldAddress);
	m_rootWorldTransform0 = worldTrans0;
	m_rootWorldTransform1 = worldTrans1;
	m_manifoldAddress = manifoldAddress;    
	m_spuManifold = spuManifold;

	m_combinedFriction = calculateCombinedFriction(friction0,friction1);
	m_combinedRestitution = calculateCombinedRestitution(restitution0,restitution1);
	m_isSwapped = isSwapped;
 }

 void SpuContactResult::setShapeIdentifiersA(int partId0,int index0)
 {
	
 }

 void SpuContactResult::setShapeIdentifiersB(int partId1,int index1)
 {
	
 }



 ///return true if it requires a dma transfer back
bool ManifoldResultAddContactPoint(const btVector3& normalOnBInWorld,
								   const btVector3& pointInWorld,
								   float depth,
								   btPersistentManifold* manifoldPtr,
								   btTransform& transA,
								   btTransform& transB,
									btScalar	combinedFriction,
									btScalar	combinedRestitution,
								   bool isSwapped)
{
	
//	float contactTreshold = manifoldPtr->getContactBreakingThreshold();

	//spu_printf("SPU: add contactpoint, depth:%f, contactTreshold %f, manifoldPtr %llx\n",depth,contactTreshold,manifoldPtr);

#ifdef DEBUG_SPU_COLLISION_DETECTION
	spu_printf("SPU: contactTreshold %f\n",contactTreshold);
#endif //DEBUG_SPU_COLLISION_DETECTION
	//if (depth > manifoldPtr->getContactBreakingThreshold())
	//	return false;

	if (depth > manifoldPtr->getContactProcessingThreshold())
		return false;



	btVector3 pointA;
	btVector3 localA;
	btVector3 localB;
	btVector3 normal;


	if (isSwapped)
	{
		normal = normalOnBInWorld * -1;
		pointA = pointInWorld + normal * depth;
		localA = transA.invXform(pointA );
		localB = transB.invXform(pointInWorld);
	}
	else
	{
		normal = normalOnBInWorld;
		pointA = pointInWorld + normal * depth;
		localA = transA.invXform(pointA );
		localB = transB.invXform(pointInWorld);
	}

	btManifoldPoint newPt(localA,localB,normal,depth);
	newPt.m_positionWorldOnA = pointA;
	newPt.m_positionWorldOnB = pointInWorld;

	newPt.m_combinedFriction = combinedFriction;
	newPt.m_combinedRestitution = combinedRestitution;


	int insertIndex = manifoldPtr->getCacheEntry(newPt);
	if (insertIndex >= 0)
	{
		// we need to replace the current contact point, otherwise small errors will accumulate (spheres start rolling etc)
		manifoldPtr->replaceContactPoint(newPt,insertIndex);
		return true;
		
	} else
	{

		/*
		///@todo: SPU callbacks, either immediate (local on the SPU), or deferred
		//User can override friction and/or restitution
		if (gContactAddedCallback &&
			//and if either of the two bodies requires custom material
			 ((m_body0->m_collisionFlags & btCollisionObject::customMaterialCallback) ||
			   (m_body1->m_collisionFlags & btCollisionObject::customMaterialCallback)))
		{
			//experimental feature info, for per-triangle material etc.
			(*gContactAddedCallback)(newPt,m_body0,m_partId0,m_index0,m_body1,m_partId1,m_index1);
		}
		*/

		manifoldPtr->addManifoldPoint(newPt);
		return true;

	}
	return false;
	
}


void SpuContactResult::writeDoubleBufferedManifold(btPersistentManifold* lsManifold, btPersistentManifold* mmManifold)
{
	///only write back the contact information on SPU. Other platforms avoid copying, and use the data in-place
	///see SpuFakeDma.cpp 'cellDmaLargeGetReadOnly'
#if defined (__SPU__) || defined (USE_LIBSPE2)
    memcpy(g_manifoldDmaExport.getFront(),lsManifold,sizeof(btPersistentManifold));

    g_manifoldDmaExport.swapBuffers();
    ppu_address_t mmAddr = (ppu_address_t)mmManifold;
    g_manifoldDmaExport.backBufferDmaPut(mmAddr, sizeof(btPersistentManifold), DMA_TAG(9));
	// Should there be any kind of wait here?  What if somebody tries to use this tag again?  What if we call this function again really soon?
	//no, the swapBuffers does the wait
#endif
}

void SpuContactResult::addContactPoint(const btVector3& normalOnBInWorld,const btVector3& pointInWorld,btScalar depth)
{
#ifdef DEBUG_SPU_COLLISION_DETECTION
	spu_printf("*** SpuContactResult::addContactPoint: depth = %f\n",depth);
	spu_printf("*** normal = %f,%f,%f\n",normalOnBInWorld.getX(),normalOnBInWorld.getY(),normalOnBInWorld.getZ());
	spu_printf("*** position = %f,%f,%f\n",pointInWorld.getX(),pointInWorld.getY(),pointInWorld.getZ());
#endif //DEBUG_SPU_COLLISION_DETECTION
	

#ifdef DEBUG_SPU_COLLISION_DETECTION
 //   int sman = sizeof(rage::phManifold);
//	spu_printf("sizeof_manifold = %i\n",sman);
#endif //DEBUG_SPU_COLLISION_DETECTION

	btPersistentManifold* localManifold = m_spuManifold;

	btVector3	normalB(normalOnBInWorld.getX(),normalOnBInWorld.getY(),normalOnBInWorld.getZ());
	btVector3	pointWrld(pointInWorld.getX(),pointInWorld.getY(),pointInWorld.getZ());

	//process the contact point
	const bool retVal = ManifoldResultAddContactPoint(normalB,
		pointWrld,
		depth,
		localManifold,
		m_rootWorldTransform0,
		m_rootWorldTransform1,
		m_combinedFriction,
		m_combinedRestitution,
		m_isSwapped);
	m_RequiresWriteBack = m_RequiresWriteBack || retVal;
}

void SpuContactResult::flush()
{

	if (m_spuManifold && m_spuManifold->getNumContacts())
	{
		m_spuManifold->refreshContactPoints(m_rootWorldTransform0,m_rootWorldTransform1);
		m_RequiresWriteBack = true;
	}


	if (m_RequiresWriteBack)
	{
#ifdef DEBUG_SPU_COLLISION_DETECTION
		spu_printf("SPU: Start SpuContactResult::flush (Put) DMA\n");
		spu_printf("Num contacts:%d\n", m_spuManifold->getNumContacts());
		spu_printf("Manifold address: %llu\n", m_manifoldAddress);
#endif //DEBUG_SPU_COLLISION_DETECTION
	//	spu_printf("writeDoubleBufferedManifold\n");
		writeDoubleBufferedManifold(m_spuManifold, (btPersistentManifold*)m_manifoldAddress);
#ifdef DEBUG_SPU_COLLISION_DETECTION
		spu_printf("SPU: Finished (Put) DMA\n");
#endif //DEBUG_SPU_COLLISION_DETECTION
	}
	m_spuManifold = NULL;
	m_RequiresWriteBack = false;
}


