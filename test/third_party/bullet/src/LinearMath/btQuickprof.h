
/***************************************************************************************************
**
** Real-Time Hierarchical Profiling for Game Programming Gems 3
**
** by Greg Hjelstrom & Byon Garrabrant
**
***************************************************************************************************/

// Credits: The Clock class was inspired by the Timer classes in 
// Ogre (www.ogre3d.org).



#ifndef BT_QUICK_PROF_H
#define BT_QUICK_PROF_H

//To disable built-in profiling, please comment out next line
//#define BT_NO_PROFILE 1
#ifndef BT_NO_PROFILE
#include <stdio.h>//@todo remove this, backwards compatibility
#include "btScalar.h"
#include "btAlignedAllocator.h"
#include <new>





#define USE_BT_CLOCK 1

#ifdef USE_BT_CLOCK

///The btClock is a portable basic clock that measures accurate time in seconds, use for profiling.
class btClock
{
public:
	btClock();

	btClock(const btClock& other);
	btClock& operator=(const btClock& other);

	~btClock();

	/// Resets the initial reference time.
	void reset();

	/// Returns the time in ms since the last call to reset or since 
	/// the btClock was created.
	unsigned long int getTimeMilliseconds();

	/// Returns the time in us since the last call to reset or since 
	/// the Clock was created.
	unsigned long int getTimeMicroseconds();
private:
	struct btClockData* m_data;
};

#endif //USE_BT_CLOCK




///A node in the Profile Hierarchy Tree
class	CProfileNode {

public:
	CProfileNode( const char * name, CProfileNode * parent );
	~CProfileNode( void );

	CProfileNode * Get_Sub_Node( const char * name );

	CProfileNode * Get_Parent( void )		{ return Parent; }
	CProfileNode * Get_Sibling( void )		{ return Sibling; }
	CProfileNode * Get_Child( void )			{ return Child; }

	void				CleanupMemory();
	void				Reset( void );
	void				Call( void );
	bool				Return( void );

	const char *	Get_Name( void )				{ return Name; }
	int				Get_Total_Calls( void )		{ return TotalCalls; }
	float				Get_Total_Time( void )		{ return TotalTime; }

protected:

	const char *	Name;
	int				TotalCalls;
	float				TotalTime;
	unsigned long int			StartTime;
	int				RecursionCounter;

	CProfileNode *	Parent;
	CProfileNode *	Child;
	CProfileNode *	Sibling;
};

///An iterator to navigate through the tree
class CProfileIterator
{
public:
	// Access all the children of the current parent
	void				First(void);
	void				Next(void);
	bool				Is_Done(void);
	bool                Is_Root(void) { return (CurrentParent->Get_Parent() == 0); }

	void				Enter_Child( int index );		// Make the given child the new parent
	void				Enter_Largest_Child( void );	// Make the largest child the new parent
	void				Enter_Parent( void );			// Make the current parent's parent the new parent

	// Access the current child
	const char *	Get_Current_Name( void )			{ return CurrentChild->Get_Name(); }
	int				Get_Current_Total_Calls( void )	{ return CurrentChild->Get_Total_Calls(); }
	float				Get_Current_Total_Time( void )	{ return CurrentChild->Get_Total_Time(); }

	// Access the current parent
	const char *	Get_Current_Parent_Name( void )			{ return CurrentParent->Get_Name(); }
	int				Get_Current_Parent_Total_Calls( void )	{ return CurrentParent->Get_Total_Calls(); }
	float				Get_Current_Parent_Total_Time( void )	{ return CurrentParent->Get_Total_Time(); }

protected:

	CProfileNode *	CurrentParent;
	CProfileNode *	CurrentChild;

	CProfileIterator( CProfileNode * start );
	friend	class		CProfileManager;
};


///The Manager for the Profile system
class	CProfileManager {
public:
	static	void						Start_Profile( const char * name );
	static	void						Stop_Profile( void );

	static	void						CleanupMemory(void)
	{
		Root.CleanupMemory();
	}

	static	void						Reset( void );
	static	void						Increment_Frame_Counter( void );
	static	int						Get_Frame_Count_Since_Reset( void )		{ return FrameCounter; }
	static	float						Get_Time_Since_Reset( void );

	static	CProfileIterator *	Get_Iterator( void )	
	{ 
		
		return new CProfileIterator( &Root ); 
	}
	static	void						Release_Iterator( CProfileIterator * iterator ) { delete ( iterator); }

	static void	dumpRecursive(CProfileIterator* profileIterator, int spacing);

	static void	dumpAll();

private:
	static	CProfileNode			Root;
	static	CProfileNode *			CurrentNode;
	static	int						FrameCounter;
	static	unsigned long int					ResetTime;
};


///ProfileSampleClass is a simple way to profile a function's scope
///Use the BT_PROFILE macro at the start of scope to time
class	CProfileSample {
public:
	CProfileSample( const char * name )
	{ 
		CProfileManager::Start_Profile( name ); 
	}

	~CProfileSample( void )					
	{ 
		CProfileManager::Stop_Profile(); 
	}
};


#define	BT_PROFILE( name )			CProfileSample __profile( name )

#else

#define	BT_PROFILE( name )

#endif //#ifndef BT_NO_PROFILE



#endif //BT_QUICK_PROF_H


