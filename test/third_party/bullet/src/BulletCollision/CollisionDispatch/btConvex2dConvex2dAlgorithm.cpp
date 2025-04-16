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

#include "btConvex2dConvex2dAlgorithm.h"

//#include <stdio.h>
#include "BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h"
#include "BulletCollision/BroadphaseCollision/btBroadphaseInterface.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btConvexShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"


#include "BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h"
#include "BulletCollision/BroadphaseCollision/btBroadphaseProxy.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionDispatch/btManifoldResult.h"

#include "BulletCollision/NarrowPhaseCollision/btConvexPenetrationDepthSolver.h"
#include "BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.h"
#include "BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkConvexCast.h"



#include "BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"

#include "BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.h"

#include "BulletCollision/NarrowPhaseCollision/btGjkEpa2.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h"


btConvex2dConvex2dAlgorithm::CreateFunc::CreateFunc(btSimplexSolverInterface*			simplexSolver, btConvexPenetrationDepthSolver* pdSolver)
{
	m_numPerturbationIterations = 0;
	m_minimumPointsPerturbationThreshold = 3;
	m_simplexSolver = simplexSolver;
	m_pdSolver = pdSolver;
}

btConvex2dConvex2dAlgorithm::CreateFunc::~CreateFunc() 
{ 
}

btConvex2dConvex2dAlgorithm::btConvex2dConvex2dAlgorithm(btPersistentManifold* mf,const btCollisionAlgorithmConstructionInfo& ci,btCollisionObject* body0,btCollisionObject* body1,btSimplexSolverInterface* simplexSolver, btConvexPenetrationDepthSolver* pdSolver,int numPerturbationIterations, int minimumPointsPerturbationThreshold)
: btActivatingCollisionAlgorithm(ci,body0,body1),
m_simplexSolver(simplexSolver),
m_pdSolver(pdSolver),
m_ownManifold (false),
m_manifoldPtr(mf),
m_lowLevelOfDetail(false),
 m_numPerturbationIterations(numPerturbationIterations),
m_minimumPointsPerturbationThreshold(minimumPointsPerturbationThreshold)
{
	(void)body0;
	(void)body1;
}




btConvex2dConvex2dAlgorithm::~btConvex2dConvex2dAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void	btConvex2dConvex2dAlgorithm ::setLowLevelOfDetail(bool useLowLevel)
{
	m_lowLevelOfDetail = useLowLevel;
}



extern btScalar gContactBreakingThreshold;


