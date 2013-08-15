/*    globals.c
 *
 *    Copyright (C) 1995, 1999, 2000, 2001, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'For the rest, they shall represent the other Free Peoples of the World:
 *  Elves, Dwarves, and Men.'                                --Elrond
 *
 *     [p.275 of _The Lord of the Rings_, II/iii: "The Ring Goes South"]
 */

/* This file exists to #include "perl.h" _ONCE_ with
 * PERL_IN_GLOBALS_C defined. That causes various global varaiables
 * in perl.h and other files it includes to be _defined_ (and initialized)
 * rather than just declared.
 *
 * There is a #include "perlapi.h" which makes use of the fact
 * that the object file created from this file will be included by linker
 * (to resolve global variables). perlapi.h mention various other "API"
 * functions not used by perl itself, but the functions get
 * pulled into the perl executable via the refrerence here.
 *
*/

#include "INTERN.h"
#define PERL_IN_GLOBALS_C
#include "perl.h"

#include "perlapi.h"		/* bring in PL_force_link_funcs */

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
