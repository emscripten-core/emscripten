MSTRINGIFY(

cbuffer IntegrateCB : register( b0 )
{
	int numNodes;
	float solverdt;
	int padding1;
	int padding2;
};

// Node indices for each link
StructuredBuffer<float> g_vertexInverseMasses : register( t0 );

RWStructuredBuffer<float4> g_vertexPositions : register( u0 );
RWStructuredBuffer<float4> g_vertexVelocity : register( u1 );
RWStructuredBuffer<float4> g_vertexPreviousPositions : register( u2 );
RWStructuredBuffer<float4> g_vertexForceAccumulator : register( u3 );

[numthreads(128, 1, 1)]
void 
IntegrateKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int nodeID = DTid.x;
	if( nodeID < numNodes )
	{	
		float3 position = g_vertexPositions[nodeID].xyz;
		float3 velocity = g_vertexVelocity[nodeID].xyz;
		float3 force = g_vertexForceAccumulator[nodeID].xyz;
		float inverseMass = g_vertexInverseMasses[nodeID];
		
		g_vertexPreviousPositions[nodeID] = float4(position, 0.f);
		velocity += force * inverseMass * solverdt;
		position += velocity * solverdt;
		
		g_vertexForceAccumulator[nodeID] = float4(0.f, 0.f, 0.f, 0.0f);
		g_vertexPositions[nodeID] = float4(position, 0.f);
		g_vertexVelocity[nodeID] = float4(velocity, 0.f);	
	}
}

);