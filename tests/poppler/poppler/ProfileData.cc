//========================================================================
//
// ProfileData.cc
//
// Copyright 2005 Jonathan Blandford <jrb@gnome.org>
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <stddef.h>
#include "ProfileData.h"

//------------------------------------------------------------------------
// ProfileData
//------------------------------------------------------------------------

ProfileData::ProfileData() {
	count = 0;
	total = 0.0;
	min = 0.0;
	max = 0.0;
}

void
ProfileData::addElement (double elapsed) {
	if (count == 0) {
		min = elapsed;
		max = elapsed;
	} else {
		if (elapsed < min)
			min = elapsed;
		if (elapsed > max)
			max = elapsed;
	}
	total += elapsed;
	count ++;
}

