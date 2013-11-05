/* -*- mode: C -*-
 *
 * Copyright (c) 2007-2010 The University of Utah
 * All rights reserved.
 *
 * This file is part of `csmith', a random generator of C programs.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RANDOM_RUNTIME_H
#define RANDOM_RUNTIME_H

#ifdef CSMITH_MINIMAL
#include "csmith_minimal.h"
#else

/*****************************************************************************/

#include <string.h>

#define __STDC_LIMIT_MACROS
#include "random_inc.h"

static uint32_t crc32_tab[256];
static uint32_t crc32_context = 0xFFFFFFFFUL;

static void 
crc32_gentab (void)
{
	uint32_t crc;
	const uint32_t poly = 0xEDB88320UL;
	int i, j;
	
	for (i = 0; i < 256; i++) {
		crc = i;
		for (j = 8; j > 0; j--) {
			if (crc & 1) {
				crc = (crc >> 1) ^ poly;
			} else {
				crc >>= 1;
			}
		}
		crc32_tab[i] = crc;
	}
}

static void 
crc32_byte (uint8_t b) {
	crc32_context = 
		((crc32_context >> 8) & 0x00FFFFFF) ^ 
		crc32_tab[(crc32_context ^ b) & 0xFF];
}

#if defined(__SPLAT__) || defined (__COMPCERT__) || defined(NO_LONGLONG)
static void 
crc32_8bytes (uint32_t val)
{
	crc32_byte ((val>>0) & 0xff);
	crc32_byte ((val>>8) & 0xff);
	crc32_byte ((val>>16) & 0xff);
	crc32_byte ((val>>24) & 0xff);
}

static void 
transparent_crc (uint32_t val, char* vname, int flag)
{
	crc32_8bytes(val);
	if (flag) {
  		printf("...checksum after hashing %s : %X\n", vname, crc32_context ^ 0xFFFFFFFFU);
	}
}
#else
static void 
crc32_8bytes (uint64_t val)
{
	crc32_byte ((val>>0) & 0xff);
	crc32_byte ((val>>8) & 0xff);
	crc32_byte ((val>>16) & 0xff);
	crc32_byte ((val>>24) & 0xff);
	crc32_byte ((val>>32) & 0xff);
	crc32_byte ((val>>40) & 0xff);
	crc32_byte ((val>>48) & 0xff);
	crc32_byte ((val>>56) & 0xff);
}

static void 
transparent_crc (uint64_t val, char* vname, int flag)
{
	crc32_8bytes(val);
	if (flag) {
  		printf("...checksum after hashing %s : %lX\n", vname, crc32_context ^ 0xFFFFFFFFUL);
	}
}
#endif

/*****************************************************************************/

#endif

#endif /* RANDOM_RUNTIME_H */

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */

/* End of file. */
