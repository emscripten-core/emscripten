/*
 * Copyright (c) 1983, 1987, 1989
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
 */

/*
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *	@(#)resolv.h	8.1 (Berkeley) 6/2/93
 *	$BINDId: resolv.h,v 8.31 2000/03/30 20:16:50 vixie Exp $
 */

#ifndef _RESOLV_H_

/* These headers are needed for types used in the `struct res_state'
   declaration.  */
#include <sys/types.h>
#include <netinet/in.h>

#ifndef __need_res_state
# define _RESOLV_H_

# include <sys/param.h>
# include <sys/cdefs.h>
# include <stdio.h>
# include <arpa/nameser.h>
#endif

#ifndef __res_state_defined
# define __res_state_defined

typedef enum { res_goahead, res_nextns, res_modified, res_done, res_error }
	res_sendhookact;

typedef res_sendhookact (*res_send_qhook) (struct sockaddr_in * const *__ns,
					   const u_char **__query,
					   int *__querylen,
					   u_char *__ans,
					   int __anssiz,
					   int *__resplen);

typedef res_sendhookact (*res_send_rhook) (const struct sockaddr_in *__ns,
					   const u_char *__query,
					   int __querylen,
					   u_char *__ans,
					   int __anssiz,
					   int *__resplen);

/*
 * Global defines and variables for resolver stub.
 */
# define MAXNS			3	/* max # name servers we'll track */
# define MAXDFLSRCH		3	/* # default domain levels to try */
# define MAXDNSRCH		6	/* max # domains in search path */
# define LOCALDOMAINPARTS	2	/* min levels in name that is "local" */

# define RES_TIMEOUT		5	/* min. seconds between retries */
# define MAXRESOLVSORT		10	/* number of net to sort on */
# define RES_MAXNDOTS		15	/* should reflect bit field size */
# define RES_MAXRETRANS		30	/* only for resolv.conf/RES_OPTIONS */
# define RES_MAXRETRY		5	/* only for resolv.conf/RES_OPTIONS */
# define RES_DFLRETRY		2	/* Default #/tries. */
# define RES_MAXTIME		65535	/* Infinity, in milliseconds. */

struct __res_state {
	int	retrans;		/* retransmition time interval */
	int	retry;			/* number of times to retransmit */
	u_long	options;		/* option flags - see below. */
	int	nscount;		/* number of name servers */
	struct sockaddr_in
		nsaddr_list[MAXNS];	/* address of name server */
# define nsaddr	nsaddr_list[0]		/* for backward compatibility */
	u_short	id;			/* current message id */
	/* 2 byte hole here.  */
	char	*dnsrch[MAXDNSRCH+1];	/* components of domain to search */
	char	defdname[256];		/* default domain (deprecated) */
	u_long	pfcode;			/* RES_PRF_ flags - see below. */
	unsigned ndots:4;		/* threshold for initial abs. query */
	unsigned nsort:4;		/* number of elements in sort_list[] */
	unsigned ipv6_unavail:1;	/* connecting to IPv6 server failed */
	unsigned unused:23;
	struct {
		struct in_addr	addr;
		u_int32_t	mask;
	} sort_list[MAXRESOLVSORT];
	/* 4 byte hole here on 64-bit architectures.  */
	res_send_qhook qhook;		/* query hook */
	res_send_rhook rhook;		/* response hook */
	int	res_h_errno;		/* last one set for this context */
	int	_vcsock;		/* PRIVATE: for res_send VC i/o */
	u_int	_flags;			/* PRIVATE: see below */
	/* 4 byte hole here on 64-bit architectures.  */
	union {
		char	pad[52];	/* On an i386 this means 512b total. */
		struct {
			u_int16_t		nscount;
			u_int16_t		nsmap[MAXNS];
			int			nssocks[MAXNS];
			u_int16_t		nscount6;
			u_int16_t		nsinit;
			struct sockaddr_in6	*nsaddrs[MAXNS];
#ifdef _LIBC
			unsigned long long int	initstamp
			  __attribute__((packed));
#else
			unsigned int		_initstamp[2];
#endif
		} _ext;
	} _u;
};

typedef struct __res_state *res_state;
# undef __need_res_state
#endif

#ifdef _RESOLV_H_
/*
 * Revision information.  This is the release date in YYYYMMDD format.
 * It can change every day so the right thing to do with it is use it
 * in preprocessor commands such as "#if (__RES > 19931104)".  Do not
 * compare for equality; rather, use it to determine whether your resolver
 * is new enough to contain a certain feature.
 */

#define	__RES	19991006

/*
 * Resolver configuration file.
 * Normally not present, but may contain the address of the
 * inital name server(s) to query and the domain search list.
 */

#ifndef _PATH_RESCONF
#define _PATH_RESCONF        "/etc/resolv.conf"
#endif

struct res_sym {
	int	number;		/* Identifying number, like T_MX */
	char *	name;		/* Its symbolic name, like "MX" */
	char *	humanname;	/* Its fun name, like "mail exchanger" */
};

