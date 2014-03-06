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

#include "btRigidBody.h"
#include "BulletCollision/CollisionShapes/btConvexShape.h"
#include "LinearMath/btMinMax.h"
#include "LinearMath/btTransformUtil.h"
#include "LinearMath/btMotionState.h"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "LinearMath/btSerializer.h"

//'temporarily' global variables
btScalar	gDeactivationTime = btScalar(2.);
bool	gDisableDeactivation = false;
static int uniqueId = 0;


btRigidBody::btRigidBody(const btRigidBody::btRigidBodyConstructionInfo& constructionInfo)
{
	setupRigidBody(constructionInfo);
}

btRigidBody::btRigidBody(btScalar mass, btMotionState *motionState, btCollisionShape *collisionShape, const btVector3 &localInertia)
{
	btRigidBodyConstructionInfo cinfo(mass,motionState,collisionShape,localInertia);
	setupRigidBody(cinfo);
}

void	btRigidBody::setupRigidBody(const btRigidBody::btRigidBodyConstructionInfo& constructionInfo)
{

	m_internalType=CO_RIGID_BODY;

	m_linearVelocity.setValue(btScalar(0.0), btScalar(0.0), btScalar(0.0));
	m_angularVelocity.setValue(btScalar(0.),btScalar(0.),btScalar(0.));
	m_angularFactor.setValue(1,1,1);
	m_linearFactor.setValue(1,1,1);
	m_gravity.setValue(btScalar(0.0), btScalar(0.0), btScalar(0.0));
	m_gravity_acceleration.setValue(btScalar(0.0), btScalar(0.0), btScalar(0.0));
	m_totalForce.setValue(btScalar(0.0), btScalar(0.0), btScalar(0.0));
	m_totalTorque.setValue(btScalar(0.0), btScalar(0.0), btScalar(0.0)),
    setDamping(constructionInfo.m_linearDamping, constructionInfo.m_angularDamping);

	m_linearSleepingThreshold = constructionInfo.m_linearSleepingThreshold;
	m_angularSleepingThreshold = constructionInfo.m_angularSleepingThreshold;
	m_optionalMotionState = constructionInfo.m_motionState;
	m_contactSolverType = 0;
	m_frictionSolverType = 0;
	m_additionalDamping = constructionInfo.m_additionalDamping;
	m_additionalDampingFactor = constructionInfo.m_additionalDampingFactor;
	m_additionalLinearDampingThresholdSqr = constructionInfo.m_additionalLinearDampingThresholdSqr;
	m_additionalAngularDampingThresholdSqr = constructionInfo.m_additionalAngularDampingThresholdSqr;
	m_additionalAngularDampingFactor = constructionInfo.m_additionalAngularDampingFactor;

	if (m_optionalMotionState)
	{
		m_optionalMotionState->getWorldTransform(m_worldTransform);
	} else
	{
		m_worldTransform = constructionInfo.m_startWorldTransform;
	}

	m_interpolationWorldTransform = m_worldTransform;
	m_interpolationLinearVelocity.setValue(0,0,0);
	m_interpolationAngularVelocity.setValue(0,0,0);
	
	//moved to btCollisionObject
	m_friction = constructionInfo.m_friction;
	m_restitution = constructionInfo.m_restitution;

	setCollisionShape( constructionInfo.m_collisionShape );
	m_debugBodyId = uniqueId++;
	
	setMassProps(constructionInfo.m_mass, constructionInfo.m_localInertia);
	updateInertiaTensor();

	m_rigidbodyFlags = 0;


	m_deltaLinearVelocity.setZero();
	m_deltaAngularVelocity.setZero();
	m_invMass = m_inverseMass*m_linearFactor;
	m_pushVelocity.setZero();
	m_turnVelocity.setZero();

	

}


void btRigidBody::predictIntegratedTransform(btScalar timeStep,btTransform& predictedTransform) 
{
	btTransformUtil::integrateTransform(m_worldTransform,m_linearVelocity,m_angularVelocity,timeStep,predictedTransform);
}

void			btRigidBody::saveKinematicState(btScalar timeStep)
{
	//todo: clamp to some (user definable) safe minimum timestep, to limit maximum angular/linear velocities
	if (timeStep != btScalar(0.))
	{
		//if we use motionstate to synchronize world transforms, get the new kinematic/animated world transform
		if (getMotionState())
			getMotionState()->getWorldTransform(m_worldTransform);
		btVector3 linVel,angVel;
		
		btTransformUtil::calculateVelocity(m_interpolationWorldTransform,m_worldTransform,timeStep,m_linearVelocity,m_angularVelocity);
		m_interpolationLinearVelocity = m_linearVelocity;
		m_interpolationAngularVelocity = m_angularVelocity;
		m_interpolationWorldTransform = m_worldTransform;
		//printf("angular = %f %f %f\n",m_angularVelocity.getX(),m_angularVelocity.getY(),m_angularVelocity.getZ());
	}
}
	
