MSTRINGIFY(

cbuffer UpdatePositionsFromVelocitiesCB : register( b0 )
{
	int numNodes;
	float solverSDT;
	int padding1;
	int padding2;
};


StructuredBuffer<float4> g_vertexVelocities : register( t0 );

RWStructuredBuffer<float4> g_vertexPreviousPositions : register( u0 );
RWStructuredBuffer<float4> g_vertexCurrentPosition : register( u1 );


[numthreads(128, 1, 1)]
void 
UpdatePositionsFromVelocitiesKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int vertexID = DTid.x;
	if( vertexID < numNodes )
	{	
		float3 previousPosition = g_vertexPreviousPositions[vertexID].xyz;
		float3 velocity = g_vertexVelocities[vertexID].xyz;
		
		float3 newPosition = previousPosition + velocity*solverSDT;
		
		g_vertexCurrentPosition[vertexID] = float4(newPosition, 0.f);
		g_vertexPreviousPositions[vertexID] = float4(newPosition, 0.f);
	}
}

);