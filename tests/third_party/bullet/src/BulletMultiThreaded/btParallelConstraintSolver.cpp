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


#include "btParallelConstraintSolver.h"
#include "BulletDynamics/ConstraintSolver/btContactSolverInfo.h"
#include "BulletCollision/BroadphaseCollision/btDispatcher.h"
#include "LinearMath/btPoolAllocator.h"

#include "BulletMultiThreaded/vectormath2bullet.h"

#include "LinearMath/btQuickprof.h"
#include "BulletMultiThreaded/btThreadSupportInterface.h"
#ifdef PFX_USE_FREE_VECTORMATH
#include "vecmath/vmInclude.h"
#else
#include "vectormath/vmInclude.h"
#endif //PFX_USE_FREE_VECTORMATH

#include "HeapManager.h"

#include "PlatformDefinitions.h"

//#include "PfxSimdUtils.h"
#include "LinearMath/btScalar.h"

#include "TrbStateVec.h"



/////////////////


#define TMP_BUFF_BYTES (15*1024*1024)
unsigned char ATTRIBUTE_ALIGNED128(tmp_buff[TMP_BUFF_BYTES]);



// Project Gauss Seidel or the equivalent Sequential Impulse
 inline void resolveSingleConstraintRowGeneric(PfxSolverBody& body1,PfxSolverBody& body2,const btSolverConstraint& c)
{

	btScalar deltaImpulse = c.m_rhs-btScalar(c.m_appliedImpulse)*c.m_cfm;
	const btScalar deltaVel1Dotn	=	c.m_contactNormal.dot(getBtVector3(body1.mDeltaLinearVelocity)) 	+ c.m_relpos1CrossNormal.dot(getBtVector3(body1.mDeltaAngularVelocity));
	const btScalar deltaVel2Dotn	=	-c.m_contactNormal.dot(getBtVector3(body2.mDeltaLinearVelocity)) + c.m_relpos2CrossNormal.dot(getBtVector3(body2.mDeltaAngularVelocity));

//	const btScalar delta_rel_vel	=	deltaVel1Dotn-deltaVel2Dotn;
	deltaImpulse	-=	deltaVel1Dotn*c.m_jacDiagABInv;
	deltaImpulse	-=	deltaVel2Dotn*c.m_jacDiagABInv;

	const btScalar sum = btScalar(c.m_appliedImpulse) + deltaImpulse;
	if (sum < c.m_lowerLimit)
	{
		deltaImpulse = c.m_lowerLimit-c.m_appliedImpulse;
		c.m_appliedImpulse = c.m_lowerLimit;
	}
	else if (sum > c.m_upperLimit) 
	{
		deltaImpulse = c.m_upperLimit-c.m_appliedImpulse;
		c.m_appliedImpulse = c.m_upperLimit;
	}
	else
	{
		c.m_appliedImpulse = sum;
	}
	

	if (body1.mMassInv)
	{
		btVector3 linearComponent = c.m_contactNormal*body1.mMassInv;
		body1.mDeltaLinearVelocity += vmVector3(linearComponent.getX()*deltaImpulse,linearComponent.getY()*deltaImpulse,linearComponent.getZ()*deltaImpulse);
		btVector3 tmp=c.m_angularComponentA*(btVector3(deltaImpulse,deltaImpulse,deltaImpulse));
		body1.mDeltaAngularVelocity += vmVector3(tmp.getX(),tmp.getY(),tmp.getZ());
	}

	if (body2.mMassInv)
	{
		btVector3 linearComponent = -c.m_contactNormal*body2.mMassInv;
		body2.mDeltaLinearVelocity += vmVector3(linearComponent.getX()*deltaImpulse,linearComponent.getY()*deltaImpulse,linearComponent.getZ()*deltaImpulse);
		btVector3 tmp = c.m_angularComponentB*((btVector3(deltaImpulse,deltaImpulse,deltaImpulse)));//*m_angularFactor);
		body2.mDeltaAngularVelocity += vmVector3(tmp.getX(),tmp.getY(),tmp.getZ());
	}

	//body1.internalApplyImpulse(c.m_contactNormal*body1.internalGetInvMass(),c.m_angularComponentA,deltaImpulse);
	//body2.internalApplyImpulse(-c.m_contactNormal*body2.internalGetInvMass(),c.m_angularComponentB,deltaImpulse);

}

 
static SIMD_FORCE_INLINE
void pfxSolveLinearConstraintRow(btConstraintRow &constraint,
	vmVector3 &deltaLinearVelocityA,vmVector3 &deltaAngularVelocityA,
	float massInvA,const vmMatrix3 &inertiaInvA,const vmVector3 &rA,
	vmVector3 &deltaLinearVelocityB,vmVector3 &deltaAngularVelocityB,
	float massInvB,const vmMatrix3 &inertiaInvB,const vmVector3 &rB)
{
	const vmVector3 normal(btReadVector3(constraint.m_normal));
	btScalar deltaImpulse = constraint.m_rhs;
	vmVector3 dVA = deltaLinearVelocityA + cross(deltaAngularVelocityA,rA);
	vmVector3 dVB = deltaLinearVelocityB + cross(deltaAngularVelocityB,rB);
	deltaImpulse -= constraint.m_jacDiagInv * dot(normal,dVA-dVB);
	btScalar oldImpulse = constraint.m_accumImpulse;
	constraint.m_accumImpulse = btClamped(oldImpulse + deltaImpulse,constraint.m_lowerLimit,constraint.m_upperLimit);
	deltaImpulse = constraint.m_accumImpulse - oldImpulse;
	deltaLinearVelocityA += deltaImpulse * massInvA * normal;
	deltaAngularVelocityA += deltaImpulse * inertiaInvA * cross(rA,normal);
	deltaLinearVelocityB -= deltaImpulse * massInvB * normal;
	deltaAngularVelocityB -= deltaImpulse * inertiaInvB * cross(rB,normal);

}
 
void btSolveContactConstraint(
	btConstraintRow &constraintResponse,
	btConstraintRow &constraintFriction1,
	btConstraintRow &constraintFriction2,
	const vmVector3 &contactPointA,
	const vmVector3 &contactPointB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	float friction
	)
{
	vmVector3 rA = rotate(solverBodyA.mOrientation,contactPointA);
	vmVector3 rB = rotate(solverBodyB.mOrientation,contactPointB);

	pfxSolveLinearConstraintRow(constraintResponse,
		solverBodyA.mDeltaLinearVelocity,solverBodyA.mDeltaAngularVelocity,solverBodyA.mMassInv,solverBodyA.mInertiaInv,rA,
		solverBodyB.mDeltaLinearVelocity,solverBodyB.mDeltaAngularVelocity,solverBodyB.mMassInv,solverBodyB.mInertiaInv,rB);

	float mf = friction*fabsf(constraintResponse.m_accumImpulse);
	constraintFriction1.m_lowerLimit = -mf;
	constraintFriction1.m_upperLimit =  mf;
	constraintFriction2.m_lowerLimit = -mf;
	constraintFriction2.m_upperLimit =  mf;

	pfxSolveLinearConstraintRow(constraintFriction1,
		solverBodyA.mDeltaLinearVelocity,solverBodyA.mDeltaAngularVelocity,solverBodyA.mMassInv,solverBodyA.mInertiaInv,rA,
		solverBodyB.mDeltaLinearVelocity,solverBodyB.mDeltaAngularVelocity,solverBodyB.mMassInv,solverBodyB.mInertiaInv,rB);

	pfxSolveLinearConstraintRow(constraintFriction2,
		solverBodyA.mDeltaLinearVelocity,solverBodyA.mDeltaAngularVelocity,solverBodyA.mMassInv,solverBodyA.mInertiaInv,rA,
		solverBodyB.mDeltaLinearVelocity,solverBodyB.mDeltaAngularVelocity,solverBodyB.mMassInv,solverBodyB.mInertiaInv,rB);
}