void	btRigidBody::getAabb(btVector3& aabbMin,btVector3& aabbMax) const
{
	getCollisionShape()->getAabb(m_worldTransform,aabbMin,aabbMax);
}




void btRigidBody::setGravity(const btVector3& acceleration) 
{
	if (m_inverseMass != btScalar(0.0))
	{
		m_gravity = acceleration * (btScalar(1.0) / m_inverseMass);
	}
	m_gravity_acceleration = acceleration;
}






void btRigidBody::setDamping(btScalar lin_damping, btScalar ang_damping)
{
	m_linearDamping = btClamped(lin_damping, (btScalar)btScalar(0.0), (btScalar)btScalar(1.0));
	m_angularDamping = btClamped(ang_damping, (btScalar)btScalar(0.0), (btScalar)btScalar(1.0));
}




///applyDamping damps the velocity, using the given m_linearDamping and m_angularDamping
void			btRigidBody::applyDamping(btScalar timeStep)
{
	//On new damping: see discussion/issue report here: http://code.google.com/p/bullet/issues/detail?id=74
	//todo: do some performance comparisons (but other parts of the engine are probably bottleneck anyway

//#define USE_OLD_DAMPING_METHOD 1
#ifdef USE_OLD_DAMPING_METHOD
	m_linearVelocity *= GEN_clamped((btScalar(1.) - timeStep * m_linearDamping), (btScalar)btScalar(0.0), (btScalar)btScalar(1.0));
	m_angularVelocity *= GEN_clamped((btScalar(1.) - timeStep * m_angularDamping), (btScalar)btScalar(0.0), (btScalar)btScalar(1.0));
#else
	m_linearVelocity *= btPow(btScalar(1)-m_linearDamping, timeStep);
	m_angularVelocity *= btPow(btScalar(1)-m_angularDamping, timeStep);
#endif

	if (m_additionalDamping)
	{
		//Additional damping can help avoiding lowpass jitter motion, help stability for ragdolls etc.
		//Such damping is undesirable, so once the overall simulation quality of the rigid body dynamics system has improved, this should become obsolete
		if ((m_angularVelocity.length2() < m_additionalAngularDampingThresholdSqr) &&
			(m_linearVelocity.length2() < m_additionalLinearDampingThresholdSqr))
		{
			m_angularVelocity *= m_additionalDampingFactor;
			m_linearVelocity *= m_additionalDampingFactor;
		}
	

		btScalar speed = m_linearVelocity.length();
		if (speed < m_linearDamping)
		{
			btScalar dampVel = btScalar(0.005);
			if (speed > dampVel)
			{
				btVector3 dir = m_linearVelocity.normalized();
				m_linearVelocity -=  dir * dampVel;
			} else
			{
				m_linearVelocity.setValue(btScalar(0.),btScalar(0.),btScalar(0.));
			}
		}

		btScalar angSpeed = m_angularVelocity.length();
		if (angSpeed < m_angularDamping)
		{
			btScalar angDampVel = btScalar(0.005);
			if (angSpeed > angDampVel)
			{
				btVector3 dir = m_angularVelocity.normalized();
				m_angularVelocity -=  dir * angDampVel;
			} else
			{
				m_angularVelocity.setValue(btScalar(0.),btScalar(0.),btScalar(0.));
			}
		}
	}
}


void btRigidBody::applyGravity()
{
	if (isStaticOrKinematicObject())
		return;
	
	applyCentralForce(m_gravity);	

}

void btRigidBody::proceedToTransform(const btTransform& newTrans)
{
	setCenterOfMassTransform( newTrans );
}
	

void btRigidBody::setMassProps(btScalar mass, const btVector3& inertia)
{
	if (mass == btScalar(0.))
	{
		m_collisionFlags |= btCollisionObject::CF_STATIC_OBJECT;
		m_inverseMass = btScalar(0.);
	} else
	{
		m_collisionFlags &= (~btCollisionObject::CF_STATIC_OBJECT);
		m_inverseMass = btScalar(1.0) / mass;
	}

	//Fg = m * a
	m_gravity = mass * m_gravity_acceleration;
	
	m_invInertiaLocal.setValue(inertia.x() != btScalar(0.0) ? btScalar(1.0) / inertia.x(): btScalar(0.0),
				   inertia.y() != btScalar(0.0) ? btScalar(1.0) / inertia.y(): btScalar(0.0),
				   inertia.z() != btScalar(0.0) ? btScalar(1.0) / inertia.z(): btScalar(0.0));

	m_invMass = m_linearFactor*m_inverseMass;
}

	

void btRigidBody::updateInertiaTensor() 
{
	m_invInertiaTensorWorld = m_worldTransform.getBasis().scaled(m_invInertiaLocal) * m_worldTransform.getBasis().transpose();
}


void btRigidBody::integrateVelocities(btScalar step) 
{
	if (isStaticOrKinematicObject())
		return;

	m_linearVelocity += m_totalForce * (m_inverseMass * step);
	m_angularVelocity += m_invInertiaTensorWorld * m_totalTorque * step;

#define MAX_ANGVEL SIMD_HALF_PI
	/// clamp angular velocity. collision calculations will fail on higher angular velocities	
	btScalar angvel = m_angularVelocity.length();
	if (angvel*step > MAX_ANGVEL)
	{
		m_angularVelocity *= (MAX_ANGVEL/step) /angvel;
	}

}

