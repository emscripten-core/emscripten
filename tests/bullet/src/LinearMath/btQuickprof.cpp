/*

***************************************************************************************************
**
** profile.cpp
**
** Real-Time Hierarchical Profiling for Game Programming Gems 3
**
** by Greg Hjelstrom & Byon Garrabrant
**
***************************************************************************************************/

// Credits: The Clock class was inspired by the Timer classes in 
// Ogre (www.ogre3d.org).

#include "btQuickprof.h"

#ifndef BT_NO_PROFILE


static btClock gProfileClock;


#ifdef __CELLOS_LV2__
#include <sys/sys_time.h>
#include <sys/time_util.h>
#include <stdio.h>
#endif

#if defined (SUNOS) || defined (__SUNOS__) 
#include <stdio.h> 
#endif

#if defined(WIN32) || defined(_WIN32)

#define BT_USE_WINDOWS_TIMERS
#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOMCX
#define NOIME 

#ifdef _XBOX
	#include <Xtl.h>
#else //_XBOX
	#include <windows.h>
#endif //_XBOX

#include <time.h>


#else //_WIN32
#include <sys/time.h>
#endif //_WIN32

#define mymin(a,b) (a > b ? a : b)

struct btClockData
{

#ifdef BT_USE_WINDOWS_TIMERS
	LARGE_INTEGER mClockFrequency;
	DWORD mStartTick;
	LONGLONG mPrevElapsedTime;
	LARGE_INTEGER mStartTime;
#else
#ifdef __CELLOS_LV2__
	uint64_t	mStartTime;
#else
	struct timeval mStartTime;
#endif
#endif //__CELLOS_LV2__

};

///The btClock is a portable basic clock that measures accurate time in seconds, use for profiling.
btClock::btClock()
{
	m_data = new btClockData;
#ifdef BT_USE_WINDOWS_TIMERS
	QueryPerformanceFrequency(&m_data->mClockFrequency);
#endif
	reset();
}

btClock::~btClock()
{
	delete m_data;
}

btClock::btClock(const btClock& other)
{
	m_data = new btClockData;
	*m_data = *other.m_data;
}

btClock& btClock::operator=(const btClock& other)
{
	*m_data = *other.m_data;
	return *this;
}


	/// Resets the initial reference time.
void btClock::reset()
{
#ifdef BT_USE_WINDOWS_TIMERS
	QueryPerformanceCounter(&m_data->mStartTime);
	m_data->mStartTick = GetTickCount();
	m_data->mPrevElapsedTime = 0;
#else
#ifdef __CELLOS_LV2__

	typedef uint64_t  ClockSize;
	ClockSize newTime;
	//__asm __volatile__( "mftb %0" : "=r" (newTime) : : "memory");
	SYS_TIMEBASE_GET( newTime );
	m_data->mStartTime = newTime;
#else
	gettimeofday(&m_data->mStartTime, 0);
#endif
#endif
}

