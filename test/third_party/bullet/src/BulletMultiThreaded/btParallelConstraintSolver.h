/*
   Copyright (C) 2010 Sony Computer Entertainment Inc.
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

#ifndef __BT_PARALLEL_CONSTRAINT_SOLVER_H
#define __BT_PARALLEL_CONSTRAINT_SOLVER_H

#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"




#include "LinearMath/btScalar.h"
#include "PlatformDefinitions.h"


#define PFX_MAX_SOLVER_PHASES 64
#define PFX_MAX_SOLVER_BATCHES 16
#define PFX_MAX_SOLVER_PAIRS  128
#define PFX_MIN_SOLVER_PAIRS  16

#ifdef __CELLOS_LV2__
ATTRIBUTE_ALIGNED128(struct) PfxParallelBatch {
#else
ATTRIBUTE_ALIGNED16(struct) PfxParallelBatch {
#endif
	uint16_t pairIndices[PFX_MAX_SOLVER_PAIRS];
};

#ifdef __CELLOS_LV2__
ATTRIBUTE_ALIGNED128(struct) PfxParallelGroup {
#else
ATTRIBUTE_ALIGNED16(struct) PfxParallelGroup {
#endif
	uint16_t numPhases;
	uint16_t numBatches[PFX_MAX_SOLVER_PHASES];
	uint16_t numPairs[PFX_MAX_SOLVER_PHASES*PFX_MAX_SOLVER_BATCHES];
};



ATTRIBUTE_ALIGNED16(struct) PfxSortData16 {
	union {
		uint8_t   i8data[16];
		uint16_t  i16data[8];
		uint32_t  i32data[4];
#ifdef __SPU__
		vec_uint4 vdata;
#endif
	};

#ifdef __SPU__
	void set8(int elem,uint8_t data)   {vdata=(vec_uint4)spu_insert(data,(vec_uchar16)vdata,elem);}
	void set16(int elem,uint16_t data) {vdata=(vec_uint4)spu_insert(data,(vec_ushort8)vdata,elem);}
	void set32(int elem,uint32_t data) {vdata=(vec_uint4)spu_insert(data,(vec_uint4)vdata,elem);}
	uint8_t get8(int elem)   const {return spu_extract((vec_uchar16)vdata,elem);}
	uint16_t get16(int elem) const {return spu_extract((vec_ushort8)vdata,elem);}
	uint32_t get32(int elem) const {return spu_extract((vec_uint4)vdata,elem);}
#else
	void set8(int elem,uint8_t data)   {i8data[elem] = data;}
	void set16(int elem,uint16_t data) {i16data[elem] = data;}
	void set32(int elem,uint32_t data) {i32data[elem] = data;}
	uint8_t get8(int elem)   const {return i8data[elem];}
	uint16_t get16(int elem) const {return i16data[elem];}
	uint32_t get32(int elem) const {return i32data[elem];}
#endif
};

typedef PfxSortData16 PfxConstraintPair;


//J	PfxBroadphasePair‚Æ‹¤’Ê

SIMD_FORCE_INLINE void pfxSetConstraintId(PfxConstraintPair &pair,uint32_t i)	{pair.set32(2,i);}
SIMD_FORCE_INLINE void pfxSetNumConstraints(PfxConstraintPair &pair,uint8_t n)	{pair.set8(7,n);}

SIMD_FORCE_INLINE uint32_t pfxGetConstraintId1(const PfxConstraintPair &pair)	{return pair.get32(2);}
SIMD_FORCE_INLINE uint8_t  pfxGetNumConstraints(const PfxConstraintPair &pair)	{return pair.get8(7);}

typedef PfxSortData16 PfxBroadphasePair;

SIMD_FORCE_INLINE void pfxSetRigidBodyIdA(PfxBroadphasePair &pair,uint16_t i)	{pair.set16(0,i);}
SIMD_FORCE_INLINE void pfxSetRigidBodyIdB(PfxBroadphasePair &pair,uint16_t i)	{pair.set16(1,i);}
SIMD_FORCE_INLINE void pfxSetMotionMaskA(PfxBroadphasePair &pair,uint8_t i)		{pair.set8(4,i);}
SIMD_FORCE_INLINE void pfxSetMotionMaskB(PfxBroadphasePair &pair,uint8_t i)		{pair.set8(5,i);}
SIMD_FORCE_INLINE void pfxSetBroadphaseFlag(PfxBroadphasePair &pair,uint8_t f)	{pair.set8(6,(pair.get8(6)&0xf0)|(f&0x0f));}
SIMD_FORCE_INLINE void pfxSetActive(PfxBroadphasePair &pair,bool b)			{pair.set8(6,(pair.get8(6)&0x0f)|((b?1:0)<<4));}
SIMD_FORCE_INLINE void pfxSetContactId(PfxBroadphasePair &pair,uint32_t i)		{pair.set32(2,i);}

SIMD_FORCE_INLINE uint16_t pfxGetRigidBodyIdA(const PfxBroadphasePair &pair)	{return pair.get16(0);}
SIMD_FORCE_INLINE uint16_t pfxGetRigidBodyIdB(const PfxBroadphasePair &pair)	{return pair.get16(1);}
SIMD_FORCE_INLINE uint8_t  pfxGetMotionMaskA(const PfxBroadphasePair &pair)		{return pair.get8(4);}
SIMD_FORCE_INLINE uint8_t  pfxGetMotionMaskB(const PfxBroadphasePair &pair)		{return pair.get8(5);}
SIMD_FORCE_INLINE uint8_t  pfxGetBroadphaseFlag(const PfxBroadphasePair &pair)	{return pair.get8(6)&0x0f;}
SIMD_FORCE_INLINE bool     pfxGetActive(const PfxBroadphasePair &pair)			{return (pair.get8(6)>>4)!=0;}
SIMD_FORCE_INLINE uint32_t pfxGetContactId1(const PfxBroadphasePair &pair)		{return pair.get32(2);}



#if defined(__PPU__) || defined (__SPU__)
ATTRIBUTE_ALIGNED128(struct) PfxSolverBody {
#else
ATTRIBUTE_ALIGNED16(struct) PfxSolverBody {
#endif
	vmVector3 mDeltaLinearVelocity;
	vmVector3 mDeltaAngularVelocity;
	vmMatrix3 mInertiaInv;
	vmQuat    mOrientation;
	float   mMassInv;
	float   friction;
	float   restitution;
	float   unused;
	float   unused2;
	float   unused3;
	float   unused4;
	float   unused5;
};


#ifdef __PPU__
#include "SpuDispatch/BulletPE2ConstraintSolverSpursSupport.h"
#endif

static SIMD_FORCE_INLINE vmVector3 btReadVector3(const double* p)
{
	float tmp[3] = {float(p[0]),float(p[1]),float(p[2])};
	vmVector3 v;
	loadXYZ(v, tmp);
	return v;
}

static SIMD_FORCE_INLINE vmQuat btReadQuat(const double* p)
{
	float tmp[4] = {float(p[0]),float(p[1]),float(p[2]),float(p[4])};
	vmQuat vq;
	loadXYZW(vq, tmp);
	return vq;
}

static SIMD_FORCE_INLINE void btStoreVector3(const vmVector3 &src, double* p)
{
	float tmp[3];
	vmVector3 v = src;
	storeXYZ(v, tmp);
	p[0] = tmp[0];
	p[1] = tmp[1];
	p[2] = tmp[2];
}


static SIMD_FORCE_INLINE vmVector3 btReadVector3(const float* p)
{
	vmVector3 v;
	loadXYZ(v, p);
	return v;
}

static SIMD_FORCE_INLINE vmQuat btReadQuat(const float* p)
{
	vmQuat vq;
	loadXYZW(vq, p);
	return vq;
}

static SIMD_FORCE_INLINE void btStoreVector3(const vmVector3 &src, float* p)
{
	vmVector3 v = src;
	storeXYZ(v, p);
}




class btPersistentManifold;

enum {
	PFX_CONSTRAINT_SOLVER_CMD_SETUP_SOLVER_BODIES,
	PFX_CONSTRAINT_SOLVER_CMD_SETUP_CONTACT_CONSTRAINTS,
	PFX_CONSTRAINT_SOLVER_CMD_SETUP_JOINT_CONSTRAINTS,
	PFX_CONSTRAINT_SOLVER_CMD_SOLVE_CONSTRAINTS,
	PFX_CONSTRAINT_SOLVER_CMD_POST_SOLVER
};


struct PfxSetupContactConstraintsIO {
	PfxConstraintPair *offsetContactPairs;
	uint32_t numContactPairs1;
	btPersistentManifold*	offsetContactManifolds;
	class TrbState *offsetRigStates;
	struct PfxSolverBody *offsetSolverBodies;
	uint32_t numRigidBodies;
	float separateBias;
	float timeStep;
	class btCriticalSection* criticalSection;
};



struct PfxSolveConstraintsIO {
	PfxParallelGroup *contactParallelGroup;
	PfxParallelBatch *contactParallelBatches;
	PfxConstraintPair *contactPairs;
	uint32_t numContactPairs;
	btPersistentManifold *offsetContactManifolds;
	PfxParallelGroup *jointParallelGroup;
	PfxParallelBatch *jointParallelBatches;
	PfxConstraintPair *jointPairs;
	uint32_t numJointPairs;
	struct btSolverConstraint* offsetSolverConstraints;
	TrbState *offsetRigStates1;
	PfxSolverBody *offsetSolverBodies;
	uint32_t numRigidBodies;
	uint32_t iteration;

	uint32_t	taskId;
	
	class btBarrier* barrier;

};

struct PfxPostSolverIO {
	TrbState *states;
	PfxSolverBody *solverBodies;
	uint32_t numRigidBodies;
};

ATTRIBUTE_ALIGNED16(struct) btConstraintSolverIO {
	uint8_t cmd;
	union {
		PfxSetupContactConstraintsIO setupContactConstraints;
		PfxSolveConstraintsIO solveConstraints;
		PfxPostSolverIO postSolver;
	};
	
	//SPU only
	uint32_t barrierAddr2;
	uint32_t criticalsectionAddr2;
	uint32_t maxTasks1;
};




void	SolverThreadFunc(void* userPtr,void* lsMemory);
void*	SolverlsMemoryFunc();
///The btParallelConstraintSolver performs computations on constraint rows in parallel
///Using the cross-platform threading it supports Windows, Linux, Mac OSX and PlayStation 3 Cell SPUs
class btParallelConstraintSolver : public btSequentialImpulseConstraintSolver
{
	
protected:
	struct btParallelSolverMemoryCache*	m_memoryCache;

	class btThreadSupportInterface*	m_solverThreadSupport;

	struct btConstraintSolverIO* m_solverIO;
	class btBarrier*			m_barrier;
	class btCriticalSection*	m_criticalSection;


public:

	btParallelConstraintSolver(class btThreadSupportInterface* solverThreadSupport);
	
	virtual ~btParallelConstraintSolver();

	virtual btScalar solveGroup(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifold,int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& info, btIDebugDraw* debugDrawer, btStackAlloc* stackAlloc,btDispatcher* dispatcher);

};



#endif //__BT_PARALLEL_CONSTRAINT_SOLVER_H