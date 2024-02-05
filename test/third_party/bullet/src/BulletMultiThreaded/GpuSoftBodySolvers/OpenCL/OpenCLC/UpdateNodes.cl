MSTRINGIFY(

//#define float3 float4

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
	__global float4 * g_vertexForces)
{
	int nodeID = get_global_id(0);
	if( nodeID < numNodes )
	{	
		float3 position = g_vertexPositions[nodeID].xyz;
		float3 previousPosition = g_vertexPreviousPositions[nodeID].xyz;
		float3 velocity = g_vertexVelocities[nodeID].xyz;
		int clothIndex = g_vertexClothIndices[nodeID];
		float velocityCorrectionCoefficient = g_clothVelocityCorrectionCoefficients[clothIndex];
		float dampingFactor = g_clothDampingFactor[clothIndex];
		float velocityCoefficient = (1.f - dampingFactor);
		
		float3 difference = position - previousPosition;
				
		velocity += difference*velocityCorrectionCoefficient*isolverdt;
		
		// Damp the velocity
		velocity *= velocityCoefficient;
		
		g_vertexVelocities[nodeID] = (float4)(velocity, 0.f);
		g_vertexForces[nodeID] = (float4)(0.f, 0.f, 0.f, 0.f);								
	}
}

);