btQuaternion btRigidBody::getOrientation() const
{
		btQuaternion orn;
		m_worldTransform.getBasis().getRotation(orn);
		return orn;
}
	
	
void btRigidBody::setCenterOfMassTransform(const btTransform& xform)
{

	if (isStaticOrKinematicObject())
	{
		m_interpolationWorldTransform = m_worldTransform;
	} else
	{
		m_interpolationWorldTransform = xform;
	}
	m_interpolationLinearVelocity = getLinearVelocity();
	m_interpolationAngularVelocity = getAngularVelocity();
	m_worldTransform = xform;
	updateInertiaTensor();
}


bool btRigidBody::checkCollideWithOverride(btCollisionObject* co)
{
	btRigidBody* otherRb = btRigidBody::upcast(co);
	if (!otherRb)
		return true;

	for (int i = 0; i < m_constraintRefs.size(); ++i)
	{
		btTypedConstraint* c = m_constraintRefs[i];
		if (&c->getRigidBodyA() == otherRb || &c->getRigidBodyB() == otherRb)
			return false;
	}

	return true;
}

void	btRigidBody::internalWritebackVelocity(btScalar timeStep)
{
    (void) timeStep;
	if (m_inverseMass)
	{
		setLinearVelocity(getLinearVelocity()+ m_deltaLinearVelocity);
		setAngularVelocity(getAngularVelocity()+m_deltaAngularVelocity);
		
		//correct the position/orientation based on push/turn recovery
		btTransform newTransform;
		btTransformUtil::integrateTransform(getWorldTransform(),m_pushVelocity,m_turnVelocity,timeStep,newTransform);
		setWorldTransform(newTransform);
		//m_originalBody->setCompanionId(-1);
	}
//	m_deltaLinearVelocity.setZero();
//	m_deltaAngularVelocity .setZero();
//	m_pushVelocity.setZero();
//	m_turnVelocity.setZero();
}



void btRigidBody::addConstraintRef(btTypedConstraint* c)
{
	int index = m_constraintRefs.findLinearSearch(c);
	if (index == m_constraintRefs.size())
		m_constraintRefs.push_back(c); 

	m_checkCollideWith = true;
}

void btRigidBody::removeConstraintRef(btTypedConstraint* c)
{
	m_constraintRefs.remove(c);
	m_checkCollideWith = m_constraintRefs.size() > 0;
}

int	btRigidBody::calculateSerializeBufferSize()	const
{
	int sz = sizeof(btRigidBodyData);
	return sz;
}

	///fills the dataBuffer and returns the struct name (and 0 on failure)
const char*	btRigidBody::serialize(void* dataBuffer, class btSerializer* serializer) const
{
	btRigidBodyData* rbd = (btRigidBodyData*) dataBuffer;

	btCollisionObject::serialize(&rbd->m_collisionObjectData, serializer);

	m_invInertiaTensorWorld.serialize(rbd->m_invInertiaTensorWorld);
	m_linearVelocity.serialize(rbd->m_linearVelocity);
	m_angularVelocity.serialize(rbd->m_angularVelocity);
	rbd->m_inverseMass = m_inverseMass;
	m_angularFactor.serialize(rbd->m_angularFactor);
	m_linearFactor.serialize(rbd->m_linearFactor);
	m_gravity.serialize(rbd->m_gravity);
	m_gravity_acceleration.serialize(rbd->m_gravity_acceleration);
	m_invInertiaLocal.serialize(rbd->m_invInertiaLocal);
	m_totalForce.serialize(rbd->m_totalForce);
	m_totalTorque.serialize(rbd->m_totalTorque);
	rbd->m_linearDamping = m_linearDamping;
	rbd->m_angularDamping = m_angularDamping;
	rbd->m_additionalDamping = m_additionalDamping;
	rbd->m_additionalDampingFactor = m_additionalDampingFactor;
	rbd->m_additionalLinearDampingThresholdSqr = m_additionalLinearDampingThresholdSqr;
	rbd->m_additionalAngularDampingThresholdSqr = m_additionalAngularDampingThresholdSqr;
	rbd->m_additionalAngularDampingFactor = m_additionalAngularDampingFactor;
	rbd->m_linearSleepingThreshold=m_linearSleepingThreshold;
	rbd->m_angularSleepingThreshold = m_angularSleepingThreshold;

	return btRigidBodyDataName;
}



void btRigidBody::serializeSingleObject(class btSerializer* serializer) const
{
	btChunk* chunk = serializer->allocate(calculateSerializeBufferSize(),1);
	const char* structType = serialize(chunk->m_oldPtr, serializer);
	serializer->finalizeChunk(chunk,structType,BT_RIGIDBODY_CODE,(void*)this);
}


