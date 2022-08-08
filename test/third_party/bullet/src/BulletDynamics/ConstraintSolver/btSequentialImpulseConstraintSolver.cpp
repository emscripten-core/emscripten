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

//#define COMPUTE_IMPULSE_DENOM 1
//It is not necessary (redundant) to refresh contact manifolds, this refresh has been moved to the collision algorithms.

#include "btSequentialImpulseConstraintSolver.h"
#include "BulletCollision/NarrowPhaseCollision/btPersistentManifold.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "btContactConstraint.h"
#include "btSolve2LinearConstraint.h"
#include "btContactSolverInfo.h"
#include "LinearMath/btIDebugDraw.h"
#include "btJacobianEntry.h"
#include "LinearMath/btMinMax.h"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include <new>
#include "LinearMath/btStackAlloc.h"
#include "LinearMath/btQuickprof.h"
#include "btSolverBody.h"
#include "btSolverConstraint.h"
#include "LinearMath/btAlignedObjectArray.h"
#include <string.h> //for memset

int		gNumSplitImpulseRecoveries = 0;

btSequentialImpulseConstraintSolver::btSequentialImpulseConstraintSolver()
:m_btSeed2(0)
{

}

btSequentialImpulseConstraintSolver::~btSequentialImpulseConstraintSolver()
{
}

#ifdef USE_SIMD
#include <emmintrin.h>
#define btVecSplat(x, e) _mm_shuffle_ps(x, x, _MM_SHUFFLE(e,e,e,e))
static inline __m128 btSimdDot3( __m128 vec0, __m128 vec1 )
{
	__m128 result = _mm_mul_ps( vec0, vec1);
	return _mm_add_ps( btVecSplat( result, 0 ), _mm_add_ps( btVecSplat( result, 1 ), btVecSplat( result, 2 ) ) );
}
#endif//USE_SIMD

// Project Gauss Seidel or the equivalent Sequential Impulse
void btSequentialImpulseConstraintSolver::resolveSingleConstraintRowGenericSIMD(btRigidBody& body1,btRigidBody& body2,const btSolverConstraint& c)
{
#ifdef USE_SIMD
	__m128 cpAppliedImp = _mm_set1_ps(c.m_appliedImpulse);
	__m128	lowerLimit1 = _mm_set1_ps(c.m_lowerLimit);
	__m128	upperLimit1 = _mm_set1_ps(c.m_upperLimit);
	__m128 deltaImpulse = _mm_sub_ps(_mm_set1_ps(c.m_rhs), _mm_mul_ps(_mm_set1_ps(c.m_appliedImpulse),_mm_set1_ps(c.m_cfm)));
	__m128 deltaVel1Dotn	=	_mm_add_ps(btSimdDot3(c.m_contactNormal.mVec128,body1.internalGetDeltaLinearVelocity().mVec128), btSimdDot3(c.m_relpos1CrossNormal.mVec128,body1.internalGetDeltaAngularVelocity().mVec128));
	__m128 deltaVel2Dotn	=	_mm_sub_ps(btSimdDot3(c.m_relpos2CrossNormal.mVec128,body2.internalGetDeltaAngularVelocity().mVec128),btSimdDot3((c.m_contactNormal).mVec128,body2.internalGetDeltaLinearVelocity().mVec128));
	deltaImpulse	=	_mm_sub_ps(deltaImpulse,_mm_mul_ps(deltaVel1Dotn,_mm_set1_ps(c.m_jacDiagABInv)));
	deltaImpulse	=	_mm_sub_ps(deltaImpulse,_mm_mul_ps(deltaVel2Dotn,_mm_set1_ps(c.m_jacDiagABInv)));
	btSimdScalar sum = _mm_add_ps(cpAppliedImp,deltaImpulse);
	btSimdScalar resultLowerLess,resultUpperLess;
	resultLowerLess = _mm_cmplt_ps(sum,lowerLimit1);
	resultUpperLess = _mm_cmplt_ps(sum,upperLimit1);
	__m128 lowMinApplied = _mm_sub_ps(lowerLimit1,cpAppliedImp);
	deltaImpulse = _mm_or_ps( _mm_and_ps(resultLowerLess, lowMinApplied), _mm_andnot_ps(resultLowerLess, deltaImpulse) );
	c.m_appliedImpulse = _mm_or_ps( _mm_and_ps(resultLowerLess, lowerLimit1), _mm_andnot_ps(resultLowerLess, sum) );
	__m128 upperMinApplied = _mm_sub_ps(upperLimit1,cpAppliedImp);
	deltaImpulse = _mm_or_ps( _mm_and_ps(resultUpperLess, deltaImpulse), _mm_andnot_ps(resultUpperLess, upperMinApplied) );
	c.m_appliedImpulse = _mm_or_ps( _mm_and_ps(resultUpperLess, c.m_appliedImpulse), _mm_andnot_ps(resultUpperLess, upperLimit1) );
	__m128	linearComponentA = _mm_mul_ps(c.m_contactNormal.mVec128,body1.internalGetInvMass().mVec128);
	__m128	linearComponentB = _mm_mul_ps((c.m_contactNormal).mVec128,body2.internalGetInvMass().mVec128);
	__m128 impulseMagnitude = deltaImpulse;
	body1.internalGetDeltaLinearVelocity().mVec128 = _mm_add_ps(body1.internalGetDeltaLinearVelocity().mVec128,_mm_mul_ps(linearComponentA,impulseMagnitude));
	body1.internalGetDeltaAngularVelocity().mVec128 = _mm_add_ps(body1.internalGetDeltaAngularVelocity().mVec128 ,_mm_mul_ps(c.m_angularComponentA.mVec128,impulseMagnitude));
	body2.internalGetDeltaLinearVelocity().mVec128 = _mm_sub_ps(body2.internalGetDeltaLinearVelocity().mVec128,_mm_mul_ps(linearComponentB,impulseMagnitude));
	body2.internalGetDeltaAngularVelocity().mVec128 = _mm_add_ps(body2.internalGetDeltaAngularVelocity().mVec128 ,_mm_mul_ps(c.m_angularComponentB.mVec128,impulseMagnitude));
#else
	resolveSingleConstraintRowGeneric(body1,body2,c);
#endif
}

