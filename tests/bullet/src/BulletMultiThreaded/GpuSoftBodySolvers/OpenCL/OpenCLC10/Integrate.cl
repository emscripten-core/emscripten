MSTRINGIFY(

// Node indices for each link



__kernel void
IntegrateKernel( 
	const int numNodes,
	const float solverdt,
	__global float * g_vertexInverseMasses,
	__global float4 * g_vertexPositions,
	__global float4 * g_vertexVelocity,
	__global float4 * g_vertexPreviousPositions,
	__global float4 * g_vertexForceAccumulator GUID_ARG)
{
	int nodeID = get_global_id(0);
	if( nodeID < numNodes )
	{	
		float4 position   = g_vertexPositions[nodeID];
		float4 velocity   = g_vertexVelocity[nodeID];
		float4 force      = g_vertexForceAccumulator[nodeID];
		float inverseMass = g_vertexInverseMasses[nodeID];
		
		g_vertexPreviousPositions[nodeID] = position;
		velocity += force * inverseMass * solverdt;
		position += velocity * solverdt;
		
		g_vertexForceAccumulator[nodeID] = (float4)(0.f, 0.f, 0.f, 0.0f);
		g_vertexPositions[nodeID]        = position;
		g_vertexVelocity[nodeID]         = velocity;	
	}
}

);