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

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//               K E R N E L    F U N C T I O N S 
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

// calculate position in uniform grid
BT_GPU___device__ int3 bt3DGrid_calcGridPos(float4 p)
{
    int3 gridPos;
    gridPos.x = (int)floor((p.x - BT_GPU_params.m_worldOriginX) / BT_GPU_params.m_cellSizeX);
    gridPos.y = (int)floor((p.y - BT_GPU_params.m_worldOriginY) / BT_GPU_params.m_cellSizeY);
    gridPos.z = (int)floor((p.z - BT_GPU_params.m_worldOriginZ) / BT_GPU_params.m_cellSizeZ);
    return gridPos;
} // bt3DGrid_calcGridPos()

//----------------------------------------------------------------------------------------

// calculate address in grid from position (clamping to edges)
BT_GPU___device__ uint bt3DGrid_calcGridHash(int3 gridPos)
{
    gridPos.x = BT_GPU_max(0, BT_GPU_min(gridPos.x, (int)BT_GPU_params.m_gridSizeX - 1));
    gridPos.y = BT_GPU_max(0, BT_GPU_min(gridPos.y, (int)BT_GPU_params.m_gridSizeY - 1));
    gridPos.z = BT_GPU_max(0, BT_GPU_min(gridPos.z, (int)BT_GPU_params.m_gridSizeZ - 1));
    return BT_GPU___mul24(BT_GPU___mul24(gridPos.z, BT_GPU_params.m_gridSizeY), BT_GPU_params.m_gridSizeX) + BT_GPU___mul24(gridPos.y, BT_GPU_params.m_gridSizeX) + gridPos.x;
} // bt3DGrid_calcGridHash()

//----------------------------------------------------------------------------------------

// calculate grid hash value for each body using its AABB
BT_GPU___global__ void calcHashAABBD(bt3DGrid3F1U* pAABB, uint2* pHash, uint numBodies)
{
    int index = BT_GPU___mul24(BT_GPU_blockIdx.x, BT_GPU_blockDim.x) + BT_GPU_threadIdx.x;
    if(index >= (int)numBodies)
	{
		return;
	}
	bt3DGrid3F1U bbMin = pAABB[index*2];
	bt3DGrid3F1U bbMax = pAABB[index*2 + 1];
	float4 pos;
	pos.x = (bbMin.fx + bbMax.fx) * 0.5f;
	pos.y = (bbMin.fy + bbMax.fy) * 0.5f;
	pos.z = (bbMin.fz + bbMax.fz) * 0.5f;
    // get address in grid
    int3 gridPos = bt3DGrid_calcGridPos(pos);
    uint gridHash = bt3DGrid_calcGridHash(gridPos);
    // store grid hash and body index
    pHash[index] = BT_GPU_make_uint2(gridHash, index);
} // calcHashAABBD()

//----------------------------------------------------------------------------------------

BT_GPU___global__ void findCellStartD(uint2* pHash, uint* cellStart, uint numBodies)
{
    int index = BT_GPU___mul24(BT_GPU_blockIdx.x, BT_GPU_blockDim.x) + BT_GPU_threadIdx.x;
    if(index >= (int)numBodies)
	{
		return;
	}
    uint2 sortedData = pHash[index];
	// Load hash data into shared memory so that we can look 
	// at neighboring body's hash value without loading
	// two hash values per thread
	BT_GPU___shared__ uint sharedHash[257];
	sharedHash[BT_GPU_threadIdx.x+1] = sortedData.x;
	if((index > 0) && (BT_GPU_threadIdx.x == 0))
	{
		// first thread in block must load neighbor body hash
		volatile uint2 prevData = pHash[index-1];
		sharedHash[0] = prevData.x;
	}
	BT_GPU___syncthreads();
	if((index == 0) || (sortedData.x != sharedHash[BT_GPU_threadIdx.x]))
	{
		cellStart[sortedData.x] = index;
	}
} // findCellStartD()

//----------------------------------------------------------------------------------------