// Project Gauss Seidel or the equivalent Sequential Impulse
 void btSequentialImpulseConstraintSolver::resolveSingleConstraintRowGeneric(btRigidBody& body1,btRigidBody& body2,const btSolverConstraint& c)
{
	btScalar deltaImpulse = c.m_rhs-btScalar(c.m_appliedImpulse)*c.m_cfm;
	const btScalar deltaVel1Dotn	=	c.m_contactNormal.dot(body1.internalGetDeltaLinearVelocity()) 	+ c.m_relpos1CrossNormal.dot(body1.internalGetDeltaAngularVelocity());
	const btScalar deltaVel2Dotn	=	-c.m_contactNormal.dot(body2.internalGetDeltaLinearVelocity()) + c.m_relpos2CrossNormal.dot(body2.internalGetDeltaAngularVelocity());

//	const btScalar delta_rel_vel	=	deltaVel1Dotn-deltaVel2Dotn;
	deltaImpulse	-=	deltaVel1Dotn*c.m_jacDiagABInv;
	deltaImpulse	-=	deltaVel2Dotn*c.m_jacDiagABInv;

	const btScalar sum = btScalar(c.m_appliedImpulse) + deltaImpulse;
	if (sum < c.m_lowerLimit)
	{
		deltaImpulse = c.m_lowerLimit-c.m_appliedImpulse;
		c.m_appliedImpulse = c.m_lowerLimit;
	}
	else if (sum > c.m_upperLimit) 
	{
		deltaImpulse = c.m_upperLimit-c.m_appliedImpulse;
		c.m_appliedImpulse = c.m_upperLimit;
	}
	else
	{
		c.m_appliedImpulse = sum;
	}
		body1.internalApplyImpulse(c.m_contactNormal*body1.internalGetInvMass(),c.m_angularComponentA,deltaImpulse);
		body2.internalApplyImpulse(-c.m_contactNormal*body2.internalGetInvMass(),c.m_angularComponentB,deltaImpulse);
}

 void btSequentialImpulseConstraintSolver::resolveSingleConstraintRowLowerLimitSIMD(btRigidBody& body1,btRigidBody& body2,const btSolverConstraint& c)
{
#ifdef USE_SIMD
	__m128 cpAppliedImp = _mm_set1_ps(c.m_appliedImpulse);
	__m128	lowerLimit1 = _mm_set1_ps(c.m_lowerLimit);
	__m128	upperLimit1 = _mm_set1_ps(c.m_upperLimit);
	__m128 deltaImpulse = _mm_sub_ps(_mm_set1_ps(c.m_rhs), _mm_mul_ps(_mm_set1_ps(c.m_appliedImpulse),_mm_set1_ps(c.m_cfm)));
	__m128 deltaVel1Dotn	=	_mm_add_ps(btSimdDot3(c.m_contactNormal.mVec128,body1.internalGetDeltaLinearVelocity().mVec128), btSimdDot3(c.m_relpos1CrossNormal.mVec128,body1.internalGetDeltaAngularVelocity().mVec128));
	__m128 deltaVel2Dotn	=	_mm_sub_ps(btSimdDot3(c.m_relpos2CrossNormal.mVec128,body2.internalGetDeltaAngularVelocity().mVec128),btSimdDot3((c.m_contactNormal).mVec128,body2.internalGetDeltaLinearVelocity().mVec128));
	deltaImpulse	=	_mm_sub_ps(deltaImpulse,_mm_mul_ps(deltaVel1Dotn,_mm_set1_ps(c.m_jacDiagABInv)));
	deltaImpulse	=	_mm_sub_ps(deltaImpulse,_mm_mul_ps(deltaVel2Dotn,_mm_set1_ps(c.m_jacDiagABInv)));
	btSimdScalar sum = _mm_add_ps(cpAppliedImp,deltaImpulse);
	btSimdScalar resultLowerLess,resultUpperLess;
	resultLowerLess = _mm_cmplt_ps(sum,lowerLimit1);
	resultUpperLess = _mm_cmplt_ps(sum,upperLimit1);
	__m128 lowMinApplied = _mm_sub_ps(lowerLimit1,cpAppliedImp);
	deltaImpulse = _mm_or_ps( _mm_and_ps(resultLowerLess, lowMinApplied), _mm_andnot_ps(resultLowerLess, deltaImpulse) );
	c.m_appliedImpulse = _mm_or_ps( _mm_and_ps(resultLowerLess, lowerLimit1), _mm_andnot_ps(resultLowerLess, sum) );
	__m128	linearComponentA = _mm_mul_ps(c.m_contactNormal.mVec128,body1.internalGetInvMass().mVec128);
	__m128	linearComponentB = _mm_mul_ps((c.m_contactNormal).mVec128,body2.internalGetInvMass().mVec128);
	__m128 impulseMagnitude = deltaImpulse;
	body1.internalGetDeltaLinearVelocity().mVec128 = _mm_add_ps(body1.internalGetDeltaLinearVelocity().mVec128,_mm_mul_ps(linearComponentA,impulseMagnitude));
	body1.internalGetDeltaAngularVelocity().mVec128 = _mm_add_ps(body1.internalGetDeltaAngularVelocity().mVec128 ,_mm_mul_ps(c.m_angularComponentA.mVec128,impulseMagnitude));
	body2.internalGetDeltaLinearVelocity().mVec128 = _mm_sub_ps(body2.internalGetDeltaLinearVelocity().mVec128,_mm_mul_ps(linearComponentB,impulseMagnitude));
	body2.internalGetDeltaAngularVelocity().mVec128 = _mm_add_ps(body2.internalGetDeltaAngularVelocity().mVec128 ,_mm_mul_ps(c.m_angularComponentB.mVec128,impulseMagnitude));
#else
	resolveSingleConstraintRowLowerLimit(body1,body2,c);
#endif
}

// Project Gauss Seidel or the equivalent Sequential Impulse
 void btSequentialImpulseConstraintSolver::resolveSingleConstraintRowLowerLimit(btRigidBody& body1,btRigidBody& body2,const btSolverConstraint& c)
{
	btScalar deltaImpulse = c.m_rhs-btScalar(c.m_appliedImpulse)*c.m_cfm;
	const btScalar deltaVel1Dotn	=	c.m_contactNormal.dot(body1.internalGetDeltaLinearVelocity()) 	+ c.m_relpos1CrossNormal.dot(body1.internalGetDeltaAngularVelocity());
	const btScalar deltaVel2Dotn	=	-c.m_contactNormal.dot(body2.internalGetDeltaLinearVelocity()) + c.m_relpos2CrossNormal.dot(body2.internalGetDeltaAngularVelocity());

	deltaImpulse	-=	deltaVel1Dotn*c.m_jacDiagABInv;
	deltaImpulse	-=	deltaVel2Dotn*c.m_jacDiagABInv;
	const btScalar sum = btScalar(c.m_appliedImpulse) + deltaImpulse;
	if (sum < c.m_lowerLimit)
	{
		deltaImpulse = c.m_lowerLimit-c.m_appliedImpulse;
		c.m_appliedImpulse = c.m_lowerLimit;
	}
	else
	{
		c.m_appliedImpulse = sum;
	}
	body1.internalApplyImpulse(c.m_contactNormal*body1.internalGetInvMass(),c.m_angularComponentA,deltaImpulse);
	body2.internalApplyImpulse(-c.m_contactNormal*body2.internalGetInvMass(),c.m_angularComponentB,deltaImpulse);
}