/*
 * Resolver flags (used to be discrete per-module statics ints).
 */
#define	RES_F_VC	0x00000001	/* socket is TCP */
#define	RES_F_CONN	0x00000002	/* socket is connected */
#define RES_F_EDNS0ERR	0x00000004	/* EDNS0 caused errors */

/* res_findzonecut() options */
#define	RES_EXHAUSTIVE	0x00000001	/* always do all queries */

/*
 * Resolver options (keep these in synch with res_debug.c, please)
 */
#define RES_INIT	0x00000001	/* address initialized */
#define RES_DEBUG	0x00000002	/* print debug messages */
#define RES_AAONLY	0x00000004	/* authoritative answers only (!IMPL)*/
#define RES_USEVC	0x00000008	/* use virtual circuit */
#define RES_PRIMARY	0x00000010	/* query primary server only (!IMPL) */
#define RES_IGNTC	0x00000020	/* ignore trucation errors */
#define RES_RECURSE	0x00000040	/* recursion desired */
#define RES_DEFNAMES	0x00000080	/* use default domain name */
#define RES_STAYOPEN	0x00000100	/* Keep TCP socket open */
#define RES_DNSRCH	0x00000200	/* search up local domain tree */
#define	RES_INSECURE1	0x00000400	/* type 1 security disabled */
#define	RES_INSECURE2	0x00000800	/* type 2 security disabled */
#define	RES_NOALIASES	0x00001000	/* shuts off HOSTALIASES feature */
#define	RES_USE_INET6	0x00002000	/* use/map IPv6 in gethostbyname() */
#define RES_ROTATE	0x00004000	/* rotate ns list after each query */
#define	RES_NOCHECKNAME	0x00008000	/* do not check names for sanity. */
#define	RES_KEEPTSIG	0x00010000	/* do not strip TSIG records */
#define	RES_BLAST	0x00020000	/* blast all recursive servers */
#define RES_USEBSTRING	0x00040000	/* IPv6 reverse lookup with byte
					   strings */
#define RES_NOIP6DOTINT	0x00080000	/* Do not use .ip6.int in IPv6
					   reverse lookup */
#define RES_USE_EDNS0	0x00100000	/* Use EDNS0.  */
#define RES_SNGLKUP	0x00200000	/* one outstanding request at a time */
#define RES_SNGLKUPREOP	0x00400000	/* -"-, but open new socket for each
					   request */
#define RES_USE_DNSSEC	0x00800000	/* use DNSSEC using OK bit in OPT */
#define RES_NOTLDQUERY	0x01000000	/* Do not look up unqualified name
					   as a TLD.  */

#define RES_DEFAULT	(RES_RECURSE|RES_DEFNAMES|RES_DNSRCH|RES_NOIP6DOTINT)

/*
 * Resolver "pfcode" values.  Used by dig.
 */
#define RES_PRF_STATS	0x00000001
#define RES_PRF_UPDATE	0x00000002
#define RES_PRF_CLASS   0x00000004
#define RES_PRF_CMD	0x00000008
#define RES_PRF_QUES	0x00000010
#define RES_PRF_ANS	0x00000020
#define RES_PRF_AUTH	0x00000040
#define RES_PRF_ADD	0x00000080
#define RES_PRF_HEAD1	0x00000100
#define RES_PRF_HEAD2	0x00000200
#define RES_PRF_TTLID	0x00000400
#define RES_PRF_HEADX	0x00000800
#define RES_PRF_QUERY	0x00001000
#define RES_PRF_REPLY	0x00002000
#define RES_PRF_INIT	0x00004000
/*			0x00008000	*/

/* Things involving an internal (static) resolver context. */
__BEGIN_DECLS
extern struct __res_state *__res_state(void) __attribute__ ((__const__));
__END_DECLS
#define _res (*__res_state())

#ifndef __BIND_NOSTATIC
#define fp_nquery		__fp_nquery
#define fp_query		__fp_query
#define hostalias		__hostalias
#define p_query			__p_query
#define res_close		__res_close
#define res_init		__res_init
#define res_isourserver		__res_isourserver
#define res_mkquery		__res_mkquery
#define res_query		__res_query
#define res_querydomain		__res_querydomain
#define res_search		__res_search
#define res_send		__res_send

__BEGIN_DECLS
void		fp_nquery (const u_char *, int, FILE *) __THROW;
void		fp_query (const u_char *, FILE *) __THROW;
const char *	hostalias (const char *) __THROW;
void		p_query (const u_char *) __THROW;
void		res_close (void) __THROW;
int		res_init (void) __THROW;
int		res_isourserver (const struct sockaddr_in *) __THROW;
int		res_mkquery (int, const char *, int, int, const u_char *,
			     int, const u_char *, u_char *, int) __THROW;
int		res_query (const char *, int, int, u_char *, int) __THROW;
int		res_querydomain (const char *, const char *, int, int,
				 u_char *, int) __THROW;