/// Returns the time in ms since the last call to reset or since 
/// the btClock was created.
unsigned long int btClock::getTimeMilliseconds()
{
#ifdef BT_USE_WINDOWS_TIMERS
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	LONGLONG elapsedTime = currentTime.QuadPart - 
		m_data->mStartTime.QuadPart;
		// Compute the number of millisecond ticks elapsed.
	unsigned long msecTicks = (unsigned long)(1000 * elapsedTime / 
		m_data->mClockFrequency.QuadPart);
		// Check for unexpected leaps in the Win32 performance counter.  
	// (This is caused by unexpected data across the PCI to ISA 
		// bridge, aka south bridge.  See Microsoft KB274323.)
		unsigned long elapsedTicks = GetTickCount() - m_data->mStartTick;
		signed long msecOff = (signed long)(msecTicks - elapsedTicks);
		if (msecOff < -100 || msecOff > 100)
		{
			// Adjust the starting time forwards.
			LONGLONG msecAdjustment = mymin(msecOff * 
				m_data->mClockFrequency.QuadPart / 1000, elapsedTime - 
				m_data->mPrevElapsedTime);
			m_data->mStartTime.QuadPart += msecAdjustment;
			elapsedTime -= msecAdjustment;

			// Recompute the number of millisecond ticks elapsed.
			msecTicks = (unsigned long)(1000 * elapsedTime / 
				m_data->mClockFrequency.QuadPart);
		}

		// Store the current elapsed time for adjustments next time.
		m_data->mPrevElapsedTime = elapsedTime;

		return msecTicks;
#else

#ifdef __CELLOS_LV2__
		uint64_t freq=sys_time_get_timebase_frequency();
		double dFreq=((double) freq) / 1000.0;
		typedef uint64_t  ClockSize;
		ClockSize newTime;
		SYS_TIMEBASE_GET( newTime );
		//__asm __volatile__( "mftb %0" : "=r" (newTime) : : "memory");

		return (unsigned long int)((double(newTime-m_data->mStartTime)) / dFreq);
#else

		struct timeval currentTime;
		gettimeofday(&currentTime, 0);
		return (currentTime.tv_sec - m_data->mStartTime.tv_sec) * 1000 + 
			(currentTime.tv_usec - m_data->mStartTime.tv_usec) / 1000;
#endif //__CELLOS_LV2__
#endif
}

	/// Returns the time in us since the last call to reset or since 
	/// the Clock was created.
unsigned long int btClock::getTimeMicroseconds()
{
#ifdef BT_USE_WINDOWS_TIMERS
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);
		LONGLONG elapsedTime = currentTime.QuadPart - 
			m_data->mStartTime.QuadPart;

		// Compute the number of millisecond ticks elapsed.
		unsigned long msecTicks = (unsigned long)(1000 * elapsedTime / 
			m_data->mClockFrequency.QuadPart);

		// Check for unexpected leaps in the Win32 performance counter.  
		// (This is caused by unexpected data across the PCI to ISA 
		// bridge, aka south bridge.  See Microsoft KB274323.)
		unsigned long elapsedTicks = GetTickCount() - m_data->mStartTick;
		signed long msecOff = (signed long)(msecTicks - elapsedTicks);
		if (msecOff < -100 || msecOff > 100)
		{
			// Adjust the starting time forwards.
			LONGLONG msecAdjustment = mymin(msecOff * 
				m_data->mClockFrequency.QuadPart / 1000, elapsedTime - 
				m_data->mPrevElapsedTime);
			m_data->mStartTime.QuadPart += msecAdjustment;
			elapsedTime -= msecAdjustment;
		}

		// Store the current elapsed time for adjustments next time.
		m_data->mPrevElapsedTime = elapsedTime;

		// Convert to microseconds.
		unsigned long usecTicks = (unsigned long)(1000000 * elapsedTime / 
			m_data->mClockFrequency.QuadPart);

		return usecTicks;
#else

#ifdef __CELLOS_LV2__
		uint64_t freq=sys_time_get_timebase_frequency();
		double dFreq=((double) freq)/ 1000000.0;
		typedef uint64_t  ClockSize;
		ClockSize newTime;
		//__asm __volatile__( "mftb %0" : "=r" (newTime) : : "memory");
		SYS_TIMEBASE_GET( newTime );

		return (unsigned long int)((double(newTime-m_data->mStartTime)) / dFreq);
#else

		struct timeval currentTime;
		gettimeofday(&currentTime, 0);
		return (currentTime.tv_sec - m_data->mStartTime.tv_sec) * 1000000 + 
			(currentTime.tv_usec - m_data->mStartTime.tv_usec);
#endif//__CELLOS_LV2__
#endif 
}





inline void Profile_Get_Ticks(unsigned long int * ticks)
{
	*ticks = gProfileClock.getTimeMicroseconds();
}

inline float Profile_Get_Tick_Rate(void)
{
//	return 1000000.f;
	return 1000.f;

}



/***************************************************************************************************
**
** CProfileNode
**
***************************************************************************************************/

/***********************************************************************************************
 * INPUT:                                                                                      *
 * name - pointer to a static string which is the name of this profile node                    *
 * parent - parent pointer                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The name is assumed to be a static pointer, only the pointer is stored and compared for     *
 * efficiency reasons.                                                                         *
 *=============================================================================================*/
