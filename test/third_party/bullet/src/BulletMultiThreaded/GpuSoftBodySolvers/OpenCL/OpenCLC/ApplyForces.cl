MSTRINGIFY(

/*#define float3 float4
float dot3(float3 a, float3 b)
{
   return a.x*b.x + a.y*b.y + a.z*b.z;
}*/

float3 projectOnAxis( float3 v, float3 a )
{
	return (a*dot(v, a));
}

__kernel void 
ApplyForcesKernel(
	const uint numNodes,
	const float solverdt,
	const float epsilon,
	__global int * g_vertexClothIdentifier,
	__global float4 * g_vertexNormal,
	__global float * g_vertexArea,
	__global float * g_vertexInverseMass,
	__global float * g_clothLiftFactor,
	__global float * g_clothDragFactor,
	__global float4 * g_clothWindVelocity,
	__global float4 * g_clothAcceleration,
	__global float * g_clothMediumDensity,
	__global float4 * g_vertexForceAccumulator,
	__global float4 * g_vertexVelocity)
{
	unsigned int nodeID = get_global_id(0);
	if( nodeID < numNodes )
	{		
		int clothId  = g_vertexClothIdentifier[nodeID];
		float nodeIM = g_vertexInverseMass[nodeID];
		
		if( nodeIM > 0.0f )
		{
			float3 nodeV  = g_vertexVelocity[nodeID].xyz;
			float3 normal = g_vertexNormal[nodeID].xyz;
			float area    = g_vertexArea[nodeID];
			float3 nodeF  = g_vertexForceAccumulator[nodeID].xyz;
			
			// Read per-cloth values
			float3 clothAcceleration = g_clothAcceleration[clothId].xyz;
			float3 clothWindVelocity = g_clothWindVelocity[clothId].xyz;
			float liftFactor = g_clothLiftFactor[clothId];
			float dragFactor = g_clothDragFactor[clothId];
			float mediumDensity = g_clothMediumDensity[clothId];
		
			// Apply the acceleration to the cloth rather than do this via a force
			nodeV += (clothAcceleration*solverdt);

			g_vertexVelocity[nodeID] = (float4)(nodeV, 0.f);

			float3 relativeWindVelocity = nodeV - clothWindVelocity;
			float relativeSpeedSquared = dot(relativeWindVelocity, relativeWindVelocity);
			
			if( relativeSpeedSquared > epsilon )
			{
				// Correct direction of normal relative to wind direction and get dot product
				normal = normal * (dot(normal, relativeWindVelocity) < 0 ? -1.f : 1.f);
				float dvNormal = dot(normal, relativeWindVelocity);
				if( dvNormal > 0 )
				{
					float3 force = (float3)(0.f, 0.f, 0.f);
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
									
					g_vertexForceAccumulator[nodeID] = (float4)(nodeF, 0.0f);	
				}
			}
		}
	}
}

);