int		res_search (const char *, int, int, u_char *, int) __THROW;
int		res_send (const u_char *, int, u_char *, int) __THROW;
__END_DECLS
#endif

#define b64_ntop		__b64_ntop
#define b64_pton		__b64_pton
#define dn_comp			__dn_comp
#define dn_count_labels		__dn_count_labels
#define dn_expand		__dn_expand
#define dn_skipname		__dn_skipname
#define fp_resstat		__fp_resstat
#define loc_aton		__loc_aton
#define loc_ntoa		__loc_ntoa
#define p_cdname		__p_cdname
#define p_cdnname		__p_cdnname
#define p_class			__p_class
#define p_fqname		__p_fqname
#define p_fqnname		__p_fqnname
#define p_option		__p_option
#define p_secstodate		__p_secstodate
#define p_section		__p_section
#define p_time			__p_time
#define p_type			__p_type
#define p_rcode			__p_rcode
#define putlong			__putlong
#define putshort		__putshort
#define res_dnok		__res_dnok
#define res_hnok		__res_hnok
#define res_hostalias		__res_hostalias
#define res_mailok		__res_mailok
#define res_nameinquery		__res_nameinquery
#define res_nclose		__res_nclose
#define res_ninit		__res_ninit
#define res_nmkquery		__res_nmkquery
#define res_npquery		__res_npquery
#define res_nquery		__res_nquery
#define res_nquerydomain	__res_nquerydomain
#define res_nsearch		__res_nsearch
#define res_nsend		__res_nsend
#define res_nisourserver	__res_nisourserver
#define res_ownok		__res_ownok
#define res_queriesmatch	__res_queriesmatch
#define res_randomid		__res_randomid
#define sym_ntop		__sym_ntop
#define sym_ntos		__sym_ntos
#define sym_ston		__sym_ston
__BEGIN_DECLS
int		res_hnok (const char *) __THROW;
int		res_ownok (const char *) __THROW;
int		res_mailok (const char *) __THROW;
int		res_dnok (const char *) __THROW;
int		sym_ston (const struct res_sym *, const char *, int *) __THROW;
const char *	sym_ntos (const struct res_sym *, int, int *) __THROW;
const char *	sym_ntop (const struct res_sym *, int, int *) __THROW;
int		b64_ntop (u_char const *, size_t, char *, size_t) __THROW;
int		b64_pton (char const *, u_char *, size_t) __THROW;
int		loc_aton (const char *__ascii, u_char *__binary) __THROW;
const char *	loc_ntoa (const u_char *__binary, char *__ascii) __THROW;
int		dn_skipname (const u_char *, const u_char *) __THROW;
void		putlong (u_int32_t, u_char *) __THROW;
void		putshort (u_int16_t, u_char *) __THROW;
const char *	p_class (int) __THROW;
const char *	p_time (u_int32_t) __THROW;
const char *	p_type (int) __THROW;
const char *	p_rcode (int) __THROW;
const u_char *	p_cdnname (const u_char *, const u_char *, int, FILE *)
     __THROW;
const u_char *	p_cdname (const u_char *, const u_char *, FILE *) __THROW;
const u_char *	p_fqnname (const u_char *__cp, const u_char *__msg,
			   int, char *, int) __THROW;
const u_char *	p_fqname (const u_char *, const u_char *, FILE *) __THROW;
const char *	p_option (u_long __option) __THROW;
char *		p_secstodate (u_long) __THROW;
int		dn_count_labels (const char *) __THROW;
int		dn_comp (const char *, u_char *, int, u_char **, u_char **)
     __THROW;
int		dn_expand (const u_char *, const u_char *, const u_char *,
			   char *, int) __THROW;
u_int		res_randomid (void) __THROW;
int		res_nameinquery (const char *, int, int,
				 const u_char *, const u_char *) __THROW;
int		res_queriesmatch (const u_char *, const u_char *,
				  const u_char *, const u_char *) __THROW;
const char *	p_section (int __section, int __opcode) __THROW;
/* Things involving a resolver context. */
int		res_ninit (res_state) __THROW;
int		res_nisourserver (const res_state,
				  const struct sockaddr_in *) __THROW;
void		fp_resstat (const res_state, FILE *) __THROW;
void		res_npquery (const res_state, const u_char *, int, FILE *)
     __THROW;
const char *	res_hostalias (const res_state, const char *, char *, size_t)
     __THROW;
int		res_nquery (res_state, const char *, int, int, u_char *, int)
     __THROW;
int		res_nsearch (res_state, const char *, int, int, u_char *, int)
     __THROW;
int		res_nquerydomain (res_state, const char *, const char *, int,
				  int, u_char *, int) __THROW;
int		res_nmkquery (res_state, int, const char *, int, int,
			      const u_char *, int, const u_char *, u_char *,
			      int) __THROW;
int		res_nsend (res_state, const u_char *, int, u_char *, int)
     __THROW;
void		res_nclose (res_state) __THROW;
__END_DECLS
#endif

#endif /* !_RESOLV_H_ */