void CustomSolveConstraintsTaskParallel(
	const PfxParallelGroup *contactParallelGroup,const PfxParallelBatch *contactParallelBatches,
	PfxConstraintPair *contactPairs,uint32_t numContactPairs,
	btPersistentManifold* offsetContactManifolds,
	const PfxParallelGroup *jointParallelGroup,const PfxParallelBatch *jointParallelBatches,
	PfxConstraintPair *jointPairs,uint32_t numJointPairs,
	btSolverConstraint* offsetSolverConstraints,
	TrbState *offsetRigStates,
	PfxSolverBody *offsetSolverBodies,
	uint32_t numRigidBodies,
	int iteration,unsigned int taskId,unsigned int numTasks,btBarrier *barrier)
{

	PfxSolverBody staticBody;
	staticBody.mMassInv = 0.f;
	staticBody.mDeltaAngularVelocity=vmVector3(0,0,0);
	staticBody.mDeltaLinearVelocity =vmVector3(0,0,0);


	for(int k=0;k<iteration+1;k++) {
		// Joint
		for(uint32_t phaseId=0;phaseId<jointParallelGroup->numPhases;phaseId++) {
			for(uint32_t batchId=0;batchId<jointParallelGroup->numBatches[phaseId];batchId++) {
				uint32_t numPairs = jointParallelGroup->numPairs[phaseId*PFX_MAX_SOLVER_BATCHES+batchId];
				if(batchId%numTasks == taskId && numPairs > 0) {
					const PfxParallelBatch &batch = jointParallelBatches[phaseId*PFX_MAX_SOLVER_BATCHES+batchId];
					for(uint32_t i=0;i<numPairs;i++) {
						PfxConstraintPair &pair = jointPairs[batch.pairIndices[i]];
						uint16_t iA = pfxGetRigidBodyIdA(pair);
						uint16_t iB = pfxGetRigidBodyIdB(pair);

						
						PfxSolverBody &solverBodyA = iA != 65535 ? offsetSolverBodies[iA] : staticBody;
						PfxSolverBody &solverBodyB = iB != 65535 ? offsetSolverBodies[iB] : staticBody;

						if(k==0) {
							
						}
						else {
							btSolverConstraint* constraintRow = &offsetSolverConstraints[pfxGetContactId1(pair)];
							int numRows = pfxGetNumConstraints(pair);
							int i;
							for (i=0;i<numRows;i++)
							{
								resolveSingleConstraintRowGeneric(solverBodyA,solverBodyB,constraintRow[i]);
							}
							
						}
					}
				}
			}

			barrier->sync();
		}

		// Contact
		for(uint32_t phaseId=0;phaseId<contactParallelGroup->numPhases;phaseId++) {
			for(uint32_t batchId=0;batchId<contactParallelGroup->numBatches[phaseId];batchId++) {
				uint32_t numPairs = contactParallelGroup->numPairs[phaseId*PFX_MAX_SOLVER_BATCHES+batchId];
				if(batchId%numTasks == taskId && numPairs > 0) {
					const PfxParallelBatch &batch = contactParallelBatches[phaseId*PFX_MAX_SOLVER_BATCHES+batchId];
					for(uint32_t i=0;i<numPairs;i++) {
						PfxConstraintPair &pair = contactPairs[batch.pairIndices[i]];
						uint16_t iA = pfxGetRigidBodyIdA(pair);
						uint16_t iB = pfxGetRigidBodyIdB(pair);

						btPersistentManifold& contact = offsetContactManifolds[pfxGetConstraintId1(pair)];


						PfxSolverBody &solverBodyA = offsetSolverBodies[iA];
						PfxSolverBody &solverBodyB = offsetSolverBodies[iB];
						
						for(int j=0;j<contact.getNumContacts();j++) {
							btManifoldPoint& cp = contact.getContactPoint(j);
							
							if(k==0) {
								vmVector3 rA = rotate(solverBodyA.mOrientation,btReadVector3(cp.m_localPointA));
								vmVector3 rB = rotate(solverBodyB.mOrientation,btReadVector3(cp.m_localPointB));
								
								for(int k=0;k<3;k++) {
									vmVector3 normal = btReadVector3(cp.mConstraintRow[k].m_normal);
									float deltaImpulse = cp.mConstraintRow[k].m_accumImpulse;
									solverBodyA.mDeltaLinearVelocity += deltaImpulse * solverBodyA.mMassInv * normal;
									solverBodyA.mDeltaAngularVelocity += deltaImpulse * solverBodyA.mInertiaInv * cross(rA,normal);
									solverBodyB.mDeltaLinearVelocity -= deltaImpulse * solverBodyB.mMassInv * normal;
									solverBodyB.mDeltaAngularVelocity -= deltaImpulse * solverBodyB.mInertiaInv * cross(rB,normal);
								}
							}
							else {
								btSolveContactConstraint(
									cp.mConstraintRow[0],
									cp.mConstraintRow[1],
									cp.mConstraintRow[2],
									btReadVector3(cp.m_localPointA),
									btReadVector3(cp.m_localPointB),
									solverBodyA,
									solverBodyB,
									cp.m_combinedFriction
									);
							}
						}
					}
				}
			}

			if (barrier)
				barrier->sync();
		}
	}
}

void CustomPostSolverTask(
	TrbState *states,
	PfxSolverBody *solverBodies,
	uint32_t numRigidBodies)
{
	for(uint32_t i=0;i<numRigidBodies;i++) {
		TrbState &state = states[i];
		PfxSolverBody &solverBody = solverBodies[i];
		state.setLinearVelocity(state.getLinearVelocity()+solverBody.mDeltaLinearVelocity);
		state.setAngularVelocity(state.getAngularVelocity()+solverBody.mDeltaAngularVelocity);
	}
}

void*	SolverlsMemoryFunc()
{
	//don't create local store memory, just return 0
	return 0;
}


static SIMD_FORCE_INLINE
void pfxGetPlaneSpace(const vmVector3& n, vmVector3& p, vmVector3& q)
{
	if(fabsf(n[2]) > 0.707f) {
		// choose p in y-z plane
		float a = n[1]*n[1] + n[2]*n[2];
		float k = 1.0f/sqrtf(a);
		p[0] = 0;
		p[1] = -n[2]*k;
		p[2] = n[1]*k;
		// set q = n x p
		q[0] = a*k;
		q[1] = -n[0]*p[2];
		q[2] = n[0]*p[1];
	}
	else {
		// choose p in x-y plane
		float a = n[0]*n[0] + n[1]*n[1];
		float k = 1.0f/sqrtf(a);
		p[0] = -n[1]*k;
		p[1] = n[0]*k;
		p[2] = 0;
		// set q = n x p
		q[0] = -n[2]*p[1];
		q[1] = n[2]*p[0];
		q[2] = a*k;
	}
}



