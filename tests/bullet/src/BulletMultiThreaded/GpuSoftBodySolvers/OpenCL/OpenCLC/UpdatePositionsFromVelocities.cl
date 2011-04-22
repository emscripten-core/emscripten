MSTRINGIFY(

//#define float3 float4

__kernel void 
UpdatePositionsFromVelocitiesKernel( 
	const int numNodes,
	const float solverSDT,
	__global float4 * g_vertexVelocities,
	__global float4 * g_vertexPreviousPositions,
	__global float4 * g_vertexCurrentPosition)
{
	int vertexID = get_global_id(0);
	if( vertexID < numNodes )
	{	
		float3 previousPosition = g_vertexPreviousPositions[vertexID].xyz;
		float3 velocity         = g_vertexVelocities[vertexID].xyz;
		
		float3 newPosition      = previousPosition + velocity*solverSDT;
		
		g_vertexCurrentPosition[vertexID]   = (float4)(newPosition, 0.f);
		g_vertexPreviousPositions[vertexID] = (float4)(newPosition, 0.f);
	}
}

);