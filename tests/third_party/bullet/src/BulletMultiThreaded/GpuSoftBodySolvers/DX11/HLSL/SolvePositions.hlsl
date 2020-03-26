MSTRINGIFY(

cbuffer SolvePositionsFromLinksKernelCB : register( b0 )
{
	int startLink;
	int numLinks;
	float kst;
	float ti;
};

// Node indices for each link
StructuredBuffer<int2> g_linksVertexIndices : register( t0 );

StructuredBuffer<float> g_linksMassLSC : register( t1 );
StructuredBuffer<float> g_linksRestLengthSquared : register( t2 );
StructuredBuffer<float> g_verticesInverseMass : register( t3 );

RWStructuredBuffer<float4> g_vertexPositions : register( u0 );

[numthreads(128, 1, 1)]
void 
SolvePositionsFromLinksKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int linkID = DTid.x + startLink;
	if( DTid.x < numLinks )
	{	
		float massLSC = g_linksMassLSC[linkID];
		float restLengthSquared = g_linksRestLengthSquared[linkID];
		
		if( massLSC > 0.0f )
		{		
			int2 nodeIndices = g_linksVertexIndices[linkID];
			int node0 = nodeIndices.x;
			int node1 = nodeIndices.y;
			
			float3 position0 = g_vertexPositions[node0].xyz;
			float3 position1 = g_vertexPositions[node1].xyz;

			float inverseMass0 = g_verticesInverseMass[node0];
			float inverseMass1 = g_verticesInverseMass[node1]; 

			float3 del = position1 - position0;
			float len = dot(del, del);
			float k = ((restLengthSquared - len)/(massLSC*(restLengthSquared+len)))*kst;
			position0 = position0 - del*(k*inverseMass0);
			position1 = position1 + del*(k*inverseMass1);

			g_vertexPositions[node0] = float4(position0, 0.f);
			g_vertexPositions[node1] = float4(position1, 0.f);

		}
	}
}

);