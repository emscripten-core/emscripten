MSTRINGIFY(

/*#define float3 float4

float dot3(float3 a, float3 b)
{
   return a.x*b.x + a.y*b.y + a.z*b.z;
}*/

__kernel void 
UpdateConstantsKernel( 
	const int numLinks,
	__global int2 * g_linksVertexIndices,
	__global float4 * g_vertexPositions,
	__global float * g_vertexInverseMasses,
	__global float * g_linksMaterialLSC,
	__global float * g_linksMassLSC,
	__global float * g_linksRestLengthSquared,
	__global float * g_linksRestLengths)
{
	int linkID = get_global_id(0);
	if( linkID < numLinks )
	{	
		int2 nodeIndices = g_linksVertexIndices[linkID];
		int node0 = nodeIndices.x;
		int node1 = nodeIndices.y;
		float linearStiffnessCoefficient = g_linksMaterialLSC[ linkID ];
		
		float3 position0   = g_vertexPositions[node0].xyz;
		float3 position1   = g_vertexPositions[node1].xyz;
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