#define PFX_CONTACT_SLOP 0.001f

void btSetupContactConstraint(
	btConstraintRow &constraintResponse,
	btConstraintRow &constraintFriction1,
	btConstraintRow &constraintFriction2,
	float penetrationDepth,
	float restitution,
	float friction,
	const vmVector3 &contactNormal,
	const vmVector3 &contactPointA,
	const vmVector3 &contactPointB,
	const TrbState &stateA,
	const TrbState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	float separateBias,
	float timeStep
	)
{
	vmVector3 rA = rotate(solverBodyA.mOrientation,contactPointA);
	vmVector3 rB = rotate(solverBodyB.mOrientation,contactPointB);

	vmMatrix3 K = vmMatrix3::scale(vmVector3(solverBodyA.mMassInv + solverBodyB.mMassInv)) - 
			crossMatrix(rA) * solverBodyA.mInertiaInv * crossMatrix(rA) - 
			crossMatrix(rB) * solverBodyB.mInertiaInv * crossMatrix(rB);

	vmVector3 vA = stateA.getLinearVelocity() + cross(stateA.getAngularVelocity(),rA);
	vmVector3 vB = stateB.getLinearVelocity() + cross(stateB.getAngularVelocity(),rB);
	vmVector3 vAB = vA-vB;

	vmVector3 tangent1,tangent2;
	btPlaneSpace1(contactNormal,tangent1,tangent2);

//	constraintResponse.m_accumImpulse = 0.f;
//	constraintFriction1.m_accumImpulse = 0.f;
//	constraintFriction2.m_accumImpulse = 0.f;

	// Contact Constraint
	{
		vmVector3 normal = contactNormal;

		float denom = dot(K*normal,normal);

		constraintResponse.m_rhs = -(1.0f+restitution)*dot(vAB,normal); // velocity error
		constraintResponse.m_rhs -= (separateBias * btMin(0.0f,penetrationDepth+PFX_CONTACT_SLOP)) / timeStep; // position error
		constraintResponse.m_rhs /= denom;
		constraintResponse.m_jacDiagInv = 1.0f/denom;
		constraintResponse.m_lowerLimit = 0.0f;
		constraintResponse.m_upperLimit = SIMD_INFINITY;
		btStoreVector3(normal,constraintResponse.m_normal);
	}

	// Friction Constraint 1
	{
		vmVector3 normal = tangent1;

		float denom = dot(K*normal,normal);

		constraintFriction1.m_jacDiagInv = 1.0f/denom;
		constraintFriction1.m_rhs = -dot(vAB,normal);
		constraintFriction1.m_rhs *= constraintFriction1.m_jacDiagInv;
		constraintFriction1.m_lowerLimit = 0.0f;
		constraintFriction1.m_upperLimit = SIMD_INFINITY;
		btStoreVector3(normal,constraintFriction1.m_normal);
	}
	
	// Friction Constraint 2
	{
		vmVector3 normal = tangent2;

		float denom = dot(K*normal,normal);

		constraintFriction2.m_jacDiagInv = 1.0f/denom;
		constraintFriction2.m_rhs = -dot(vAB,normal);
		constraintFriction2.m_rhs *= constraintFriction2.m_jacDiagInv;
		constraintFriction2.m_lowerLimit = 0.0f;
		constraintFriction2.m_upperLimit = SIMD_INFINITY;
		btStoreVector3(normal,constraintFriction2.m_normal);
	}
}


void CustomSetupContactConstraintsTask(
	PfxConstraintPair *contactPairs,uint32_t numContactPairs,
	btPersistentManifold*	offsetContactManifolds,
	TrbState *offsetRigStates,
	PfxSolverBody *offsetSolverBodies,
	uint32_t numRigidBodies,
	float separateBias,
	float timeStep)
{
	for(uint32_t i=0;i<numContactPairs;i++) {
		PfxConstraintPair &pair = contactPairs[i];
		if(!pfxGetActive(pair) || pfxGetNumConstraints(pair) == 0 ||
			((pfxGetMotionMaskA(pair)&PFX_MOTION_MASK_STATIC) && (pfxGetMotionMaskB(pair)&PFX_MOTION_MASK_STATIC)) ) {
			continue;
		}

		uint16_t iA = pfxGetRigidBodyIdA(pair);
		uint16_t iB = pfxGetRigidBodyIdB(pair);

		int id = pfxGetConstraintId1(pair);
		btPersistentManifold& contact = offsetContactManifolds[id];


		TrbState &stateA = offsetRigStates[iA];
//		PfxRigBody &bodyA = offsetRigBodies[iA];
		PfxSolverBody &solverBodyA = offsetSolverBodies[iA];

		TrbState &stateB = offsetRigStates[iB];
//		PfxRigBody &bodyB = offsetRigBodies[iB];
		PfxSolverBody &solverBodyB = offsetSolverBodies[iB];
		
		float restitution = 0.5f * (solverBodyA.restitution + solverBodyB.restitution);
		//if(contact.getDuration() > 1) restitution = 0.0f;
		
		float friction = sqrtf(solverBodyA.friction * solverBodyB.friction);

		for(int j=0;j<contact.getNumContacts();j++) {
			btManifoldPoint& cp = contact.getContactPoint(j);
			
			btSetupContactConstraint(
				cp.mConstraintRow[0],
				cp.mConstraintRow[1],
				cp.mConstraintRow[2],
				cp.getDistance(),
				restitution,
				friction,
				btReadVector3(cp.m_normalWorldOnB),//.mConstraintRow[0].m_normal),
				btReadVector3(cp.m_localPointA),
				btReadVector3(cp.m_localPointB),
				stateA,
				stateB,
				solverBodyA,
				solverBodyB,
				separateBias,
				timeStep
				);
		}

		//contact.setCompositeFriction(friction);
	}
}

