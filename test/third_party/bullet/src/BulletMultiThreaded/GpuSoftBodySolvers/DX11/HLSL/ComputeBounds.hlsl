MSTRINGIFY(

cbuffer ComputeBoundsCB : register( b0 )
{
	int numNodes;
	int numSoftBodies;
	int padding1;
	int padding2;
};

// Node indices for each link
StructuredBuffer<int> g_vertexClothIdentifier : register( t0 );
StructuredBuffer<float4> g_vertexPositions : register( t1 );

RWStructuredBuffer<uint4> g_clothMinBounds : register( u0 );
RWStructuredBuffer<uint4> g_clothMaxBounds : register( u1 );

groupshared uint4 clothMinBounds[256];
groupshared uint4 clothMaxBounds[256];

[numthreads(128, 1, 1)]
void 
ComputeBoundsKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	const unsigned int UINT_MAX = 0xffffffff;

	// Init min and max bounds arrays
	if( GTid.x < numSoftBodies )
	{
		clothMinBounds[GTid.x] = uint4(UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX);
		clothMaxBounds[GTid.x] = uint4(0,0,0,0);
	}

	AllMemoryBarrierWithGroupSync();

	int nodeID = DTid.x;
	if( nodeID < numNodes )
	{	
		int clothIdentifier = g_vertexClothIdentifier[nodeID];
		if( clothIdentifier >= 0 )
		{
			float3 position = g_vertexPositions[nodeID].xyz;

			// Reinterpret position as uint
			uint3 positionUInt = uint3(asuint(position.x), asuint(position.y), asuint(position.z));
		
			// Invert sign bit of positives and whole of negatives to allow comparison as unsigned ints
			//positionUInt.x ^= uint((-int(positionUInt.x >> 31) | 0x80000000));
			//positionUInt.y ^= uint((-int(positionUInt.y >> 31) | 0x80000000));
			//positionUInt.z ^= uint((-int(positionUInt.z >> 31) | 0x80000000));
			positionUInt.x ^= (1+~(positionUInt.x >> 31) | 0x80000000);
			positionUInt.y ^= (1+~(positionUInt.y >> 31) | 0x80000000);		
			positionUInt.z ^= (1+~(positionUInt.z >> 31) | 0x80000000);
		
			// Min/max with the LDS values
			InterlockedMin(clothMinBounds[clothIdentifier].x, positionUInt.x);
			InterlockedMin(clothMinBounds[clothIdentifier].y, positionUInt.y);
			InterlockedMin(clothMinBounds[clothIdentifier].z, positionUInt.z);

			InterlockedMax(clothMaxBounds[clothIdentifier].x, positionUInt.x);
			InterlockedMax(clothMaxBounds[clothIdentifier].y, positionUInt.y);
			InterlockedMax(clothMaxBounds[clothIdentifier].z, positionUInt.z);
		}
	}
	
	AllMemoryBarrierWithGroupSync();


	// Use global atomics to update the global versions of the data
	if( GTid.x < numSoftBodies )
	{
		InterlockedMin(g_clothMinBounds[GTid.x].x, clothMinBounds[GTid.x].x);
		InterlockedMin(g_clothMinBounds[GTid.x].y, clothMinBounds[GTid.x].y);
		InterlockedMin(g_clothMinBounds[GTid.x].z, clothMinBounds[GTid.x].z);

		InterlockedMax(g_clothMaxBounds[GTid.x].x, clothMaxBounds[GTid.x].x);		
		InterlockedMax(g_clothMaxBounds[GTid.x].y, clothMaxBounds[GTid.x].y);
		InterlockedMax(g_clothMaxBounds[GTid.x].z, clothMaxBounds[GTid.x].z);
	}
}


);