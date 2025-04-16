


MSTRINGIFY(


float mydot3(float4 a, float4 b)
{
   return a.x*b.x + a.y*b.y + a.z*b.z;
}


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
	__global float4 * g_vertexPositions GUID_ARG)
	
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
			
			float4 position0 = g_vertexPositions[node0];
			float4 position1 = g_vertexPositions[node1];

			float inverseMass0 = g_verticesInverseMass[node0];
			float inverseMass1 = g_verticesInverseMass[node1]; 

			float4 del = position1 - position0;
			float len  = mydot3(del, del);
			float k    = ((restLengthSquared - len)/(massLSC*(restLengthSquared+len)))*kst;
			position0 = position0 - del*(k*inverseMass0);
			position1 = position1 + del*(k*inverseMass1);

			g_vertexPositions[node0] = position0;
			g_vertexPositions[node1] = position1;

		}
	}
}

);