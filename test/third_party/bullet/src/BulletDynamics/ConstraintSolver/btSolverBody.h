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

#ifndef BT_SOLVER_BODY_H
#define BT_SOLVER_BODY_H

class	btRigidBody;
#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "LinearMath/btAlignedAllocator.h"
#include "LinearMath/btTransformUtil.h"

///Until we get other contributions, only use SIMD on Windows, when using Visual Studio 2008 or later, and not double precision
#ifdef BT_USE_SSE
#define USE_SIMD 1
#endif //


#ifdef USE_SIMD

struct	btSimdScalar
{
	SIMD_FORCE_INLINE	btSimdScalar()
	{

	}

	SIMD_FORCE_INLINE	btSimdScalar(float	fl)
	:m_vec128 (_mm_set1_ps(fl))
	{
	}

	SIMD_FORCE_INLINE	btSimdScalar(__m128 v128)
		:m_vec128(v128)
	{
	}
	union
	{
		__m128		m_vec128;
		float		m_floats[4];
		int			m_ints[4];
		btScalar	m_unusedPadding;
	};
	SIMD_FORCE_INLINE	__m128	get128()
	{
		return m_vec128;
	}

	SIMD_FORCE_INLINE	const __m128	get128() const
	{
		return m_vec128;
	}

	SIMD_FORCE_INLINE	void	set128(__m128 v128)
	{
		m_vec128 = v128;
	}

	SIMD_FORCE_INLINE	operator       __m128()       
	{ 
		return m_vec128; 
	}
	SIMD_FORCE_INLINE	operator const __m128() const 
	{ 
		return m_vec128; 
	}
	
	SIMD_FORCE_INLINE	operator float() const 
	{ 
		return m_floats[0]; 
	}

};

///@brief Return the elementwise product of two btSimdScalar
SIMD_FORCE_INLINE btSimdScalar 
operator*(const btSimdScalar& v1, const btSimdScalar& v2) 
{
	return btSimdScalar(_mm_mul_ps(v1.get128(),v2.get128()));
}

///@brief Return the elementwise product of two btSimdScalar
SIMD_FORCE_INLINE btSimdScalar 
operator+(const btSimdScalar& v1, const btSimdScalar& v2) 
{
	return btSimdScalar(_mm_add_ps(v1.get128(),v2.get128()));
}


#else
#define btSimdScalar btScalar
#endif

///The btSolverBody is an internal datastructure for the constraint solver. Only necessary data is packed to increase cache coherence/performance.
ATTRIBUTE_ALIGNED64 (struct)	btSolverBodyObsolete
{
	BT_DECLARE_ALIGNED_ALLOCATOR();
	btVector3		m_deltaLinearVelocity;
	btVector3		m_deltaAngularVelocity;
	btVector3		m_angularFactor;
	btVector3		m_invMass;
	btRigidBody*	m_originalBody;
	btVector3		m_pushVelocity;
	btVector3		m_turnVelocity;

	
	SIMD_FORCE_INLINE void	getVelocityInLocalPointObsolete(const btVector3& rel_pos, btVector3& velocity ) const
	{
		if (m_originalBody)
			velocity = m_originalBody->getLinearVelocity()+m_deltaLinearVelocity + (m_originalBody->getAngularVelocity()+m_deltaAngularVelocity).cross(rel_pos);
		else
			velocity.setValue(0,0,0);
	}

	SIMD_FORCE_INLINE void	getAngularVelocity(btVector3& angVel) const
	{
		if (m_originalBody)
			angVel = m_originalBody->getAngularVelocity()+m_deltaAngularVelocity;
		else
			angVel.setValue(0,0,0);
	}


	//Optimization for the iterative solver: avoid calculating constant terms involving inertia, normal, relative position
	SIMD_FORCE_INLINE void applyImpulse(const btVector3& linearComponent, const btVector3& angularComponent,const btScalar impulseMagnitude)
	{
		//if (m_invMass)
		{
			m_deltaLinearVelocity += linearComponent*impulseMagnitude;
			m_deltaAngularVelocity += angularComponent*(impulseMagnitude*m_angularFactor);
		}
	}

	SIMD_FORCE_INLINE void internalApplyPushImpulse(const btVector3& linearComponent, const btVector3& angularComponent,btScalar impulseMagnitude)
	{
		if (m_originalBody)
		{
			m_pushVelocity += linearComponent*impulseMagnitude;
			m_turnVelocity += angularComponent*(impulseMagnitude*m_angularFactor);
		}
	}
	
	void	writebackVelocity()
	{
		if (m_originalBody)
		{
			m_originalBody->setLinearVelocity(m_originalBody->getLinearVelocity()+ m_deltaLinearVelocity);
			m_originalBody->setAngularVelocity(m_originalBody->getAngularVelocity()+m_deltaAngularVelocity);
			
			//m_originalBody->setCompanionId(-1);
		}
	}


	void	writebackVelocity(btScalar timeStep)
	{
        (void) timeStep;
		if (m_originalBody)
		{
			m_originalBody->setLinearVelocity(m_originalBody->getLinearVelocity()+ m_deltaLinearVelocity);
			m_originalBody->setAngularVelocity(m_originalBody->getAngularVelocity()+m_deltaAngularVelocity);
			
			//correct the position/orientation based on push/turn recovery
			btTransform newTransform;
			btTransformUtil::integrateTransform(m_originalBody->getWorldTransform(),m_pushVelocity,m_turnVelocity,timeStep,newTransform);
			m_originalBody->setWorldTransform(newTransform);
			
			//m_originalBody->setCompanionId(-1);
		}
	}
	


};

#endif //BT_SOLVER_BODY_H


