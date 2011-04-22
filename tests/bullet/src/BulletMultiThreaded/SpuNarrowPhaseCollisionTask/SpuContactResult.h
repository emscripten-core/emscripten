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

#ifndef SPU_CONTACT_RESULT2_H
#define SPU_CONTACT_RESULT2_H


#ifndef _WIN32
#include <stdint.h>
#endif



#include "../SpuDoubleBuffer.h"


#include "LinearMath/btTransform.h"


#include "BulletCollision/NarrowPhaseCollision/btPersistentManifold.h"
#include "BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h"

class btCollisionShape;


struct SpuCollisionPairInput
{
	ppu_address_t m_collisionShapes[2];
	btCollisionShape*	m_spuCollisionShapes[2];

	ppu_address_t m_persistentManifoldPtr;
	btVector3	m_primitiveDimensions0;
	btVector3	m_primitiveDimensions1;
	int		m_shapeType0;
	int		m_shapeType1;	
	float	m_collisionMargin0;
	float	m_collisionMargin1;

	btTransform	m_worldTransform0;
	btTransform m_worldTransform1;
	
	bool	m_isSwapped;
	bool    m_useEpa;
};


struct SpuClosestPointInput : public btDiscreteCollisionDetectorInterface::ClosestPointInput
{
	struct SpuConvexPolyhedronVertexData* m_convexVertexData[2];
};

///SpuContactResult exports the contact points using double-buffered DMA transfers, only when needed
///So when an existing contact point is duplicated, no transfer/refresh is performed.
class SpuContactResult : public btDiscreteCollisionDetectorInterface::Result
{
    btTransform		m_rootWorldTransform0;
	btTransform		m_rootWorldTransform1;
	ppu_address_t	m_manifoldAddress;

    btPersistentManifold* m_spuManifold;
	bool m_RequiresWriteBack;
	btScalar	m_combinedFriction;
	btScalar	m_combinedRestitution;
	
	bool m_isSwapped;

	DoubleBuffer<btPersistentManifold, 1> g_manifoldDmaExport;

	public:
		SpuContactResult();
		virtual ~SpuContactResult();

		btPersistentManifold*	GetSpuManifold() const
		{
			return m_spuManifold;
		}

		virtual void setShapeIdentifiersA(int partId0,int index0);
		virtual void setShapeIdentifiersB(int partId1,int index1);

		void	setContactInfo(btPersistentManifold* spuManifold, ppu_address_t	manifoldAddress,const btTransform& worldTrans0,const btTransform& worldTrans1, btScalar restitution0,btScalar restitution1, btScalar friction0,btScalar friction01, bool isSwapped);


        void writeDoubleBufferedManifold(btPersistentManifold* lsManifold, btPersistentManifold* mmManifold);

        virtual void addContactPoint(const btVector3& normalOnBInWorld,const btVector3& pointInWorld,btScalar depth);

		void flush();
};



#endif //SPU_CONTACT_RESULT2_H

