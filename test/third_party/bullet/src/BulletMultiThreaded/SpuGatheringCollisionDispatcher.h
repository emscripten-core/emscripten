/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://bulletphysics.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#ifndef BT_SPU_GATHERING_COLLISION__DISPATCHER_H
#define BT_SPU_GATHERING_COLLISION__DISPATCHER_H

#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"


///Tuning value to optimized SPU utilization 
///Too small value means Task overhead is large compared to computation (too fine granularity)
///Too big value might render some SPUs are idle, while a few other SPUs are doing all work.
//#define SPU_BATCHSIZE_BROADPHASE_PAIRS 8
//#define SPU_BATCHSIZE_BROADPHASE_PAIRS 16
//#define SPU_BATCHSIZE_BROADPHASE_PAIRS 64
#define SPU_BATCHSIZE_BROADPHASE_PAIRS 128
//#define SPU_BATCHSIZE_BROADPHASE_PAIRS 256
//#define SPU_BATCHSIZE_BROADPHASE_PAIRS 512
//#define SPU_BATCHSIZE_BROADPHASE_PAIRS 1024



class SpuCollisionTaskProcess;

///SpuGatheringCollisionDispatcher can use SPU to gather and calculate collision detection
///Time of Impact, Closest Points and Penetration Depth.
class SpuGatheringCollisionDispatcher : public btCollisionDispatcher
{
	
	SpuCollisionTaskProcess*	m_spuCollisionTaskProcess;
	
protected:

	class	btThreadSupportInterface*	m_threadInterface;

	unsigned int	m_maxNumOutstandingTasks;
	

public:

	//can be used by SPU collision algorithms	
	SpuCollisionTaskProcess*	getSpuCollisionTaskProcess()
	{
			return m_spuCollisionTaskProcess;
	}
	
	SpuGatheringCollisionDispatcher (class	btThreadSupportInterface*	threadInterface, unsigned int	maxNumOutstandingTasks,btCollisionConfiguration* collisionConfiguration);
	
	virtual ~SpuGatheringCollisionDispatcher();

	bool	supportsDispatchPairOnSpu(int proxyType0,int proxyType1);

	virtual void	dispatchAllCollisionPairs(btOverlappingPairCache* pairCache,const btDispatcherInfo& dispatchInfo,btDispatcher* dispatcher) ;

};



#endif //BT_SPU_GATHERING_COLLISION__DISPATCHER_H