void	SolverThreadFunc(void* userPtr,void* lsMemory)
{
	btConstraintSolverIO* io = (btConstraintSolverIO*)(userPtr);//arg->io);
	btCriticalSection* criticalsection = io->setupContactConstraints.criticalSection;
	

	//CustomCriticalSection *criticalsection = &io->m_cs;
	switch(io->cmd) {

		case PFX_CONSTRAINT_SOLVER_CMD_SOLVE_CONSTRAINTS:
		CustomSolveConstraintsTaskParallel(
			io->solveConstraints.contactParallelGroup,
			io->solveConstraints.contactParallelBatches,
			io->solveConstraints.contactPairs,
			io->solveConstraints.numContactPairs,
			io->solveConstraints.offsetContactManifolds,

			io->solveConstraints.jointParallelGroup,
			io->solveConstraints.jointParallelBatches,
			io->solveConstraints.jointPairs,
			io->solveConstraints.numJointPairs,
			io->solveConstraints.offsetSolverConstraints,
			io->solveConstraints.offsetRigStates1,
			io->solveConstraints.offsetSolverBodies,
			io->solveConstraints.numRigidBodies,
			io->solveConstraints.iteration,

			io->solveConstraints.taskId,
			io->maxTasks1,
			io->solveConstraints.barrier
			);
		break;

		case PFX_CONSTRAINT_SOLVER_CMD_POST_SOLVER:
			CustomPostSolverTask(	io->postSolver.states,io->postSolver.solverBodies,	io->postSolver.numRigidBodies);
			break;


		case PFX_CONSTRAINT_SOLVER_CMD_SETUP_CONTACT_CONSTRAINTS:
		{
			bool empty = false;
			while(!empty) {
				int start,batch;
				
				criticalsection->lock();

				start = (int)criticalsection->getSharedParam(0);
				batch = (int)criticalsection->getSharedParam(1);

				//PFX_PRINTF("taskId %d start %d num %d\n",arg->taskId,start,batch);

				// 次のバッファをセット
				int nextStart = start + batch;
				int rest = btMax((int)io->setupContactConstraints.numContactPairs1 - nextStart,0);
				int nextBatch = (rest > batch)?batch:rest;

				criticalsection->setSharedParam(0,nextStart);
                criticalsection->setSharedParam(1,nextBatch);

				criticalsection->unlock();
				
				if(batch > 0) {
					CustomSetupContactConstraintsTask(
						io->setupContactConstraints.offsetContactPairs+start,batch,
						io->setupContactConstraints.offsetContactManifolds,
						io->setupContactConstraints.offsetRigStates,
//						io->setupContactConstraints.offsetRigBodies,
						io->setupContactConstraints.offsetSolverBodies,
						io->setupContactConstraints.numRigidBodies,
						io->setupContactConstraints.separateBias,
						io->setupContactConstraints.timeStep);
				}
				else {
					empty = true;
				}
			}
		}
		break;

		default:
			{
				btAssert(0);
			}
	}

}


void CustomSetupContactConstraintsNew(
	PfxConstraintPair *contactPairs1,uint32_t numContactPairs,
	btPersistentManifold *offsetContactManifolds,
	TrbState *offsetRigStates,
	PfxSolverBody *offsetSolverBodies,
	uint32_t numRigidBodies,
	float separationBias,
	float timeStep,
	class btThreadSupportInterface* threadSupport,
	btCriticalSection* criticalSection,
	btConstraintSolverIO *io 
	)
{
	int maxTasks = threadSupport->getNumTasks();

	int div = (int)maxTasks * 4;
	int batch = ((int)numContactPairs + div - 1) / div;
#ifdef __PPU__
		BulletPE2ConstraintSolverSpursSupport* spursThread = (BulletPE2ConstraintSolverSpursSupport*) threadSupport;
#endif
	if (criticalSection)
	{
		criticalSection->setSharedParam(0,0);
		criticalSection->setSharedParam(1,btMin(batch,64)); // batched number
	} else
	{
#ifdef __PPU__
		spursThread->setSharedParam(0,0);
		spursThread->setSharedParam(1,btMin(batch,64)); // batched number
#endif //__PPU__
	}

	for(int t=0;t<maxTasks;t++) {
		io[t].cmd = PFX_CONSTRAINT_SOLVER_CMD_SETUP_CONTACT_CONSTRAINTS;
		io[t].setupContactConstraints.offsetContactPairs = contactPairs1;
		io[t].setupContactConstraints.numContactPairs1 = numContactPairs;
		io[t].setupContactConstraints.offsetRigStates = offsetRigStates;
		io[t].setupContactConstraints.offsetContactManifolds = offsetContactManifolds;		
		io[t].setupContactConstraints.offsetSolverBodies = offsetSolverBodies;
		io[t].setupContactConstraints.numRigidBodies = numRigidBodies;
		io[t].setupContactConstraints.separateBias = separationBias;
		io[t].setupContactConstraints.timeStep = timeStep;
		io[t].setupContactConstraints.criticalSection = criticalSection;
		io[t].maxTasks1 = maxTasks;
#ifdef __PPU__
		io[t].barrierAddr2 = (unsigned int)spursThread->getBarrierAddress();
		io[t].criticalsectionAddr2 = (unsigned int)spursThread->getCriticalSectionAddress();
#endif
	

//#define SEQUENTIAL_SETUP
#ifdef SEQUENTIAL_SETUP
		CustomSetupContactConstraintsTask(contactPairs1,numContactPairs,offsetContactManifolds,offsetRigStates,offsetSolverBodies,numRigidBodies,separationBias,timeStep);
#else
		threadSupport->sendRequest(1,(ppu_address_t)&io[t],t);
#endif

	}
#ifndef SEQUENTIAL_SETUP
	unsigned int arg0,arg1;
	for(int t=0;t<maxTasks;t++) {
		arg0 = t;
		threadSupport->waitForResponse(&arg0,&arg1);
	}
#endif //SEQUENTIAL_SETUP

}


void CustomSplitConstraints(
	PfxConstraintPair *pairs,uint32_t numPairs,
	PfxParallelGroup &group,PfxParallelBatch *batches,
	uint32_t numTasks,
	uint32_t numRigidBodies,
	void *poolBuff,
	uint32_t poolBytes
	)
{
	HeapManager pool((unsigned char*)poolBuff,poolBytes);

	// ステートチェック用ビットフラグテーブル
	int bufSize = sizeof(uint8_t)*numRigidBodies;
	bufSize = ((bufSize+127)>>7)<<7; // 128 bytes alignment
	uint8_t *bodyTable = (uint8_t*)pool.allocate(bufSize,HeapManager::ALIGN128);

	// ペアチェック用ビットフラグテーブル
	uint32_t *pairTable;
	size_t allocSize = sizeof(uint32_t)*((numPairs+31)/32);
	pairTable = (uint32_t*)pool.allocate(allocSize);
	memset(pairTable,0,allocSize);

	// 目標とする分割数
	uint32_t targetCount = btMax(uint32_t(PFX_MIN_SOLVER_PAIRS),btMin(numPairs / (numTasks*2),uint32_t(PFX_MAX_SOLVER_PAIRS)));
	uint32_t startIndex = 0;
	
	uint32_t phaseId;
	uint32_t batchId;
	uint32_t totalCount=0;
	
	uint32_t maxBatches = btMin(numTasks,uint32_t(PFX_MAX_SOLVER_BATCHES));
	
	for(phaseId=0;phaseId<PFX_MAX_SOLVER_PHASES&&totalCount<numPairs;phaseId++) {
		bool startIndexCheck = true;
		
		group.numBatches[phaseId] = 0;
		
		uint32_t i = startIndex;
		
        // チェック用ビットフラグテーブルをクリア
		memset(bodyTable,0xff,bufSize);
		
		for(batchId=0;i<numPairs&&totalCount<numPairs&&batchId<maxBatches;batchId++) {
			uint32_t pairCount=0;
			
			PfxParallelBatch &batch = batches[phaseId*PFX_MAX_SOLVER_BATCHES+batchId];
			uint32_t pairId = 0;
			
			for(;i<numPairs&&pairCount<targetCount;i++) {
				uint32_t idxP = i>>5;
				uint32_t maskP = 1L << (i & 31);
				
				//pair is already assigned to a phase/batch
				if(pairTable[idxP] & maskP) {
					continue;
				}
				
				uint32_t idxA = pfxGetRigidBodyIdA(pairs[i]);
				uint32_t idxB = pfxGetRigidBodyIdB(pairs[i]);

				// 両方ともアクティブでない、または衝突点が０のペアは登録対象からはずす
				if(!pfxGetActive(pairs[i]) || pfxGetNumConstraints(pairs[i]) == 0 ||
					((pfxGetMotionMaskA(pairs[i])&PFX_MOTION_MASK_STATIC) && (pfxGetMotionMaskB(pairs[i])&PFX_MOTION_MASK_STATIC)) ) {
					if(startIndexCheck) 
						startIndex++;
					//assign pair -> skip it because it has no constraints
					pairTable[idxP] |= maskP;
					totalCount++;
					continue;
				}
				
				// 依存性のチェック
				if( (bodyTable[idxA] != batchId && bodyTable[idxA] != 0xff) || 
					(bodyTable[idxB] != batchId && bodyTable[idxB] != 0xff) ) {
					startIndexCheck = false;
					//bodies of the pair are already assigned to another batch within this phase
					continue;
				}
				
				// 依存性判定テーブルに登録
				if(pfxGetMotionMaskA(pairs[i])&PFX_MOTION_MASK_DYNAMIC) 
						bodyTable[idxA] = batchId;
				if(pfxGetMotionMaskB(pairs[i])&PFX_MOTION_MASK_DYNAMIC) 
						bodyTable[idxB] = batchId;
				
				if(startIndexCheck) 
					startIndex++;
				
				pairTable[idxP] |= maskP;
				//add the pair 'i' to the current batch
				batch.pairIndices[pairId++] = i;
				pairCount++;
			}

			group.numPairs[phaseId*PFX_MAX_SOLVER_BATCHES+batchId] = (uint16_t)pairId;
			totalCount += pairCount;
		}

		group.numBatches[phaseId] = batchId;
	}

	group.numPhases = phaseId;

	pool.clear();
}



