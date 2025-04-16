MSTRINGIFY(

cbuffer UpdateVelocitiesFromPositionsWithVelocitiesCB : register( b0 )
{
	int numNodes;
	float isolverdt;
	int padding1;
	int padding2;
};


StructuredBuffer<float4> g_vertexPositions : register( t0 );
StructuredBuffer<float4> g_vertexPreviousPositions : register( t1 );
StructuredBuffer<int> g_vertexClothIndices : register( t2 );
StructuredBuffer<float> g_clothVelocityCorrectionCoefficients : register( t3 );
StructuredBuffer<float> g_clothDampingFactor : register( t4 );

RWStructuredBuffer<float4> g_vertexVelocities : register( u0 );
RWStructuredBuffer<float4> g_vertexForces : register( u1 );


[numthreads(128, 1, 1)]
void 
updateVelocitiesFromPositionsWithVelocitiesKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int nodeID = DTid.x;
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
		
		g_vertexVelocities[nodeID] = float4(velocity, 0.f);
		g_vertexForces[nodeID] = float4(0.f, 0.f, 0.f, 0.f);								
	}
}

);