/*****************************************************************************
* getopt.c - competent and free getopt library.
* $Header: /cvsroot/freegetopt/freegetopt/getopt.c,v 1.2 2003/10/26 03:10:20 vindaci Exp $
*
* Copyright (c)2002-2003 Mark K. Kim
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*   * Neither the original author of this software nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"


/* 2009-10-12 Camilla Berglund <elmindreda@elmindreda.org>
 *
 * Removed unused global static variable 'ID'.
 */


char* optarg = NULL;
int optind = 0;
int opterr = 1;
int optopt = '?';


static char** prev_argv = NULL;        /* Keep a copy of argv and argc to */
static int prev_argc = 0;              /*    tell if getopt params change */
static int argv_index = 0;             /* Option we're checking */
static int argv_index2 = 0;            /* Option argument we're checking */
static int opt_offset = 0;             /* Index into compounded "-option" */
static int dashdash = 0;               /* True if "--" option reached */
static int nonopt = 0;                 /* How many nonopts we've found */

static void increment_index()
{
	/* Move onto the next option */
	if(argv_index < argv_index2)
	{
		while(prev_argv[++argv_index] && prev_argv[argv_index][0] != '-'
				&& argv_index < argv_index2+1);
	}
	else argv_index++;
	opt_offset = 1;
}


/*
* Permutes argv[] so that the argument currently being processed is moved
* to the end.
*/
static int permute_argv_once()
{
	/* Movability check */
	if(argv_index + nonopt >= prev_argc) return 1;
	/* Move the current option to the end, bring the others to front */
	else
	{
		char* tmp = prev_argv[argv_index];

		/* Move the data */
		memmove(&prev_argv[argv_index], &prev_argv[argv_index+1],
				sizeof(char**) * (prev_argc - argv_index - 1));
		prev_argv[prev_argc - 1] = tmp;

		nonopt++;
		return 0;
	}
}


int getopt(int argc, char** argv, char* optstr)
{
	int c = 0;

	/* If we have new argv, reinitialize */
	if(prev_argv != argv || prev_argc != argc)
	{
		/* Initialize variables */
		prev_argv = argv;
		prev_argc = argc;
		argv_index = 1;
		argv_index2 = 1;
		opt_offset = 1;
		dashdash = 0;
		nonopt = 0;
	}

	/* Jump point in case we want to ignore the current argv_index */
	getopt_top:

	/* Misc. initializations */
	optarg = NULL;

	/* Dash-dash check */
	if(argv[argv_index] && !strcmp(argv[argv_index], "--"))
	{
		dashdash = 1;
		increment_index();
	}

	/* If we're at the end of argv, that's it. */
	if(argv[argv_index] == NULL)
	{
		c = -1;
	}
	/* Are we looking at a string? Single dash is also a string */
	else if(dashdash || argv[argv_index][0] != '-' || !strcmp(argv[argv_index], "-"))
	{
		/* If we want a string... */
		if(optstr[0] == '-')
		{
			c = 1;
			optarg = argv[argv_index];
			increment_index();
		}
		/* If we really don't want it (we're in POSIX mode), we're done */
		else if(optstr[0] == '+' || getenv("POSIXLY_CORRECT"))
		{
			c = -1;

			/* Everything else is a non-opt argument */
			nonopt = argc - argv_index;
		}
		/* If we mildly don't want it, then move it back */
		else
		{
			if(!permute_argv_once()) goto getopt_top;
			else c = -1;
		}
	}
	/* Otherwise we're looking at an option */
	else
	{
		char* opt_ptr = NULL;

		/* Grab the option */
		c = argv[argv_index][opt_offset++];

		/* Is the option in the optstr? */
		if(optstr[0] == '-') opt_ptr = strchr(optstr+1, c);
		else opt_ptr = strchr(optstr, c);
		/* Invalid argument */
		if(!opt_ptr)
		{
			if(opterr)
			{
				fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
			}

			optopt = c;
			c = '?';

			/* Move onto the next option */
			increment_index();
		}
		/* Option takes argument */
		else if(opt_ptr[1] == ':')
		{
			/* ie, -oARGUMENT, -xxxoARGUMENT, etc. */
			if(argv[argv_index][opt_offset] != '\0')
			{
				optarg = &argv[argv_index][opt_offset];
				increment_index();
			}
			/* ie, -o ARGUMENT (only if it's a required argument) */
			else if(opt_ptr[2] != ':')
			{
				/* One of those "you're not expected to understand this" moment */
				if(argv_index2 < argv_index) argv_index2 = argv_index;
				while(argv[++argv_index2] && argv[argv_index2][0] == '-');
				optarg = argv[argv_index2];

				/* Don't cross into the non-option argument list */
				if(argv_index2 + nonopt >= prev_argc) optarg = NULL;

				/* Move onto the next option */
				increment_index();
			}
			else
			{
				/* Move onto the next option */
				increment_index();
			}

			/* In case we got no argument for an option with required argument */
			if(optarg == NULL && opt_ptr[2] != ':')
			{
				optopt = c;
				c = '?';

				if(opterr)
				{
					fprintf(stderr,"%s: option requires an argument -- %c\n",
							argv[0], optopt);
				}
			}
		}
		/* Option does not take argument */
		else
		{
			/* Next argv_index */
			if(argv[argv_index][opt_offset] == '\0')
			{
				increment_index();
			}
		}
	}

	/* Calculate optind */
	if(c == -1)
	{
		optind = argc - nonopt;
	}
	else
	{
		optind = argv_index;
	}

	return c;
}


/* vim:ts=3
*/
