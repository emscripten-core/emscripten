
MSTRINGIFY(




__kernel void 
UpdatePositionsFromVelocitiesKernel( 
	const int numNodes,
	const float solverSDT,
	__global float4 * g_vertexVelocities,
	__global float4 * g_vertexPreviousPositions,
	__global float4 * g_vertexCurrentPosition GUID_ARG)
{
	int vertexID = get_global_id(0);
	if( vertexID < numNodes )
	{	
		float4 previousPosition = g_vertexPreviousPositions[vertexID];
		float4 velocity         = g_vertexVelocities[vertexID];
		
		float4 newPosition      = previousPosition + velocity*solverSDT;
		
		g_vertexCurrentPosition[vertexID]   = newPosition;
		g_vertexPreviousPositions[vertexID] = newPosition;
	}
}

);