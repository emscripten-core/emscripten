MSTRINGIFY(

cbuffer ApplyForcesCB : register( b0 )
{
	unsigned int numNodes;
	float solverdt;
	float epsilon;
	int padding3;
};


StructuredBuffer<int> g_vertexClothIdentifier : register( t0 );
StructuredBuffer<float4> g_vertexNormal : register( t1 );
StructuredBuffer<float> g_vertexArea : register( t2 );
StructuredBuffer<float> g_vertexInverseMass : register( t3 );
// TODO: These could be combined into a lift/drag factor array along with medium density
StructuredBuffer<float> g_clothLiftFactor : register( t4 );
StructuredBuffer<float> g_clothDragFactor : register( t5 );
StructuredBuffer<float4> g_clothWindVelocity : register( t6 );
StructuredBuffer<float4> g_clothAcceleration : register( t7 );
StructuredBuffer<float> g_clothMediumDensity : register( t8 );

RWStructuredBuffer<float4> g_vertexForceAccumulator : register( u0 );
RWStructuredBuffer<float4> g_vertexVelocity : register( u1 );

float3 projectOnAxis( float3 v, float3 a )
{
	return (a*dot(v, a));
}

[numthreads(128, 1, 1)]
void 
ApplyForcesKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	unsigned int nodeID = DTid.x;
	if( nodeID < numNodes )
	{		
		int clothId = g_vertexClothIdentifier[nodeID];
		float nodeIM = g_vertexInverseMass[nodeID];
		
		if( nodeIM > 0.0f )
		{
			float3 nodeV = g_vertexVelocity[nodeID].xyz;
			float3 normal = g_vertexNormal[nodeID].xyz;
			float area = g_vertexArea[nodeID];
			float3 nodeF = g_vertexForceAccumulator[nodeID].xyz;
			
			// Read per-cloth values
			float3 clothAcceleration = g_clothAcceleration[clothId].xyz;
			float3 clothWindVelocity = g_clothWindVelocity[clothId].xyz;
			float liftFactor = g_clothLiftFactor[clothId];
			float dragFactor = g_clothDragFactor[clothId];
			float mediumDensity = g_clothMediumDensity[clothId];
		
			// Apply the acceleration to the cloth rather than do this via a force
			nodeV += (clothAcceleration*solverdt);

			g_vertexVelocity[nodeID] = float4(nodeV, 0.f);

			float3 relativeWindVelocity = nodeV - clothWindVelocity;
			float relativeSpeedSquared = dot(relativeWindVelocity, relativeWindVelocity);
			
			if( relativeSpeedSquared > epsilon )
			{
				// Correct direction of normal relative to wind direction and get dot product
				normal = normal * (dot(normal, relativeWindVelocity) < 0 ? -1.f : 1.f);
				float dvNormal = dot(normal, relativeWindVelocity);
				if( dvNormal > 0 )
				{
					float3 force = float3(0.f, 0.f, 0.f);
					float c0 = area * dvNormal * relativeSpeedSquared / 2.f;
					float c1 = c0 * mediumDensity;
					force += normal * (-c1 * liftFactor);
					force += normalize(relativeWindVelocity)*(-c1 * dragFactor);
					
					float dtim = solverdt * nodeIM;
					float3 forceDTIM = force * dtim;
					
					float3 nodeFPlusForce = nodeF + force;
					
					// m_nodesf[i] -= ProjectOnAxis(m_nodesv[i], force.normalized())/dtim;	
					float3 nodeFMinus = nodeF - (projectOnAxis(nodeV, normalize(force))/dtim);
					
					nodeF = nodeFPlusForce;
					if( dot(forceDTIM, forceDTIM) > dot(nodeV, nodeV) )
						nodeF = nodeFMinus;
									
					g_vertexForceAccumulator[nodeID] = float4(nodeF, 0.0f);	
				}
			}
		}
	}
}

);