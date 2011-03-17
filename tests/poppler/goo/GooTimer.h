//========================================================================
//
// GooTimer.cc
//
// This file is licensed under GPLv2 or later
//
// Copyright 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright 2007 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Inspired by gtimer.c in glib, which is Copyright 2000 by the GLib Team
//
//========================================================================

#ifndef GOOTIMER_H
#define GOOTIMER_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "gtypes.h"
#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

//------------------------------------------------------------------------
// GooTimer
//------------------------------------------------------------------------

class GooTimer {
public:

  // Create a new timer.
  GooTimer();

  void start();
  void stop();
  double getElapsed();

private:
#ifdef HAVE_GETTIMEOFDAY
  struct timeval start_time;
  struct timeval end_time;
#elif defined(_WIN32)
  LARGE_INTEGER start_time;
  LARGE_INTEGER end_time;
#endif
  GBool active;
};

#endif
