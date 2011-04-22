MSTRINGIFY(
#pragma OPENCL EXTENSION cl_amd_printf : enable \n



__kernel void
ComputeBoundsKernel( 
	int numNodes,
	int numSoftBodies,
	__global int * g_vertexClothIdentifier,
	__global float4 * g_vertexPositions,
	volatile __global uint * g_clothMinBounds,
	volatile __global uint * g_clothMaxBounds,
	volatile __local uint * clothMinBounds,
	volatile __local uint * clothMaxBounds)
{
	// Init min and max bounds arrays
	if( get_local_id(0) < numSoftBodies )
	{
		
		clothMinBounds[get_local_id(0)*4] = UINT_MAX;
		clothMinBounds[get_local_id(0)*4+1] = UINT_MAX;
		clothMinBounds[get_local_id(0)*4+2] = UINT_MAX;
		clothMinBounds[get_local_id(0)*4+3] = UINT_MAX;
		clothMaxBounds[get_local_id(0)*4] = 0;
		clothMaxBounds[get_local_id(0)*4+1] = 0;
		clothMaxBounds[get_local_id(0)*4+2] = 0;
		clothMaxBounds[get_local_id(0)*4+3] = 0;

	}


	barrier(CLK_GLOBAL_MEM_FENCE);

	int nodeID = get_global_id(0);
	if( nodeID < numNodes )
	{	
		int clothIdentifier = g_vertexClothIdentifier[get_global_id(0)];
		if( clothIdentifier >= 0 )
		{
			float3 position = g_vertexPositions[get_global_id(0)].xyz;

			/* Reinterpret position as uint */
			uint3 positionUInt = (uint3)(as_uint(position.x), as_uint(position.y), as_uint(position.z));
		
			/* Invert sign bit of positives and whole of negatives to allow comparison as unsigned ints */
			positionUInt.x ^= (1+~(positionUInt.x >> 31) | 0x80000000);
			positionUInt.y ^= (1+~(positionUInt.y >> 31) | 0x80000000);		
			positionUInt.z ^= (1+~(positionUInt.z >> 31) | 0x80000000);
		
			/* Min/max with the LDS values */
			atomic_min(&(clothMinBounds[clothIdentifier*4]), positionUInt.x);
			atomic_min(&(clothMinBounds[clothIdentifier*4+1]), positionUInt.y);
			atomic_min(&(clothMinBounds[clothIdentifier*4+2]), positionUInt.z);

			atomic_max(&(clothMaxBounds[clothIdentifier*4]), positionUInt.x);
			atomic_max(&(clothMaxBounds[clothIdentifier*4+1]), positionUInt.y);
			atomic_max(&(clothMaxBounds[clothIdentifier*4+2]), positionUInt.z);
		}
	}
	
	barrier(CLK_GLOBAL_MEM_FENCE);


	/* Use global atomics to update the global versions of the data*/
	if( get_local_id(0) < numSoftBodies )
	{
		/*atomic_min(&(g_clothMinBounds[get_local_id(0)].x), clothMinBounds[get_local_id(0)].x);*/
		atomic_min(&(g_clothMinBounds[get_local_id(0)*4]), clothMinBounds[get_local_id(0)*4]);
		atomic_min(&(g_clothMinBounds[get_local_id(0)*4+1]), clothMinBounds[get_local_id(0)*4+1]);
		atomic_min(&(g_clothMinBounds[get_local_id(0)*4+2]), clothMinBounds[get_local_id(0)*4+2]);

		atomic_max(&(g_clothMaxBounds[get_local_id(0)*4]), clothMaxBounds[get_local_id(0)*4]);		
		atomic_max(&(g_clothMaxBounds[get_local_id(0)*4+1]), clothMaxBounds[get_local_id(0)*4+1]);
		atomic_max(&(g_clothMaxBounds[get_local_id(0)*4+2]), clothMaxBounds[get_local_id(0)*4+2]);
	}
}


);
