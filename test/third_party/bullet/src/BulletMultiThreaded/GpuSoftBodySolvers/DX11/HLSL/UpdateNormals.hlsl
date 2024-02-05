MSTRINGIFY(

cbuffer UpdateSoftBodiesCB : register( b0 )
{
	unsigned int numNodes;
	unsigned int startFace;
	unsigned int numFaces;
	float epsilon;
};


// Node indices for each link
StructuredBuffer<int4> g_triangleVertexIndexSet : register( t0 );
StructuredBuffer<float4> g_vertexPositions : register( t1 );
StructuredBuffer<int> g_vertexTriangleCount : register( t2 );

RWStructuredBuffer<float4> g_vertexNormals : register( u0 );
RWStructuredBuffer<float> g_vertexArea : register( u1 );
RWStructuredBuffer<float4> g_triangleNormals : register( u2 );
RWStructuredBuffer<float> g_triangleArea : register( u3 );


[numthreads(128, 1, 1)]
void 
ResetNormalsAndAreasKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	if( DTid.x < numNodes )
	{
		g_vertexNormals[DTid.x] = float4(0.0f, 0.0f, 0.0f, 0.0f);
		g_vertexArea[DTid.x] = 0.0f;
	}
}


[numthreads(128, 1, 1)]
void 
UpdateSoftBodiesKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int faceID = DTid.x + startFace;
	if( DTid.x < numFaces )
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
		
		g_triangleNormals[faceID] = float4(normalize(faceNormal), 0.f);
		g_vertexNormals[nodeIndex0] = float4(nodeNormal0, 0.f);
		g_vertexNormals[nodeIndex1] = float4(nodeNormal1, 0.f);
		g_vertexNormals[nodeIndex2] = float4(nodeNormal2, 0.f);
		g_triangleArea[faceID] = triangleArea;
		g_vertexArea[nodeIndex0] = vertexArea0;
		g_vertexArea[nodeIndex1] = vertexArea1;
		g_vertexArea[nodeIndex2] = vertexArea2;
	}
}

[numthreads(128, 1, 1)]
void 
NormalizeNormalsAndAreasKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	if( DTid.x < numNodes )
	{
		float4 normal = g_vertexNormals[DTid.x];
		float area = g_vertexArea[DTid.x];
		int numTriangles = g_vertexTriangleCount[DTid.x];
		
		float vectorLength = length(normal);
		
		g_vertexNormals[DTid.x] = normalize(normal);
		g_vertexArea[DTid.x] = area/float(numTriangles);
	}
}

);