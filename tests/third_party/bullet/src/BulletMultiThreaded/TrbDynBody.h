/*
   Copyright (C) 2009 Sony Computer Entertainment Inc.
   All rights reserved.

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/

#ifndef BT_RB_DYN_BODY_H__
#define BT_RB_DYN_BODY_H__

#include "vectormath/vmInclude.h"
using namespace Vectormath::Aos;

#include "TrbStateVec.h"

class CollObject;

class TrbDynBody
{
public:
	TrbDynBody()
	{
		fMass   = 0.0f;
		fCollObject = NULL;
		fElasticity = 0.2f;
		fFriction = 0.8f;
	}

	// Get methods
	float          getMass() const {return fMass;};
	float          getElasticity() const {return fElasticity;}
	float          getFriction() const {return fFriction;}
	CollObject*    getCollObject() const {return fCollObject;}
	const Matrix3 &getBodyInertia() const {return fIBody;}
	const Matrix3 &getBodyInertiaInv() const {return fIBodyInv;}
	float          getMassInv() const {return fMassInv;}

	// Set methods
	void           setMass(float mass) {fMass=mass;fMassInv=mass>0.0f?1.0f/mass:0.0f;}
	void           setBodyInertia(const Matrix3 bodyInertia) {fIBody = bodyInertia;fIBodyInv = inverse(bodyInertia);}
	void           setElasticity(float elasticity) {fElasticity = elasticity;}
	void           setFriction(float friction) {fFriction = friction;}
	void           setCollObject(CollObject *collObj) {fCollObject = collObj;}
	
	void           setBodyInertiaInv(const Matrix3 bodyInertiaInv) 
	{
		fIBody = inverse(bodyInertiaInv);
		fIBodyInv = bodyInertiaInv;
	}
	void           setMassInv(float invMass) {
		fMass= invMass>0.0f ? 1.0f/invMass :0.0f;
		fMassInv=invMass;
	}


private:
	// Rigid Body constants
	float          fMass;        // Rigid Body mass
	float          fMassInv;     // Inverse of mass
	Matrix3        fIBody;       // Inertia matrix in body's coords
	Matrix3        fIBodyInv;    // Inertia matrix inverse in body's coords
	float          fElasticity;  // Coefficient of restitution
	float          fFriction;    // Coefficient of friction

public:
	CollObject*    fCollObject;  // Collision object corresponding the RB
} __attribute__ ((aligned(16)));

#endif //BT_RB_DYN_BODY_H__

