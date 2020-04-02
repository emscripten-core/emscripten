MSTRINGIFY(
float mydot3(float4 a, float4 b)
{
   return a.x*b.x + a.y*b.y + a.z*b.z;
}


typedef struct 
{
	int firstObject;
	int endObject;
} CollisionObjectIndices;

typedef struct 
{
	float4 shapeTransform[4]; // column major 4x4 matrix
	float4 linearVelocity;
	float4 angularVelocity;

	int softBodyIdentifier;
	int collisionShapeType;
	

	// Shape information
	// Compressed from the union
	float radius;
	float halfHeight;
	int upAxis;
		
	float margin;
	float friction;

	int padding0;
	
} CollisionShapeDescription;

// From btBroadphaseProxy.h
__constant int CAPSULE_SHAPE_PROXYTYPE = 10;


/* Multiply column-major matrix against vector */
float4 matrixVectorMul( float4 matrix[4], float4 vector )
{
	float4 returnVector;
	float4 row0 = (float4)(matrix[0].x, matrix[1].x, matrix[2].x, matrix[3].x);
	float4 row1 = (float4)(matrix[0].y, matrix[1].y, matrix[2].y, matrix[3].y);
	float4 row2 = (float4)(matrix[0].z, matrix[1].z, matrix[2].z, matrix[3].z);
	float4 row3 = (float4)(matrix[0].w, matrix[1].w, matrix[2].w, matrix[3].w);
	returnVector.x = dot(row0, vector);
	returnVector.y = dot(row1, vector);
	returnVector.z = dot(row2, vector);
	returnVector.w = dot(row3, vector);
	return returnVector;
}

__kernel void 
SolveCollisionsAndUpdateVelocitiesKernel( 
	const int numNodes,
	const float isolverdt,
	__global int *g_vertexClothIdentifier,
	__global float4 *g_vertexPreviousPositions,
	__global float * g_perClothFriction,
	__global float * g_clothDampingFactor,
	__global CollisionObjectIndices * g_perClothCollisionObjectIndices,
	__global CollisionShapeDescription * g_collisionObjectDetails,
	__global float4 * g_vertexForces,
	__global float4 *g_vertexVelocities,
	__global float4 *g_vertexPositions,
	__local CollisionShapeDescription *localCollisionShapes)
{
	int nodeID = get_global_id(0);
	float4 forceOnVertex = (float4)(0.f, 0.f, 0.f, 0.f);

	int clothIdentifier = g_vertexClothIdentifier[nodeID];

	// Abort if this is not a valid cloth
	if( clothIdentifier < 0 )
		return;
	

	float4 position = (float4)(g_vertexPositions[nodeID].xyz, 1.f);
	float4 previousPosition = (float4)(g_vertexPreviousPositions[nodeID].xyz, 1.f);
	float clothFriction = g_perClothFriction[clothIdentifier];
	float dampingFactor = g_clothDampingFactor[clothIdentifier];
	float velocityCoefficient = (1.f - dampingFactor);		
	
	// Update velocity	
	float4 difference = position - previousPosition;
	float4 velocity = difference*velocityCoefficient*isolverdt;			
	CollisionObjectIndices collisionObjectIndices = g_perClothCollisionObjectIndices[clothIdentifier];
	
	int numObjects = collisionObjectIndices.endObject - collisionObjectIndices.firstObject;
	if( numObjects > 0 )
	{
		// We have some possible collisions to deal with
		
		// First load all of the collision objects into LDS
		int numObjects = collisionObjectIndices.endObject - collisionObjectIndices.firstObject;
		if( get_local_id(0) < numObjects )
		{
			localCollisionShapes[get_local_id(0)] = g_collisionObjectDetails[ collisionObjectIndices.firstObject + get_local_id(0) ];
		}
	}

	// Safe as the vertices are padded so that not more than one soft body is in a group
	barrier(CLK_LOCAL_MEM_FENCE);

	// Annoyingly, even though I know the flow control is not varying, the compiler will not let me skip this
	if( numObjects > 0 )
	{
		
		
		// We have some possible collisions to deal with
		for( int collision = 0; collision < numObjects; ++collision )
		{
			//CollisionShapeDescription shapeDescription = localCollisionShapes[collision];
			float colliderFriction = localCollisionShapes[collision].friction;
		
			if( localCollisionShapes[collision].collisionShapeType == CAPSULE_SHAPE_PROXYTYPE )
			{
				// Colliding with a capsule

				float capsuleHalfHeight = localCollisionShapes[collision].halfHeight;
				float capsuleRadius = localCollisionShapes[collision].radius;
				float capsuleMargin = localCollisionShapes[collision].margin;
				int capsuleupAxis = localCollisionShapes[collision].upAxis;

				float4 worldTransform[4];
				worldTransform[0] = localCollisionShapes[collision].shapeTransform[0];
				worldTransform[1] = localCollisionShapes[collision].shapeTransform[1];
				worldTransform[2] = localCollisionShapes[collision].shapeTransform[2];
				worldTransform[3] = localCollisionShapes[collision].shapeTransform[3];

				//float4 c1 = (float4)(0.f, -capsuleHalfHeight, 0.f, 1.f); 
				//float4 c2 = (float4)(0.f, +capsuleHalfHeight, 0.f, 1.f);
				// Correctly define capsule centerline vector 
				float4 c1 = (float4)(0.f, 0.f, 0.f, 1.f); 
				float4 c2 = (float4)(0.f, 0.f, 0.f, 1.f);
				c1.x = select( 0.f, -capsuleHalfHeight, capsuleupAxis == 0 );
				c1.y = select( 0.f, -capsuleHalfHeight, capsuleupAxis == 1 );
				c1.z = select( 0.f, -capsuleHalfHeight, capsuleupAxis == 2 );
				c2.x = -c1.x;
				c2.y = -c1.y;
				c2.z = -c1.z;

				float4 worldC1 = matrixVectorMul(worldTransform, c1);
				float4 worldC2 = matrixVectorMul(worldTransform, c2);
				float4 segment = (worldC2 - worldC1);


				// compute distance of tangent to vertex along line segment in capsule
				float distanceAlongSegment = -( mydot3( (worldC1 - position), segment ) / mydot3(segment, segment) );

				float4 closestPoint = (worldC1 + (float4)(segment * distanceAlongSegment));
				float distanceFromLine = length(position - closestPoint);
				float distanceFromC1 = length(worldC1 - position);
				float distanceFromC2 = length(worldC2 - position);
					
				// Final distance from collision, point to push from, direction to push in
				// for impulse force
				float dist;
				float4 normalVector;
				if( distanceAlongSegment < 0 )
				{
					dist = distanceFromC1;
					normalVector = normalize(position - worldC1);
				} else if( distanceAlongSegment > 1.f ) {
					dist = distanceFromC2;
					normalVector = normalize(position - worldC2);	
				} else {
					dist = distanceFromLine;
					normalVector = normalize(position - closestPoint);
				}
						
				float4 colliderLinearVelocity = localCollisionShapes[collision].linearVelocity;
				float4 colliderAngularVelocity = localCollisionShapes[collision].angularVelocity;
				float4 velocityOfSurfacePoint = colliderLinearVelocity + cross(colliderAngularVelocity, position - (float4)(worldTransform[0].w, worldTransform[1].w, worldTransform[2].w, 0.f));

				float minDistance = capsuleRadius + capsuleMargin;
					
				// In case of no collision, this is the value of velocity
				velocity = (position - previousPosition) * velocityCoefficient * isolverdt;
					
					
				// Check for a collision
				if( dist < minDistance )
				{
					// Project back to surface along normal
					position = position + (float4)((minDistance - dist)*normalVector*0.9f);
					velocity = (position - previousPosition) * velocityCoefficient * isolverdt;
					float4 relativeVelocity = velocity - velocityOfSurfacePoint;

					float4 p1 = (float4)(normalize(cross(normalVector, segment)).xyz, 0.f);
					float4 p2 = (float4)(normalize(cross(p1, normalVector)).xyz, 0.f);
					// Full friction is sum of velocities in each direction of plane
					float4 frictionVector = p1*mydot3(relativeVelocity, p1) + p2*mydot3(relativeVelocity, p2);

					// Real friction is peak friction corrected by friction coefficients
					frictionVector = frictionVector * (colliderFriction*clothFriction);

					float approachSpeed = dot(relativeVelocity, normalVector);

					if( approachSpeed <= 0.0f )
						forceOnVertex -= frictionVector;
				}
					
			}
		}
	}
	
	g_vertexVelocities[nodeID] = (float4)(velocity.xyz, 0.f);	

	// Update external force
	g_vertexForces[nodeID] = (float4)(forceOnVertex.xyz, 0.f);

	g_vertexPositions[nodeID] = (float4)(position.xyz, 0.f);
}

);
