MSTRINGIFY(

// Node indices for each link

//#define float3 float4

__kernel void
IntegrateKernel( 
	const int numNodes,
	const float solverdt,
	__global float * g_vertexInverseMasses,
	__global float4 * g_vertexPositions,
	__global float4 * g_vertexVelocity,
	__global float4 * g_vertexPreviousPositions,
	__global float4 * g_vertexForceAccumulator)
{
	int nodeID = get_global_id(0);
	if( nodeID < numNodes )
	{	
		float3 position   = g_vertexPositions[nodeID].xyz;
		float3 velocity   = g_vertexVelocity[nodeID].xyz;
		float3 force      = g_vertexForceAccumulator[nodeID].xyz;
		float inverseMass = g_vertexInverseMasses[nodeID];
		
		g_vertexPreviousPositions[nodeID] = (float4)(position, 0.f);
		velocity += force * inverseMass * solverdt;
		position += velocity * solverdt;
		
		g_vertexForceAccumulator[nodeID] = (float4)(0.f, 0.f, 0.f, 0.0f);
		g_vertexPositions[nodeID]        = (float4)(position, 0.f);
		g_vertexVelocity[nodeID]         = (float4)(velocity, 0.f);	
	}
}

);