BT_GPU___device__ uint cudaTestAABBOverlap(bt3DGrid3F1U min0, bt3DGrid3F1U max0, bt3DGrid3F1U min1, bt3DGrid3F1U max1)
{
	return	(min0.fx <= max1.fx)&& (min1.fx <= max0.fx) && 
			(min0.fy <= max1.fy)&& (min1.fy <= max0.fy) && 
			(min0.fz <= max1.fz)&& (min1.fz <= max0.fz); 
} // cudaTestAABBOverlap()
 
//----------------------------------------------------------------------------------------

BT_GPU___device__ void findPairsInCell(	int3	gridPos,
										uint    index,
										uint2*  pHash,
										uint*   pCellStart,
										bt3DGrid3F1U* pAABB, 
										uint*   pPairBuff,
										uint2*	pPairBuffStartCurr,
										uint	numBodies)
{
    if (	(gridPos.x < 0) || (gridPos.x > (int)BT_GPU_params.m_gridSizeX - 1)
		||	(gridPos.y < 0) || (gridPos.y > (int)BT_GPU_params.m_gridSizeY - 1)
		||  (gridPos.z < 0) || (gridPos.z > (int)BT_GPU_params.m_gridSizeZ - 1)) 
    {
		return;
	}
    uint gridHash = bt3DGrid_calcGridHash(gridPos);
    // get start of bucket for this cell
    uint bucketStart = pCellStart[gridHash];
    if (bucketStart == 0xffffffff)
	{
        return;   // cell empty
	}
	// iterate over bodies in this cell
    uint2 sortedData = pHash[index];
	uint unsorted_indx = sortedData.y;
    bt3DGrid3F1U min0 = BT_GPU_FETCH(pAABB, unsorted_indx*2); 
	bt3DGrid3F1U max0 = BT_GPU_FETCH(pAABB, unsorted_indx*2 + 1);
	uint handleIndex =  min0.uw;
	uint2 start_curr = pPairBuffStartCurr[handleIndex];
	uint start = start_curr.x;
	uint curr = start_curr.y;
	uint2 start_curr_next = pPairBuffStartCurr[handleIndex+1];
	uint curr_max = start_curr_next.x - start - 1;
	uint bucketEnd = bucketStart + BT_GPU_params.m_maxBodiesPerCell;
	bucketEnd = (bucketEnd > numBodies) ? numBodies : bucketEnd;
	for(uint index2 = bucketStart; index2 < bucketEnd; index2++) 
	{
        uint2 cellData = pHash[index2];
        if (cellData.x != gridHash)
        {
			break;   // no longer in same bucket
		}
		uint unsorted_indx2 = cellData.y;
        if (unsorted_indx2 < unsorted_indx) // check not colliding with self
        {   
			bt3DGrid3F1U min1 = BT_GPU_FETCH(pAABB, unsorted_indx2*2);
			bt3DGrid3F1U max1 = BT_GPU_FETCH(pAABB, unsorted_indx2*2 + 1);
			if(cudaTestAABBOverlap(min0, max0, min1, max1))
			{
				uint handleIndex2 = min1.uw;
				uint k;
				for(k = 0; k < curr; k++)
				{
					uint old_pair = pPairBuff[start+k] & (~BT_3DGRID_PAIR_ANY_FLG);
					if(old_pair == handleIndex2)
					{
						pPairBuff[start+k] |= BT_3DGRID_PAIR_FOUND_FLG;
						break;
					}
				}
				if(k == curr)
				{
					if(curr >= curr_max) 
					{ // not a good solution, but let's avoid crash
						break;
					}
					pPairBuff[start+curr] = handleIndex2 | BT_3DGRID_PAIR_NEW_FLG;
					curr++;
				}
			}
		}
	}
	pPairBuffStartCurr[handleIndex] = BT_GPU_make_uint2(start, curr);
    return;
} // findPairsInCell()

//----------------------------------------------------------------------------------------

