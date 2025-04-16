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


#include "btCollisionObject.h"
#include "LinearMath/btSerializer.h"

btCollisionObject::btCollisionObject()
	:	m_anisotropicFriction(1.f,1.f,1.f),
	m_hasAnisotropicFriction(false),
	m_contactProcessingThreshold(BT_LARGE_FLOAT),
		m_broadphaseHandle(0),
		m_collisionShape(0),
		m_extensionPointer(0),
		m_rootCollisionShape(0),
		m_collisionFlags(btCollisionObject::CF_STATIC_OBJECT),
		m_islandTag1(-1),
		m_companionId(-1),
		m_activationState1(1),
		m_deactivationTime(btScalar(0.)),
		m_friction(btScalar(0.5)),
		m_restitution(btScalar(0.)),
		m_internalType(CO_COLLISION_OBJECT),
		m_userObjectPointer(0),
		m_hitFraction(btScalar(1.)),
		m_ccdSweptSphereRadius(btScalar(0.)),
		m_ccdMotionThreshold(btScalar(0.)),
		m_checkCollideWith(false)
{
	m_worldTransform.setIdentity();
}

btCollisionObject::~btCollisionObject()
{
}

void btCollisionObject::setActivationState(int newState) 
{ 
	if ( (m_activationState1 != DISABLE_DEACTIVATION) && (m_activationState1 != DISABLE_SIMULATION))
		m_activationState1 = newState;
}

void btCollisionObject::forceActivationState(int newState)
{
	m_activationState1 = newState;
}

void btCollisionObject::activate(bool forceActivation)
{
	if (forceActivation || !(m_collisionFlags & (CF_STATIC_OBJECT|CF_KINEMATIC_OBJECT)))
	{
		setActivationState(ACTIVE_TAG);
		m_deactivationTime = btScalar(0.);
	}
}

const char* btCollisionObject::serialize(void* dataBuffer, btSerializer* serializer) const
{

	btCollisionObjectData* dataOut = (btCollisionObjectData*)dataBuffer;

	m_worldTransform.serialize(dataOut->m_worldTransform);
	m_interpolationWorldTransform.serialize(dataOut->m_interpolationWorldTransform);
	m_interpolationLinearVelocity.serialize(dataOut->m_interpolationLinearVelocity);
	m_interpolationAngularVelocity.serialize(dataOut->m_interpolationAngularVelocity);
	m_anisotropicFriction.serialize(dataOut->m_anisotropicFriction);
	dataOut->m_hasAnisotropicFriction = m_hasAnisotropicFriction;
	dataOut->m_contactProcessingThreshold = m_contactProcessingThreshold;
	dataOut->m_broadphaseHandle = 0;
	dataOut->m_collisionShape = serializer->getUniquePointer(m_collisionShape);
	dataOut->m_rootCollisionShape = 0;//@todo
	dataOut->m_collisionFlags = m_collisionFlags;
	dataOut->m_islandTag1 = m_islandTag1;
	dataOut->m_companionId = m_companionId;
	dataOut->m_activationState1 = m_activationState1;
	dataOut->m_activationState1 = m_activationState1;
	dataOut->m_deactivationTime = m_deactivationTime;
	dataOut->m_friction = m_friction;
	dataOut->m_restitution = m_restitution;
	dataOut->m_internalType = m_internalType;
	
	char* name = (char*) serializer->findNameForPointer(this);
	dataOut->m_name = (char*)serializer->getUniquePointer(name);
	if (dataOut->m_name)
	{
		serializer->serializeName(name);
	}
	dataOut->m_hitFraction = m_hitFraction;
	dataOut->m_ccdSweptSphereRadius = m_ccdSweptSphereRadius;
	dataOut->m_ccdMotionThreshold = m_ccdMotionThreshold;
	dataOut->m_ccdMotionThreshold = m_ccdMotionThreshold;
	dataOut->m_checkCollideWith = m_checkCollideWith;

	return btCollisionObjectDataName;
}


void btCollisionObject::serializeSingleObject(class btSerializer* serializer) const
{
	int len = calculateSerializeBufferSize();
	btChunk* chunk = serializer->allocate(len,1);
	const char* structType = serialize(chunk->m_oldPtr, serializer);
	serializer->finalizeChunk(chunk,structType,BT_COLLISIONOBJECT_CODE,(void*)this);
}