void	btSequentialImpulseConstraintSolver::resolveSplitPenetrationImpulseCacheFriendly(
        btRigidBody& body1,
        btRigidBody& body2,
        const btSolverConstraint& c)
{
		if (c.m_rhsPenetration)
        {
			gNumSplitImpulseRecoveries++;
			btScalar deltaImpulse = c.m_rhsPenetration-btScalar(c.m_appliedPushImpulse)*c.m_cfm;
			const btScalar deltaVel1Dotn	=	c.m_contactNormal.dot(body1.internalGetPushVelocity()) 	+ c.m_relpos1CrossNormal.dot(body1.internalGetTurnVelocity());
			const btScalar deltaVel2Dotn	=	-c.m_contactNormal.dot(body2.internalGetPushVelocity()) + c.m_relpos2CrossNormal.dot(body2.internalGetTurnVelocity());

			deltaImpulse	-=	deltaVel1Dotn*c.m_jacDiagABInv;
			deltaImpulse	-=	deltaVel2Dotn*c.m_jacDiagABInv;
			const btScalar sum = btScalar(c.m_appliedPushImpulse) + deltaImpulse;
			if (sum < c.m_lowerLimit)
			{
				deltaImpulse = c.m_lowerLimit-c.m_appliedPushImpulse;
				c.m_appliedPushImpulse = c.m_lowerLimit;
			}
			else
			{
				c.m_appliedPushImpulse = sum;
			}
			body1.internalApplyPushImpulse(c.m_contactNormal*body1.internalGetInvMass(),c.m_angularComponentA,deltaImpulse);
			body2.internalApplyPushImpulse(-c.m_contactNormal*body2.internalGetInvMass(),c.m_angularComponentB,deltaImpulse);
        }
}

 void btSequentialImpulseConstraintSolver::resolveSplitPenetrationSIMD(btRigidBody& body1,btRigidBody& body2,const btSolverConstraint& c)
{
#ifdef USE_SIMD
	if (!c.m_rhsPenetration)
		return;

	gNumSplitImpulseRecoveries++;

	__m128 cpAppliedImp = _mm_set1_ps(c.m_appliedPushImpulse);
	__m128	lowerLimit1 = _mm_set1_ps(c.m_lowerLimit);
	__m128	upperLimit1 = _mm_set1_ps(c.m_upperLimit);
	__m128 deltaImpulse = _mm_sub_ps(_mm_set1_ps(c.m_rhsPenetration), _mm_mul_ps(_mm_set1_ps(c.m_appliedPushImpulse),_mm_set1_ps(c.m_cfm)));
	__m128 deltaVel1Dotn	=	_mm_add_ps(btSimdDot3(c.m_contactNormal.mVec128,body1.internalGetPushVelocity().mVec128), btSimdDot3(c.m_relpos1CrossNormal.mVec128,body1.internalGetTurnVelocity().mVec128));
	__m128 deltaVel2Dotn	=	_mm_sub_ps(btSimdDot3(c.m_relpos2CrossNormal.mVec128,body2.internalGetTurnVelocity().mVec128),btSimdDot3((c.m_contactNormal).mVec128,body2.internalGetPushVelocity().mVec128));
	deltaImpulse	=	_mm_sub_ps(deltaImpulse,_mm_mul_ps(deltaVel1Dotn,_mm_set1_ps(c.m_jacDiagABInv)));
	deltaImpulse	=	_mm_sub_ps(deltaImpulse,_mm_mul_ps(deltaVel2Dotn,_mm_set1_ps(c.m_jacDiagABInv)));
	btSimdScalar sum = _mm_add_ps(cpAppliedImp,deltaImpulse);
	btSimdScalar resultLowerLess,resultUpperLess;
	resultLowerLess = _mm_cmplt_ps(sum,lowerLimit1);
	resultUpperLess = _mm_cmplt_ps(sum,upperLimit1);
	__m128 lowMinApplied = _mm_sub_ps(lowerLimit1,cpAppliedImp);
	deltaImpulse = _mm_or_ps( _mm_and_ps(resultLowerLess, lowMinApplied), _mm_andnot_ps(resultLowerLess, deltaImpulse) );
	c.m_appliedImpulse = _mm_or_ps( _mm_and_ps(resultLowerLess, lowerLimit1), _mm_andnot_ps(resultLowerLess, sum) );
	__m128	linearComponentA = _mm_mul_ps(c.m_contactNormal.mVec128,body1.internalGetInvMass().mVec128);
	__m128	linearComponentB = _mm_mul_ps((c.m_contactNormal).mVec128,body2.internalGetInvMass().mVec128);
	__m128 impulseMagnitude = deltaImpulse;
	body1.internalGetPushVelocity().mVec128 = _mm_add_ps(body1.internalGetPushVelocity().mVec128,_mm_mul_ps(linearComponentA,impulseMagnitude));
	body1.internalGetTurnVelocity().mVec128 = _mm_add_ps(body1.internalGetTurnVelocity().mVec128 ,_mm_mul_ps(c.m_angularComponentA.mVec128,impulseMagnitude));
	body2.internalGetPushVelocity().mVec128 = _mm_sub_ps(body2.internalGetPushVelocity().mVec128,_mm_mul_ps(linearComponentB,impulseMagnitude));
	body2.internalGetTurnVelocity().mVec128 = _mm_add_ps(body2.internalGetTurnVelocity().mVec128 ,_mm_mul_ps(c.m_angularComponentB.mVec128,impulseMagnitude));
#else
	resolveSplitPenetrationImpulseCacheFriendly(body1,body2,c);
#endif
}



unsigned long btSequentialImpulseConstraintSolver::btRand2()
{
	m_btSeed2 = (1664525L*m_btSeed2 + 1013904223L) & 0xffffffff;
	return m_btSeed2;
}



//See ODE: adam's all-int straightforward(?) dRandInt (0..n-1)
int btSequentialImpulseConstraintSolver::btRandInt2 (int n)
{
	// seems good; xor-fold and modulus
	const unsigned long un = static_cast<unsigned long>(n);
	unsigned long r = btRand2();

	// note: probably more aggressive than it needs to be -- might be
	//       able to get away without one or two of the innermost branches.
	if (un <= 0x00010000UL) {
		r ^= (r >> 16);
		if (un <= 0x00000100UL) {
			r ^= (r >> 8);
			if (un <= 0x00000010UL) {
				r ^= (r >> 4);
				if (un <= 0x00000004UL) {
					r ^= (r >> 2);
					if (un <= 0x00000002UL) {
						r ^= (r >> 1);
					}
				}
			}
		}
	}

	return (int) (r % un);
}


#if 0
void	btSequentialImpulseConstraintSolver::initSolverBody(btSolverBody* solverBody, btCollisionObject* collisionObject)
{
	btRigidBody* rb = collisionObject? btRigidBody::upcast(collisionObject) : 0;

	solverBody->internalGetDeltaLinearVelocity().setValue(0.f,0.f,0.f);
	solverBody->internalGetDeltaAngularVelocity().setValue(0.f,0.f,0.f);
	solverBody->internalGetPushVelocity().setValue(0.f,0.f,0.f);
	solverBody->internalGetTurnVelocity().setValue(0.f,0.f,0.f);

	if (rb)
	{
		solverBody->internalGetInvMass() = btVector3(rb->getInvMass(),rb->getInvMass(),rb->getInvMass())*rb->getLinearFactor();
		solverBody->m_originalBody = rb;
		solverBody->m_angularFactor = rb->getAngularFactor();
	} else
	{
		solverBody->internalGetInvMass().setValue(0,0,0);
		solverBody->m_originalBody = 0;
		solverBody->m_angularFactor.setValue(1,1,1);
	}
}
#endif





btScalar btSequentialImpulseConstraintSolver::restitutionCurve(btScalar rel_vel, btScalar restitution)
{
	btScalar rest = restitution * -rel_vel;
	return rest;
}



void	applyAnisotropicFriction(btCollisionObject* colObj,btVector3& frictionDirection);
void	applyAnisotropicFriction(btCollisionObject* colObj,btVector3& frictionDirection)
{
	if (colObj && colObj->hasAnisotropicFriction())
	{
		// transform to local coordinates
		btVector3 loc_lateral = frictionDirection * colObj->getWorldTransform().getBasis();
		const btVector3& friction_scaling = colObj->getAnisotropicFriction();
		//apply anisotropic friction
		loc_lateral *= friction_scaling;
		// ... and transform it back to global coordinates
		frictionDirection = colObj->getWorldTransform().getBasis() * loc_lateral;
	}
}


