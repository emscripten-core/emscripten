/*
 * grandom.cc
 *
 * This file is licensed under the GPLv2 or later
 *
 * Pseudo-random number generation
 *
 * Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
 */

#include <config.h>
#include "grandom.h"
#include "gtypes.h"

#ifdef HAVE_RAND_R // rand_r backend (POSIX)

static GBool initialized = gFalse;

#include <stdlib.h>
#include <time.h>
static unsigned int seed;

static void initialize() {
  if (!initialized) {
    seed = time(NULL);
    initialized = gTrue;
  }
}

void grandom_fill(Guchar *buff, int size)
{
  initialize();
  while (size--)
    *buff++ = rand_r(&seed) % 256;
}

double grandom_double()
{
  initialize();
  return rand_r(&seed) / (1 + (double)RAND_MAX);
}

#else // srand+rand backend (unsafe, because it may interfere with the application)

static GBool initialized = gFalse;

#include <stdlib.h>
#include <time.h>

static void initialize() {
  if (!initialized) {
    srand(time(NULL));
    initialized = gTrue;
  }
}

void grandom_fill(Guchar *buff, int size)
{
  initialize();
  while (size--)
    *buff++ = rand() % 256;
}

double grandom_double()
{
  initialize();
  return rand() / (1 + (double)RAND_MAX);
}

#endif
