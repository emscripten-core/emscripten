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

#ifndef BT_TRBSTATEVEC_H__
#define BT_TRBSTATEVEC_H__

#include <stdlib.h>
#ifdef PFX_USE_FREE_VECTORMATH
#include "vecmath/vmInclude.h"
#else
#include "vectormath/vmInclude.h"
#endif //PFX_USE_FREE_VECTORMATH


#include "PlatformDefinitions.h"


static inline vmVector3 read_Vector3(const float* p)
{
	vmVector3 v;
	loadXYZ(v, p);
	return v;
}

static inline vmQuat read_Quat(const float* p)
{
	vmQuat vq;
	loadXYZW(vq, p);
	return vq;
}

static inline void store_Vector3(const vmVector3 &src, float* p)
{
	vmVector3 v = src;
	storeXYZ(v, p);
}

static inline void store_Quat(const vmQuat &src, float* p)
{
	vmQuat vq = src;
	storeXYZW(vq, p);
}

// Motion Type
enum {
	PfxMotionTypeFixed = 0,
	PfxMotionTypeActive,
	PfxMotionTypeKeyframe,
	PfxMotionTypeOneWay,
	PfxMotionTypeTrigger,
	PfxMotionTypeCount
};

#define PFX_MOTION_MASK_DYNAMIC 0x0a // Active,OneWay
#define PFX_MOTION_MASK_STATIC  0x95 // Fixed,Keyframe,Trigger,Sleeping
#define PFX_MOTION_MASK_SLEEP   0x0e // Can sleep
#define PFX_MOTION_MASK_TYPE    0x7f

//
// Rigid Body state
//

#ifdef __CELLOS_LV2__
ATTRIBUTE_ALIGNED128(class) TrbState
#else
ATTRIBUTE_ALIGNED16(class) TrbState
#endif

{
public:
	TrbState()
	{
		setMotionType(PfxMotionTypeActive);
		contactFilterSelf=contactFilterTarget=0xffffffff;
		deleted = 0;
		mSleeping = 0;
		useSleep = 1;
		trbBodyIdx=0;
		mSleepCount=0;
		useCcd = 0;
		useContactCallback = 0;
		useSleepCallback = 0;
		linearDamping = 1.0f;
		angularDamping = 0.99f;
	}

	TrbState(const uint8_t m, const vmVector3& x, const vmQuat& q, const vmVector3& v, const vmVector3& omega );
	
	uint16_t	mSleepCount;
	uint8_t		mMotionType;
	uint8_t		deleted            : 1;
	uint8_t		mSleeping           : 1;
	uint8_t		useSleep           : 1;
	uint8_t		useCcd		       : 1;
	uint8_t		useContactCallback : 1;
	uint8_t		useSleepCallback   : 1;

	uint16_t	trbBodyIdx;
	uint32_t	contactFilterSelf;
	uint32_t	contactFilterTarget;

	float		center[3];		// AABB center(World)
	float		half[3];		// AABB half(World)

	float		linearDamping;
	float		angularDamping;
	
	float		deltaLinearVelocity[3];
	float		deltaAngularVelocity[3];

	float     fX[3];				// position
	float     fQ[4];				// orientation
	float     fV[3];				// velocity
	float     fOmega[3];			// angular velocity

	inline void setZero();      // Zeroes out the elements
	inline void setIdentity();  // Sets the rotation to identity and zeroes out the other elements

	bool		isDeleted() const {return deleted==1;}

	uint16_t	getRigidBodyId() const {return trbBodyIdx;}
	void		setRigidBodyId(uint16_t i) {trbBodyIdx = i;}


	uint32_t	getContactFilterSelf() const {return contactFilterSelf;}
	void		setContactFilterSelf(uint32_t filter) {contactFilterSelf = filter;}

	uint32_t	getContactFilterTarget() const {return contactFilterTarget;}
	void		setContactFilterTarget(uint32_t filter) {contactFilterTarget = filter;}

	float getLinearDamping() const {return linearDamping;}
	float getAngularDamping() const {return angularDamping;}

	void setLinearDamping(float damping) {linearDamping=damping;}
	void setAngularDamping(float damping) {angularDamping=damping;}


	uint8_t		getMotionType() const {return mMotionType;}
	void		setMotionType(uint8_t t) {mMotionType = t;mSleeping=0;mSleepCount=0;}

	uint8_t		getMotionMask() const {return (1<<mMotionType)|(mSleeping<<7);}

	bool		isAsleep() const {return mSleeping==1;}
	bool		isAwake() const {return mSleeping==0;}

	void		wakeup() {mSleeping=0;mSleepCount=0;}
	void		sleep() {if(useSleep) {mSleeping=1;mSleepCount=0;}}

	uint8_t		getUseSleep() const {return useSleep;}
	void		setUseSleep(uint8_t b) {useSleep=b;}

	uint8_t		getUseCcd() const {return useCcd;}
	void		setUseCcd(uint8_t b) {useCcd=b;}

	uint8_t		getUseContactCallback() const {return useContactCallback;}
	void		setUseContactCallback(uint8_t b) {useContactCallback=b;}

	uint8_t		getUseSleepCallback() const {return useSleepCallback;}
	void		setUseSleepCallback(uint8_t b) {useSleepCallback=b;}

	void	 	incrementSleepCount() {mSleepCount++;}
	void		resetSleepCount() {mSleepCount=0;}
	uint16_t	getSleepCount() const {return mSleepCount;}

	vmVector3 getPosition() const {return read_Vector3(fX);}
	vmQuat    getOrientation() const {return read_Quat(fQ);}
	vmVector3 getLinearVelocity() const {return read_Vector3(fV);}
	vmVector3 getAngularVelocity() const {return read_Vector3(fOmega);}
	vmVector3 getDeltaLinearVelocity() const {return read_Vector3(deltaLinearVelocity);}
	vmVector3 getDeltaAngularVelocity() const {return read_Vector3(deltaAngularVelocity);}

	void setPosition(const vmVector3 &pos) {store_Vector3(pos, fX);}
	void setLinearVelocity(const vmVector3 &vel) {store_Vector3(vel, fV);}
	void setAngularVelocity(const vmVector3 &vel) {store_Vector3(vel, fOmega);}
	void setDeltaLinearVelocity(const vmVector3 &vel) {store_Vector3(vel, deltaLinearVelocity);}
	void setDeltaAngularVelocity(const vmVector3 &vel) {store_Vector3(vel, deltaAngularVelocity);}
	void setOrientation(const vmQuat &rot) {store_Quat(rot, fQ);}

	inline void setAuxils(const vmVector3 &centerLocal,const vmVector3 &halfLocal);
	inline void	setAuxilsCcd(const vmVector3 &centerLocal,const vmVector3 &halfLocal,float timeStep);
	inline	void reset();
};

