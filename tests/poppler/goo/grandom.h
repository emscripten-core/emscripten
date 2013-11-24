/*
 * grandom.h
 *
 * This file is licensed under the GPLv2 or later
 *
 * Pseudo-random number generation
 *
 * Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
 */

#ifndef GRANDOM_H
#define GRANDOM_H

#include "gtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Fills the given buffer with random bytes
 */
extern void grandom_fill(Guchar *buff, int size);

/*
 * Returns a random number in [0,1)
 */
extern double grandom_double();

#ifdef __cplusplus
}
#endif

#endif
