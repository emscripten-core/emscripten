MSTRINGIFY(

cbuffer SolvePositionsFromLinksKernelCB : register( b0 )
{
	unsigned int numNodes;
	float isolverdt;
	int padding0;
	int padding1;
};

struct CollisionObjectIndices
{
	int firstObject;
	int endObject;
};

struct CollisionShapeDescription
{
	float4x4 shapeTransform;
	float4 linearVelocity;
	float4 angularVelocity;

	int softBodyIdentifier;
	int collisionShapeType;
	

	// Shape information
	// Compressed from the union
	float radius;
	float halfHeight;
		
	float margin;
	float friction;

	int padding0;
	int padding1;
	
};

// From btBroadphaseProxy.h
static const int CAPSULE_SHAPE_PROXYTYPE = 10;

// Node indices for each link
StructuredBuffer<int> g_vertexClothIdentifier : register( t0 );
StructuredBuffer<float4> g_vertexPreviousPositions : register( t1 );
StructuredBuffer<float> g_perClothFriction : register( t2 );
StructuredBuffer<float> g_clothDampingFactor : register( t3 );
StructuredBuffer<CollisionObjectIndices> g_perClothCollisionObjectIndices : register( t4 );
StructuredBuffer<CollisionShapeDescription> g_collisionObjectDetails : register( t5 );

RWStructuredBuffer<float4> g_vertexForces : register( u0 );
RWStructuredBuffer<float4> g_vertexVelocities : register( u1 );
RWStructuredBuffer<float4> g_vertexPositions : register( u2 );

[numthreads(128, 1, 1)]
void 
SolveCollisionsAndUpdateVelocitiesKernel( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
	int nodeID = DTid.x;
	float3 forceOnVertex = float3(0.f, 0.f, 0.f);
	if( DTid.x < numNodes )
	{	
		int clothIdentifier = g_vertexClothIdentifier[nodeID];
		float4 position = float4(g_vertexPositions[nodeID].xyz, 1.f);
		float4 previousPosition = float4(g_vertexPreviousPositions[nodeID].xyz, 1.f);
		float3 velocity;
		float clothFriction = g_perClothFriction[clothIdentifier];
		float dampingFactor = g_clothDampingFactor[clothIdentifier];
		float velocityCoefficient = (1.f - dampingFactor);		
		CollisionObjectIndices collisionObjectIndices = g_perClothCollisionObjectIndices[clothIdentifier];
	
		if( collisionObjectIndices.firstObject != collisionObjectIndices.endObject )
		{
			velocity = float3(15, 0, 0);

			// We have some possible collisions to deal with
			for( int collision = collisionObjectIndices.firstObject; collision < collisionObjectIndices.endObject; ++collision )
			{
				CollisionShapeDescription shapeDescription = g_collisionObjectDetails[collision];
				float colliderFriction = shapeDescription.friction;
		
				if( shapeDescription.collisionShapeType == CAPSULE_SHAPE_PROXYTYPE )
				{
					// Colliding with a capsule

					float capsuleHalfHeight = shapeDescription.halfHeight;
					float capsuleRadius = shapeDescription.radius;
					float capsuleMargin = shapeDescription.margin;
					float4x4 worldTransform = shapeDescription.shapeTransform;

					float4 c1 = float4(0.f, -capsuleHalfHeight, 0.f, 1.f); 
					float4 c2 = float4(0.f, +capsuleHalfHeight, 0.f, 1.f);
					float4 worldC1 = mul(worldTransform, c1);
					float4 worldC2 = mul(worldTransform, c2);
					float3 segment = (worldC2 - worldC1).xyz;

					// compute distance of tangent to vertex along line segment in capsule
					float distanceAlongSegment = -( dot( (worldC1 - position).xyz, segment ) / dot(segment, segment) );

					float4 closestPoint = (worldC1 + float4(segment * distanceAlongSegment, 0.f));
					float distanceFromLine = length(position - closestPoint);
					float distanceFromC1 = length(worldC1 - position);
					float distanceFromC2 = length(worldC2 - position);
					
					// Final distance from collision, point to push from, direction to push in
					// for impulse force
					float dist;
					float3 normalVector;
					if( distanceAlongSegment < 0 )
					{
						dist = distanceFromC1;
						normalVector = normalize(position - worldC1).xyz;
					} else if( distanceAlongSegment > 1.f ) {
						dist = distanceFromC2;
						normalVector = normalize(position - worldC2).xyz;	
					} else {
						dist = distanceFromLine;
						normalVector = normalize(position - closestPoint).xyz;
					}
						
					float3 colliderLinearVelocity = shapeDescription.linearVelocity.xyz;
					float3 colliderAngularVelocity = shapeDescription.angularVelocity.xyz;
					float3 velocityOfSurfacePoint = colliderLinearVelocity + cross(colliderAngularVelocity, position.xyz - worldTransform._m03_m13_m23);

					float minDistance = capsuleRadius + capsuleMargin;
					
					// In case of no collision, this is the value of velocity
					velocity = (position - previousPosition).xyz * velocityCoefficient * isolverdt;
					
					
					// Check for a collision
					if( dist < minDistance )
					{
						// Project back to surface along normal
						position = position + float4((minDistance - dist)*normalVector*0.9, 0.f);
						velocity = (position - previousPosition).xyz * velocityCoefficient * isolverdt;
						float3 relativeVelocity = velocity - velocityOfSurfacePoint;

						float3 p1 = normalize(cross(normalVector, segment));
						float3 p2 = normalize(cross(p1, normalVector));
						// Full friction is sum of velocities in each direction of plane
						float3 frictionVector = p1*dot(relativeVelocity, p1) + p2*dot(relativeVelocity, p2);

						// Real friction is peak friction corrected by friction coefficients
						frictionVector = frictionVector * (colliderFriction*clothFriction);

						float approachSpeed = dot(relativeVelocity, normalVector);

						if( approachSpeed <= 0.0 )
							forceOnVertex -= frictionVector;
					}
					
				}
			}
		} else {
			// Update velocity	
			float3 difference = position.xyz - previousPosition.xyz;
			velocity = difference*velocityCoefficient*isolverdt;			
		}

		g_vertexVelocities[nodeID] = float4(velocity, 0.f);	

		// Update external force
		g_vertexForces[nodeID] = float4(forceOnVertex, 0.f);

		g_vertexPositions[nodeID] = float4(position.xyz, 0.f);
	}
}

);