inline
TrbState::TrbState(const uint8_t m, const vmVector3& x, const vmQuat& q, const vmVector3& v, const vmVector3& omega)
{
	setMotionType(m);
	fX[0] = x[0];
	fX[1] = x[1];
	fX[2] = x[2];
	fQ[0] = q[0];
	fQ[1] = q[1];
	fQ[2] = q[2];
	fQ[3] = q[3];
	fV[0] = v[0];
	fV[1] = v[1];
	fV[2] = v[2];
	fOmega[0] = omega[0];
	fOmega[1] = omega[1];
	fOmega[2] = omega[2];
	contactFilterSelf=contactFilterTarget=0xffff;
	trbBodyIdx=0;
	mSleeping = 0;
	deleted = 0;
	useSleep = 1;
	useCcd = 0;
	useContactCallback = 0;
	useSleepCallback = 0;
	mSleepCount=0;
	linearDamping = 1.0f;
	angularDamping = 0.99f;
}

inline void
TrbState::setIdentity()
{
	fX[0] = 0.0f;
	fX[1] = 0.0f;
	fX[2] = 0.0f;
	fQ[0] = 0.0f;
	fQ[1] = 0.0f;
	fQ[2] = 0.0f;
	fQ[3] = 1.0f;
	fV[0] = 0.0f;
	fV[1] = 0.0f;
	fV[2] = 0.0f;
	fOmega[0] = 0.0f;
	fOmega[1] = 0.0f;
	fOmega[2] = 0.0f;
}

inline void
TrbState::setZero()
{
	fX[0] = 0.0f;
	fX[1] = 0.0f;
	fX[2] = 0.0f;
	fQ[0] = 0.0f;
	fQ[1] = 0.0f;
	fQ[2] = 0.0f;
	fQ[3] = 0.0f;
	fV[0] = 0.0f;
	fV[1] = 0.0f;
	fV[2] = 0.0f;
	fOmega[0] = 0.0f;
	fOmega[1] = 0.0f;
	fOmega[2] = 0.0f;
}

inline void
TrbState::setAuxils(const vmVector3 &centerLocal,const vmVector3 &halfLocal)
{
	vmVector3 centerW = getPosition() + rotate(getOrientation(),centerLocal);
	vmVector3 halfW = absPerElem(vmMatrix3(getOrientation())) * halfLocal;
	center[0] = centerW[0];
	center[1] = centerW[1];
	center[2] = centerW[2];
	half[0] = halfW[0];
	half[1] = halfW[1];
	half[2] = halfW[2];
}

inline void
TrbState::setAuxilsCcd(const vmVector3 &centerLocal,const vmVector3 &halfLocal,float timeStep)
{
	vmVector3 centerW = getPosition() + rotate(getOrientation(),centerLocal);
	vmVector3 halfW = absPerElem(vmMatrix3(getOrientation())) * halfLocal;

	vmVector3 diffvec = getLinearVelocity()*timeStep;

	vmVector3 newCenter = centerW + diffvec;
	vmVector3 aabbMin = minPerElem(newCenter - halfW,centerW - halfW);
	vmVector3 aabbMax = maxPerElem(newCenter + halfW,centerW + halfW);
	
	centerW = 0.5f * (aabbMin + aabbMax);
	halfW =0.5f * (aabbMax - aabbMin);

	center[0] = centerW[0];
	center[1] = centerW[1];
	center[2] = centerW[2];

	half[0] = halfW[0];
	half[1] = halfW[1];
	half[2] = halfW[2];
}

inline
void TrbState::reset()
{
#if 0
	mSleepCount = 0;
	mMotionType = PfxMotionTypeActive;
	mDeleted = 0;
	mSleeping = 0;
	mUseSleep = 1;
	mUseCcd = 0;
	mUseContactCallback = 0;
	mUseSleepCallback = 0;
	mRigidBodyId = 0;
	mContactFilterSelf = 0xffffffff;
	mContactFilterTarget = 0xffffffff;
	mLinearDamping = 1.0f;
	mAngularDamping = 0.99f;
	mPosition = vmVector3(0.0f);
	mOrientation = vmQuat::identity();
	mLinearVelocity = vmVector3(0.0f);
	mAngularVelocity = vmVector3(0.0f);
#endif

	setMotionType(PfxMotionTypeActive);
	contactFilterSelf=contactFilterTarget=0xffffffff;
	deleted = 0;
	mSleeping = 0;
	useSleep = 1;
	trbBodyIdx=0;
	mSleepCount=0;
	useCcd = 0;
	useContactCallback = 0;
	useSleepCallback = 0;
	linearDamping = 1.0f;
	angularDamping = 0.99f;
}

#endif //BT_TRBSTATEVEC_H__