void btSequentialImpulseConstraintSolver::setupFrictionConstraint(btSolverConstraint& solverConstraint, const btVector3& normalAxis,btRigidBody* solverBodyA,btRigidBody* solverBodyB,btManifoldPoint& cp,const btVector3& rel_pos1,const btVector3& rel_pos2,btCollisionObject* colObj0,btCollisionObject* colObj1, btScalar relaxation, btScalar desiredVelocity, btScalar cfmSlip)
{


	btRigidBody* body0=btRigidBody::upcast(colObj0);
	btRigidBody* body1=btRigidBody::upcast(colObj1);

	solverConstraint.m_contactNormal = normalAxis;

	solverConstraint.m_solverBodyA = body0 ? body0 : &getFixedBody();
	solverConstraint.m_solverBodyB = body1 ? body1 : &getFixedBody();

	solverConstraint.m_friction = cp.m_combinedFriction;
	solverConstraint.m_originalContactPoint = 0;

	solverConstraint.m_appliedImpulse = 0.f;
	solverConstraint.m_appliedPushImpulse = 0.f;

	{
		btVector3 ftorqueAxis1 = rel_pos1.cross(solverConstraint.m_contactNormal);
		solverConstraint.m_relpos1CrossNormal = ftorqueAxis1;
		solverConstraint.m_angularComponentA = body0 ? body0->getInvInertiaTensorWorld()*ftorqueAxis1*body0->getAngularFactor() : btVector3(0,0,0);
	}
	{
		btVector3 ftorqueAxis1 = rel_pos2.cross(-solverConstraint.m_contactNormal);
		solverConstraint.m_relpos2CrossNormal = ftorqueAxis1;
		solverConstraint.m_angularComponentB = body1 ? body1->getInvInertiaTensorWorld()*ftorqueAxis1*body1->getAngularFactor() : btVector3(0,0,0);
	}

#ifdef COMPUTE_IMPULSE_DENOM
	btScalar denom0 = rb0->computeImpulseDenominator(pos1,solverConstraint.m_contactNormal);
	btScalar denom1 = rb1->computeImpulseDenominator(pos2,solverConstraint.m_contactNormal);
#else
	btVector3 vec;
	btScalar denom0 = 0.f;
	btScalar denom1 = 0.f;
	if (body0)
	{
		vec = ( solverConstraint.m_angularComponentA).cross(rel_pos1);
		denom0 = body0->getInvMass() + normalAxis.dot(vec);
	}
	if (body1)
	{
		vec = ( -solverConstraint.m_angularComponentB).cross(rel_pos2);
		denom1 = body1->getInvMass() + normalAxis.dot(vec);
	}


#endif //COMPUTE_IMPULSE_DENOM
	btScalar denom = relaxation/(denom0+denom1);
	solverConstraint.m_jacDiagABInv = denom;

#ifdef _USE_JACOBIAN
	solverConstraint.m_jac =  btJacobianEntry (
		rel_pos1,rel_pos2,solverConstraint.m_contactNormal,
		body0->getInvInertiaDiagLocal(),
		body0->getInvMass(),
		body1->getInvInertiaDiagLocal(),
		body1->getInvMass());
#endif //_USE_JACOBIAN


	{
		btScalar rel_vel;
		btScalar vel1Dotn = solverConstraint.m_contactNormal.dot(body0?body0->getLinearVelocity():btVector3(0,0,0)) 
			+ solverConstraint.m_relpos1CrossNormal.dot(body0?body0->getAngularVelocity():btVector3(0,0,0));
		btScalar vel2Dotn = -solverConstraint.m_contactNormal.dot(body1?body1->getLinearVelocity():btVector3(0,0,0)) 
			+ solverConstraint.m_relpos2CrossNormal.dot(body1?body1->getAngularVelocity():btVector3(0,0,0));

		rel_vel = vel1Dotn+vel2Dotn;

//		btScalar positionalError = 0.f;

		btSimdScalar velocityError =  desiredVelocity - rel_vel;
		btSimdScalar	velocityImpulse = velocityError * btSimdScalar(solverConstraint.m_jacDiagABInv);
		solverConstraint.m_rhs = velocityImpulse;
		solverConstraint.m_cfm = cfmSlip;
		solverConstraint.m_lowerLimit = 0;
		solverConstraint.m_upperLimit = 1e10f;
	}
}



btSolverConstraint&	btSequentialImpulseConstraintSolver::addFrictionConstraint(const btVector3& normalAxis,btRigidBody* solverBodyA,btRigidBody* solverBodyB,int frictionIndex,btManifoldPoint& cp,const btVector3& rel_pos1,const btVector3& rel_pos2,btCollisionObject* colObj0,btCollisionObject* colObj1, btScalar relaxation, btScalar desiredVelocity, btScalar cfmSlip)
{
	btSolverConstraint& solverConstraint = m_tmpSolverContactFrictionConstraintPool.expandNonInitializing();
	solverConstraint.m_frictionIndex = frictionIndex;
	setupFrictionConstraint(solverConstraint, normalAxis, solverBodyA, solverBodyB, cp, rel_pos1, rel_pos2, 
							colObj0, colObj1, relaxation, desiredVelocity, cfmSlip);
	return solverConstraint;
}

int	btSequentialImpulseConstraintSolver::getOrInitSolverBody(btCollisionObject& body)
{
#if 0
	int solverBodyIdA = -1;

	if (body.getCompanionId() >= 0)
	{
		//body has already been converted
		solverBodyIdA = body.getCompanionId();
	} else
	{
		btRigidBody* rb = btRigidBody::upcast(&body);
		if (rb && rb->getInvMass())
		{
			solverBodyIdA = m_tmpSolverBodyPool.size();
			btSolverBody& solverBody = m_tmpSolverBodyPool.expand();
			initSolverBody(&solverBody,&body);
			body.setCompanionId(solverBodyIdA);
		} else
		{
			return 0;//assume first one is a fixed solver body
		}
	}
	return solverBodyIdA;
#endif
	return 0;
}
#include <stdio.h>


void btSequentialImpulseConstraintSolver::setupContactConstraint(btSolverConstraint& solverConstraint, 
																 btCollisionObject* colObj0, btCollisionObject* colObj1,
																 btManifoldPoint& cp, const btContactSolverInfo& infoGlobal,
																 btVector3& vel, btScalar& rel_vel, btScalar& relaxation,
																 btVector3& rel_pos1, btVector3& rel_pos2)
{
			btRigidBody* rb0 = btRigidBody::upcast(colObj0);
			btRigidBody* rb1 = btRigidBody::upcast(colObj1);

			const btVector3& pos1 = cp.getPositionWorldOnA();
			const btVector3& pos2 = cp.getPositionWorldOnB();

//			btVector3 rel_pos1 = pos1 - colObj0->getWorldTransform().getOrigin(); 
//			btVector3 rel_pos2 = pos2 - colObj1->getWorldTransform().getOrigin();
			rel_pos1 = pos1 - colObj0->getWorldTransform().getOrigin(); 
			rel_pos2 = pos2 - colObj1->getWorldTransform().getOrigin();

			relaxation = 1.f;

			btVector3 torqueAxis0 = rel_pos1.cross(cp.m_normalWorldOnB);
			solverConstraint.m_angularComponentA = rb0 ? rb0->getInvInertiaTensorWorld()*torqueAxis0*rb0->getAngularFactor() : btVector3(0,0,0);
			btVector3 torqueAxis1 = rel_pos2.cross(cp.m_normalWorldOnB);		
			solverConstraint.m_angularComponentB = rb1 ? rb1->getInvInertiaTensorWorld()*-torqueAxis1*rb1->getAngularFactor() : btVector3(0,0,0);

				{
#ifdef COMPUTE_IMPULSE_DENOM
					btScalar denom0 = rb0->computeImpulseDenominator(pos1,cp.m_normalWorldOnB);
					btScalar denom1 = rb1->computeImpulseDenominator(pos2,cp.m_normalWorldOnB);
#else							
					btVector3 vec;
					btScalar denom0 = 0.f;
					btScalar denom1 = 0.f;
					if (rb0)
					{
						vec = ( solverConstraint.m_angularComponentA).cross(rel_pos1);
						denom0 = rb0->getInvMass() + cp.m_normalWorldOnB.dot(vec);
					}
					if (rb1)
					{
						vec = ( -solverConstraint.m_angularComponentB).cross(rel_pos2);
						denom1 = rb1->getInvMass() + cp.m_normalWorldOnB.dot(vec);
					}
#endif //COMPUTE_IMPULSE_DENOM		

					btScalar denom = relaxation/(denom0+denom1);
					solverConstraint.m_jacDiagABInv = denom;
				}

				solverConstraint.m_contactNormal = cp.m_normalWorldOnB;
				solverConstraint.m_relpos1CrossNormal = rel_pos1.cross(cp.m_normalWorldOnB);
				solverConstraint.m_relpos2CrossNormal = rel_pos2.cross(-cp.m_normalWorldOnB);




			btVector3 vel1 = rb0 ? rb0->getVelocityInLocalPoint(rel_pos1) : btVector3(0,0,0);
			btVector3 vel2 = rb1 ? rb1->getVelocityInLocalPoint(rel_pos2) : btVector3(0,0,0);
			vel  = vel1 - vel2;
			rel_vel = cp.m_normalWorldOnB.dot(vel);

				btScalar penetration = cp.getDistance()+infoGlobal.m_linearSlop;


				solverConstraint.m_friction = cp.m_combinedFriction;

				btScalar restitution = 0.f;
				
				if (cp.m_lifeTime>infoGlobal.m_restingContactRestitutionThreshold)
				{
					restitution = 0.f;
				} else
				{
					restitution =  restitutionCurve(rel_vel, cp.m_combinedRestitution);
					if (restitution <= btScalar(0.))
					{
						restitution = 0.f;
					};
				}


				///warm starting (or zero if disabled)
				if (infoGlobal.m_solverMode & SOLVER_USE_WARMSTARTING)
				{
					solverConstraint.m_appliedImpulse = cp.m_appliedImpulse * infoGlobal.m_warmstartingFactor;
					if (rb0)
						rb0->internalApplyImpulse(solverConstraint.m_contactNormal*rb0->getInvMass()*rb0->getLinearFactor(),solverConstraint.m_angularComponentA,solverConstraint.m_appliedImpulse);
					if (rb1)
						rb1->internalApplyImpulse(solverConstraint.m_contactNormal*rb1->getInvMass()*rb1->getLinearFactor(),-solverConstraint.m_angularComponentB,-(btScalar)solverConstraint.m_appliedImpulse);
				} else
				{
					solverConstraint.m_appliedImpulse = 0.f;
				}

				solverConstraint.m_appliedPushImpulse = 0.f;

				{
					btScalar rel_vel;
					btScalar vel1Dotn = solverConstraint.m_contactNormal.dot(rb0?rb0->getLinearVelocity():btVector3(0,0,0)) 
						+ solverConstraint.m_relpos1CrossNormal.dot(rb0?rb0->getAngularVelocity():btVector3(0,0,0));
					btScalar vel2Dotn = -solverConstraint.m_contactNormal.dot(rb1?rb1->getLinearVelocity():btVector3(0,0,0)) 
						+ solverConstraint.m_relpos2CrossNormal.dot(rb1?rb1->getAngularVelocity():btVector3(0,0,0));

					rel_vel = vel1Dotn+vel2Dotn;

					btScalar positionalError = 0.f;
					btScalar	velocityError = restitution - rel_vel;// * damping;

					if (penetration>0)
					{
						positionalError = 0;
						velocityError -= penetration / infoGlobal.m_timeStep;
					} else
					{
						positionalError = -penetration * infoGlobal.m_erp/infoGlobal.m_timeStep;
					}

					btScalar  penetrationImpulse = positionalError*solverConstraint.m_jacDiagABInv;
					btScalar velocityImpulse = velocityError *solverConstraint.m_jacDiagABInv;
					if (!infoGlobal.m_splitImpulse || (penetration > infoGlobal.m_splitImpulsePenetrationThreshold))
					{
						//combine position and velocity into rhs
						solverConstraint.m_rhs = penetrationImpulse+velocityImpulse;
						solverConstraint.m_rhsPenetration = 0.f;
					} else
					{
						//split position and velocity into rhs and m_rhsPenetration
						solverConstraint.m_rhs = velocityImpulse;
						solverConstraint.m_rhsPenetration = penetrationImpulse;
					}
					solverConstraint.m_cfm = 0.f;
					solverConstraint.m_lowerLimit = 0;
					solverConstraint.m_upperLimit = 1e10f;
				}




}



