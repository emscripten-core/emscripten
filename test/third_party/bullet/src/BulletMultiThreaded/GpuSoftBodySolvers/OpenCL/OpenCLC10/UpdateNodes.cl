MSTRINGIFY(


__kernel void 
updateVelocitiesFromPositionsWithVelocitiesKernel( 
	int numNodes,
	float isolverdt,
	__global float4 * g_vertexPositions,
	__global float4 * g_vertexPreviousPositions,
	__global int * g_vertexClothIndices,
	__global float *g_clothVelocityCorrectionCoefficients,
	__global float * g_clothDampingFactor,
	__global float4 * g_vertexVelocities,
	__global float4 * g_vertexForces GUID_ARG)
{
	int nodeID = get_global_id(0);
	if( nodeID < numNodes )
	{	
		float4 position = g_vertexPositions[nodeID];
		float4 previousPosition = g_vertexPreviousPositions[nodeID];
		float4 velocity = g_vertexVelocities[nodeID];
		int clothIndex = g_vertexClothIndices[nodeID];
		float velocityCorrectionCoefficient = g_clothVelocityCorrectionCoefficients[clothIndex];
		float dampingFactor = g_clothDampingFactor[clothIndex];
		float velocityCoefficient = (1.f - dampingFactor);
		
		float4 difference = position - previousPosition;
				
		velocity += difference*velocityCorrectionCoefficient*isolverdt;
		
		// Damp the velocity
		velocity *= velocityCoefficient;
		
		g_vertexVelocities[nodeID] = velocity;
		g_vertexForces[nodeID] = (float4)(0.f, 0.f, 0.f, 0.f);								
	}
}

);