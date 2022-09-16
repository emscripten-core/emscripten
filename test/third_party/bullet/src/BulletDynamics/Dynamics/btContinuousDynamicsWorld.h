/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_CONTINUOUS_DYNAMICS_WORLD_H
#define BT_CONTINUOUS_DYNAMICS_WORLD_H

#include "btDiscreteDynamicsWorld.h"

///btContinuousDynamicsWorld adds optional (per object) continuous collision detection for fast moving objects to the btDiscreteDynamicsWorld.
///This copes with fast moving objects that otherwise would tunnel/miss collisions.
///Under construction, don't use yet! Please use btDiscreteDynamicsWorld instead.
class btContinuousDynamicsWorld : public btDiscreteDynamicsWorld
{

	void	updateTemporalAabbs(btScalar timeStep);

	public:

		btContinuousDynamicsWorld(btDispatcher* dispatcher,btBroadphaseInterface* pairCache,btConstraintSolver* constraintSolver,btCollisionConfiguration* collisionConfiguration);
		virtual ~btContinuousDynamicsWorld();
		
		///time stepping with calculation of time of impact for selected fast moving objects
		virtual void	internalSingleStepSimulation( btScalar timeStep);

		virtual void	calculateTimeOfImpacts(btScalar timeStep);

		virtual btDynamicsWorldType	getWorldType() const
		{
			return BT_CONTINUOUS_DYNAMICS_WORLD;
		}

};

#endif //BT_CONTINUOUS_DYNAMICS_WORLD_H