void btSequentialImpulseConstraintSolver::setFrictionConstraintImpulse( btSolverConstraint& solverConstraint, 
																		btRigidBody* rb0, btRigidBody* rb1, 
																 btManifoldPoint& cp, const btContactSolverInfo& infoGlobal)
{
					if (infoGlobal.m_solverMode & SOLVER_USE_FRICTION_WARMSTARTING)
					{
						{
							btSolverConstraint& frictionConstraint1 = m_tmpSolverContactFrictionConstraintPool[solverConstraint.m_frictionIndex];
							if (infoGlobal.m_solverMode & SOLVER_USE_WARMSTARTING)
							{
								frictionConstraint1.m_appliedImpulse = cp.m_appliedImpulseLateral1 * infoGlobal.m_warmstartingFactor;
								if (rb0)
									rb0->internalApplyImpulse(frictionConstraint1.m_contactNormal*rb0->getInvMass()*rb0->getLinearFactor(),frictionConstraint1.m_angularComponentA,frictionConstraint1.m_appliedImpulse);
								if (rb1)
									rb1->internalApplyImpulse(frictionConstraint1.m_contactNormal*rb1->getInvMass()*rb1->getLinearFactor(),-frictionConstraint1.m_angularComponentB,-(btScalar)frictionConstraint1.m_appliedImpulse);
							} else
							{
								frictionConstraint1.m_appliedImpulse = 0.f;
							}
						}

						if ((infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS))
						{
							btSolverConstraint& frictionConstraint2 = m_tmpSolverContactFrictionConstraintPool[solverConstraint.m_frictionIndex+1];
							if (infoGlobal.m_solverMode & SOLVER_USE_WARMSTARTING)
							{
								frictionConstraint2.m_appliedImpulse = cp.m_appliedImpulseLateral2 * infoGlobal.m_warmstartingFactor;
								if (rb0)
									rb0->internalApplyImpulse(frictionConstraint2.m_contactNormal*rb0->getInvMass(),frictionConstraint2.m_angularComponentA,frictionConstraint2.m_appliedImpulse);
								if (rb1)
									rb1->internalApplyImpulse(frictionConstraint2.m_contactNormal*rb1->getInvMass(),-frictionConstraint2.m_angularComponentB,-(btScalar)frictionConstraint2.m_appliedImpulse);
							} else
							{
								frictionConstraint2.m_appliedImpulse = 0.f;
							}
						}
					} else
					{
						btSolverConstraint& frictionConstraint1 = m_tmpSolverContactFrictionConstraintPool[solverConstraint.m_frictionIndex];
						frictionConstraint1.m_appliedImpulse = 0.f;
						if ((infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS))
						{
							btSolverConstraint& frictionConstraint2 = m_tmpSolverContactFrictionConstraintPool[solverConstraint.m_frictionIndex+1];
							frictionConstraint2.m_appliedImpulse = 0.f;
						}
					}
}




