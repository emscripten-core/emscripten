/*
 * parseargs.h
 *
 * Command line argument parser.
 *
 * Copyright 1996-2003 Glyph & Cog, LLC
 */

/*========================================================================

 Modified under the Poppler project - http://poppler.freedesktop.org

 All changes made under the Poppler project to this file are licensed
 under GPL version 2 or later

 Copyright (C) 2008 Albert Astals Cid <aacid@kde.org>

 To see a description of the changes please see the Changelog file that
 came with your tarball or type make ChangeLog if you are building from git

========================================================================*/

#ifndef PARSEARGS_H
#define PARSEARGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "goo/gtypes.h"

/*
 * Argument kinds.
 */
typedef enum {
  argFlag,			/* flag (present / not-present) */
				/*   [val: GBool *]             */
  argInt,			/* integer arg    */
				/*   [val: int *] */
  argFP,			/* floating point arg */
				/*   [val: double *]  */
  argString,			/* string arg      */
				/*   [val: char *] */
  /* dummy entries -- these show up in the usage listing only; */
  /* useful for X args, for example                            */
  argFlagDummy,
  argIntDummy,
  argFPDummy,
  argStringDummy
} ArgKind;

/*
 * Argument descriptor.
 */
typedef struct {
  char *arg;			/* the command line switch */
  ArgKind kind;			/* kind of arg */
  void *val;			/* place to store value */
  int size;			/* for argString: size of string */
  char *usage;			/* usage string */
} ArgDesc;

/*
 * Parse command line.  Removes all args which are found in the arg
 * descriptor list <args>.  Stops parsing if "--" is found (and removes
 * it).  Returns gFalse if there was an error.
 */
extern GBool parseArgs(const ArgDesc *args, int *argc, char *argv[]);

/*
 * Print usage message, based on arg descriptor list.
 */
extern void printUsage(char *program, char *otherArgs, const ArgDesc *args);

/*
 * Check if a string is a valid integer or floating point number.
 */
extern GBool isInt(char *s);
extern GBool isFP(char *s);

#ifdef __cplusplus
}
#endif

#endif
