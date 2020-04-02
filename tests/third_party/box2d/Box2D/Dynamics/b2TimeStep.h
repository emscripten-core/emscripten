/*
* Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_TIME_STEP_H
#define B2_TIME_STEP_H

#include <Box2D/Common/b2Math.h>

/// Profiling data. Times are in milliseconds.
struct b2Profile
{
	float32 step;
	float32 collide;
	float32 solve;
	float32 solveInit;
	float32 solveVelocity;
	float32 solvePosition;
	float32 broadphase;
	float32 solveTOI;
};

/// This is an internal structure.
struct b2TimeStep
{
	float32 dt;			// time step
	float32 inv_dt;		// inverse time step (0 if dt == 0).
	float32 dtRatio;	// dt * inv_dt0
	int32 velocityIterations;
	int32 positionIterations;
	bool warmStarting;
};

/// This is an internal structure.
struct b2Position
{
	b2Vec2 c;
	float32 a;
};

/// This is an internal structure.
struct b2Velocity
{
	b2Vec2 v;
	float32 w;
};

/// Solver Data
struct b2SolverData
{
	b2TimeStep step;
	b2Position* positions;
	b2Velocity* velocities;
};

#endif
