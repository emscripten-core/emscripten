MSTRINGIFY(

__kernel void 
PrepareLinksKernel( 
	const int numLinks,
	__global int2 * g_linksVertexIndices,
	__global float * g_linksMassLSC,
	__global float4 * g_nodesPreviousPosition,
	__global float * g_linksLengthRatio,
	__global float4 * g_linksCurrentLength)
{
	int linkID = get_global_id(0);
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
		linkLengthRatio = 1.0f/linkLengthRatio;
		
		g_linksCurrentLength[linkID] = linkCurrentLength;
		g_linksLengthRatio[linkID]   = linkLengthRatio;		
	}
}

);