void	btSequentialImpulseConstraintSolver::convertContact(btPersistentManifold* manifold,const btContactSolverInfo& infoGlobal)
{
	btCollisionObject* colObj0=0,*colObj1=0;

	colObj0 = (btCollisionObject*)manifold->getBody0();
	colObj1 = (btCollisionObject*)manifold->getBody1();


	btRigidBody* solverBodyA = btRigidBody::upcast(colObj0);
	btRigidBody* solverBodyB = btRigidBody::upcast(colObj1);

	///avoid collision response between two static objects
	if ((!solverBodyA || !solverBodyA->getInvMass()) && (!solverBodyB || !solverBodyB->getInvMass()))
		return;

	for (int j=0;j<manifold->getNumContacts();j++)
	{

		btManifoldPoint& cp = manifold->getContactPoint(j);

		if (cp.getDistance() <= manifold->getContactProcessingThreshold())
		{
			btVector3 rel_pos1;
			btVector3 rel_pos2;
			btScalar relaxation;
			btScalar rel_vel;
			btVector3 vel;

			int frictionIndex = m_tmpSolverContactConstraintPool.size();
			btSolverConstraint& solverConstraint = m_tmpSolverContactConstraintPool.expandNonInitializing();
			btRigidBody* rb0 = btRigidBody::upcast(colObj0);
			btRigidBody* rb1 = btRigidBody::upcast(colObj1);
			solverConstraint.m_solverBodyA = rb0? rb0 : &getFixedBody();
			solverConstraint.m_solverBodyB = rb1? rb1 : &getFixedBody();
			solverConstraint.m_originalContactPoint = &cp;

			setupContactConstraint(solverConstraint, colObj0, colObj1, cp, infoGlobal, vel, rel_vel, relaxation, rel_pos1, rel_pos2);

//			const btVector3& pos1 = cp.getPositionWorldOnA();
//			const btVector3& pos2 = cp.getPositionWorldOnB();

			/////setup the friction constraints

			solverConstraint.m_frictionIndex = m_tmpSolverContactFrictionConstraintPool.size();

			if (!(infoGlobal.m_solverMode & SOLVER_ENABLE_FRICTION_DIRECTION_CACHING) || !cp.m_lateralFrictionInitialized)
			{
				cp.m_lateralFrictionDir1 = vel - cp.m_normalWorldOnB * rel_vel;
				btScalar lat_rel_vel = cp.m_lateralFrictionDir1.length2();
				if (!(infoGlobal.m_solverMode & SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION) && lat_rel_vel > SIMD_EPSILON)
				{
					cp.m_lateralFrictionDir1 /= btSqrt(lat_rel_vel);
					if((infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS))
					{
						cp.m_lateralFrictionDir2 = cp.m_lateralFrictionDir1.cross(cp.m_normalWorldOnB);
						cp.m_lateralFrictionDir2.normalize();//??
						applyAnisotropicFriction(colObj0,cp.m_lateralFrictionDir2);
						applyAnisotropicFriction(colObj1,cp.m_lateralFrictionDir2);
						addFrictionConstraint(cp.m_lateralFrictionDir2,solverBodyA,solverBodyB,frictionIndex,cp,rel_pos1,rel_pos2,colObj0,colObj1, relaxation);
					}

					applyAnisotropicFriction(colObj0,cp.m_lateralFrictionDir1);
					applyAnisotropicFriction(colObj1,cp.m_lateralFrictionDir1);
					addFrictionConstraint(cp.m_lateralFrictionDir1,solverBodyA,solverBodyB,frictionIndex,cp,rel_pos1,rel_pos2,colObj0,colObj1, relaxation);
					cp.m_lateralFrictionInitialized = true;
				} else
				{
					//re-calculate friction direction every frame, todo: check if this is really needed
					btPlaneSpace1(cp.m_normalWorldOnB,cp.m_lateralFrictionDir1,cp.m_lateralFrictionDir2);
					if ((infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS))
					{
						applyAnisotropicFriction(colObj0,cp.m_lateralFrictionDir2);
						applyAnisotropicFriction(colObj1,cp.m_lateralFrictionDir2);
						addFrictionConstraint(cp.m_lateralFrictionDir2,solverBodyA,solverBodyB,frictionIndex,cp,rel_pos1,rel_pos2,colObj0,colObj1, relaxation);
					}

					applyAnisotropicFriction(colObj0,cp.m_lateralFrictionDir1);
					applyAnisotropicFriction(colObj1,cp.m_lateralFrictionDir1);
					addFrictionConstraint(cp.m_lateralFrictionDir1,solverBodyA,solverBodyB,frictionIndex,cp,rel_pos1,rel_pos2,colObj0,colObj1, relaxation);

					cp.m_lateralFrictionInitialized = true;
				}

			} else
			{
				addFrictionConstraint(cp.m_lateralFrictionDir1,solverBodyA,solverBodyB,frictionIndex,cp,rel_pos1,rel_pos2,colObj0,colObj1, relaxation,cp.m_contactMotion1, cp.m_contactCFM1);
				if ((infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS))
					addFrictionConstraint(cp.m_lateralFrictionDir2,solverBodyA,solverBodyB,frictionIndex,cp,rel_pos1,rel_pos2,colObj0,colObj1, relaxation, cp.m_contactMotion2, cp.m_contactCFM2);
			}
			
			setFrictionConstraintImpulse( solverConstraint, rb0, rb1, cp, infoGlobal);

		}
	}
}


