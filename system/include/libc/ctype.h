#ifndef _CTYPE_H_
#define _CTYPE_H_

#include "_ansi.h"

_BEGIN_STD_C

int _EXFUN(isalnum, (int __c));
int _EXFUN(isalpha, (int __c));
int _EXFUN(iscntrl, (int __c));
int _EXFUN(isdigit, (int __c));
int _EXFUN(isgraph, (int __c));
int _EXFUN(islower, (int __c));
int _EXFUN(isprint, (int __c));
int _EXFUN(ispunct, (int __c));
int _EXFUN(isspace, (int __c));
int _EXFUN(isupper, (int __c));
int _EXFUN(isxdigit,(int __c));
int _EXFUN(tolower, (int __c));
int _EXFUN(toupper, (int __c));

#if !defined(__STRICT_ANSI__) || defined(__cplusplus) || __STDC_VERSION__ >= 199901L
int _EXFUN(isblank, (int __c));
#endif

#ifndef __STRICT_ANSI__
int _EXFUN(isascii, (int __c));
int _EXFUN(toascii, (int __c));
#define _tolower(__c) ((unsigned char)(__c) - 'A' + 'a')
#define _toupper(__c) ((unsigned char)(__c) - 'a' + 'A')
#endif

/* XXX Emscripten
#ifndef _MB_CAPABLE
_CONST
#endif
 extern	__IMPORT char	*__ctype_ptr__;
*/

#if 0 /* ndef __cplusplus XXX Emscripten: Do not use the macros here. always use the simple functions */

/* XXX Emscripten - these confuse libc++. moved to inside ifndef __cplusplus, and added CTYPE_ */
#define	CTYPE__U	01
#define	CTYPE__L	02
#define	CTYPE__N	04
#define	CTYPE__S	010
#define CTYPE__P	020
#define CTYPE__C	040
#define CTYPE__X	0100
#define	CTYPE__B	0200

/* These macros are intentionally written in a manner that will trigger
   a gcc -Wall warning if the user mistakenly passes a 'char' instead
   of an int containing an 'unsigned char'.  Note that the sizeof will
   always be 1, which is what we want for mapping EOF to __ctype_ptr__[0];
   the use of a raw index inside the sizeof triggers the gcc warning if
   __c was of type char, and sizeof masks side effects of the extra __c.
   Meanwhile, the real index to __ctype_ptr__+1 must be cast to int,
   since isalpha(0x100000001LL) must equal isalpha(1), rather than being
   an out-of-bounds reference on a 64-bit machine.  */
#define __ctype_lookup(__c) ((__ctype_ptr__+sizeof(""[__c]))[(int)(__c)])

#define	isalpha(__c)	(__ctype_lookup(__c)&(CTYPE__U|CTYPE__L))
#define	isupper(__c)	((__ctype_lookup(__c)&(CTYPE__U|CTYPE__L))==CTYPE__U)
#define	islower(__c)	((__ctype_lookup(__c)&(CTYPE__U|CTYPE__L))==CTYPE__L)
#define	isdigit(__c)	(__ctype_lookup(__c)&CTYPE__N)
#define	isxdigit(__c)	(__ctype_lookup(__c)&(CTYPE__X|CTYPE__N))
#define	isspace(__c)	(__ctype_lookup(__c)&CTYPE__S)
#define ispunct(__c)	(__ctype_lookup(__c)&CTYPE__P)
#define isalnum(__c)	(__ctype_lookup(__c)&(CTYPE__U|CTYPE__L|CTYPE__N))
#define isprint(__c)	(__ctype_lookup(__c)&(CTYPE__P|CTYPE__U|CTYPE__L|CTYPE__N|CTYPE__B))
#define	isgraph(__c)	(__ctype_lookup(__c)&(CTYPE__P|CTYPE__U|CTYPE__L|CTYPE__N))
#define iscntrl(__c)	(__ctype_lookup(__c)&CTYPE__C)

/* XXX: EMSCRIPTEN: We alter the names of __typeof__ declarations to
   reduce the chance of them conflicting when expanded */

#if defined(__GNUC__) && \
    (!defined(__STRICT_ANSI__) || __STDC_VERSION__ >= 199901L)
#define isblank(__c) \
  __extension__ ({ __typeof__ (__c) __ctb_x = (__c);		\
        (__ctype_lookup(__ctb_x)&_B) || (int) (__ctb_x) == '\t';})
#endif


/* Non-gcc versions will get the library versions, and will be
   slightly slower.  These macros are not NLS-aware so they are
   disabled if the system supports the extended character sets. */
# if defined(__GNUC__)
#  if !defined (_MB_EXTENDED_CHARSETS_ISO) && !defined (_MB_EXTENDED_CHARSETS_WINDOWS)
#   define toupper(__c) \
  __extension__ ({ __typeof__ (__c) __cttu_x = (__c);	\
      islower (__cttu_x) ? (int) __cttu_x - 'a' + 'A' : (int) __cttu_x;})
#   define tolower(__c) \
  __extension__ ({ __typeof__ (__c) __cttl_x = (__c);	\
      isupper (__cttl_x) ? (int) __cttl_x - 'A' + 'a' : (int) __cttl_x;})
#  else /* _MB_EXTENDED_CHARSETS* */
/* Allow a gcc warning if the user passed 'char', but defer to the
   function.  */
#   define toupper(__c) \
  __extension__ ({ __typeof__ (__c) __cttu_x = (__c);	\
      (void) __ctype_ptr__[__cttu_x]; (toupper) (__cttu_x);})
#   define tolower(__c) \
  __extension__ ({ __typeof__ (__c) __cttl_x = (__c);	\
      (void) __ctype_ptr__[__cttl_x]; (tolower) (__cttl_x);})
#  endif /* _MB_EXTENDED_CHARSETS* */
# endif /* __GNUC__ */
#endif /* !__cplusplus */

#ifndef __STRICT_ANSI__
#define isascii(__c)	((unsigned)(__c)<=0177)
#define toascii(__c)	((__c)&0177)
#endif

/* For C++ backward-compatibility only.  */
extern	__IMPORT _CONST char	_ctype_[];

_END_STD_C

/*
 * Copyright (c) 2000, 2005, 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ctype.h	8.4 (Berkeley) 1/21/94
 */

#define _CTYPE_A  0x00000400   /* Alpha */
#define _CTYPE_C  0x00000002   /* Control */
#define _CTYPE_D  0x00000800   /* Digit */
#define _CTYPE_L  0x00000200   /* Lower */
#define _CTYPE_P  0x00000004   /* Punct */
#define _CTYPE_S  0x00002000   /* Space */
#define _CTYPE_U  0x00000100   /* Upper */
#define _CTYPE_X  0x00001000   /* X digit */
#define _CTYPE_B  0x00000001   /* Blank */
#define _CTYPE_R  0x00004000   /* Print */

#endif /* _CTYPE_H_ */
