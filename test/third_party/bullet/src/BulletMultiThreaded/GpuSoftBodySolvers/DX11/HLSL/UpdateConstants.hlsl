MSTRINGIFY(

cbuffer UpdateConstantsCB : register( b0 )
{
	int numLinks;
	int padding0;
	int padding1;
	int padding2;
};

// Node indices for each link
StructuredBuffer<int2> g_linksVertexIndices : register( t0 );
StructuredBuffer<float4> g_vertexPositions : register( t1 );
StructuredBuffer<float> g_vertexInverseMasses : register( t2 );
StructuredBuffer<float> g_linksMaterialLSC : register( t3 );

RWStructuredBuffer<float> g_linksMassLSC : register( u0 );
RWStructuredBuffer<float> g_linksRestLengthSquared : register( u1 );
RWStructuredBuffer<float> g_linksRestLengths : register( u2 );

[numthreads(128, 1, 1)]
void 
UpdateConstantsKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int linkID = DTid.x;
	if( linkID < numLinks )
	{	
		int2 nodeIndices = g_linksVertexIndices[linkID];
		int node0 = nodeIndices.x;
		int node1 = nodeIndices.y;
		float linearStiffnessCoefficient = g_linksMaterialLSC[ linkID ];
		
		float3 position0 = g_vertexPositions[node0].xyz;
		float3 position1 = g_vertexPositions[node1].xyz;
		float inverseMass0 = g_vertexInverseMasses[node0];
		float inverseMass1 = g_vertexInverseMasses[node1];

		float3 difference = position0 - position1;
		float length2 = dot(difference, difference);
		float length = sqrt(length2);
	
		g_linksRestLengths[linkID] = length;
		g_linksMassLSC[linkID] = (inverseMass0 + inverseMass1)/linearStiffnessCoefficient;
		g_linksRestLengthSquared[linkID] = length*length;		
	}
}

);