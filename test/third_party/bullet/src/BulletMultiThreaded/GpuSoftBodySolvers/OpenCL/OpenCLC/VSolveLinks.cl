MSTRINGIFY(

__kernel void 
VSolveLinksKernel( 
	int startLink,
	int numLinks,
	float kst,
	__global int2 * g_linksVertexIndices,
	__global float * g_linksLengthRatio,
	__global float4 * g_linksCurrentLength,
	__global float * g_vertexInverseMass,
	__global float4 * g_vertexVelocity)
{
	int linkID = get_global_id(0) + startLink;
	if( get_global_id(0) < numLinks )
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

		g_vertexVelocity[node0] = (float4)(vertexVelocity0, 0.f);
		g_vertexVelocity[node1] = (float4)(vertexVelocity1, 0.f);
	}
}

);