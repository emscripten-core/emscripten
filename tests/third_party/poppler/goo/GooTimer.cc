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

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "GooTimer.h"
#include <string.h>

#define USEC_PER_SEC 1000000

//------------------------------------------------------------------------
// GooTimer
//------------------------------------------------------------------------

GooTimer::GooTimer() {
  start();
}

void GooTimer::start() {
#ifdef HAVE_GETTIMEOFDAY
  gettimeofday(&start_time, NULL);
#elif defined(_WIN32)
  QueryPerformanceCounter(&start_time);
#endif
  active = true;
}

void GooTimer::stop() {
#ifdef HAVE_GETTIMEOFDAY
  gettimeofday(&end_time, NULL);
#elif defined(_WIN32)
  QueryPerformanceCounter(&end_time);
#endif
  active = false;
}

#ifdef HAVE_GETTIMEOFDAY
double GooTimer::getElapsed()
{
  double total;
  struct timeval elapsed;

  if (active)
    gettimeofday(&end_time, NULL);

  if (start_time.tv_usec > end_time.tv_usec) {
      end_time.tv_usec += USEC_PER_SEC;
      end_time.tv_sec--;
  }

  elapsed.tv_usec = end_time.tv_usec - start_time.tv_usec;
  elapsed.tv_sec = end_time.tv_sec - start_time.tv_sec;

  total = elapsed.tv_sec + ((double) elapsed.tv_usec / 1e6);
  if (total < 0)
      total = 0;

  return total;
}
#elif defined(_WIN32)
double GooTimer::getElapsed()
{
  LARGE_INTEGER   freq;
  double          time_in_secs;
  QueryPerformanceFrequency(&freq);

  if (active)
    QueryPerformanceCounter(&end_time);

  time_in_secs = (double)(end_time.QuadPart-start_time.QuadPart)/(double)freq.QuadPart;
  return time_in_secs * 1000.0;

}
#else
double GooTimer::getElapsed()
{
#warning "no support for GooTimer"
  return 0;
}
#endif