BT_GPU___global__ void findOverlappingPairsD(	bt3DGrid3F1U*	pAABB, uint2* pHash, uint* pCellStart, 
												uint* pPairBuff, uint2* pPairBuffStartCurr, uint numBodies)
{
    int index = BT_GPU___mul24(BT_GPU_blockIdx.x, BT_GPU_blockDim.x) + BT_GPU_threadIdx.x;
    if(index >= (int)numBodies)
	{
		return;
	}
    uint2 sortedData = pHash[index];
	uint unsorted_indx = sortedData.y;
	bt3DGrid3F1U bbMin = BT_GPU_FETCH(pAABB, unsorted_indx*2);
	bt3DGrid3F1U bbMax = BT_GPU_FETCH(pAABB, unsorted_indx*2 + 1);
	float4 pos;
	pos.x = (bbMin.fx + bbMax.fx) * 0.5f;
	pos.y = (bbMin.fy + bbMax.fy) * 0.5f;
	pos.z = (bbMin.fz + bbMax.fz) * 0.5f;
    // get address in grid
    int3 gridPos = bt3DGrid_calcGridPos(pos);
    // examine only neighbouring cells
    for(int z=-1; z<=1; z++) {
        for(int y=-1; y<=1; y++) {
            for(int x=-1; x<=1; x++) {
                findPairsInCell(gridPos + BT_GPU_make_int3(x, y, z), index, pHash, pCellStart, pAABB, pPairBuff, pPairBuffStartCurr, numBodies);
            }
        }
    }
} // findOverlappingPairsD()

//----------------------------------------------------------------------------------------

BT_GPU___global__ void findPairsLargeD(	bt3DGrid3F1U* pAABB, uint2* pHash, uint* pCellStart, uint* pPairBuff, 
										uint2* pPairBuffStartCurr, uint numBodies, uint numLarge)
{
    int index = BT_GPU___mul24(BT_GPU_blockIdx.x, BT_GPU_blockDim.x) + BT_GPU_threadIdx.x;
    if(index >= (int)numBodies)
	{
		return;
	}
    uint2 sortedData = pHash[index];
	uint unsorted_indx = sortedData.y;
	bt3DGrid3F1U min0 = BT_GPU_FETCH(pAABB, unsorted_indx*2);
	bt3DGrid3F1U max0 = BT_GPU_FETCH(pAABB, unsorted_indx*2 + 1);
	uint handleIndex =  min0.uw;
	uint2 start_curr = pPairBuffStartCurr[handleIndex];
	uint start = start_curr.x;
	uint curr = start_curr.y;
	uint2 start_curr_next = pPairBuffStartCurr[handleIndex+1];
	uint curr_max = start_curr_next.x - start - 1;
    for(uint i = 0; i < numLarge; i++)
    {
		uint indx2 = numBodies + i;
		bt3DGrid3F1U min1 = BT_GPU_FETCH(pAABB, indx2*2);
		bt3DGrid3F1U max1 = BT_GPU_FETCH(pAABB, indx2*2 + 1);
		if(cudaTestAABBOverlap(min0, max0, min1, max1))
		{
			uint k;
			uint handleIndex2 =  min1.uw;
			for(k = 0; k < curr; k++)
			{
				uint old_pair = pPairBuff[start+k] & (~BT_3DGRID_PAIR_ANY_FLG);
				if(old_pair == handleIndex2)
				{
					pPairBuff[start+k] |= BT_3DGRID_PAIR_FOUND_FLG;
					break;
				}
			}
			if(k == curr)
			{
				pPairBuff[start+curr] = handleIndex2 | BT_3DGRID_PAIR_NEW_FLG;
				if(curr >= curr_max) 
				{ // not a good solution, but let's avoid crash
					break;
				}
				curr++;
			}
		}
    }
	pPairBuffStartCurr[handleIndex] = BT_GPU_make_uint2(start, curr);
    return;
} // findPairsLargeD()

//----------------------------------------------------------------------------------------

