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

#include "btGjkPairDetector.h"
#include "BulletCollision/CollisionShapes/btConvexShape.h"
#include "BulletCollision/NarrowPhaseCollision/btSimplexSolverInterface.h"
#include "BulletCollision/NarrowPhaseCollision/btConvexPenetrationDepthSolver.h"



#if defined(DEBUG) || defined (_DEBUG)
//#define TEST_NON_VIRTUAL 1
#include <stdio.h> //for debug printf
#ifdef __SPU__
#include <spu_printf.h>
#define printf spu_printf
//#define DEBUG_SPU_COLLISION_DETECTION 1
#endif //__SPU__
#endif

//must be above the machine epsilon
#define REL_ERROR2 btScalar(1.0e-6)

//temp globals, to improve GJK/EPA/penetration calculations
int gNumDeepPenetrationChecks = 0;
int gNumGjkChecks = 0;


btGjkPairDetector::btGjkPairDetector(const btConvexShape* objectA,const btConvexShape* objectB,btSimplexSolverInterface* simplexSolver,btConvexPenetrationDepthSolver*	penetrationDepthSolver)
:m_cachedSeparatingAxis(btScalar(0.),btScalar(1.),btScalar(0.)),
m_penetrationDepthSolver(penetrationDepthSolver),
m_simplexSolver(simplexSolver),
m_minkowskiA(objectA),
m_minkowskiB(objectB),
m_shapeTypeA(objectA->getShapeType()),
m_shapeTypeB(objectB->getShapeType()),
m_marginA(objectA->getMargin()),
m_marginB(objectB->getMargin()),
m_ignoreMargin(false),
m_lastUsedMethod(-1),
m_catchDegeneracies(1)
{
}
btGjkPairDetector::btGjkPairDetector(const btConvexShape* objectA,const btConvexShape* objectB,int shapeTypeA,int shapeTypeB,btScalar marginA, btScalar marginB, btSimplexSolverInterface* simplexSolver,btConvexPenetrationDepthSolver*	penetrationDepthSolver)
:m_cachedSeparatingAxis(btScalar(0.),btScalar(1.),btScalar(0.)),
m_penetrationDepthSolver(penetrationDepthSolver),
m_simplexSolver(simplexSolver),
m_minkowskiA(objectA),
m_minkowskiB(objectB),
m_shapeTypeA(shapeTypeA),
m_shapeTypeB(shapeTypeB),
m_marginA(marginA),
m_marginB(marginB),
m_ignoreMargin(false),
m_lastUsedMethod(-1),
m_catchDegeneracies(1)
{
}

void	btGjkPairDetector::getClosestPoints(const ClosestPointInput& input,Result& output,class btIDebugDraw* debugDraw,bool swapResults)
{
	(void)swapResults;

	getClosestPointsNonVirtual(input,output,debugDraw);
}

