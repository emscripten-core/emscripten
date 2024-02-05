MSTRINGIFY(

cbuffer VSolveLinksCB : register( b0 )
{
	int startLink;
	int numLinks;
	float kst;
	int padding;
};

// Node indices for each link
StructuredBuffer<int2> g_linksVertexIndices : register( t0 );

StructuredBuffer<float> g_linksLengthRatio : register( t1 );
StructuredBuffer<float4> g_linksCurrentLength : register( t2 );
StructuredBuffer<float> g_vertexInverseMass : register( t3 );

RWStructuredBuffer<float4> g_vertexVelocity : register( u0 );

[numthreads(128, 1, 1)]
void 
VSolveLinksKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int linkID = DTid.x + startLink;
	if( DTid.x < numLinks )
	{		
		int2 nodeIndices = g_linksVertexIndices[linkID];
		int node0 = nodeIndices.x;
		int node1 = nodeIndices.y;
		
		float linkLengthRatio = g_linksLengthRatio[linkID];
		float3 linkCurrentLength = g_linksCurrentLength[linkID].xyz;
		
		float3 vertexVelocity0 = g_vertexVelocity[node0].xyz;
		float3 vertexVelocity1 = g_vertexVelocity[node1].xyz;

		float vertexInverseMass0 = g_vertexInverseMass[node0];
		float vertexInverseMass1 = g_vertexInverseMass[node1]; 

		float3 nodeDifference = vertexVelocity0 - vertexVelocity1;
		float dotResult = dot(linkCurrentLength, nodeDifference);
		float j = -dotResult*linkLengthRatio*kst;
		
		float3 velocityChange0 = linkCurrentLength*(j*vertexInverseMass0);
		float3 velocityChange1 = linkCurrentLength*(j*vertexInverseMass1);
		
		vertexVelocity0 += velocityChange0;
		vertexVelocity1 -= velocityChange1;

		g_vertexVelocity[node0] = float4(vertexVelocity0, 0.f);
		g_vertexVelocity[node1] = float4(vertexVelocity1, 0.f);
	}
}

);