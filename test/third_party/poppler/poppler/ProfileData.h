//========================================================================
//
// ProfileData.h
//
// Copyright 2005 Jonathan Blandford <jrb@gnome.org>
//
//========================================================================

#ifndef PROFILE_DATA_H
#define PROFILE_DATA_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

//------------------------------------------------------------------------
// ProfileData
//------------------------------------------------------------------------

class ProfileData {
public:

  // Constructor.
  ProfileData ();

  // Destructor.
  ~ProfileData() {}

  void addElement (double elapsed);
  int getCount () { return count; }
  double getTotal () { return total; }
  double getMin () { return max; }
  double getMax () { return max; }
private:
  int count;			// size of <elems> array
  double total;			// number of elements in array
  double min;			// reference count
  double max;			// reference count
};

#endif