void CustomSolveConstraintsParallel(
	PfxConstraintPair *contactPairs,uint32_t numContactPairs,
	
	PfxConstraintPair *jointPairs,uint32_t numJointPairs,
	btPersistentManifold* offsetContactManifolds,
	btSolverConstraint* offsetSolverConstraints,
	TrbState *offsetRigStates,
	PfxSolverBody *offsetSolverBodies,
	uint32_t numRigidBodies,
	struct btConstraintSolverIO* io,
	class btThreadSupportInterface* threadSupport,
	int iteration,
	void* poolBuf,
	int poolBytes,
	class btBarrier* barrier)
	{

	int maxTasks = threadSupport->getNumTasks();
//	config.taskManager->setTaskEntry(PFX_SOLVER_ENTRY);

	HeapManager pool((unsigned char*)poolBuf,poolBytes);

	{
		PfxParallelGroup *cgroup = (PfxParallelGroup*)pool.allocate(sizeof(PfxParallelGroup));
		PfxParallelBatch *cbatches = (PfxParallelBatch*)pool.allocate(sizeof(PfxParallelBatch)*(PFX_MAX_SOLVER_PHASES*PFX_MAX_SOLVER_BATCHES),128);
		PfxParallelGroup *jgroup = (PfxParallelGroup*)pool.allocate(sizeof(PfxParallelGroup));
		PfxParallelBatch *jbatches = (PfxParallelBatch*)pool.allocate(sizeof(PfxParallelBatch)*(PFX_MAX_SOLVER_PHASES*PFX_MAX_SOLVER_BATCHES),128);
		
		uint32_t tmpBytes = poolBytes - 2 * (sizeof(PfxParallelGroup) + sizeof(PfxParallelBatch)*(PFX_MAX_SOLVER_PHASES*PFX_MAX_SOLVER_BATCHES) + 128);
		void *tmpBuff = pool.allocate(tmpBytes);
		
		{
			BT_PROFILE("CustomSplitConstraints");
			CustomSplitConstraints(contactPairs,numContactPairs,*cgroup,cbatches,maxTasks,numRigidBodies,tmpBuff,tmpBytes);
			CustomSplitConstraints(jointPairs,numJointPairs,*jgroup,jbatches,maxTasks,numRigidBodies,tmpBuff,tmpBytes);
		}

		{
			BT_PROFILE("PFX_CONSTRAINT_SOLVER_CMD_SOLVE_CONSTRAINTS");
//#define SOLVE_SEQUENTIAL
#ifdef SOLVE_SEQUENTIAL
		CustomSolveConstraintsTask(
			io->solveConstraints.contactParallelGroup,
			io->solveConstraints.contactParallelBatches,
			io->solveConstraints.contactPairs,
			io->solveConstraints.numContactPairs,
			io->solveConstraints.offsetContactManifolds,

			io->solveConstraints.jointParallelGroup,
			io->solveConstraints.jointParallelBatches,
			io->solveConstraints.jointPairs,
			io->solveConstraints.numJointPairs,
			io->solveConstraints.offsetJoints,

			io->solveConstraints.offsetRigStates,
			io->solveConstraints.offsetSolverBodies,
			io->solveConstraints.numRigidBodies,
			io->solveConstraints.iteration,0,1,0);//arg->taskId,1,0);//,arg->maxTasks,arg->barrier);
#else
		for(int t=0;t<maxTasks;t++) {
			io[t].cmd = PFX_CONSTRAINT_SOLVER_CMD_SOLVE_CONSTRAINTS;
			io[t].solveConstraints.contactParallelGroup = cgroup;
			io[t].solveConstraints.contactParallelBatches = cbatches;
			io[t].solveConstraints.contactPairs = contactPairs;
			io[t].solveConstraints.numContactPairs = numContactPairs;
			io[t].solveConstraints.offsetContactManifolds = offsetContactManifolds;
			io[t].solveConstraints.jointParallelGroup = jgroup;
			io[t].solveConstraints.jointParallelBatches = jbatches;
			io[t].solveConstraints.jointPairs = jointPairs;
			io[t].solveConstraints.numJointPairs = numJointPairs;
			io[t].solveConstraints.offsetSolverConstraints = offsetSolverConstraints;
			io[t].solveConstraints.offsetRigStates1 = offsetRigStates;
			io[t].solveConstraints.offsetSolverBodies = offsetSolverBodies;
			io[t].solveConstraints.numRigidBodies = numRigidBodies;
			io[t].solveConstraints.iteration = iteration;
			io[t].solveConstraints.taskId = t;
			io[t].solveConstraints.barrier = barrier;

		io[t].maxTasks1 = maxTasks;
#ifdef __PPU__
		BulletPE2ConstraintSolverSpursSupport* spursThread = (BulletPE2ConstraintSolverSpursSupport*) threadSupport;
		io[t].barrierAddr2 = (unsigned int) spursThread->getBarrierAddress();
		io[t].criticalsectionAddr2 = (unsigned int)spursThread->getCriticalSectionAddress();
#endif

			threadSupport->sendRequest(1,(ppu_address_t)&io[t],t);
		}

		unsigned int arg0,arg1;
		for(int t=0;t<maxTasks;t++) {
			arg0 = t;
			threadSupport->waitForResponse(&arg0,&arg1);
		}
#endif
		}
		pool.clear();
	}

	{
			BT_PROFILE("PFX_CONSTRAINT_SOLVER_CMD_POST_SOLVER");
		int batch = ((int)numRigidBodies + maxTasks - 1) / maxTasks;
		int rest = (int)numRigidBodies;
		int start = 0;

		for(int t=0;t<maxTasks;t++) {
			int num = (rest - batch ) > 0 ? batch : rest;
			io[t].cmd = PFX_CONSTRAINT_SOLVER_CMD_POST_SOLVER;
			io[t].postSolver.states = offsetRigStates + start;
			io[t].postSolver.solverBodies = offsetSolverBodies + start;
			io[t].postSolver.numRigidBodies = (uint32_t)num;
		io[t].maxTasks1 = maxTasks;
#ifdef __PPU__
		BulletPE2ConstraintSolverSpursSupport* spursThread = (BulletPE2ConstraintSolverSpursSupport*) threadSupport;
		io[t].barrierAddr2 = (unsigned int)spursThread->getBarrierAddress();
		io[t].criticalsectionAddr2 = (unsigned int)spursThread->getCriticalSectionAddress();
#endif

#ifdef SOLVE_SEQUENTIAL
			CustomPostSolverTask(	io[t].postSolver.states,io[t].postSolver.solverBodies,	io[t].postSolver.numRigidBodies);
#else
			threadSupport->sendRequest(1,(ppu_address_t)&io[t],t);
#endif
			rest -= num;
			start += num;
		}

		unsigned int arg0,arg1;
		for(int t=0;t<maxTasks;t++) {
#ifndef SOLVE_SEQUENTIAL
			arg0 = t;
			threadSupport->waitForResponse(&arg0,&arg1);
#endif
		}
	}

}



