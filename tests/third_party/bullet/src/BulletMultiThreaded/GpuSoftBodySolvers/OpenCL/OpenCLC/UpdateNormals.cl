MSTRINGIFY(

//#define float3 float4

/*float length3(float3 a)
{
	a.w = 0;
	return length(a);
}

float normalize3(float3 a)
{
	a.w = 0;
	return normalize(a);
}*/

__kernel void 
ResetNormalsAndAreasKernel(
	const unsigned int numNodes,
	__global float4 * g_vertexNormals,
	__global float * g_vertexArea)
{
	if( get_global_id(0) < numNodes )
	{
		g_vertexNormals[get_global_id(0)] = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
		g_vertexArea[get_global_id(0)]    = 0.0f;
	}
}

__kernel void 
UpdateSoftBodiesKernel(
	const unsigned int startFace,
	const unsigned int numFaces,
	__global int4 * g_triangleVertexIndexSet,
	__global float4 * g_vertexPositions,
	__global float4 * g_vertexNormals,
	__global float * g_vertexArea,
	__global float4 * g_triangleNormals,
	__global float * g_triangleArea)
{
	int faceID = get_global_id(0) + startFace;
	if( get_global_id(0) < numFaces )
	{		
		int4 triangleIndexSet = g_triangleVertexIndexSet[ faceID ];
		int nodeIndex0 = triangleIndexSet.x;
		int nodeIndex1 = triangleIndexSet.y;
		int nodeIndex2 = triangleIndexSet.z;

		float3 node0 = g_vertexPositions[nodeIndex0].xyz;
		float3 node1 = g_vertexPositions[nodeIndex1].xyz;
		float3 node2 = g_vertexPositions[nodeIndex2].xyz;
		float3 nodeNormal0 = g_vertexNormals[nodeIndex0].xyz;
		float3 nodeNormal1 = g_vertexNormals[nodeIndex1].xyz;
		float3 nodeNormal2 = g_vertexNormals[nodeIndex2].xyz;
		float vertexArea0 = g_vertexArea[nodeIndex0];
		float vertexArea1 = g_vertexArea[nodeIndex1];
		float vertexArea2 = g_vertexArea[nodeIndex2];
		
		float3 vector0 = node1 - node0;
		float3 vector1 = node2 - node0;
		
		float3 faceNormal = cross(vector0.xyz, vector1.xyz);
		float triangleArea = length(faceNormal);

		nodeNormal0 = nodeNormal0 + faceNormal;
		nodeNormal1 = nodeNormal1 + faceNormal;
		nodeNormal2 = nodeNormal2 + faceNormal;
		vertexArea0 = vertexArea0 + triangleArea;
		vertexArea1 = vertexArea1 + triangleArea;
		vertexArea2 = vertexArea2 + triangleArea;
		
		g_triangleNormals[faceID] = (float4)(normalize(faceNormal), 0.f);
		g_vertexNormals[nodeIndex0] = (float4)(nodeNormal0, 0.f);
		g_vertexNormals[nodeIndex1] = (float4)(nodeNormal1, 0.f);
		g_vertexNormals[nodeIndex2] = (float4)(nodeNormal2, 0.f);
		g_triangleArea[faceID] = triangleArea;
		g_vertexArea[nodeIndex0] = vertexArea0;
		g_vertexArea[nodeIndex1] = vertexArea1;
		g_vertexArea[nodeIndex2] = vertexArea2;
	}
}

__kernel void 
NormalizeNormalsAndAreasKernel( 
	const unsigned int numNodes,
	__global int * g_vertexTriangleCount,
	__global float4 * g_vertexNormals,
	__global float * g_vertexArea)
{
	if( get_global_id(0) < numNodes )
	{
		float4 normal = g_vertexNormals[get_global_id(0)];
		float area = g_vertexArea[get_global_id(0)];
		int numTriangles = g_vertexTriangleCount[get_global_id(0)];
		
		float vectorLength = length(normal);
		
		g_vertexNormals[get_global_id(0)] = normalize(normal);
		g_vertexArea[get_global_id(0)] = area/(float)(numTriangles);
	}
}

);