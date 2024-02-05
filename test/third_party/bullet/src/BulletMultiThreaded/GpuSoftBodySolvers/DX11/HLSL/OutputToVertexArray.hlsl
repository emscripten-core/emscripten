MSTRINGIFY(

cbuffer OutputToVertexArrayCB : register( b0 )
{
	int startNode;
	int numNodes;
	int positionOffset;
	int positionStride;
	
	int normalOffset;	
	int normalStride;
	int padding1;
	int padding2;
};


StructuredBuffer<float4> g_vertexPositions : register( t0 );
StructuredBuffer<float4> g_vertexNormals : register( t1 );

RWBuffer<float> g_vertexBuffer : register( u0 );


[numthreads(128, 1, 1)]
void 
OutputToVertexArrayWithNormalsKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int nodeID = DTid.x;
	if( nodeID < numNodes )
	{			
		float4 position = g_vertexPositions[nodeID + startNode];
		float4 normal = g_vertexNormals[nodeID + startNode];
		
		// Stride should account for the float->float4 conversion
		int positionDestination = nodeID * positionStride + positionOffset;		
		g_vertexBuffer[positionDestination] = position.x;
		g_vertexBuffer[positionDestination+1] = position.y;
		g_vertexBuffer[positionDestination+2] = position.z;
		
		int normalDestination = nodeID * normalStride + normalOffset;
		g_vertexBuffer[normalDestination] = normal.x;
		g_vertexBuffer[normalDestination+1] = normal.y;
		g_vertexBuffer[normalDestination+2] = normal.z;		
	}
}

[numthreads(128, 1, 1)]
void 
OutputToVertexArrayWithoutNormalsKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int nodeID = DTid.x;
	if( nodeID < numNodes )
	{			
		float4 position = g_vertexPositions[nodeID + startNode];
		float4 normal = g_vertexNormals[nodeID + startNode];
		
		// Stride should account for the float->float4 conversion
		int positionDestination = nodeID * positionStride + positionOffset;		
		g_vertexBuffer[positionDestination] = position.x;
		g_vertexBuffer[positionDestination+1] = position.y;
		g_vertexBuffer[positionDestination+2] = position.z;		
	}
}
);