BT_GPU___global__ void computePairCacheChangesD(uint* pPairBuff, uint2* pPairBuffStartCurr, 
												uint* pPairScan, bt3DGrid3F1U* pAABB, uint numBodies)
{
    int index = BT_GPU___mul24(BT_GPU_blockIdx.x, BT_GPU_blockDim.x) + BT_GPU_threadIdx.x;
    if(index >= (int)numBodies)
	{
		return;
	}
	bt3DGrid3F1U bbMin = pAABB[index * 2];
	uint handleIndex = bbMin.uw;
	uint2 start_curr = pPairBuffStartCurr[handleIndex];
	uint start = start_curr.x;
	uint curr = start_curr.y;
	uint *pInp = pPairBuff + start;
	uint num_changes = 0;
	for(uint k = 0; k < curr; k++, pInp++)
	{
		if(!((*pInp) & BT_3DGRID_PAIR_FOUND_FLG))
		{
			num_changes++;
		}
	}
	pPairScan[index+1] = num_changes;
} // computePairCacheChangesD()

//----------------------------------------------------------------------------------------

BT_GPU___global__ void squeezeOverlappingPairBuffD(uint* pPairBuff, uint2* pPairBuffStartCurr, uint* pPairScan,
												   uint* pPairOut, bt3DGrid3F1U* pAABB, uint numBodies)
{
    int index = BT_GPU___mul24(BT_GPU_blockIdx.x, BT_GPU_blockDim.x) + BT_GPU_threadIdx.x;
    if(index >= (int)numBodies)
	{
		return;
	}
	bt3DGrid3F1U bbMin = pAABB[index * 2];
	uint handleIndex = bbMin.uw;
	uint2 start_curr = pPairBuffStartCurr[handleIndex];
	uint start = start_curr.x;
	uint curr = start_curr.y;
	uint* pInp = pPairBuff + start;
	uint* pOut = pPairOut + pPairScan[index];
	uint* pOut2 = pInp;
	uint num = 0; 
	for(uint k = 0; k < curr; k++, pInp++)
	{
		if(!((*pInp) & BT_3DGRID_PAIR_FOUND_FLG))
		{
			*pOut = *pInp;
			pOut++;
		}
		if((*pInp) & BT_3DGRID_PAIR_ANY_FLG)
		{
			*pOut2 = (*pInp) & (~BT_3DGRID_PAIR_ANY_FLG);
			pOut2++;
			num++;
		}
	}
	pPairBuffStartCurr[handleIndex] = BT_GPU_make_uint2(start, num);
} // squeezeOverlappingPairBuffD()


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//               E N D   O F    K E R N E L    F U N C T I O N S 
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