btScalar btSequentialImpulseConstraintSolver::solveGroupCacheFriendlySetup(btCollisionObject** bodies, int numBodies, btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc)
{
	BT_PROFILE("solveGroupCacheFriendlySetup");
	(void)stackAlloc;
	(void)debugDrawer;


	if (!(numConstraints + numManifolds))
	{
		//		printf("empty\n");
		return 0.f;
	}

	if (infoGlobal.m_splitImpulse)
	{
		for (int i = 0; i < numBodies; i++)
		{
			btRigidBody* body = btRigidBody::upcast(bodies[i]);
			if (body)
			{	
				body->internalGetDeltaLinearVelocity().setZero();
				body->internalGetDeltaAngularVelocity().setZero();
				body->internalGetPushVelocity().setZero();
				body->internalGetTurnVelocity().setZero();
			}
		}
	}
	else
	{
		for (int i = 0; i < numBodies; i++)
		{
			btRigidBody* body = btRigidBody::upcast(bodies[i]);
			if (body)
			{	
				body->internalGetDeltaLinearVelocity().setZero();
				body->internalGetDeltaAngularVelocity().setZero();
			}
		}
	}

	if (1)
	{
		int j;
		for (j=0;j<numConstraints;j++)
		{
			btTypedConstraint* constraint = constraints[j];
			constraint->buildJacobian();
			constraint->internalSetAppliedImpulse(0.0f);
		}
	}
	//btRigidBody* rb0=0,*rb1=0;

	//if (1)
	{
		{

			int totalNumRows = 0;
			int i;
			
			m_tmpConstraintSizesPool.resize(numConstraints);
			//calculate the total number of contraint rows
			for (i=0;i<numConstraints;i++)
			{
				btTypedConstraint::btConstraintInfo1& info1 = m_tmpConstraintSizesPool[i];
				if (constraints[i]->isEnabled())
				{
					constraints[i]->getInfo1(&info1);
				} else
				{
					info1.m_numConstraintRows = 0;
					info1.nub = 0;
				}
				totalNumRows += info1.m_numConstraintRows;
			}
			m_tmpSolverNonContactConstraintPool.resize(totalNumRows);

			
			///setup the btSolverConstraints
			int currentRow = 0;

			for (i=0;i<numConstraints;i++)
			{
				const btTypedConstraint::btConstraintInfo1& info1 = m_tmpConstraintSizesPool[i];
				
				if (info1.m_numConstraintRows)
				{
					btAssert(currentRow<totalNumRows);

					btSolverConstraint* currentConstraintRow = &m_tmpSolverNonContactConstraintPool[currentRow];
					btTypedConstraint* constraint = constraints[i];


					btRigidBody& rbA = constraint->getRigidBodyA();
					btRigidBody& rbB = constraint->getRigidBodyB();

					
					int j;
					for ( j=0;j<info1.m_numConstraintRows;j++)
					{
						memset(&currentConstraintRow[j],0,sizeof(btSolverConstraint));
						currentConstraintRow[j].m_lowerLimit = -SIMD_INFINITY;
						currentConstraintRow[j].m_upperLimit = SIMD_INFINITY;
						currentConstraintRow[j].m_appliedImpulse = 0.f;
						currentConstraintRow[j].m_appliedPushImpulse = 0.f;
						currentConstraintRow[j].m_solverBodyA = &rbA;
						currentConstraintRow[j].m_solverBodyB = &rbB;
					}

					rbA.internalGetDeltaLinearVelocity().setValue(0.f,0.f,0.f);
					rbA.internalGetDeltaAngularVelocity().setValue(0.f,0.f,0.f);
					rbB.internalGetDeltaLinearVelocity().setValue(0.f,0.f,0.f);
					rbB.internalGetDeltaAngularVelocity().setValue(0.f,0.f,0.f);



					btTypedConstraint::btConstraintInfo2 info2;
					info2.fps = 1.f/infoGlobal.m_timeStep;
					info2.erp = infoGlobal.m_erp;
					info2.m_J1linearAxis = currentConstraintRow->m_contactNormal;
					info2.m_J1angularAxis = currentConstraintRow->m_relpos1CrossNormal;
					info2.m_J2linearAxis = 0;
					info2.m_J2angularAxis = currentConstraintRow->m_relpos2CrossNormal;
					info2.rowskip = sizeof(btSolverConstraint)/sizeof(btScalar);//check this
					///the size of btSolverConstraint needs be a multiple of btScalar
					btAssert(info2.rowskip*sizeof(btScalar)== sizeof(btSolverConstraint));
					info2.m_constraintError = &currentConstraintRow->m_rhs;
					currentConstraintRow->m_cfm = infoGlobal.m_globalCfm;
					info2.m_damping = infoGlobal.m_damping;
					info2.cfm = &currentConstraintRow->m_cfm;
					info2.m_lowerLimit = &currentConstraintRow->m_lowerLimit;
					info2.m_upperLimit = &currentConstraintRow->m_upperLimit;
					info2.m_numIterations = infoGlobal.m_numIterations;
					constraints[i]->getInfo2(&info2);

					if (currentConstraintRow->m_upperLimit>constraints[i]->getBreakingImpulseThreshold())
					{
						currentConstraintRow->m_upperLimit = constraints[i]->getBreakingImpulseThreshold();
					}

					if (currentConstraintRow->m_lowerLimit<-constraints[i]->getBreakingImpulseThreshold())
					{
						currentConstraintRow->m_lowerLimit = -constraints[i]->getBreakingImpulseThreshold();
					}



					///finalize the constraint setup
					for ( j=0;j<info1.m_numConstraintRows;j++)
					{
						btSolverConstraint& solverConstraint = currentConstraintRow[j];
						solverConstraint.m_originalContactPoint = constraint;

						{
							const btVector3& ftorqueAxis1 = solverConstraint.m_relpos1CrossNormal;
							solverConstraint.m_angularComponentA = constraint->getRigidBodyA().getInvInertiaTensorWorld()*ftorqueAxis1*constraint->getRigidBodyA().getAngularFactor();
						}
						{
							const btVector3& ftorqueAxis2 = solverConstraint.m_relpos2CrossNormal;
							solverConstraint.m_angularComponentB = constraint->getRigidBodyB().getInvInertiaTensorWorld()*ftorqueAxis2*constraint->getRigidBodyB().getAngularFactor();
						}

						{
							btVector3 iMJlA = solverConstraint.m_contactNormal*rbA.getInvMass();
							btVector3 iMJaA = rbA.getInvInertiaTensorWorld()*solverConstraint.m_relpos1CrossNormal;
							btVector3 iMJlB = solverConstraint.m_contactNormal*rbB.getInvMass();//sign of normal?
							btVector3 iMJaB = rbB.getInvInertiaTensorWorld()*solverConstraint.m_relpos2CrossNormal;

							btScalar sum = iMJlA.dot(solverConstraint.m_contactNormal);
							sum += iMJaA.dot(solverConstraint.m_relpos1CrossNormal);
							sum += iMJlB.dot(solverConstraint.m_contactNormal);
							sum += iMJaB.dot(solverConstraint.m_relpos2CrossNormal);

							solverConstraint.m_jacDiagABInv = btScalar(1.)/sum;
						}


						///fix rhs
						///todo: add force/torque accelerators
						{
							btScalar rel_vel;
							btScalar vel1Dotn = solverConstraint.m_contactNormal.dot(rbA.getLinearVelocity()) + solverConstraint.m_relpos1CrossNormal.dot(rbA.getAngularVelocity());
							btScalar vel2Dotn = -solverConstraint.m_contactNormal.dot(rbB.getLinearVelocity()) + solverConstraint.m_relpos2CrossNormal.dot(rbB.getAngularVelocity());

							rel_vel = vel1Dotn+vel2Dotn;

							btScalar restitution = 0.f;
							btScalar positionalError = solverConstraint.m_rhs;//already filled in by getConstraintInfo2
							btScalar	velocityError = restitution - rel_vel * info2.m_damping;
							btScalar	penetrationImpulse = positionalError*solverConstraint.m_jacDiagABInv;
							btScalar	velocityImpulse = velocityError *solverConstraint.m_jacDiagABInv;
							solverConstraint.m_rhs = penetrationImpulse+velocityImpulse;
							solverConstraint.m_appliedImpulse = 0.f;

						}
					}
				}
				currentRow+=m_tmpConstraintSizesPool[i].m_numConstraintRows;
			}
		}

		{
			int i;
			btPersistentManifold* manifold = 0;
//			btCollisionObject* colObj0=0,*colObj1=0;


			for (i=0;i<numManifolds;i++)
			{
				manifold = manifoldPtr[i];
				convertContact(manifold,infoGlobal);
			}
		}
	}

	btContactSolverInfo info = infoGlobal;



	int numConstraintPool = m_tmpSolverContactConstraintPool.size();
	int numFrictionPool = m_tmpSolverContactFrictionConstraintPool.size();

	///@todo: use stack allocator for such temporarily memory, same for solver bodies/constraints
	m_orderTmpConstraintPool.resize(numConstraintPool);
	m_orderFrictionConstraintPool.resize(numFrictionPool);
	{
		int i;
		for (i=0;i<numConstraintPool;i++)
		{
			m_orderTmpConstraintPool[i] = i;
		}
		for (i=0;i<numFrictionPool;i++)
		{
			m_orderFrictionConstraintPool[i] = i;
		}
	}

	return 0.f;

}

btScalar btSequentialImpulseConstraintSolver::solveSingleIteration(int iteration, btCollisionObject** /*bodies */,int /*numBodies*/,btPersistentManifold** /*manifoldPtr*/, int /*numManifolds*/,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* /*debugDrawer*/,btStackAlloc* /*stackAlloc*/)
{

	int numConstraintPool = m_tmpSolverContactConstraintPool.size();
	int numFrictionPool = m_tmpSolverContactFrictionConstraintPool.size();

	int j;

	if (infoGlobal.m_solverMode & SOLVER_RANDMIZE_ORDER)
	{
		if ((iteration & 7) == 0) {
			for (j=0; j<numConstraintPool; ++j) {
				int tmp = m_orderTmpConstraintPool[j];
				int swapi = btRandInt2(j+1);
				m_orderTmpConstraintPool[j] = m_orderTmpConstraintPool[swapi];
				m_orderTmpConstraintPool[swapi] = tmp;
			}

			for (j=0; j<numFrictionPool; ++j) {
				int tmp = m_orderFrictionConstraintPool[j];
				int swapi = btRandInt2(j+1);
				m_orderFrictionConstraintPool[j] = m_orderFrictionConstraintPool[swapi];
				m_orderFrictionConstraintPool[swapi] = tmp;
			}
		}
	}

	if (infoGlobal.m_solverMode & SOLVER_SIMD)
	{
		///solve all joint constraints, using SIMD, if available
		for (j=0;j<m_tmpSolverNonContactConstraintPool.size();j++)
		{
			btSolverConstraint& constraint = m_tmpSolverNonContactConstraintPool[j];
			resolveSingleConstraintRowGenericSIMD(*constraint.m_solverBodyA,*constraint.m_solverBodyB,constraint);
		}

		for (j=0;j<numConstraints;j++)
		{
			constraints[j]->solveConstraintObsolete(constraints[j]->getRigidBodyA(),constraints[j]->getRigidBodyB(),infoGlobal.m_timeStep);
		}

		///solve all contact constraints using SIMD, if available
		int numPoolConstraints = m_tmpSolverContactConstraintPool.size();
		for (j=0;j<numPoolConstraints;j++)
		{
			const btSolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[m_orderTmpConstraintPool[j]];
			resolveSingleConstraintRowLowerLimitSIMD(*solveManifold.m_solverBodyA,*solveManifold.m_solverBodyB,solveManifold);

		}
		///solve all friction constraints, using SIMD, if available
		int numFrictionPoolConstraints = m_tmpSolverContactFrictionConstraintPool.size();
		for (j=0;j<numFrictionPoolConstraints;j++)
		{
			btSolverConstraint& solveManifold = m_tmpSolverContactFrictionConstraintPool[m_orderFrictionConstraintPool[j]];
			btScalar totalImpulse = m_tmpSolverContactConstraintPool[solveManifold.m_frictionIndex].m_appliedImpulse;

			if (totalImpulse>btScalar(0))
			{
				solveManifold.m_lowerLimit = -(solveManifold.m_friction*totalImpulse);
				solveManifold.m_upperLimit = solveManifold.m_friction*totalImpulse;

				resolveSingleConstraintRowGenericSIMD(*solveManifold.m_solverBodyA,	*solveManifold.m_solverBodyB,solveManifold);
			}
		}
	} else
	{

		///solve all joint constraints
		for (j=0;j<m_tmpSolverNonContactConstraintPool.size();j++)
		{
			btSolverConstraint& constraint = m_tmpSolverNonContactConstraintPool[j];
			resolveSingleConstraintRowGeneric(*constraint.m_solverBodyA,*constraint.m_solverBodyB,constraint);
		}

		for (j=0;j<numConstraints;j++)
		{
			constraints[j]->solveConstraintObsolete(constraints[j]->getRigidBodyA(),constraints[j]->getRigidBodyB(),infoGlobal.m_timeStep);
		}
		///solve all contact constraints
		int numPoolConstraints = m_tmpSolverContactConstraintPool.size();
		for (j=0;j<numPoolConstraints;j++)
		{
			const btSolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[m_orderTmpConstraintPool[j]];
			resolveSingleConstraintRowLowerLimit(*solveManifold.m_solverBodyA,*solveManifold.m_solverBodyB,solveManifold);
		}
		///solve all friction constraints
		int numFrictionPoolConstraints = m_tmpSolverContactFrictionConstraintPool.size();
		for (j=0;j<numFrictionPoolConstraints;j++)
		{
			btSolverConstraint& solveManifold = m_tmpSolverContactFrictionConstraintPool[m_orderFrictionConstraintPool[j]];
			btScalar totalImpulse = m_tmpSolverContactConstraintPool[solveManifold.m_frictionIndex].m_appliedImpulse;

			if (totalImpulse>btScalar(0))
			{
				solveManifold.m_lowerLimit = -(solveManifold.m_friction*totalImpulse);
				solveManifold.m_upperLimit = solveManifold.m_friction*totalImpulse;

				resolveSingleConstraintRowGeneric(*solveManifold.m_solverBodyA,*solveManifold.m_solverBodyB,solveManifold);
			}
		}
	}
	return 0.f;
}


