/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

cbuffer OutputToVertexArrayCB : register( b0 )
{
	int startNode;
	int numNodes;
	int offsetX;
	int strideX;
	
	int offsetN;	
	int strideN;
	int padding1;
	int padding2;
};


StructuredBuffer<float4> g_nodesx : register( t0 );
StructuredBuffer<float4> g_nodesn : register( t1 );

RWStructuredBuffer<float> g_vertexBuffer : register( u0 );


[numthreads(128, 1, 1)]
void 
OutputToVertexArrayKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int nodeID = DTid.x;
	if( nodeID < numNodes )
	{			
		float4 nodeX = g_nodesx[nodeID + startNode];
		float4 nodeN = g_nodesn[nodeID + startNode];
		
		// Stride should account for the float->float4 conversion
		int positionDestination = nodeID * strideX + offsetX;		
		g_vertexBuffer[positionDestination] = nodeX.x;
		g_vertexBuffer[positionDestination+1] = nodeX.y;
		g_vertexBuffer[positionDestination+2] = nodeX.z;
		
		int normalDestination = nodeID * strideN + offsetN;
		g_vertexBuffer[normalDestination] = nodeN.x;
		g_vertexBuffer[normalDestination+1] = nodeN.y;
		g_vertexBuffer[normalDestination+2] = nodeN.z;		
	}
}