extern "C"
{

//----------------------------------------------------------------------------------------

void BT_GPU_PREF(calcHashAABB)(bt3DGrid3F1U* pAABB, unsigned int* hash,	unsigned int numBodies)
{
    int numThreads, numBlocks;
    BT_GPU_PREF(computeGridSize)(numBodies, 256, numBlocks, numThreads);
    // execute the kernel
    BT_GPU_EXECKERNEL(numBlocks, numThreads, calcHashAABBD, (pAABB, (uint2*)hash, numBodies));
    // check if kernel invocation generated an error
    BT_GPU_CHECK_ERROR("calcHashAABBD kernel execution failed");
} // calcHashAABB()

//----------------------------------------------------------------------------------------

void BT_GPU_PREF(findCellStart(unsigned int* hash, unsigned int* cellStart, unsigned int numBodies, unsigned int numCells))
{
    int numThreads, numBlocks;
    BT_GPU_PREF(computeGridSize)(numBodies, 256, numBlocks, numThreads);
	BT_GPU_SAFE_CALL(BT_GPU_Memset(cellStart, 0xffffffff, numCells*sizeof(uint)));
	BT_GPU_EXECKERNEL(numBlocks, numThreads, findCellStartD, ((uint2*)hash, (uint*)cellStart, numBodies));
    BT_GPU_CHECK_ERROR("Kernel execution failed: findCellStartD");
} // findCellStart()

//----------------------------------------------------------------------------------------

void BT_GPU_PREF(findOverlappingPairs(bt3DGrid3F1U* pAABB, unsigned int* pHash,	unsigned int* pCellStart, unsigned int*	pPairBuff, unsigned int*	pPairBuffStartCurr, unsigned int	numBodies))
{
#if B_CUDA_USE_TEX
    BT_GPU_SAFE_CALL(cudaBindTexture(0, pAABBTex, pAABB, numBodies * 2 * sizeof(bt3DGrid3F1U)));
#endif
    int numThreads, numBlocks;
    BT_GPU_PREF(computeGridSize)(numBodies, 64, numBlocks, numThreads);
    BT_GPU_EXECKERNEL(numBlocks, numThreads, findOverlappingPairsD, (pAABB,(uint2*)pHash,(uint*)pCellStart,(uint*)pPairBuff,(uint2*)pPairBuffStartCurr,numBodies));
    BT_GPU_CHECK_ERROR("Kernel execution failed: bt_CudaFindOverlappingPairsD");
#if B_CUDA_USE_TEX
    BT_GPU_SAFE_CALL(cudaUnbindTexture(pAABBTex));
#endif
} // findOverlappingPairs()

//----------------------------------------------------------------------------------------

void BT_GPU_PREF(findPairsLarge(bt3DGrid3F1U* pAABB, unsigned int* pHash, unsigned int* pCellStart, unsigned int* pPairBuff, unsigned int* pPairBuffStartCurr, unsigned int numBodies, unsigned int numLarge))
{
#if B_CUDA_USE_TEX
    BT_GPU_SAFE_CALL(cudaBindTexture(0, pAABBTex, pAABB, (numBodies+numLarge) * 2 * sizeof(bt3DGrid3F1U)));
#endif
    int numThreads, numBlocks;
    BT_GPU_PREF(computeGridSize)(numBodies, 64, numBlocks, numThreads);
    BT_GPU_EXECKERNEL(numBlocks, numThreads, findPairsLargeD, (pAABB,(uint2*)pHash,(uint*)pCellStart,(uint*)pPairBuff,(uint2*)pPairBuffStartCurr,numBodies,numLarge));
    BT_GPU_CHECK_ERROR("Kernel execution failed: btCuda_findPairsLargeD");
#if B_CUDA_USE_TEX
    BT_GPU_SAFE_CALL(cudaUnbindTexture(pAABBTex));
#endif
} // findPairsLarge()

//----------------------------------------------------------------------------------------

void BT_GPU_PREF(computePairCacheChanges(unsigned int* pPairBuff, unsigned int* pPairBuffStartCurr, unsigned int* pPairScan, bt3DGrid3F1U* pAABB, unsigned int numBodies))
{
    int numThreads, numBlocks;
    BT_GPU_PREF(computeGridSize)(numBodies, 256, numBlocks, numThreads);
    BT_GPU_EXECKERNEL(numBlocks, numThreads, computePairCacheChangesD, ((uint*)pPairBuff,(uint2*)pPairBuffStartCurr,(uint*)pPairScan,pAABB,numBodies));
    BT_GPU_CHECK_ERROR("Kernel execution failed: btCudaComputePairCacheChangesD");
} // computePairCacheChanges()

//----------------------------------------------------------------------------------------

void BT_GPU_PREF(squeezeOverlappingPairBuff(unsigned int* pPairBuff, unsigned int* pPairBuffStartCurr, unsigned int* pPairScan, unsigned int* pPairOut, bt3DGrid3F1U* pAABB, unsigned int numBodies))
{
    int numThreads, numBlocks;
    BT_GPU_PREF(computeGridSize)(numBodies, 256, numBlocks, numThreads);
    BT_GPU_EXECKERNEL(numBlocks, numThreads, squeezeOverlappingPairBuffD, ((uint*)pPairBuff,(uint2*)pPairBuffStartCurr,(uint*)pPairScan,(uint*)pPairOut,pAABB,numBodies));
    BT_GPU_CHECK_ERROR("Kernel execution failed: btCudaSqueezeOverlappingPairBuffD");
} // btCuda_squeezeOverlappingPairBuff()

//------------------------------------------------------------------------------------------------

} // extern "C"

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