void btSequentialImpulseConstraintSolver::solveGroupCacheFriendlySplitImpulseIterations(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc)
{
	int iteration;
	if (infoGlobal.m_splitImpulse)
	{
		if (infoGlobal.m_solverMode & SOLVER_SIMD)
		{
			for ( iteration = 0;iteration<infoGlobal.m_numIterations;iteration++)
			{
				{
					int numPoolConstraints = m_tmpSolverContactConstraintPool.size();
					int j;
					for (j=0;j<numPoolConstraints;j++)
					{
						const btSolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[m_orderTmpConstraintPool[j]];

						resolveSplitPenetrationSIMD(*solveManifold.m_solverBodyA,*solveManifold.m_solverBodyB,solveManifold);
					}
				}
			}
		}
		else
		{
			for ( iteration = 0;iteration<infoGlobal.m_numIterations;iteration++)
			{
				{
					int numPoolConstraints = m_tmpSolverContactConstraintPool.size();
					int j;
					for (j=0;j<numPoolConstraints;j++)
					{
						const btSolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[m_orderTmpConstraintPool[j]];

						resolveSplitPenetrationImpulseCacheFriendly(*solveManifold.m_solverBodyA,*solveManifold.m_solverBodyB,solveManifold);
					}
				}
			}
		}
	}
}

btScalar btSequentialImpulseConstraintSolver::solveGroupCacheFriendlyIterations(btCollisionObject** bodies ,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc)
{
	BT_PROFILE("solveGroupCacheFriendlyIterations");

	
	//should traverse the contacts random order...
	int iteration;
	{
		solveGroupCacheFriendlySplitImpulseIterations(bodies ,numBodies,manifoldPtr, numManifolds,constraints,numConstraints,infoGlobal,debugDrawer,stackAlloc);

		for ( iteration = 0;iteration<infoGlobal.m_numIterations;iteration++)
		{			
			solveSingleIteration(iteration, bodies ,numBodies,manifoldPtr, numManifolds,constraints,numConstraints,infoGlobal,debugDrawer,stackAlloc);
		}
		
	}
	return 0.f;
}

btScalar btSequentialImpulseConstraintSolver::solveGroupCacheFriendlyFinish(btCollisionObject** bodies ,int numBodies,btPersistentManifold** /*manifoldPtr*/, int /*numManifolds*/,btTypedConstraint** /*constraints*/,int /* numConstraints*/,const btContactSolverInfo& infoGlobal,btIDebugDraw* /*debugDrawer*/,btStackAlloc* /*stackAlloc*/)
{
	int numPoolConstraints = m_tmpSolverContactConstraintPool.size();
	int i,j;

	for (j=0;j<numPoolConstraints;j++)
	{

		const btSolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[j];
		btManifoldPoint* pt = (btManifoldPoint*) solveManifold.m_originalContactPoint;
		btAssert(pt);
		pt->m_appliedImpulse = solveManifold.m_appliedImpulse;
		if (infoGlobal.m_solverMode & SOLVER_USE_FRICTION_WARMSTARTING)
		{
			pt->m_appliedImpulseLateral1 = m_tmpSolverContactFrictionConstraintPool[solveManifold.m_frictionIndex].m_appliedImpulse;
			pt->m_appliedImpulseLateral2 = m_tmpSolverContactFrictionConstraintPool[solveManifold.m_frictionIndex+1].m_appliedImpulse;
		}

		//do a callback here?
	}

	numPoolConstraints = m_tmpSolverNonContactConstraintPool.size();
	for (j=0;j<numPoolConstraints;j++)
	{
		const btSolverConstraint& solverConstr = m_tmpSolverNonContactConstraintPool[j];
		btTypedConstraint* constr = (btTypedConstraint*)solverConstr.m_originalContactPoint;
		constr->internalSetAppliedImpulse(solverConstr.m_appliedImpulse);
		if (solverConstr.m_appliedImpulse>constr->getBreakingImpulseThreshold())
		{
			constr->setEnabled(false);
		}
	}


	if (infoGlobal.m_splitImpulse)
	{		
		for ( i=0;i<numBodies;i++)
		{
			btRigidBody* body = btRigidBody::upcast(bodies[i]);
			if (body)
				body->internalWritebackVelocity(infoGlobal.m_timeStep);
		}
	} else
	{
		for ( i=0;i<numBodies;i++)
		{
			btRigidBody* body = btRigidBody::upcast(bodies[i]);
			if (body)
				body->internalWritebackVelocity();
		}
	}


	m_tmpSolverContactConstraintPool.resize(0);
	m_tmpSolverNonContactConstraintPool.resize(0);
	m_tmpSolverContactFrictionConstraintPool.resize(0);

	return 0.f;
}



/// btSequentialImpulseConstraintSolver Sequentially applies impulses
btScalar btSequentialImpulseConstraintSolver::solveGroup(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc,btDispatcher* /*dispatcher*/)
{

	BT_PROFILE("solveGroup");
	//you need to provide at least some bodies
	btAssert(bodies);
	btAssert(numBodies);

	solveGroupCacheFriendlySetup( bodies, numBodies, manifoldPtr,  numManifolds,constraints, numConstraints,infoGlobal,debugDrawer, stackAlloc);

	solveGroupCacheFriendlyIterations(bodies, numBodies, manifoldPtr,  numManifolds,constraints, numConstraints,infoGlobal,debugDrawer, stackAlloc);

	solveGroupCacheFriendlyFinish(bodies, numBodies, manifoldPtr,  numManifolds,constraints, numConstraints,infoGlobal,debugDrawer, stackAlloc);
	
	return 0.f;
}

void	btSequentialImpulseConstraintSolver::reset()
{
	m_btSeed2 = 0;
}

btRigidBody& btSequentialImpulseConstraintSolver::getFixedBody()
{
	static btRigidBody s_fixed(0, 0,0);
	s_fixed.setMassProps(btScalar(0.),btVector3(btScalar(0.),btScalar(0.),btScalar(0.)));
	return s_fixed;
}