void BPE_customConstraintSolverSequentialNew(unsigned int new_num, PfxBroadphasePair *new_pairs1 ,
									btPersistentManifold* offsetContactManifolds,
									  TrbState* states,int numRigidBodies, 
									  struct PfxSolverBody* solverBodies, 
									  PfxConstraintPair* jointPairs, unsigned int numJoints,
									  btSolverConstraint* offsetSolverConstraints,
									  float separateBias,
									  float timeStep,
									  int iteration,
									  btThreadSupportInterface* solverThreadSupport,
									  btCriticalSection* criticalSection,
									  struct btConstraintSolverIO* solverIO,
									  btBarrier* barrier
									  )
{

	{
		BT_PROFILE("pfxSetupConstraints");

		for(uint32_t i=0;i<numJoints;i++) {
			// 情報の更新
			PfxConstraintPair &pair = jointPairs[i];
			int idA = pfxGetRigidBodyIdA(pair);

			if (idA != 65535)
			{
				pfxSetMotionMaskA(pair,states[pfxGetRigidBodyIdA(pair)].getMotionMask());
			}
			else
			{
				pfxSetMotionMaskA(pair,PFX_MOTION_MASK_STATIC);
			}
			int idB = pfxGetRigidBodyIdB(pair);
			if (idB!= 65535)
			{
				pfxSetMotionMaskB(pair,states[pfxGetRigidBodyIdB(pair)].getMotionMask());
			} else
			{
				pfxSetMotionMaskB(pair,PFX_MOTION_MASK_STATIC);
			}
		}

//		CustomSetupJointConstraintsSeq(			jointPairs,numJoints,joints,			states,			solverBodies,			numRigidBodies,			timeStep);

#ifdef SEQUENTIAL_SETUP
		CustomSetupContactConstraintsSeqNew(
			(PfxConstraintPair*)new_pairs1,new_num,contacts,
			states,
			solverBodies,
			numRigidBodies,
			separateBias,
			timeStep);
#else
		CustomSetupContactConstraintsNew(
			(PfxConstraintPair*)new_pairs1,new_num,
			offsetContactManifolds,
			states,
			solverBodies,
			numRigidBodies,
			separateBias,
			timeStep,
			solverThreadSupport,
			criticalSection,solverIO
			);

#endif //SEQUENTIAL_SETUP

	}
	{
		BT_PROFILE("pfxSolveConstraints");

//#define SEQUENTIAL
#ifdef SEQUENTIAL
		CustomSolveConstraintsSeq(
			(PfxConstraintPair*)new_pairs1,new_num,contacts,
			jointPairs,numJoints,
			states,
			solverBodies,
			numRigidBodies,
			separateBias,
			timeStep,
			iteration);
#else //SEQUENTIAL
		CustomSolveConstraintsParallel(
			(PfxConstraintPair*)new_pairs1,new_num,
			jointPairs,numJoints,
			offsetContactManifolds,
			offsetSolverConstraints,
			states,
			solverBodies,
			numRigidBodies,
			solverIO, solverThreadSupport,
			iteration,
			tmp_buff,
			TMP_BUFF_BYTES,
			barrier
			);

#endif //SEQUENTIAL
	}

}


struct	btParallelSolverMemoryCache
{
	btAlignedObjectArray<TrbState>	m_mystates;
	btAlignedObjectArray<PfxSolverBody>  m_mysolverbodies;
	btAlignedObjectArray<PfxBroadphasePair> m_mypairs;
	btAlignedObjectArray<PfxConstraintPair> m_jointPairs;
	
};


btConstraintSolverIO* createSolverIO(int numThreads)
{
	return new btConstraintSolverIO[numThreads];
}

btParallelConstraintSolver::btParallelConstraintSolver(btThreadSupportInterface* solverThreadSupport)
{
	
	m_solverThreadSupport = solverThreadSupport;//createSolverThreadSupport(maxNumThreads);
	m_solverIO = createSolverIO(m_solverThreadSupport->getNumTasks());

	m_barrier = m_solverThreadSupport->createBarrier();
	m_criticalSection = m_solverThreadSupport->createCriticalSection();

	m_memoryCache = new btParallelSolverMemoryCache();
}
	
btParallelConstraintSolver::~btParallelConstraintSolver()
{
	delete m_memoryCache;
	delete m_solverIO;
}



