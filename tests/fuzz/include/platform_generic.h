/* -*- mode: C -*-
 *
 *
 * Copyright (c) 2007, 2008 The University of Utah
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

#ifndef PLATFORM_GENERIC_H
#define PLATFORM_GENERIC_H

/*****************************************************************************/

#ifdef STANDALONE
extern int printf (const char *, ...);
#else
#include <stdio.h>
#endif

static void
platform_main_begin(void)
{
	/* Nothing to do. */
}

static void
platform_main_end(uint32_t crc, int flag)
{
#if defined (__FRAMAC)
    Frama_C_dump_assert_each();
#endif
	printf ("checksum = %X\n", crc);
#if defined (LOG_WRAPPERS)
	{
		int i, first;

		printf ("executed wrappers: ");
		first = 1;
		for (i=1; i<N_WRAP+1; i++) {
			if (__executed_checks[i]) {
				if (!first) {
					printf (",");
				} else {
					first = 0;
				}
				printf ("%d", i);
			}
		}
		printf ("\n");

		printf ("dead wrappers: ");
		first = 1;
		for (i=1; i<N_WRAP+1; i++) {
			if (!__executed_checks[i]) {
				if (!first) {
					printf (",");
				} else {
					first = 0;
				}
				printf ("%d", i);
			}
		}
		printf ("\n");

		printf ("wrappers that failed at least once: ");
		first = 1;
		for (i=1; i<N_WRAP+1; i++) {
			if (__failed_checks[i]) {
				if (!first) {
					printf (",");
				} else {
					first = 0;
				}
				printf ("%d", i);
			}
		}
		printf ("\n");

		printf ("wrappers that never failed (or never executed): ");
		first = 1;
		for (i=1; i<N_WRAP+1; i++) {
			if (!__failed_checks[i]) {
				if (!first) {
					printf (",");
				} else {
					first = 0;
				}
				printf ("%d", i);
			}
		}
		printf ("\n");
	}
#endif
}

#define MB (1<<20)

/*****************************************************************************/

#endif /* PLATFORM_GENERIC_H */

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */

/* End of file. */