//
// Convex-Convex collision algorithm
//
void btConvex2dConvex2dAlgorithm ::processCollision (btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{

	if (!m_manifoldPtr)
	{
		//swapped?
		m_manifoldPtr = m_dispatcher->getNewManifold(body0,body1);
		m_ownManifold = true;
	}
	resultOut->setPersistentManifold(m_manifoldPtr);

	//comment-out next line to test multi-contact generation
	//resultOut->getPersistentManifold()->clearManifold();


	btConvexShape* min0 = static_cast<btConvexShape*>(body0->getCollisionShape());
	btConvexShape* min1 = static_cast<btConvexShape*>(body1->getCollisionShape());

	btVector3  normalOnB;
	btVector3  pointOnBWorld;

	{


		btGjkPairDetector::ClosestPointInput input;

		btGjkPairDetector	gjkPairDetector(min0,min1,m_simplexSolver,m_pdSolver);
		//TODO: if (dispatchInfo.m_useContinuous)
		gjkPairDetector.setMinkowskiA(min0);
		gjkPairDetector.setMinkowskiB(min1);

		{
			input.m_maximumDistanceSquared = min0->getMargin() + min1->getMargin() + m_manifoldPtr->getContactBreakingThreshold();
			input.m_maximumDistanceSquared*= input.m_maximumDistanceSquared;
		}

		input.m_stackAlloc = dispatchInfo.m_stackAllocator;
		input.m_transformA = body0->getWorldTransform();
		input.m_transformB = body1->getWorldTransform();

		gjkPairDetector.getClosestPoints(input,*resultOut,dispatchInfo.m_debugDraw);

		btVector3 v0,v1;
		btVector3 sepNormalWorldSpace;

	}

	if (m_ownManifold)
	{
		resultOut->refreshContactPoints();
	}

}




btScalar	btConvex2dConvex2dAlgorithm::calculateTimeOfImpact(btCollisionObject* col0,btCollisionObject* col1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	///Rather then checking ALL pairs, only calculate TOI when motion exceeds threshold

	///Linear motion for one of objects needs to exceed m_ccdSquareMotionThreshold
	///col0->m_worldTransform,
	btScalar resultFraction = btScalar(1.);


	btScalar squareMot0 = (col0->getInterpolationWorldTransform().getOrigin() - col0->getWorldTransform().getOrigin()).length2();
	btScalar squareMot1 = (col1->getInterpolationWorldTransform().getOrigin() - col1->getWorldTransform().getOrigin()).length2();

	if (squareMot0 < col0->getCcdSquareMotionThreshold() &&
		squareMot1 < col1->getCcdSquareMotionThreshold())
		return resultFraction;


	//An adhoc way of testing the Continuous Collision Detection algorithms
	//One object is approximated as a sphere, to simplify things
	//Starting in penetration should report no time of impact
	//For proper CCD, better accuracy and handling of 'allowed' penetration should be added
	//also the mainloop of the physics should have a kind of toi queue (something like Brian Mirtich's application of Timewarp for Rigidbodies)


	/// Convex0 against sphere for Convex1
	{
		btConvexShape* convex0 = static_cast<btConvexShape*>(col0->getCollisionShape());

		btSphereShape	sphere1(col1->getCcdSweptSphereRadius()); //todo: allow non-zero sphere sizes, for better approximation
		btConvexCast::CastResult result;
		btVoronoiSimplexSolver voronoiSimplex;
		//SubsimplexConvexCast ccd0(&sphere,min0,&voronoiSimplex);
		///Simplification, one object is simplified as a sphere
		btGjkConvexCast ccd1( convex0 ,&sphere1,&voronoiSimplex);
		//ContinuousConvexCollision ccd(min0,min1,&voronoiSimplex,0);
		if (ccd1.calcTimeOfImpact(col0->getWorldTransform(),col0->getInterpolationWorldTransform(),
			col1->getWorldTransform(),col1->getInterpolationWorldTransform(),result))
		{

			//store result.m_fraction in both bodies

			if (col0->getHitFraction()> result.m_fraction)
				col0->setHitFraction( result.m_fraction );

			if (col1->getHitFraction() > result.m_fraction)
				col1->setHitFraction( result.m_fraction);

			if (resultFraction > result.m_fraction)
				resultFraction = result.m_fraction;

		}




	}

	/// Sphere (for convex0) against Convex1
	{
		btConvexShape* convex1 = static_cast<btConvexShape*>(col1->getCollisionShape());

		btSphereShape	sphere0(col0->getCcdSweptSphereRadius()); //todo: allow non-zero sphere sizes, for better approximation
		btConvexCast::CastResult result;
		btVoronoiSimplexSolver voronoiSimplex;
		//SubsimplexConvexCast ccd0(&sphere,min0,&voronoiSimplex);
		///Simplification, one object is simplified as a sphere
		btGjkConvexCast ccd1(&sphere0,convex1,&voronoiSimplex);
		//ContinuousConvexCollision ccd(min0,min1,&voronoiSimplex,0);
		if (ccd1.calcTimeOfImpact(col0->getWorldTransform(),col0->getInterpolationWorldTransform(),
			col1->getWorldTransform(),col1->getInterpolationWorldTransform(),result))
		{

			//store result.m_fraction in both bodies

			if (col0->getHitFraction()	> result.m_fraction)
				col0->setHitFraction( result.m_fraction);

			if (col1->getHitFraction() > result.m_fraction)
				col1->setHitFraction( result.m_fraction);

			if (resultFraction > result.m_fraction)
				resultFraction = result.m_fraction;

		}
	}

	return resultFraction;

}

