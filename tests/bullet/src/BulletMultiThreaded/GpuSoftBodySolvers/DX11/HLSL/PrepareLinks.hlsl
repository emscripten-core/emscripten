MSTRINGIFY(

cbuffer PrepareLinksCB : register( b0 )
{
	int numLinks;
	int padding0;
	int padding1;
	int padding2;
};

// Node indices for each link
StructuredBuffer<int2> g_linksVertexIndices : register( t0 );
StructuredBuffer<float> g_linksMassLSC : register( t1 );
StructuredBuffer<float4> g_nodesPreviousPosition : register( t2 );

RWStructuredBuffer<float> g_linksLengthRatio : register( u0 );
RWStructuredBuffer<float4> g_linksCurrentLength : register( u1 );

[numthreads(128, 1, 1)]
void 
PrepareLinksKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int linkID = DTid.x;
	if( linkID < numLinks )
	{	
		int2 nodeIndices = g_linksVertexIndices[linkID];
		int node0 = nodeIndices.x;
		int node1 = nodeIndices.y;
		
		float4 nodePreviousPosition0 = g_nodesPreviousPosition[node0];
		float4 nodePreviousPosition1 = g_nodesPreviousPosition[node1];

		float massLSC = g_linksMassLSC[linkID];
		
		float4 linkCurrentLength = nodePreviousPosition1 - nodePreviousPosition0;
		
		float linkLengthRatio = dot(linkCurrentLength, linkCurrentLength)*massLSC;
		linkLengthRatio = 1./linkLengthRatio;
		
		g_linksCurrentLength[linkID] = linkCurrentLength;
		g_linksLengthRatio[linkID] = linkLengthRatio;		
	}
}
);