btScalar btParallelConstraintSolver::solveGroup(btCollisionObject** bodies1,int numRigidBodies,btPersistentManifold** manifoldPtr,int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal, btIDebugDraw* debugDrawer, btStackAlloc* stackAlloc,btDispatcher* dispatcher)
{
	
/*	int sz = sizeof(PfxSolverBody);
	int sz2 = sizeof(vmVector3);
	int sz3 = sizeof(vmMatrix3);
	int sz4 = sizeof(vmQuat);
	int sz5 = sizeof(btConstraintRow);
	int sz6 = sizeof(btSolverConstraint);
	int sz7 = sizeof(TrbState);
*/

	btPersistentManifold* offsetContactManifolds= (btPersistentManifold*) dispatcher->getInternalManifoldPool()->getPoolAddress();

		
	m_memoryCache->m_mysolverbodies.resize(numRigidBodies);
	m_memoryCache->m_mystates.resize(numRigidBodies);

	{
			BT_PROFILE("create states and solver bodies");
	for (int i=0;i<numRigidBodies;i++)
	{
		btCollisionObject* obj = bodies1[i];
		obj->setCompanionId(i);

		PfxSolverBody& solverBody = m_memoryCache->m_mysolverbodies[i];
		btRigidBody* rb = btRigidBody::upcast(obj);
		TrbState& state = m_memoryCache->m_mystates[i];
	
		state.reset();
		const btQuaternion& orgOri = obj->getWorldTransform().getRotation();
		vmQuat orn(orgOri.getX(),orgOri.getY(),orgOri.getZ(),orgOri.getW());
		state.setPosition(getVmVector3(obj->getWorldTransform().getOrigin()));
		state.setOrientation(orn);
		state.setPosition(state.getPosition());
		state.setRigidBodyId(i);
		state.setAngularDamping(0);
		state.setLinearDamping(0);
		
		
		solverBody.mOrientation = state.getOrientation();
		solverBody.mDeltaLinearVelocity = vmVector3(0.0f);
		solverBody.mDeltaAngularVelocity = vmVector3(0.0f);
		solverBody.friction = obj->getFriction();
		solverBody.restitution = obj->getRestitution();
		
		state.resetSleepCount();
		
		//if(state.getMotionMask()&PFX_MOTION_MASK_DYNAMIC) {
		if (rb && (rb->getInvMass()>0.f))
		{
			state.setAngularVelocity(vmVector3(rb->getAngularVelocity().getX(),rb->getAngularVelocity().getY(),rb->getAngularVelocity().getZ()));
			state.setLinearVelocity(vmVector3(rb->getLinearVelocity().getX(),rb->getLinearVelocity().getY(),rb->getLinearVelocity().getZ()));
	
			state.setMotionType(PfxMotionTypeActive);
			vmMatrix3 ori(solverBody.mOrientation);
			vmMatrix3 localInvInertia = vmMatrix3::identity();
			localInvInertia.setCol(0,vmVector3(rb->getInvInertiaDiagLocal().getX(),0,0));
			localInvInertia.setCol(1,vmVector3(0, rb->getInvInertiaDiagLocal().getY(),0));
			localInvInertia.setCol(2,vmVector3(0,0, rb->getInvInertiaDiagLocal().getZ()));

			solverBody.mMassInv = rb->getInvMass();
			solverBody.mInertiaInv = ori * localInvInertia * transpose(ori);
		} else
		{
			state.setAngularVelocity(vmVector3(0));
			state.setLinearVelocity(vmVector3(0));
		
			state.setMotionType(PfxMotionTypeFixed);
			m_memoryCache->m_mysolverbodies[i].mMassInv = 0.f;
			m_memoryCache->m_mysolverbodies[i].mInertiaInv = vmMatrix3(0.0f);
		}

	}
	}



	int totalPoints = 0;
#ifndef USE_C_ARRAYS
	m_memoryCache->m_mypairs.resize(numManifolds);
	m_memoryCache->m_jointPairs.resize(numConstraints);
#endif//USE_C_ARRAYS

	int actualNumManifolds= 0;
	{
		BT_PROFILE("convert manifolds");
		for (int i1=0;i1<numManifolds;i1++)
		{
			if (manifoldPtr[i1]->getNumContacts()>0)
			{
				btPersistentManifold* m = manifoldPtr[i1];
				btCollisionObject* obA = (btCollisionObject*)m->getBody0();
				btCollisionObject* obB = (btCollisionObject*)m->getBody1();
				bool obAisActive = !obA->isStaticOrKinematicObject() && obA->isActive();
				bool obBisActive = !obB->isStaticOrKinematicObject() && obB->isActive();

				if (!obAisActive && !obBisActive)
					continue;


				//int contactId = i1;//actualNumManifolds;
				
				PfxBroadphasePair& pair = m_memoryCache->m_mypairs[actualNumManifolds];
				//init those
				float compFric = obA->getFriction()*obB->getFriction();//@todo
				int idA = obA->getCompanionId();
				int idB = obB->getCompanionId();
				
				m->m_companionIdA = idA;
				m->m_companionIdB = idB;
				
				
			//	if ((mysolverbodies[idA].mMassInv!=0)&&(mysolverbodies[idB].mMassInv!=0))
			//		continue;
				int numPosPoints=0;
				for (int p=0;p<m->getNumContacts();p++)
				{
					//btManifoldPoint& pt = m->getContactPoint(p);
					//float dist = pt.getDistance();
					//if (dist<0.001)
						numPosPoints++;
				}

				
				numPosPoints = numPosPoints;
				totalPoints+=numPosPoints;
				pfxSetRigidBodyIdA(pair,idA);
				pfxSetRigidBodyIdB(pair,idB);
				pfxSetMotionMaskA(pair,m_memoryCache->m_mystates[idA].getMotionMask());
				pfxSetMotionMaskB(pair,m_memoryCache->m_mystates[idB].getMotionMask());
				pfxSetActive(pair,numPosPoints>0);
				
				pfxSetBroadphaseFlag(pair,0);
				int contactId = m-offsetContactManifolds;
				//likely the contact pool is not contiguous, make sure to allocate large enough contact pool
				btAssert(contactId>=0);
				btAssert(contactId<dispatcher->getInternalManifoldPool()->getMaxCount());
				
				pfxSetContactId(pair,contactId);
				pfxSetNumConstraints(pair,numPosPoints);//manifoldPtr[i]->getNumContacts());
				actualNumManifolds++;
			}

		}
	}

	PfxConstraintPair* jointPairs=0;
	jointPairs = numConstraints? &m_memoryCache->m_jointPairs[0]:0;
	int actualNumJoints=0;


	btSolverConstraint* offsetSolverConstraints = 0;

	//if (1)
	{
		
		{
			BT_PROFILE("convert constraints");

			int totalNumRows = 0;
			int i;
			
			m_tmpConstraintSizesPool.resize(numConstraints);
			//calculate the total number of contraint rows
			for (i=0;i<numConstraints;i++)
			{
				btTypedConstraint::btConstraintInfo1& info1 = m_tmpConstraintSizesPool[i];
				constraints[i]->getInfo1(&info1);
				totalNumRows += info1.m_numConstraintRows;
			}
			m_tmpSolverNonContactConstraintPool.resize(totalNumRows);
			offsetSolverConstraints = &m_tmpSolverNonContactConstraintPool[0];

			
			///setup the btSolverConstraints
			int currentRow = 0;

			for (i=0;i<numConstraints;i++)
			{
				const btTypedConstraint::btConstraintInfo1& info1 = m_tmpConstraintSizesPool[i];
				
				if (info1.m_numConstraintRows)
				{
					btAssert(currentRow<totalNumRows);
					btTypedConstraint* constraint = constraints[i];
					btSolverConstraint* currentConstraintRow = &m_tmpSolverNonContactConstraintPool[currentRow];

					btRigidBody& rbA = constraint->getRigidBodyA();
					btRigidBody& rbB = constraint->getRigidBodyB();

					
					int j;
					for ( j=0;j<info1.m_numConstraintRows;j++)
					{
						memset(&currentConstraintRow[j],0,sizeof(btSolverConstraint));
						currentConstraintRow[j].m_lowerLimit = -FLT_MAX;
						currentConstraintRow[j].m_upperLimit = FLT_MAX;
						currentConstraintRow[j].m_appliedImpulse = 0.f;
						currentConstraintRow[j].m_appliedPushImpulse = 0.f;
						currentConstraintRow[j].m_solverBodyA = &rbA;
						currentConstraintRow[j].m_solverBodyB = &rbB;
					}

					rbA.internalGetDeltaLinearVelocity().setValue(0.f,0.f,0.f);
					rbA.internalGetDeltaAngularVelocity().setValue(0.f,0.f,0.f);
					rbB.internalGetDeltaLinearVelocity().setValue(0.f,0.f,0.f);
					rbB.internalGetDeltaAngularVelocity().setValue(0.f,0.f,0.f);



					btTypedConstraint::btConstraintInfo2 info2;
					info2.fps = 1.f/infoGlobal.m_timeStep;
					info2.erp = infoGlobal.m_erp;
					info2.m_J1linearAxis = currentConstraintRow->m_contactNormal;
					info2.m_J1angularAxis = currentConstraintRow->m_relpos1CrossNormal;
					info2.m_J2linearAxis = 0;
					info2.m_J2angularAxis = currentConstraintRow->m_relpos2CrossNormal;
					info2.rowskip = sizeof(btSolverConstraint)/sizeof(btScalar);//check this
					///the size of btSolverConstraint needs be a multiple of btScalar
					btAssert(info2.rowskip*sizeof(btScalar)== sizeof(btSolverConstraint));
					info2.m_constraintError = &currentConstraintRow->m_rhs;
					currentConstraintRow->m_cfm = infoGlobal.m_globalCfm;
					info2.cfm = &currentConstraintRow->m_cfm;
					info2.m_lowerLimit = &currentConstraintRow->m_lowerLimit;
					info2.m_upperLimit = &currentConstraintRow->m_upperLimit;
					info2.m_numIterations = infoGlobal.m_numIterations;
					constraints[i]->getInfo2(&info2);

					
					int idA = constraint->getRigidBodyA().getCompanionId();
					int idB = constraint->getRigidBodyB().getCompanionId();
				

					///finalize the constraint setup
					for ( j=0;j<info1.m_numConstraintRows;j++)
					{
						btSolverConstraint& solverConstraint = currentConstraintRow[j];
						solverConstraint.m_originalContactPoint = constraint;

						solverConstraint.m_companionIdA = idA;
						solverConstraint.m_companionIdB = idB;

						{
							const btVector3& ftorqueAxis1 = solverConstraint.m_relpos1CrossNormal;
							solverConstraint.m_angularComponentA = constraint->getRigidBodyA().getInvInertiaTensorWorld()*ftorqueAxis1*constraint->getRigidBodyA().getAngularFactor();
						}
						{
							const btVector3& ftorqueAxis2 = solverConstraint.m_relpos2CrossNormal;
							solverConstraint.m_angularComponentB = constraint->getRigidBodyB().getInvInertiaTensorWorld()*ftorqueAxis2*constraint->getRigidBodyB().getAngularFactor();
						}

						{
							btVector3 iMJlA = solverConstraint.m_contactNormal*rbA.getInvMass();
							btVector3 iMJaA = rbA.getInvInertiaTensorWorld()*solverConstraint.m_relpos1CrossNormal;
							btVector3 iMJlB = solverConstraint.m_contactNormal*rbB.getInvMass();//sign of normal?
							btVector3 iMJaB = rbB.getInvInertiaTensorWorld()*solverConstraint.m_relpos2CrossNormal;

							btScalar sum = iMJlA.dot(solverConstraint.m_contactNormal);
							sum += iMJaA.dot(solverConstraint.m_relpos1CrossNormal);
							sum += iMJlB.dot(solverConstraint.m_contactNormal);
							sum += iMJaB.dot(solverConstraint.m_relpos2CrossNormal);

							solverConstraint.m_jacDiagABInv = btScalar(1.)/sum;
						}


						///fix rhs
						///todo: add force/torque accelerators
						{
							btScalar rel_vel;
							btScalar vel1Dotn = solverConstraint.m_contactNormal.dot(rbA.getLinearVelocity()) + solverConstraint.m_relpos1CrossNormal.dot(rbA.getAngularVelocity());
							btScalar vel2Dotn = -solverConstraint.m_contactNormal.dot(rbB.getLinearVelocity()) + solverConstraint.m_relpos2CrossNormal.dot(rbB.getAngularVelocity());

							rel_vel = vel1Dotn+vel2Dotn;

							btScalar restitution = 0.f;
							btScalar positionalError = solverConstraint.m_rhs;//already filled in by getConstraintInfo2
							btScalar	velocityError = restitution - rel_vel;// * damping;
							btScalar	penetrationImpulse = positionalError*solverConstraint.m_jacDiagABInv;
							btScalar	velocityImpulse = velocityError *solverConstraint.m_jacDiagABInv;
							solverConstraint.m_rhs = penetrationImpulse+velocityImpulse;
							solverConstraint.m_appliedImpulse = 0.f;

						}
					}

					PfxConstraintPair& pair = jointPairs[actualNumJoints];
					
					int numConstraintRows= info1.m_numConstraintRows;
					pfxSetNumConstraints(pair,numConstraintRows);
					


					pfxSetRigidBodyIdA(pair,idA);
					pfxSetRigidBodyIdB(pair,idB);
					//is this needed?
					pfxSetMotionMaskA(pair,m_memoryCache->m_mystates[idA].getMotionMask());
					pfxSetMotionMaskB(pair,m_memoryCache->m_mystates[idB].getMotionMask());

					pfxSetActive(pair,true);
					int id = currentConstraintRow-offsetSolverConstraints;
					pfxSetContactId(pair,id);
					actualNumJoints++;


				}
				currentRow+=m_tmpConstraintSizesPool[i].m_numConstraintRows;
			}
		}
	}


	
	float separateBias=0.1;//info.m_erp;//or m_erp2?
	float timeStep=infoGlobal.m_timeStep;
	int iteration=infoGlobal.m_numIterations;

	//create a pair for each constraints, copy over info etc
	
	


	
	{
		BT_PROFILE("compute num contacts");
		int totalContacts =0;

		for (int i=0;i<actualNumManifolds;i++)
		{
			PfxConstraintPair* pair = &m_memoryCache->m_mypairs[i];
			totalContacts += pfxGetNumConstraints(*pair);
		}
		//printf("numManifolds = %d\n",numManifolds);
		//printf("totalContacts=%d\n",totalContacts);
	}
	


//	printf("actualNumManifolds=%d\n",actualNumManifolds);
	{
		BT_PROFILE("BPE_customConstraintSolverSequentialNew");
		if (numRigidBodies>0 && (actualNumManifolds+actualNumJoints)>0)
		{
//			PFX_PRINTF("num points = %d\n",totalPoints);
//			PFX_PRINTF("num points PFX = %d\n",total);
			
			
			 
			BPE_customConstraintSolverSequentialNew(
				actualNumManifolds,
				&m_memoryCache->m_mypairs[0],
				offsetContactManifolds,
				&m_memoryCache->m_mystates[0],numRigidBodies,
				&m_memoryCache->m_mysolverbodies[0],
				jointPairs,actualNumJoints,
				offsetSolverConstraints,
				separateBias,timeStep,iteration,
				m_solverThreadSupport,m_criticalSection,m_solverIO,m_barrier);
		}
	}

	//copy results back to bodies
	{
		BT_PROFILE("copy back");
		for (int i=0;i<numRigidBodies;i++)
		{
			btCollisionObject* obj = bodies1[i];
			btRigidBody* rb = btRigidBody::upcast(obj);
			TrbState& state = m_memoryCache->m_mystates[i];
			if (rb && (rb->getInvMass()>0.f))
			{
				rb->setLinearVelocity(btVector3(state.getLinearVelocity().getX(),state.getLinearVelocity().getY(),state.getLinearVelocity().getZ()));
				rb->setAngularVelocity(btVector3(state.getAngularVelocity().getX(),state.getAngularVelocity().getY(),state.getAngularVelocity().getZ()));
			}
		}
	}


	return 0.f;
}
