MSTRINGIFY(

__kernel void 
OutputToVertexArrayWithNormalsKernel( 
	const int startNode, const int numNodes, __global float *g_vertexBuffer,
	const int positionOffset, const int positionStride, const __global float4* g_vertexPositions, 
	const int normalOffset, const int normalStride, const __global float4* g_vertexNormals  )
{
	int nodeID = get_global_id(0);
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

__kernel void 
OutputToVertexArrayWithoutNormalsKernel(
	const int startNode, const int numNodes, __global float *g_vertexBuffer,
	const int positionOffset, const int positionStride, const __global float4* g_vertexPositions )
{
	int nodeID = get_global_id(0);
	if( nodeID < numNodes )
	{			
		float4 position = g_vertexPositions[nodeID + startNode];
		
		// Stride should account for the float->float4 conversion
		int positionDestination = nodeID * positionStride + positionOffset;		
		g_vertexBuffer[positionDestination] = position.x;
		g_vertexBuffer[positionDestination+1] = position.y;
		g_vertexBuffer[positionDestination+2] = position.z;		
	}
}

);