/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2007 Starbreeze Studios

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Written by: Marten Svanfeldt
*/

#ifndef BT_SPU_SYNC_H
#define	BT_SPU_SYNC_H


#include "PlatformDefinitions.h"


#if defined(WIN32)

#define WIN32_LEAN_AND_MEAN
#ifdef _XBOX
#include <Xtl.h>
#else
#include <Windows.h>
#endif

///The btSpinlock is a structure to allow multi-platform synchronization. This allows to port the SPU tasks to other platforms.
class btSpinlock
{
public:
	//typedef volatile LONG SpinVariable;
	typedef CRITICAL_SECTION SpinVariable;

	btSpinlock (SpinVariable* var)
		: spinVariable (var)
	{}

	void Init ()
	{
		//*spinVariable = 0;
		InitializeCriticalSection(spinVariable);
	}

	void Lock ()
	{
		EnterCriticalSection(spinVariable);
	}

	void Unlock ()
	{
		LeaveCriticalSection(spinVariable);
	}

private:
	SpinVariable* spinVariable;
};


#elif defined (__CELLOS_LV2__)

//#include <cell/atomic.h>
#include <cell/sync/mutex.h>

///The btSpinlock is a structure to allow multi-platform synchronization. This allows to port the SPU tasks to other platforms.
class btSpinlock
{
public:
	typedef CellSyncMutex SpinVariable;

	btSpinlock (SpinVariable* var)
		: spinVariable (var)
	{}

	void Init ()
	{
#ifndef __SPU__
		//*spinVariable = 1;
		cellSyncMutexInitialize(spinVariable);
#endif
	}



	void Lock ()
	{
#ifdef __SPU__
		// lock semaphore
		/*while (cellAtomicTestAndDecr32(atomic_buf, (uint64_t)spinVariable) == 0) 
		{

		};*/
		cellSyncMutexLock((uint64_t)spinVariable);
#endif
	}

	void Unlock ()
	{
#ifdef __SPU__
		//cellAtomicIncr32(atomic_buf, (uint64_t)spinVariable);
		cellSyncMutexUnlock((uint64_t)spinVariable);
#endif 
	}


private:
	SpinVariable*	spinVariable;
	ATTRIBUTE_ALIGNED128(uint32_t		atomic_buf[32]);
};

#else
//create a dummy implementation (without any locking) useful for serial processing
class btSpinlock
{
public:
	typedef int  SpinVariable;

	btSpinlock (SpinVariable* var)
		: spinVariable (var)
	{}

	void Init ()
	{
	}

	void Lock ()
	{
	}

	void Unlock ()
	{
	}

private:
	SpinVariable* spinVariable;
};


#endif


#endif //BT_SPU_SYNC_H