CProfileNode::CProfileNode( const char * name, CProfileNode * parent ) :
	Name( name ),
	TotalCalls( 0 ),
	TotalTime( 0 ),
	StartTime( 0 ),
	RecursionCounter( 0 ),
	Parent( parent ),
	Child( NULL ),
	Sibling( NULL )
{
	Reset();
}


void	CProfileNode::CleanupMemory()
{
	delete ( Child);
	Child = NULL;
	delete ( Sibling);
	Sibling = NULL;
}

CProfileNode::~CProfileNode( void )
{
	delete ( Child);
	delete ( Sibling);
}


/***********************************************************************************************
 * INPUT:                                                                                      *
 * name - static string pointer to the name of the node we are searching for                   *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * All profile names are assumed to be static strings so this function uses pointer compares   *
 * to find the named node.                                                                     *
 *=============================================================================================*/
CProfileNode * CProfileNode::Get_Sub_Node( const char * name )
{
	// Try to find this sub node
	CProfileNode * child = Child;
	while ( child ) {
		if ( child->Name == name ) {
			return child;
		}
		child = child->Sibling;
	}

	// We didn't find it, so add it
	
	CProfileNode * node = new CProfileNode( name, this );
	node->Sibling = Child;
	Child = node;
	return node;
}


void	CProfileNode::Reset( void )
{
	TotalCalls = 0;
	TotalTime = 0.0f;
	

	if ( Child ) {
		Child->Reset();
	}
	if ( Sibling ) {
		Sibling->Reset();
	}
}


void	CProfileNode::Call( void )
{
	TotalCalls++;
	if (RecursionCounter++ == 0) {
		Profile_Get_Ticks(&StartTime);
	}
}


bool	CProfileNode::Return( void )
{
	if ( --RecursionCounter == 0 && TotalCalls != 0 ) { 
		unsigned long int time;
		Profile_Get_Ticks(&time);
		time-=StartTime;
		TotalTime += (float)time / Profile_Get_Tick_Rate();
	}
	return ( RecursionCounter == 0 );
}


/***************************************************************************************************
**
** CProfileIterator
**
***************************************************************************************************/
CProfileIterator::CProfileIterator( CProfileNode * start )
{
	CurrentParent = start;
	CurrentChild = CurrentParent->Get_Child();
}


void	CProfileIterator::First(void)
{
	CurrentChild = CurrentParent->Get_Child();
}


void	CProfileIterator::Next(void)
{
	CurrentChild = CurrentChild->Get_Sibling();
}


bool	CProfileIterator::Is_Done(void)
{
	return CurrentChild == NULL;
}


void	CProfileIterator::Enter_Child( int index )
{
	CurrentChild = CurrentParent->Get_Child();
	while ( (CurrentChild != NULL) && (index != 0) ) {
		index--;
		CurrentChild = CurrentChild->Get_Sibling();
	}

	if ( CurrentChild != NULL ) {
		CurrentParent = CurrentChild;
		CurrentChild = CurrentParent->Get_Child();
	}
}


void	CProfileIterator::Enter_Parent( void )
{
	if ( CurrentParent->Get_Parent() != NULL ) {
		CurrentParent = CurrentParent->Get_Parent();
	}
	CurrentChild = CurrentParent->Get_Child();
}


/***************************************************************************************************
**
** CProfileManager
**
***************************************************************************************************/

CProfileNode	CProfileManager::Root( "Root", NULL );
CProfileNode *	CProfileManager::CurrentNode = &CProfileManager::Root;
int				CProfileManager::FrameCounter = 0;
unsigned long int			CProfileManager::ResetTime = 0;


/***********************************************************************************************
 * CProfileManager::Start_Profile -- Begin a named profile                                    *
 *                                                                                             *
 * Steps one level deeper into the tree, if a child already exists with the specified name     *
 * then it accumulates the profiling; otherwise a new child node is added to the profile tree. *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - name of this profiling record                                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The string used is assumed to be a static string; pointer compares are used throughout      *
 * the profiling code for efficiency.                                                          *
 *=============================================================================================*/
