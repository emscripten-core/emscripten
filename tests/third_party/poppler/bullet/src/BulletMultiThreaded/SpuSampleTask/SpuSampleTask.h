/*
Bullet Continuous Collision Detection and Physics Library, Copyright (c) 2007 Erwin Coumans

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/

#ifndef SPU_SAMPLE_TASK_H
#define SPU_SAMPLE_TASK_H

#include "../PlatformDefinitions.h"
#include "LinearMath/btScalar.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"

#include "LinearMath/btAlignedAllocator.h"


enum
{
	CMD_SAMPLE_INTEGRATE_BODIES = 1,
	CMD_SAMPLE_PREDICT_MOTION_BODIES
};



ATTRIBUTE_ALIGNED16(struct) SpuSampleTaskDesc
{
	BT_DECLARE_ALIGNED_ALLOCATOR();

	uint32_t						m_sampleCommand;
	uint32_t						m_taskId;

	uint64_t 	m_mainMemoryPtr;
	int			m_sampleValue;
	

};


void	processSampleTask(void* userPtr, void* lsMemory);
void*	createSampleLocalStoreMemory();


#endif //SPU_SAMPLE_TASK_H

