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

// Shared code for GPU-based utilities

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  Keep this file free from Bullet headers
//  will be compiled by both CPU and CUDA compilers
//	file with definitions of BT_GPU_xxx should be included first
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//----------------------------------------------------------------------------------------

#include "btGpuUtilsSharedDefs.h"

//----------------------------------------------------------------------------------------

extern "C"
{

//----------------------------------------------------------------------------------------

//Round a / b to nearest higher integer value
int BT_GPU_PREF(iDivUp)(int a, int b)
{
    return (a % b != 0) ? (a / b + 1) : (a / b);
} // iDivUp()

//----------------------------------------------------------------------------------------

// compute grid and thread block size for a given number of elements
void BT_GPU_PREF(computeGridSize)(int n, int blockSize, int &numBlocks, int &numThreads)
{
    numThreads = BT_GPU_min(blockSize, n);
    numBlocks = BT_GPU_PREF(iDivUp)(n, numThreads);
} // computeGridSize()

//----------------------------------------------------------------------------------------

} // extern "C"

