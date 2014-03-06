/*
Bullet Continuous Collision Detection and Physics Library, http://bulletphysics.org
Copyright (C) 2006, 2007 Sony Computer Entertainment Inc. 

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

// Shared definitions for GPU-based utilities

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  Keep this file free from Bullet headers
//  it is included into both CUDA and CPU code
//	file with definitions of BT_GPU_xxx should be included first
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


#ifndef BTGPUUTILSDHAREDDEFS_H
#define BTGPUUTILSDHAREDDEFS_H


extern "C"
{


//Round a / b to nearest higher integer value
int BT_GPU_PREF(iDivUp)(int a, int b);

// compute grid and thread block size for a given number of elements
void BT_GPU_PREF(computeGridSize)(int n, int blockSize, int &numBlocks, int &numThreads);

void BT_GPU_PREF(allocateArray)(void** devPtr, unsigned int size);
void BT_GPU_PREF(freeArray)(void* devPtr);
void BT_GPU_PREF(copyArrayFromDevice)(void* host, const void* device, unsigned int size);
void BT_GPU_PREF(copyArrayToDevice)(void* device, const void* host, unsigned int size);
void BT_GPU_PREF(registerGLBufferObject(unsigned int vbo));
void* BT_GPU_PREF(mapGLBufferObject(unsigned int vbo));
void BT_GPU_PREF(unmapGLBufferObject(unsigned int vbo));


} // extern "C"


#endif // BTGPUUTILSDHAREDDEFS_H