#ifdef __SPU__
void btGjkPairDetector::getClosestPointsNonVirtual(const ClosestPointInput& input,Result& output,class btIDebugDraw* debugDraw)
#else
void btGjkPairDetector::getClosestPointsNonVirtual(const ClosestPointInput& input,Result& output,class btIDebugDraw* debugDraw)
#endif
{
	m_cachedSeparatingDistance = 0.f;

	btScalar distance=btScalar(0.);
	btVector3	normalInB(btScalar(0.),btScalar(0.),btScalar(0.));
	btVector3 pointOnA,pointOnB;
	btTransform	localTransA = input.m_transformA;
	btTransform localTransB = input.m_transformB;
	btVector3 positionOffset = (localTransA.getOrigin() + localTransB.getOrigin()) * btScalar(0.5);
	localTransA.getOrigin() -= positionOffset;
	localTransB.getOrigin() -= positionOffset;

	bool check2d = m_minkowskiA->isConvex2d() && m_minkowskiB->isConvex2d();

	btScalar marginA = m_marginA;
	btScalar marginB = m_marginB;

	gNumGjkChecks++;

#ifdef DEBUG_SPU_COLLISION_DETECTION
	spu_printf("inside gjk\n");
#endif
	//for CCD we don't use margins
	if (m_ignoreMargin)
	{
		marginA = btScalar(0.);
		marginB = btScalar(0.);
#ifdef DEBUG_SPU_COLLISION_DETECTION
		spu_printf("ignoring margin\n");
#endif
	}

	m_curIter = 0;
	int gGjkMaxIter = 1000;//this is to catch invalid input, perhaps check for #NaN?
	m_cachedSeparatingAxis.setValue(0,1,0);

	bool isValid = false;
	bool checkSimplex = false;
	bool checkPenetration = true;
	m_degenerateSimplex = 0;

	m_lastUsedMethod = -1;

	{
		btScalar squaredDistance = BT_LARGE_FLOAT;
		btScalar delta = btScalar(0.);
		
		btScalar margin = marginA + marginB;
		
		

		m_simplexSolver->reset();
		
		for ( ; ; )
		//while (true)
		{

			btVector3 seperatingAxisInA = (-m_cachedSeparatingAxis)* input.m_transformA.getBasis();
			btVector3 seperatingAxisInB = m_cachedSeparatingAxis* input.m_transformB.getBasis();

#if 1

			btVector3 pInA = m_minkowskiA->localGetSupportVertexWithoutMarginNonVirtual(seperatingAxisInA);
			btVector3 qInB = m_minkowskiB->localGetSupportVertexWithoutMarginNonVirtual(seperatingAxisInB);

//			btVector3 pInA  = localGetSupportingVertexWithoutMargin(m_shapeTypeA, m_minkowskiA, seperatingAxisInA,input.m_convexVertexData[0]);//, &featureIndexA);
//			btVector3 qInB  = localGetSupportingVertexWithoutMargin(m_shapeTypeB, m_minkowskiB, seperatingAxisInB,input.m_convexVertexData[1]);//, &featureIndexB);

#else
#ifdef __SPU__
			btVector3 pInA = m_minkowskiA->localGetSupportVertexWithoutMarginNonVirtual(seperatingAxisInA);
			btVector3 qInB = m_minkowskiB->localGetSupportVertexWithoutMarginNonVirtual(seperatingAxisInB);
#else
			btVector3 pInA = m_minkowskiA->localGetSupportingVertexWithoutMargin(seperatingAxisInA);
			btVector3 qInB = m_minkowskiB->localGetSupportingVertexWithoutMargin(seperatingAxisInB);
#ifdef TEST_NON_VIRTUAL
			btVector3 pInAv = m_minkowskiA->localGetSupportingVertexWithoutMargin(seperatingAxisInA);
			btVector3 qInBv = m_minkowskiB->localGetSupportingVertexWithoutMargin(seperatingAxisInB);
			btAssert((pInAv-pInA).length() < 0.0001);
			btAssert((qInBv-qInB).length() < 0.0001);
#endif //
#endif //__SPU__
#endif


			btVector3  pWorld = localTransA(pInA);	
			btVector3  qWorld = localTransB(qInB);

#ifdef DEBUG_SPU_COLLISION_DETECTION
		spu_printf("got local supporting vertices\n");
#endif

			if (check2d)
			{
				pWorld[2] = 0.f;
				qWorld[2] = 0.f;
			}

			btVector3 w	= pWorld - qWorld;
			delta = m_cachedSeparatingAxis.dot(w);

			// potential exit, they don't overlap
			if ((delta > btScalar(0.0)) && (delta * delta > squaredDistance * input.m_maximumDistanceSquared)) 
			{
				m_degenerateSimplex = 10;
				checkSimplex=true;
				//checkPenetration = false;
				break;
			}

			//exit 0: the new point is already in the simplex, or we didn't come any closer
			if (m_simplexSolver->inSimplex(w))
			{
				m_degenerateSimplex = 1;
				checkSimplex = true;
				break;
			}
			// are we getting any closer ?
			btScalar f0 = squaredDistance - delta;
			btScalar f1 = squaredDistance * REL_ERROR2;

			if (f0 <= f1)
			{
				if (f0 <= btScalar(0.))
				{
					m_degenerateSimplex = 2;
				} else
				{
					m_degenerateSimplex = 11;
				}
				checkSimplex = true;
				break;
			}

#ifdef DEBUG_SPU_COLLISION_DETECTION
		spu_printf("addVertex 1\n");
#endif
			//add current vertex to simplex
			m_simplexSolver->addVertex(w, pWorld, qWorld);
#ifdef DEBUG_SPU_COLLISION_DETECTION
		spu_printf("addVertex 2\n");
#endif
			btVector3 newCachedSeparatingAxis;

			//calculate the closest point to the origin (update vector v)
			if (!m_simplexSolver->closest(newCachedSeparatingAxis))
			{
				m_degenerateSimplex = 3;
				checkSimplex = true;
				break;
			}

			if(newCachedSeparatingAxis.length2()<REL_ERROR2)
            {
				m_cachedSeparatingAxis = newCachedSeparatingAxis;
                m_degenerateSimplex = 6;
                checkSimplex = true;
                break;
            }

			btScalar previousSquaredDistance = squaredDistance;
			squaredDistance = newCachedSeparatingAxis.length2();
#if 0
///warning: this termination condition leads to some problems in 2d test case see Bullet/Demos/Box2dDemo
			if (squaredDistance>previousSquaredDistance)
			{
				m_degenerateSimplex = 7;
				squaredDistance = previousSquaredDistance;
                checkSimplex = false;
                break;
			}
#endif //
			

			//redundant m_simplexSolver->compute_points(pointOnA, pointOnB);

			//are we getting any closer ?
			if (previousSquaredDistance - squaredDistance <= SIMD_EPSILON * previousSquaredDistance) 
			{ 
//				m_simplexSolver->backup_closest(m_cachedSeparatingAxis);
				checkSimplex = true;
				m_degenerateSimplex = 12;
				
				break;
			}

			m_cachedSeparatingAxis = newCachedSeparatingAxis;

			  //degeneracy, this is typically due to invalid/uninitialized worldtransforms for a btCollisionObject   
              if (m_curIter++ > gGjkMaxIter)   
              {   
                      #if defined(DEBUG) || defined (_DEBUG) || defined (DEBUG_SPU_COLLISION_DETECTION)

                              printf("btGjkPairDetector maxIter exceeded:%i\n",m_curIter);   
                              printf("sepAxis=(%f,%f,%f), squaredDistance = %f, shapeTypeA=%i,shapeTypeB=%i\n",   
                              m_cachedSeparatingAxis.getX(),   
                              m_cachedSeparatingAxis.getY(),   
                              m_cachedSeparatingAxis.getZ(),   
                              squaredDistance,   
                              m_minkowskiA->getShapeType(),   
                              m_minkowskiB->getShapeType());   

                      #endif   
                      break;   

              } 


			bool check = (!m_simplexSolver->fullSimplex());
			//bool check = (!m_simplexSolver->fullSimplex() && squaredDistance > SIMD_EPSILON * m_simplexSolver->maxVertex());

			if (!check)
			{
				//do we need this backup_closest here ?
//				m_simplexSolver->backup_closest(m_cachedSeparatingAxis);
				m_degenerateSimplex = 13;
				break;
			}
		}

		if (checkSimplex)
		{
			m_simplexSolver->compute_points(pointOnA, pointOnB);
			normalInB = m_cachedSeparatingAxis;
			btScalar lenSqr =m_cachedSeparatingAxis.length2();
			
			//valid normal
			if (lenSqr < 0.0001)
			{
				m_degenerateSimplex = 5;
			} 
			if (lenSqr > SIMD_EPSILON*SIMD_EPSILON)
			{
				btScalar rlen = btScalar(1.) / btSqrt(lenSqr );
				normalInB *= rlen; //normalize
				btScalar s = btSqrt(squaredDistance);
			
				btAssert(s > btScalar(0.0));
				pointOnA -= m_cachedSeparatingAxis * (marginA / s);
				pointOnB += m_cachedSeparatingAxis * (marginB / s);
				distance = ((btScalar(1.)/rlen) - margin);
				isValid = true;
				
				m_lastUsedMethod = 1;
			} else
			{
				m_lastUsedMethod = 2;
			}
		}

		bool catchDegeneratePenetrationCase = 
			(m_catchDegeneracies && m_penetrationDepthSolver && m_degenerateSimplex && ((distance+margin) < 0.01));

		//if (checkPenetration && !isValid)
		if (checkPenetration && (!isValid || catchDegeneratePenetrationCase ))
		{
			//penetration case

			//if there is no way to handle penetrations, bail out
			if (m_penetrationDepthSolver)
			{
				// Penetration depth case.
				btVector3 tmpPointOnA,tmpPointOnB;
				
				gNumDeepPenetrationChecks++;
				m_cachedSeparatingAxis.setZero();

				bool isValid2 = m_penetrationDepthSolver->calcPenDepth( 
					*m_simplexSolver, 
					m_minkowskiA,m_minkowskiB,
					localTransA,localTransB,
					m_cachedSeparatingAxis, tmpPointOnA, tmpPointOnB,
					debugDraw,input.m_stackAlloc
					);


				if (isValid2)
				{
					btVector3 tmpNormalInB = tmpPointOnB-tmpPointOnA;
					btScalar lenSqr = tmpNormalInB.length2();
					if (lenSqr <= (SIMD_EPSILON*SIMD_EPSILON))
					{
						tmpNormalInB = m_cachedSeparatingAxis;
						lenSqr = m_cachedSeparatingAxis.length2();
					}

					if (lenSqr > (SIMD_EPSILON*SIMD_EPSILON))
					{
						tmpNormalInB /= btSqrt(lenSqr);
						btScalar distance2 = -(tmpPointOnA-tmpPointOnB).length();
						//only replace valid penetrations when the result is deeper (check)
						if (!isValid || (distance2 < distance))
						{
							distance = distance2;
							pointOnA = tmpPointOnA;
							pointOnB = tmpPointOnB;
							normalInB = tmpNormalInB;
							isValid = true;
							m_lastUsedMethod = 3;
						} else
						{
							m_lastUsedMethod = 8;
						}
					} else
					{
						m_lastUsedMethod = 9;
					}
				} else

				{
					///this is another degenerate case, where the initial GJK calculation reports a degenerate case
					///EPA reports no penetration, and the second GJK (using the supporting vector without margin)
					///reports a valid positive distance. Use the results of the second GJK instead of failing.
					///thanks to Jacob.Langford for the reproduction case
					///http://code.google.com/p/bullet/issues/detail?id=250

				
					if (m_cachedSeparatingAxis.length2() > btScalar(0.))
					{
						btScalar distance2 = (tmpPointOnA-tmpPointOnB).length()-margin;
						//only replace valid distances when the distance is less
						if (!isValid || (distance2 < distance))
						{
							distance = distance2;
							pointOnA = tmpPointOnA;
							pointOnB = tmpPointOnB;
							pointOnA -= m_cachedSeparatingAxis * marginA ;
							pointOnB += m_cachedSeparatingAxis * marginB ;
							normalInB = m_cachedSeparatingAxis;
							normalInB.normalize();
							isValid = true;
							m_lastUsedMethod = 6;
						} else
						{
							m_lastUsedMethod = 5;
						}
					}
				}
				
			}

		}
	}

	

	if (isValid && ((distance < 0) || (distance*distance < input.m_maximumDistanceSquared)))
	{
#if 0
///some debugging
//		if (check2d)
		{
			printf("n = %2.3f,%2.3f,%2.3f. ",normalInB[0],normalInB[1],normalInB[2]);
			printf("distance = %2.3f exit=%d deg=%d\n",distance,m_lastUsedMethod,m_degenerateSimplex);
		}
#endif 

		m_cachedSeparatingAxis = normalInB;
		m_cachedSeparatingDistance = distance;

		output.addContactPoint(
			normalInB,
			pointOnB+positionOffset,
			distance);

	}


}