void	CProfileManager::Start_Profile( const char * name )
{
	if (name != CurrentNode->Get_Name()) {
		CurrentNode = CurrentNode->Get_Sub_Node( name );
	} 
	
	CurrentNode->Call();
}


/***********************************************************************************************
 * CProfileManager::Stop_Profile -- Stop timing and record the results.                       *
 *=============================================================================================*/
void	CProfileManager::Stop_Profile( void )
{
	// Return will indicate whether we should back up to our parent (we may
	// be profiling a recursive function)
	if (CurrentNode->Return()) {
		CurrentNode = CurrentNode->Get_Parent();
	}
}


/***********************************************************************************************
 * CProfileManager::Reset -- Reset the contents of the profiling system                       *
 *                                                                                             *
 *    This resets everything except for the tree structure.  All of the timing data is reset.  *
 *=============================================================================================*/
void	CProfileManager::Reset( void )
{ 
	gProfileClock.reset();
	Root.Reset();
    Root.Call();
	FrameCounter = 0;
	Profile_Get_Ticks(&ResetTime);
}


/***********************************************************************************************
 * CProfileManager::Increment_Frame_Counter -- Increment the frame counter                    *
 *=============================================================================================*/
void CProfileManager::Increment_Frame_Counter( void )
{
	FrameCounter++;
}


/***********************************************************************************************
 * CProfileManager::Get_Time_Since_Reset -- returns the elapsed time since last reset         *
 *=============================================================================================*/
float CProfileManager::Get_Time_Since_Reset( void )
{
	unsigned long int time;
	Profile_Get_Ticks(&time);
	time -= ResetTime;
	return (float)time / Profile_Get_Tick_Rate();
}

#include <stdio.h>

void	CProfileManager::dumpRecursive(CProfileIterator* profileIterator, int spacing)
{
	profileIterator->First();
	if (profileIterator->Is_Done())
		return;

	float accumulated_time=0,parent_time = profileIterator->Is_Root() ? CProfileManager::Get_Time_Since_Reset() : profileIterator->Get_Current_Parent_Total_Time();
	int i;
	int frames_since_reset = CProfileManager::Get_Frame_Count_Since_Reset();
	for (i=0;i<spacing;i++)	printf(".");
	printf("----------------------------------\n");
	for (i=0;i<spacing;i++)	printf(".");
	printf("Profiling: %s (total running time: %.3f ms) ---\n",	profileIterator->Get_Current_Parent_Name(), parent_time );
	float totalTime = 0.f;

	
	int numChildren = 0;
	
	for (i = 0; !profileIterator->Is_Done(); i++,profileIterator->Next())
	{
		numChildren++;
		float current_total_time = profileIterator->Get_Current_Total_Time();
		accumulated_time += current_total_time;
		float fraction = parent_time > SIMD_EPSILON ? (current_total_time / parent_time) * 100 : 0.f;
		{
			int i;	for (i=0;i<spacing;i++)	printf(".");
		}
		printf("%d -- %s (%.2f %%) :: %.3f ms / frame (%d calls)\n",i, profileIterator->Get_Current_Name(), fraction,(current_total_time / (double)frames_since_reset),profileIterator->Get_Current_Total_Calls());
		totalTime += current_total_time;
		//recurse into children
	}

	if (parent_time < accumulated_time)
	{
		printf("what's wrong\n");
	}
	for (i=0;i<spacing;i++)	printf(".");
	printf("%s (%.3f %%) :: %.3f ms\n", "Unaccounted:",parent_time > SIMD_EPSILON ? ((parent_time - accumulated_time) / parent_time) * 100 : 0.f, parent_time - accumulated_time);
	
	for (i=0;i<numChildren;i++)
	{
		profileIterator->Enter_Child(i);
		dumpRecursive(profileIterator,spacing+3);
		profileIterator->Enter_Parent();
	}
}



void	CProfileManager::dumpAll()
{
	CProfileIterator* profileIterator = 0;
	profileIterator = CProfileManager::Get_Iterator();

	dumpRecursive(profileIterator,0);

	CProfileManager::Release_Iterator(profileIterator);
}




#endif //BT_NO_PROFILE
