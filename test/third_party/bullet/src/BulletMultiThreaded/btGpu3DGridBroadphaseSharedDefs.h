/*
Bullet Continuous Collision Detection and Physics Library, http://bulletphysics.org
Copyright (C) 2006, 2009 Sony Computer Entertainment Inc. 

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

//----------------------------------------------------------------------------------------

// Shared definitions for GPU-based 3D Grid collision detection broadphase

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  Keep this file free from Bullet headers
//  it is included into both CUDA and CPU code
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//----------------------------------------------------------------------------------------

#ifndef BTGPU3DGRIDBROADPHASESHAREDDEFS_H
#define BTGPU3DGRIDBROADPHASESHAREDDEFS_H

//----------------------------------------------------------------------------------------

#include "btGpu3DGridBroadphaseSharedTypes.h"

//----------------------------------------------------------------------------------------

extern "C"
{

//----------------------------------------------------------------------------------------

void BT_GPU_PREF(calcHashAABB)(bt3DGrid3F1U* pAABB, unsigned int* hash,	unsigned int numBodies);

void BT_GPU_PREF(findCellStart)(unsigned int* hash, unsigned int* cellStart, unsigned int numBodies, unsigned int numCells);

void BT_GPU_PREF(findOverlappingPairs)(bt3DGrid3F1U* pAABB, unsigned int* pHash,	unsigned int* pCellStart, unsigned int*	pPairBuff, unsigned int*	pPairBuffStartCurr, unsigned int	numBodies);

void BT_GPU_PREF(findPairsLarge)(bt3DGrid3F1U* pAABB, unsigned int* pHash, unsigned int* pCellStart, unsigned int* pPairBuff, unsigned int* pPairBuffStartCurr, unsigned int numBodies, unsigned int numLarge);

void BT_GPU_PREF(computePairCacheChanges)(unsigned int* pPairBuff, unsigned int* pPairBuffStartCurr, unsigned int* pPairScan, bt3DGrid3F1U* pAABB, unsigned int numBodies);

void BT_GPU_PREF(squeezeOverlappingPairBuff)(unsigned int* pPairBuff, unsigned int* pPairBuffStartCurr, unsigned int* pPairScan, unsigned int* pPairOut, bt3DGrid3F1U* pAABB, unsigned int numBodies);


//----------------------------------------------------------------------------------------

} // extern "C"

//----------------------------------------------------------------------------------------

#endif // BTGPU3DGRIDBROADPHASESHAREDDEFS_H

