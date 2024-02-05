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

#ifndef BT_CONTACT_SOLVER_INFO
#define BT_CONTACT_SOLVER_INFO

enum	btSolverMode
{
	SOLVER_RANDMIZE_ORDER = 1,
	SOLVER_FRICTION_SEPARATE = 2,
	SOLVER_USE_WARMSTARTING = 4,
	SOLVER_USE_FRICTION_WARMSTARTING = 8,
	SOLVER_USE_2_FRICTION_DIRECTIONS = 16,
	SOLVER_ENABLE_FRICTION_DIRECTION_CACHING = 32,
	SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION = 64,
	SOLVER_CACHE_FRIENDLY = 128,
	SOLVER_SIMD = 256,	//enabled for Windows, the solver innerloop is branchless SIMD, 40% faster than FPU/scalar version
	SOLVER_CUDA = 512	//will be open sourced during Game Developers Conference 2009. Much faster.
};

struct btContactSolverInfoData
{
	

	btScalar	m_tau;
	btScalar	m_damping;//global non-contact constraint damping, can be locally overridden by constraints during 'getInfo2'.
	btScalar	m_friction;
	btScalar	m_timeStep;
	btScalar	m_restitution;
	int		m_numIterations;
	btScalar	m_maxErrorReduction;
	btScalar	m_sor;
	btScalar	m_erp;//used as Baumgarte factor
	btScalar	m_erp2;//used in Split Impulse
	btScalar	m_globalCfm;//constraint force mixing
	int			m_splitImpulse;
	btScalar	m_splitImpulsePenetrationThreshold;
	btScalar	m_linearSlop;
	btScalar	m_warmstartingFactor;

	int			m_solverMode;
	int	m_restingContactRestitutionThreshold;
	int			m_minimumSolverBatchSize;


};

struct btContactSolverInfo : public btContactSolverInfoData
{

	

	inline btContactSolverInfo()
	{
		m_tau = btScalar(0.6);
		m_damping = btScalar(1.0);
		m_friction = btScalar(0.3);
		m_restitution = btScalar(0.);
		m_maxErrorReduction = btScalar(20.);
		m_numIterations = 10;
		m_erp = btScalar(0.2);
		m_erp2 = btScalar(0.1);
		m_globalCfm = btScalar(0.);
		m_sor = btScalar(1.);
		m_splitImpulse = false;
		m_splitImpulsePenetrationThreshold = -0.02f;
		m_linearSlop = btScalar(0.0);
		m_warmstartingFactor=btScalar(0.85);
		m_solverMode = SOLVER_USE_WARMSTARTING | SOLVER_SIMD;// | SOLVER_RANDMIZE_ORDER;
		m_restingContactRestitutionThreshold = 2;//resting contact lifetime threshold to disable restitution
		m_minimumSolverBatchSize = 128; //try to combine islands until the amount of constraints reaches this limit
	}
};

#endif //BT_CONTACT_SOLVER_INFO
