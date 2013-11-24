//
// HtmlUtils.h
//
//  Created on: Jun 8, 2011
//      Author: Joshua Richardson <jric@chegg.com>
//  Copyright 2011
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2011 Joshua Richardson <jric@chegg.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef HTMLUTILS_H_
#define HTMLUTILS_H_

#include <math.h> // fabs
#include "goo/gtypes.h" // GBool

// Returns true iff the difference between a and b is less than the threshold
// We always use fuzzy math when comparing decimal numbers due to imprecision
inline GBool is_within(double a, double thresh, double b) {
	return fabs(a-b) < thresh;
}

inline GBool rot_matrices_equal(const double * const mat0, const double * const mat1) {
	return is_within(mat0[0], .1, mat1[0]) && is_within(mat0[1], .1, mat1[1]) &&
			is_within(mat0[2], .1, mat1[2]) && is_within(mat0[3], .1, mat1[3]);
}

// rotation is (cos q, sin q, -sin q, cos q, 0, 0)
// sin q is zero iff there is no rotation, or 180 deg. rotation;
// for 180 rotation, cos q will be negative
inline GBool isMatRotOrSkew(const double * const mat) {
	return mat[0] < 0 || !is_within(mat[1], .1, 0);
}

// Alters the matrix so that it does not scale a vector's x component;
// If the matrix does not skew, then that will also normalize the y
//  component, keeping any rotation, but removing scaling.
inline void normalizeRotMat(double *mat) {
	double scale = fabs(mat[0] + mat[1]);
	if (!scale) return;
	for (int i = 0; i < 4; i++) mat[i] /= scale;
}

#endif /* HTMLUTILS_H_ */
