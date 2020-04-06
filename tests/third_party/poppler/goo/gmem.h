/*
 * gmem.h
 *
 * Memory routines with out-of-memory checking.
 *
 * Copyright 1996-2003 Glyph & Cog, LLC
 */

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2007-2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Jonathan Kew <jonathan_kew@sil.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef GMEM_H
#define GMEM_H

#include <stdio.h>
#include "poppler/poppler-config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Same as malloc, but prints error message and exits if malloc()
 * returns NULL.
 */
extern void *gmalloc(size_t size);
extern void *gmalloc_checkoverflow(size_t size);

/*
 * Same as realloc, but prints error message and exits if realloc()
 * returns NULL.  If <p> is NULL, calls malloc instead of realloc().
 */
extern void *grealloc(void *p, size_t size);
extern void *grealloc_checkoverflow(size_t size);

/*
 * These are similar to gmalloc and grealloc, but take an object count
 * and size.  The result is similar to allocating nObjs * objSize
 * bytes, but there is an additional error check that the total size
 * doesn't overflow an int.
 * The gmallocn_checkoverflow variant returns NULL instead of exiting
 * the application if a overflow is detected
 */
extern void *gmallocn(int nObjs, int objSize);
extern void *gmallocn_checkoverflow(int nObjs, int objSize);
extern void *gmallocn3(int a, int b, int c);
extern void *gmallocn3_checkoverflow(int a, int b, int c);
extern void *greallocn(void *p, int nObjs, int objSize);
extern void *greallocn_checkoverflow(void *p, int nObjs, int objSize);

/*
 * Same as free, but checks for and ignores NULL pointers.
 */
extern void gfree(void *p);

#ifdef DEBUG_MEM
/*
 * Report on unfreed memory.
 */
extern void gMemReport(FILE *f);
#else
#define gMemReport(f)
#endif

/*
 * Allocate memory and copy a string into it.
 */
extern char *copyString(char *s);

/*
 * Allocate memory and copy a limited-length string to it.
 */
extern char *gstrndup(const char *s, size_t n);

#ifdef __cplusplus
}
#endif

#endif
