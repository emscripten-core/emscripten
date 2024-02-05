MSTRINGIFY(

cbuffer SolvePositionsFromLinksKernelCB : register( b0 )
{
	int startWaveInBatch;
	int numWaves;
	float kst;		
	float ti;
};


// Number of batches per wavefront stored one element per logical wavefront
StructuredBuffer<int2> g_wavefrontBatchCountsVertexCounts : register( t0 );
// Set of up to maxNumVertices vertex addresses per wavefront
StructuredBuffer<int> g_vertexAddressesPerWavefront : register( t1 );

StructuredBuffer<float> g_verticesInverseMass : register( t2 );

// Per-link data layed out structured in terms of sub batches within wavefronts
StructuredBuffer<int2> g_linksVertexIndices : register( t3 );
StructuredBuffer<float> g_linksMassLSC : register( t4 );
StructuredBuffer<float> g_linksRestLengthSquared : register( t5 );

RWStructuredBuffer<float4> g_vertexPositions : register( u0 );

// Data loaded on a per-wave basis
groupshared int2 wavefrontBatchCountsVertexCounts[WAVEFRONT_BLOCK_MULTIPLIER];
groupshared float4 vertexPositionSharedData[MAX_NUM_VERTICES_PER_WAVE*WAVEFRONT_BLOCK_MULTIPLIER];
groupshared float vertexInverseMassSharedData[MAX_NUM_VERTICES_PER_WAVE*WAVEFRONT_BLOCK_MULTIPLIER];

// Storing the vertex addresses actually slowed things down a little
//groupshared int vertexAddressSharedData[MAX_NUM_VERTICES_PER_WAVE*WAVEFRONT_BLOCK_MULTIPLIER];


[numthreads(BLOCK_SIZE, 1, 1)]
void 
SolvePositionsFromLinksKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	const int laneInWavefront = (DTid.x & (WAVEFRONT_SIZE-1));
	const int wavefront = startWaveInBatch + (DTid.x / WAVEFRONT_SIZE);
	const int firstWavefrontInBlock = startWaveInBatch + Gid.x * WAVEFRONT_BLOCK_MULTIPLIER;
	const int localWavefront = wavefront - firstWavefrontInBlock;

	// Mask out in case there's a stray "wavefront" at the end that's been forced in through the multiplier	
	if( wavefront < (startWaveInBatch + numWaves) )
	{

		// Load the batch counts for the wavefronts

		int2 batchesAndVerticesWithinWavefront = g_wavefrontBatchCountsVertexCounts[wavefront];

		int batchesWithinWavefront = batchesAndVerticesWithinWavefront.x;
		int verticesUsedByWave = batchesAndVerticesWithinWavefront.y;

		// Load the vertices for the wavefronts
		for( int vertex = laneInWavefront; vertex < verticesUsedByWave; vertex+=WAVEFRONT_SIZE )
		{
			int vertexAddress = g_vertexAddressesPerWavefront[wavefront*MAX_NUM_VERTICES_PER_WAVE + vertex];

			//vertexAddressSharedData[localWavefront*MAX_NUM_VERTICES_PER_WAVE + vertex] = vertexAddress;
			vertexPositionSharedData[localWavefront*MAX_NUM_VERTICES_PER_WAVE + vertex] = g_vertexPositions[vertexAddress];
			vertexInverseMassSharedData[localWavefront*MAX_NUM_VERTICES_PER_WAVE + vertex] = g_verticesInverseMass[vertexAddress];
		}
		
		// Ensure compiler does not re-order memory operations
		AllMemoryBarrier();

		
		// Loop through the batches performing the solve on each in LDS
		int baseDataLocationForWave = WAVEFRONT_SIZE * wavefront * MAX_BATCHES_PER_WAVE;	

		//for( int batch = 0; batch < batchesWithinWavefront; ++batch )
		
		int batch = 0;
		do
		{
			int baseDataLocation = baseDataLocationForWave + WAVEFRONT_SIZE * batch;
			int locationOfValue = baseDataLocation + laneInWavefront;
			
			
			// These loads should all be perfectly linear across the WF
			int2 localVertexIndices = g_linksVertexIndices[locationOfValue];
			float massLSC = g_linksMassLSC[locationOfValue];
			float restLengthSquared = g_linksRestLengthSquared[locationOfValue];
			

			// LDS vertex addresses based on logical wavefront number in block and loaded index
			int vertexAddress0 = MAX_NUM_VERTICES_PER_WAVE * localWavefront + localVertexIndices.x;
			int vertexAddress1 = MAX_NUM_VERTICES_PER_WAVE * localWavefront + localVertexIndices.y;
			
			float3 position0 = vertexPositionSharedData[vertexAddress0].xyz;
			float3 position1 = vertexPositionSharedData[vertexAddress1].xyz;

			float inverseMass0 = vertexInverseMassSharedData[vertexAddress0];
			float inverseMass1 = vertexInverseMassSharedData[vertexAddress1]; 

			float3 del = position1 - position0;
			float len = dot(del, del);
			
			float k = 0;
			if( massLSC > 0.0f )
			{		
				k = ((restLengthSquared - len)/(massLSC*(restLengthSquared+len)))*kst;
			}
			
			position0 = position0 - del*(k*inverseMass0);
			position1 = position1 + del*(k*inverseMass1);
			
			// Ensure compiler does not re-order memory operations
			AllMemoryBarrier();				

			vertexPositionSharedData[vertexAddress0] = float4(position0, 0.f);
			vertexPositionSharedData[vertexAddress1] = float4(position1, 0.f);
			
			// Ensure compiler does not re-order memory operations
			AllMemoryBarrier();
				
			
			++batch;
		} while( batch < batchesWithinWavefront );
		
		// Update the global memory vertices for the wavefronts
		for( int vertex = laneInWavefront; vertex < verticesUsedByWave; vertex+=WAVEFRONT_SIZE )
		{
			int vertexAddress = g_vertexAddressesPerWavefront[wavefront*MAX_NUM_VERTICES_PER_WAVE + vertex];

			g_vertexPositions[vertexAddress] = vertexPositionSharedData[localWavefront*MAX_NUM_VERTICES_PER_WAVE + vertex];
		}
	}
		
}

);