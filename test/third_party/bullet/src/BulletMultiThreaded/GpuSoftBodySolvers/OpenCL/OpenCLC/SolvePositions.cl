
MSTRINGIFY(

/*#define float3 float4

float dot3(float3 a, float3 b)
{
   return a.x*b.x + a.y*b.y + a.z*b.z;
}*/

__kernel void 
SolvePositionsFromLinksKernel( 
	const int startLink,
	const int numLinks,
	const float kst,
	const float ti,
	__global int2 * g_linksVertexIndices,
	__global float * g_linksMassLSC,
	__global float * g_linksRestLengthSquared,
	__global float * g_verticesInverseMass,
	__global float4 * g_vertexPositions)
	
{
	int linkID = get_global_id(0) + startLink;
	if( get_global_id(0) < numLinks )
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
			float len  = dot(del, del);
			float k    = ((restLengthSquared - len)/(massLSC*(restLengthSquared+len)))*kst;
			position0 = position0 - del*(k*inverseMass0);
			position1 = position1 + del*(k*inverseMass1);

			g_vertexPositions[node0] = (float4)(position0, 0.f);
			g_vertexPositions[node1] = (float4)(position1, 0.f);

		}
	}
}

);