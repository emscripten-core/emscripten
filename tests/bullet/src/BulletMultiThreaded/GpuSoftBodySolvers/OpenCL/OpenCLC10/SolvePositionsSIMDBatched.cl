/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

MSTRINGIFY(

float mydot3(float4 a, float4 b)
{
   return a.x*b.x + a.y*b.y + a.z*b.z;
}

__kernel __attribute__((reqd_work_group_size(WAVEFRONT_BLOCK_MULTIPLIER*WAVEFRONT_SIZE, 1, 1)))
void 
SolvePositionsFromLinksKernel( 
	const int startWaveInBatch,
	const int numWaves,
	const float kst,
	const float ti,
	__global int2 *g_wavefrontBatchCountsVertexCounts,
	__global int *g_vertexAddressesPerWavefront,
	__global int2 * g_linksVertexIndices,
	__global float * g_linksMassLSC,
	__global float * g_linksRestLengthSquared,
	__global float * g_verticesInverseMass,
	__global float4 * g_vertexPositions,
	__local int2 *wavefrontBatchCountsVertexCounts,
	__local float4 *vertexPositionSharedData,
	__local float *vertexInverseMassSharedData)
{
	const int laneInWavefront = (get_global_id(0) & (WAVEFRONT_SIZE-1));
	const int wavefront = startWaveInBatch + (get_global_id(0) / WAVEFRONT_SIZE);
	const int firstWavefrontInBlock = startWaveInBatch + get_group_id(0) * WAVEFRONT_BLOCK_MULTIPLIER;
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

			vertexPositionSharedData[localWavefront*MAX_NUM_VERTICES_PER_WAVE + vertex] = g_vertexPositions[vertexAddress];
			vertexInverseMassSharedData[localWavefront*MAX_NUM_VERTICES_PER_WAVE + vertex] = g_verticesInverseMass[vertexAddress];
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);

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
			
			float4 position0 = vertexPositionSharedData[vertexAddress0];
			float4 position1 = vertexPositionSharedData[vertexAddress1];

			float inverseMass0 = vertexInverseMassSharedData[vertexAddress0];
			float inverseMass1 = vertexInverseMassSharedData[vertexAddress1]; 

			float4 del = position1 - position0;
			float len = mydot3(del, del);
			
			float k = 0;
			if( massLSC > 0.0f )
			{		
				k = ((restLengthSquared - len)/(massLSC*(restLengthSquared+len)))*kst;
			}
			
			position0 = position0 - del*(k*inverseMass0);
			position1 = position1 + del*(k*inverseMass1);
			
			// Ensure compiler does not re-order memory operations
			barrier(CLK_LOCAL_MEM_FENCE);

			vertexPositionSharedData[vertexAddress0] = position0;
			vertexPositionSharedData[vertexAddress1] = position1;
			
			// Ensure compiler does not re-order memory operations
			barrier(CLK_LOCAL_MEM_FENCE);
				
			
			++batch;
		} while( batch < batchesWithinWavefront );

		// Update the global memory vertices for the wavefronts
		for( int vertex = laneInWavefront; vertex < verticesUsedByWave; vertex+=WAVEFRONT_SIZE )
		{
			int vertexAddress = g_vertexAddressesPerWavefront[wavefront*MAX_NUM_VERTICES_PER_WAVE + vertex];

			g_vertexPositions[vertexAddress] = (float4)(vertexPositionSharedData[localWavefront*MAX_NUM_VERTICES_PER_WAVE + vertex].xyz, 0.f);
		}		
		
	}

}

);
