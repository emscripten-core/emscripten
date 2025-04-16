MSTRINGIFY(
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable\n
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable\n

__kernel void
ComputeBoundsKernel( 
	const int numNodes,
	const int numSoftBodies,
	__global int * g_vertexClothIdentifier,
	__global float4 * g_vertexPositions,
	/* Unfortunately, to get the atomics below to work these arrays cannot be */
	/* uint4, though that is the layout of the data */
	/* Therefore this is little-endian-only code */
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

	barrier(CLK_LOCAL_MEM_FENCE);

	int nodeID = get_global_id(0);
	if( nodeID < numNodes )
	{	
		int clothIdentifier = g_vertexClothIdentifier[nodeID];
		if( clothIdentifier >= 0 )
		{

			float4 position = (float4)(g_vertexPositions[nodeID].xyz, 0.f);

			/* Reinterpret position as uint */
			uint4 positionUInt = (uint4)(as_uint(position.x), as_uint(position.y), as_uint(position.z), 0);
		
			/* Invert sign bit of positives and whole of negatives to allow comparison as unsigned ints */
			positionUInt.x ^= (1+~(positionUInt.x >> 31) | 0x80000000);
			positionUInt.y ^= (1+~(positionUInt.y >> 31) | 0x80000000);		
			positionUInt.z ^= (1+~(positionUInt.z >> 31) | 0x80000000);
		
			// Min/max with the LDS values
			atom_min(&(clothMinBounds[clothIdentifier*4]), positionUInt.x);
			atom_min(&(clothMinBounds[clothIdentifier*4+1]), positionUInt.y);
			atom_min(&(clothMinBounds[clothIdentifier*4+2]), positionUInt.z);

			atom_max(&(clothMaxBounds[clothIdentifier*4]), positionUInt.x);
			atom_max(&(clothMaxBounds[clothIdentifier*4+1]), positionUInt.y);
			atom_max(&(clothMaxBounds[clothIdentifier*4+2]), positionUInt.z);
		}
	}
	
	barrier(CLK_LOCAL_MEM_FENCE);


	/* Use global atomics to update the global versions of the data */
	if( get_local_id(0) < numSoftBodies )
	{
		/*atom_min(&(g_clothMinBounds[get_local_id(0)].x), clothMinBounds[get_local_id(0)].x);*/
		atom_min(&(g_clothMinBounds[get_local_id(0)*4]), clothMinBounds[get_local_id(0)*4]);
		atom_min(&(g_clothMinBounds[get_local_id(0)*4+1]), clothMinBounds[get_local_id(0)*4+1]);
		atom_min(&(g_clothMinBounds[get_local_id(0)*4+2]), clothMinBounds[get_local_id(0)*4+2]);

		atom_max(&(g_clothMaxBounds[get_local_id(0)*4]), clothMaxBounds[get_local_id(0)*4]);		
		atom_max(&(g_clothMaxBounds[get_local_id(0)*4+1]), clothMaxBounds[get_local_id(0)*4+1]);
		atom_max(&(g_clothMaxBounds[get_local_id(0)*4+2]), clothMaxBounds[get_local_id(0)*4+2]);
	}
}


);
