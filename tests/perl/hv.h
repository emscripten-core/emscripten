/*    hv.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1996, 1997, 1998, 1999,
 *    2000, 2001, 2002, 2003, 2005, 2006, 2007, 2008, by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* entry in hash value chain */
struct he {
    /* Keep hent_next first in this structure, because sv_free_arenas take
       advantage of this to share code between the he arenas and the SV
       body arenas  */
    HE		*hent_next;	/* next entry in chain */
    HEK		*hent_hek;	/* hash key */
    union {
	SV	*hent_val;	/* scalar value that was hashed */
	Size_t	hent_refcount;	/* references for this shared hash key */
    } he_valu;
};

/* hash key -- defined separately for use as shared pointer */
struct hek {
    U32		hek_hash;	/* hash of key */
    I32		hek_len;	/* length of hash key */
    char	hek_key[1];	/* variable-length hash key */
    /* the hash-key is \0-terminated */
    /* after the \0 there is a byte for flags, such as whether the key
       is UTF-8 */
};

struct shared_he {
    struct he shared_he_he;
    struct hek shared_he_hek;
};

/* Subject to change.
   Don't access this directly.
   Use the funcs in mro.c
*/

struct mro_alg {
    AV *(*resolve)(pTHX_ HV* stash, U32 level);
    const char *name;
    U16 length;
    U16	kflags;	/* For the hash API - set HVhek_UTF8 if name is UTF-8 */
    U32 hash; /* or 0 */
};

struct mro_meta {
    /* a hash holding the different MROs private data.  */
    HV      *mro_linear_all;
    /* a pointer directly to the current MROs private data.  If mro_linear_all
       is NULL, this owns the SV reference, else it is just a pointer to a
       value stored in and owned by mro_linear_all.  */
    SV      *mro_linear_current;
    HV      *mro_nextmethod; /* next::method caching */
    U32     cache_gen;       /* Bumping this invalidates our method cache */
    U32     pkg_gen;         /* Bumps when local methods/@ISA change */
    const struct mro_alg *mro_which; /* which mro alg is in use? */
    HV      *isa;            /* Everything this class @ISA */
};

#define MRO_GET_PRIVATE_DATA(smeta, which)		   \
    (((smeta)->mro_which && (which) == (smeta)->mro_which) \
     ? (smeta)->mro_linear_current			   \
     : Perl_mro_get_private_data(aTHX_ (smeta), (which)))

/* Subject to change.
   Don't access this directly.
*/

union _xhvnameu {
    HEK *xhvnameu_name;		/* When xhv_name_count is 0 */
    HEK **xhvnameu_names;	/* When xhv_name_count is non-0 */
};

struct xpvhv_aux {
    union _xhvnameu xhv_name_u;	/* name, if a symbol table */
    AV		*xhv_backreferences; /* back references for weak references */
    HE		*xhv_eiter;	/* current entry of iterator */
    I32		xhv_riter;	/* current root of iterator */
/* Concerning xhv_name_count: When non-zero, xhv_name_u contains a pointer 
 * to an array of HEK pointers, this being the length. The first element is
 * the name of the stash, which may be NULL. If xhv_name_count is positive,
 * then *xhv_name is one of the effective names. If xhv_name_count is nega-
 * tive, then xhv_name_u.xhvnameu_names[1] is the first effective name.
 */
    I32		xhv_name_count;
    struct mro_meta *xhv_mro_meta;
};

/* hash structure: */
/* This structure must match the beginning of struct xpvmg in sv.h. */
struct xpvhv {
    HV*		xmg_stash;	/* class package */
    union _xmgu	xmg_u;
    STRLEN      xhv_keys;       /* total keys, including placeholders */
    STRLEN      xhv_max;        /* subscript of last element of xhv_array */
};

/* hash a key */
/* FYI: This is the "One-at-a-Time" algorithm by Bob Jenkins
 * from requirements by Colin Plumb.
 * (http://burtleburtle.net/bob/hash/doobs.html) */
/* The use of a temporary pointer and the casting games
 * is needed to serve the dual purposes of
 * (a) the hashed data being interpreted as "unsigned char" (new since 5.8,
 *     a "char" can be either signed or unsigned, depending on the compiler)
 * (b) catering for old code that uses a "char"
 *
 * The "hash seed" feature was added in Perl 5.8.1 to perturb the results
 * to avoid "algorithmic complexity attacks".
 *
 * If USE_HASH_SEED is defined, hash randomisation is done by default
 * If USE_HASH_SEED_EXPLICIT is defined, hash randomisation is done
 * only if the environment variable PERL_HASH_SEED is set.
 * For maximal control, one can define PERL_HASH_SEED.
 * (see also perl.c:perl_parse()).
 */
#ifndef PERL_HASH_SEED
#   if defined(USE_HASH_SEED) || defined(USE_HASH_SEED_EXPLICIT)
#       define PERL_HASH_SEED	PL_hash_seed
#   else
#       define PERL_HASH_SEED	0
#   endif
#endif

#define PERL_HASH(hash,str,len) PERL_HASH_INTERNAL_(hash,str,len,0)

/* Only hv.c and mod_perl should be doing this.  */
#ifdef PERL_HASH_INTERNAL_ACCESS
#define PERL_HASH_INTERNAL(hash,str,len) PERL_HASH_INTERNAL_(hash,str,len,1)
#endif

/* Common base for PERL_HASH and PERL_HASH_INTERNAL that parameterises
 * the source of the seed. Not for direct use outside of hv.c. */

#define PERL_HASH_INTERNAL_(hash,str,len,internal) \
     STMT_START	{ \
	register const char * const s_PeRlHaSh_tmp = str; \
	register const unsigned char *s_PeRlHaSh = (const unsigned char *)s_PeRlHaSh_tmp; \
	register I32 i_PeRlHaSh = len; \
	register U32 hash_PeRlHaSh = (internal ? PL_rehash_seed : PERL_HASH_SEED); \
	while (i_PeRlHaSh--) { \
	    hash_PeRlHaSh += *s_PeRlHaSh++; \
	    hash_PeRlHaSh += (hash_PeRlHaSh << 10); \
	    hash_PeRlHaSh ^= (hash_PeRlHaSh >> 6); \
	} \
	hash_PeRlHaSh += (hash_PeRlHaSh << 3); \
	hash_PeRlHaSh ^= (hash_PeRlHaSh >> 11); \
	(hash) = (hash_PeRlHaSh + (hash_PeRlHaSh << 15)); \
    } STMT_END

/*
=head1 Hash Manipulation Functions

=for apidoc AmU||HEf_SVKEY
This flag, used in the length slot of hash entries and magic structures,
specifies the structure contains an C<SV*> pointer where a C<char*> pointer
is to be expected. (For information only--not to be used).

=head1 Handy Values

=for apidoc AmU||Nullhv
Null HV pointer.

(deprecated - use C<(HV *)NULL> instead)

=head1 Hash Manipulation Functions

=for apidoc Am|char*|HvNAME|HV* stash
Returns the package name of a stash, or NULL if C<stash> isn't a stash.
See C<SvSTASH>, C<CvSTASH>.

=for apidoc Am|STRLEN|HvNAMELEN|HV *stash
Returns the length of the stash's name.

=for apidoc Am|unsigned char|HvNAMEUTF8|HV *stash
Returns true if the name is in UTF8 encoding.

=for apidoc Am|char*|HvENAME|HV* stash
Returns the effective name of a stash, or NULL if there is none. The
effective name represents a location in the symbol table where this stash
resides. It is updated automatically when packages are aliased or deleted.
A stash that is no longer in the symbol table has no effective name. This
name is preferable to C<HvNAME> for use in MRO linearisations and isa
caches.

=for apidoc Am|STRLEN|HvENAMELEN|HV *stash
Returns the length of the stash's effective name.

=for apidoc Am|unsigned char|HvENAMEUTF8|HV *stash
Returns true if the effective name is in UTF8 encoding.

=for apidoc Am|void*|HeKEY|HE* he
Returns the actual pointer stored in the key slot of the hash entry. The
pointer may be either C<char*> or C<SV*>, depending on the value of
C<HeKLEN()>.  Can be assigned to.  The C<HePV()> or C<HeSVKEY()> macros are
usually preferable for finding the value of a key.

=for apidoc Am|STRLEN|HeKLEN|HE* he
If this is negative, and amounts to C<HEf_SVKEY>, it indicates the entry
holds an C<SV*> key.  Otherwise, holds the actual length of the key.  Can
be assigned to. The C<HePV()> macro is usually preferable for finding key
lengths.

=for apidoc Am|SV*|HeVAL|HE* he
Returns the value slot (type C<SV*>) stored in the hash entry.

=for apidoc Am|U32|HeHASH|HE* he
Returns the computed hash stored in the hash entry.

=for apidoc Am|char*|HePV|HE* he|STRLEN len
Returns the key slot of the hash entry as a C<char*> value, doing any
necessary dereferencing of possibly C<SV*> keys.  The length of the string
is placed in C<len> (this is a macro, so do I<not> use C<&len>).  If you do
not care about what the length of the key is, you may use the global
variable C<PL_na>, though this is rather less efficient than using a local
variable.  Remember though, that hash keys in perl are free to contain
embedded nulls, so using C<strlen()> or similar is not a good way to find
the length of hash keys. This is very similar to the C<SvPV()> macro
described elsewhere in this document. See also C<HeUTF8>.

If you are using C<HePV> to get values to pass to C<newSVpvn()> to create a
new SV, you should consider using C<newSVhek(HeKEY_hek(he))> as it is more
efficient.

=for apidoc Am|char*|HeUTF8|HE* he
Returns whether the C<char *> value returned by C<HePV> is encoded in UTF-8,
doing any necessary dereferencing of possibly C<SV*> keys.  The value returned
will be 0 or non-0, not necessarily 1 (or even a value with any low bits set),
so B<do not> blindly assign this to a C<bool> variable, as C<bool> may be a
typedef for C<char>.

=for apidoc Am|SV*|HeSVKEY|HE* he
Returns the key as an C<SV*>, or C<NULL> if the hash entry does not
contain an C<SV*> key.

=for apidoc Am|SV*|HeSVKEY_force|HE* he
Returns the key as an C<SV*>.  Will create and return a temporary mortal
C<SV*> if the hash entry contains only a C<char*> key.

=for apidoc Am|SV*|HeSVKEY_set|HE* he|SV* sv
Sets the key to a given C<SV*>, taking care to set the appropriate flags to
indicate the presence of an C<SV*> key, and returns the same
C<SV*>.

=cut
*/

/* these hash entry flags ride on hent_klen (for use only in magic/tied HVs) */
#define HEf_SVKEY	-2	/* hent_key is an SV* */

#ifndef PERL_CORE
#  define Nullhv Null(HV*)
#endif
#define HvARRAY(hv)	((hv)->sv_u.svu_hash)
#define HvFILL(hv)	Perl_hv_fill(aTHX_ (const HV *)(hv))
#define HvMAX(hv)	((XPVHV*)  SvANY(hv))->xhv_max
/* This quite intentionally does no flag checking first. That's your
   responsibility.  */
#define HvAUX(hv)	((struct xpvhv_aux*)&(HvARRAY(hv)[HvMAX(hv)+1]))
#define HvRITER(hv)	(*Perl_hv_riter_p(aTHX_ MUTABLE_HV(hv)))
#define HvEITER(hv)	(*Perl_hv_eiter_p(aTHX_ MUTABLE_HV(hv)))
#define HvRITER_set(hv,r)	Perl_hv_riter_set(aTHX_ MUTABLE_HV(hv), r)
#define HvEITER_set(hv,e)	Perl_hv_eiter_set(aTHX_ MUTABLE_HV(hv), e)
#define HvRITER_get(hv)	(SvOOK(hv) ? HvAUX(hv)->xhv_riter : -1)
#define HvEITER_get(hv)	(SvOOK(hv) ? HvAUX(hv)->xhv_eiter : NULL)
#define HvNAME(hv)	HvNAME_get(hv)
#define HvNAMELEN(hv)   HvNAMELEN_get(hv)
#define HvENAME(hv)	HvENAME_get(hv)
#define HvENAMELEN(hv)  HvENAMELEN_get(hv)

/* Checking that hv is a valid package stash is the
   caller's responsibility */
#define HvMROMETA(hv) (HvAUX(hv)->xhv_mro_meta \
                       ? HvAUX(hv)->xhv_mro_meta \
                       : Perl_mro_meta_init(aTHX_ hv))

#define HvNAME_HEK_NN(hv)			  \
 (						  \
  HvAUX(hv)->xhv_name_count			  \
  ? *HvAUX(hv)->xhv_name_u.xhvnameu_names	  \
  : HvAUX(hv)->xhv_name_u.xhvnameu_name		  \
 )
/* This macro may go away without notice.  */
#define HvNAME_HEK(hv) \
	(SvOOK(hv) && HvAUX(hv)->xhv_name_u.xhvnameu_name ? HvNAME_HEK_NN(hv) : NULL)
#define HvNAME_get(hv) \
	((SvOOK(hv) && HvAUX(hv)->xhv_name_u.xhvnameu_name && HvNAME_HEK_NN(hv)) \
			 ? HEK_KEY(HvNAME_HEK_NN(hv)) : NULL)
#define HvNAMELEN_get(hv) \
	((SvOOK(hv) && HvAUX(hv)->xhv_name_u.xhvnameu_name && HvNAME_HEK_NN(hv)) \
				 ? HEK_LEN(HvNAME_HEK_NN(hv)) : 0)
#define HvNAMEUTF8(hv) \
	((SvOOK(hv) && HvAUX(hv)->xhv_name_u.xhvnameu_name && HvNAME_HEK_NN(hv)) \
				 ? HEK_UTF8(HvNAME_HEK_NN(hv)) : 0)
#define HvENAME_HEK_NN(hv)                                             \
 (                                                                      \
  HvAUX(hv)->xhv_name_count > 0   ? HvAUX(hv)->xhv_name_u.xhvnameu_names[0] : \
  HvAUX(hv)->xhv_name_count < -1  ? HvAUX(hv)->xhv_name_u.xhvnameu_names[1] : \
  HvAUX(hv)->xhv_name_count == -1 ? NULL                              : \
                                    HvAUX(hv)->xhv_name_u.xhvnameu_name \
 )
#define HvENAME_HEK(hv) \
	(SvOOK(hv) && HvAUX(hv)->xhv_name_u.xhvnameu_name ? HvENAME_HEK_NN(hv) : NULL)
#define HvENAME_get(hv) \
   ((SvOOK(hv) && HvAUX(hv)->xhv_name_u.xhvnameu_name && HvAUX(hv)->xhv_name_count != -1) \
			 ? HEK_KEY(HvENAME_HEK_NN(hv)) : NULL)
#define HvENAMELEN_get(hv) \
   ((SvOOK(hv) && HvAUX(hv)->xhv_name_u.xhvnameu_name && HvAUX(hv)->xhv_name_count != -1) \
				 ? HEK_LEN(HvENAME_HEK_NN(hv)) : 0)
#define HvENAMEUTF8(hv) \
   ((SvOOK(hv) && HvAUX(hv)->xhv_name_u.xhvnameu_name && HvAUX(hv)->xhv_name_count != -1) \
				 ? HEK_UTF8(HvENAME_HEK_NN(hv)) : 0)

/* the number of keys (including any placeholders) */
#define XHvTOTALKEYS(xhv)	((xhv)->xhv_keys)

/*
 * HvKEYS gets the number of keys that actually exist(), and is provided
 * for backwards compatibility with old XS code. The core uses HvUSEDKEYS
 * (keys, excluding placeholders) and HvTOTALKEYS (including placeholders)
 */
#define HvKEYS(hv)		HvUSEDKEYS(hv)
#define HvUSEDKEYS(hv)		(HvTOTALKEYS(hv) - HvPLACEHOLDERS_get(hv))
#define HvTOTALKEYS(hv)		XHvTOTALKEYS((XPVHV*)  SvANY(hv))
#define HvPLACEHOLDERS(hv)	(*Perl_hv_placeholders_p(aTHX_ MUTABLE_HV(hv)))
#define HvPLACEHOLDERS_get(hv)	(SvMAGIC(hv) ? Perl_hv_placeholders_get(aTHX_ (const HV *)hv) : 0)
#define HvPLACEHOLDERS_set(hv,p)	Perl_hv_placeholders_set(aTHX_ MUTABLE_HV(hv), p)

#define HvSHAREKEYS(hv)		(SvFLAGS(hv) & SVphv_SHAREKEYS)
#define HvSHAREKEYS_on(hv)	(SvFLAGS(hv) |= SVphv_SHAREKEYS)
#define HvSHAREKEYS_off(hv)	(SvFLAGS(hv) &= ~SVphv_SHAREKEYS)

/* This is an optimisation flag. It won't be set if all hash keys have a 0
 * flag. Currently the only flags relate to utf8.
 * Hence it won't be set if all keys are 8 bit only. It will be set if any key
 * is utf8 (including 8 bit keys that were entered as utf8, and need upgrading
 * when retrieved during iteration. It may still be set when there are no longer
 * any utf8 keys.
 * See HVhek_ENABLEHVKFLAGS for the trigger.
 */
#define HvHASKFLAGS(hv)		(SvFLAGS(hv) & SVphv_HASKFLAGS)
#define HvHASKFLAGS_on(hv)	(SvFLAGS(hv) |= SVphv_HASKFLAGS)
#define HvHASKFLAGS_off(hv)	(SvFLAGS(hv) &= ~SVphv_HASKFLAGS)

#define HvLAZYDEL(hv)		(SvFLAGS(hv) & SVphv_LAZYDEL)
#define HvLAZYDEL_on(hv)	(SvFLAGS(hv) |= SVphv_LAZYDEL)
#define HvLAZYDEL_off(hv)	(SvFLAGS(hv) &= ~SVphv_LAZYDEL)

#define HvREHASH(hv)		(SvFLAGS(hv) & SVphv_REHASH)
#define HvREHASH_on(hv)		(SvFLAGS(hv) |= SVphv_REHASH)
#define HvREHASH_off(hv)	(SvFLAGS(hv) &= ~SVphv_REHASH)

#ifndef PERL_CORE
#  define Nullhe Null(HE*)
#endif
#define HeNEXT(he)		(he)->hent_next
#define HeKEY_hek(he)		(he)->hent_hek
#define HeKEY(he)		HEK_KEY(HeKEY_hek(he))
#define HeKEY_sv(he)		(*(SV**)HeKEY(he))
#define HeKLEN(he)		HEK_LEN(HeKEY_hek(he))
#define HeKUTF8(he)  HEK_UTF8(HeKEY_hek(he))
#define HeKWASUTF8(he)  HEK_WASUTF8(HeKEY_hek(he))
#define HeKREHASH(he)  HEK_REHASH(HeKEY_hek(he))
#define HeKLEN_UTF8(he)  (HeKUTF8(he) ? -HeKLEN(he) : HeKLEN(he))
#define HeKFLAGS(he)  HEK_FLAGS(HeKEY_hek(he))
#define HeVAL(he)		(he)->he_valu.hent_val
#define HeHASH(he)		HEK_HASH(HeKEY_hek(he))
#define HePV(he,lp)		((HeKLEN(he) == HEf_SVKEY) ?		\
				 SvPV(HeKEY_sv(he),lp) :		\
				 ((lp = HeKLEN(he)), HeKEY(he)))
#define HeUTF8(he)		((HeKLEN(he) == HEf_SVKEY) ?		\
				 SvUTF8(HeKEY_sv(he)) :			\
				 (U32)HeKUTF8(he))

#define HeSVKEY(he)		((HeKEY(he) && 				\
				  HeKLEN(he) == HEf_SVKEY) ?		\
				 HeKEY_sv(he) : NULL)

#define HeSVKEY_force(he)	(HeKEY(he) ?				\
				 ((HeKLEN(he) == HEf_SVKEY) ?		\
				  HeKEY_sv(he) :			\
				  newSVpvn_flags(HeKEY(he),		\
						 HeKLEN(he), SVs_TEMP)) : \
				 &PL_sv_undef)
#define HeSVKEY_set(he,sv)	((HeKLEN(he) = HEf_SVKEY), (HeKEY_sv(he) = sv))

#ifndef PERL_CORE
#  define Nullhek Null(HEK*)
#endif
#define HEK_BASESIZE		STRUCT_OFFSET(HEK, hek_key[0])
#define HEK_HASH(hek)		(hek)->hek_hash
#define HEK_LEN(hek)		(hek)->hek_len
#define HEK_KEY(hek)		(hek)->hek_key
#define HEK_FLAGS(hek)	(*((unsigned char *)(HEK_KEY(hek))+HEK_LEN(hek)+1))

#define HVhek_UTF8	0x01 /* Key is utf8 encoded. */
#define HVhek_WASUTF8	0x02 /* Key is bytes here, but was supplied as utf8. */
#define HVhek_REHASH	0x04 /* This key is in an hv using a custom HASH . */
#define HVhek_UNSHARED	0x08 /* This key isn't a shared hash key. */
#define HVhek_FREEKEY	0x100 /* Internal flag to say key is malloc()ed.  */
#define HVhek_PLACEHOLD	0x200 /* Internal flag to create placeholder.
                               * (may change, but Storable is a core module) */
#define HVhek_KEYCANONICAL 0x400 /* Internal flag - key is in canonical form.
				    If the string is UTF-8, it cannot be
				    converted to bytes. */
#define HVhek_MASK	0xFF

/* Which flags enable HvHASKFLAGS? Somewhat a hack on a hack, as
   HVhek_REHASH is only needed because the rehash flag has to be duplicated
   into all keys as hv_iternext has no access to the hash flags. At this
   point Storable's tests get upset, because sometimes hashes are "keyed"
   and sometimes not, depending on the order of data insertion, and whether
   it triggered rehashing. So currently HVhek_REHASH is exempt.
   Similarly UNSHARED
*/
   
#define HVhek_ENABLEHVKFLAGS	(HVhek_MASK & ~(HVhek_REHASH|HVhek_UNSHARED))

#define HEK_UTF8(hek)		(HEK_FLAGS(hek) & HVhek_UTF8)
#define HEK_UTF8_on(hek)	(HEK_FLAGS(hek) |= HVhek_UTF8)
#define HEK_UTF8_off(hek)	(HEK_FLAGS(hek) &= ~HVhek_UTF8)
#define HEK_WASUTF8(hek)	(HEK_FLAGS(hek) & HVhek_WASUTF8)
#define HEK_WASUTF8_on(hek)	(HEK_FLAGS(hek) |= HVhek_WASUTF8)
#define HEK_WASUTF8_off(hek)	(HEK_FLAGS(hek) &= ~HVhek_WASUTF8)
#define HEK_REHASH(hek)		(HEK_FLAGS(hek) & HVhek_REHASH)
#define HEK_REHASH_on(hek)	(HEK_FLAGS(hek) |= HVhek_REHASH)

/* calculate HV array allocation */
#ifndef PERL_USE_LARGE_HV_ALLOC
/* Default to allocating the correct size - default to assuming that malloc()
   is not broken and is efficient at allocating blocks sized at powers-of-two.
*/   
#  define PERL_HV_ARRAY_ALLOC_BYTES(size) ((size) * sizeof(HE*))
#else
#  define MALLOC_OVERHEAD 16
#  define PERL_HV_ARRAY_ALLOC_BYTES(size) \
			(((size) < 64)					\
			 ? (size) * sizeof(HE*)				\
			 : (size) * sizeof(HE*) * 2 - MALLOC_OVERHEAD)
#endif

/* Flags for hv_iternext_flags.  */
#define HV_ITERNEXT_WANTPLACEHOLDERS	0x01	/* Don't skip placeholders.  */

#define hv_iternext(hv)	hv_iternext_flags(hv, 0)
#define hv_magic(hv, gv, how) sv_magic(MUTABLE_SV(hv), MUTABLE_SV(gv), how, NULL, 0)
#define hv_undef(hv) Perl_hv_undef_flags(aTHX_ hv, 0)

#define Perl_sharepvn(pv, len, hash) HEK_KEY(share_hek(pv, len, hash))
#define sharepvn(pv, len, hash)	     Perl_sharepvn(pv, len, hash)

#define share_hek_hek(hek)						\
    (++(((struct shared_he *)(((char *)hek)				\
			      - STRUCT_OFFSET(struct shared_he,		\
					      shared_he_hek)))		\
	->shared_he_he.he_valu.hent_refcount),				\
     hek)

#define hv_store_ent(hv, keysv, val, hash)				\
    ((HE *) hv_common((hv), (keysv), NULL, 0, 0, HV_FETCH_ISSTORE,	\
		      (val), (hash)))

#define hv_exists_ent(hv, keysv, hash)					\
    (hv_common((hv), (keysv), NULL, 0, 0, HV_FETCH_ISEXISTS, 0, (hash))	\
     ? TRUE : FALSE)
#define hv_fetch_ent(hv, keysv, lval, hash)				\
    ((HE *) hv_common((hv), (keysv), NULL, 0, 0,			\
		      ((lval) ? HV_FETCH_LVALUE : 0), NULL, (hash)))
#define hv_delete_ent(hv, key, flags, hash)				\
    (MUTABLE_SV(hv_common((hv), (key), NULL, 0, 0, (flags) | HV_DELETE,	\
			  NULL, (hash))))

#define hv_store_flags(hv, key, klen, val, hash, flags)			\
    ((SV**) hv_common((hv), NULL, (key), (klen), (flags),		\
		      (HV_FETCH_ISSTORE|HV_FETCH_JUST_SV), (val),	\
		      (hash)))

#define hv_store(hv, key, klen, val, hash)				\
    ((SV**) hv_common_key_len((hv), (key), (klen),			\
			      (HV_FETCH_ISSTORE|HV_FETCH_JUST_SV),	\
			      (val), (hash)))

#define hv_exists(hv, key, klen)					\
    (hv_common_key_len((hv), (key), (klen), HV_FETCH_ISEXISTS, NULL, 0) \
     ? TRUE : FALSE)

#define hv_fetch(hv, key, klen, lval)					\
    ((SV**) hv_common_key_len((hv), (key), (klen), (lval)		\
			      ? (HV_FETCH_JUST_SV | HV_FETCH_LVALUE)	\
			      : HV_FETCH_JUST_SV, NULL, 0))

#define hv_delete(hv, key, klen, flags)					\
    (MUTABLE_SV(hv_common_key_len((hv), (key), (klen),			\
				  (flags) | HV_DELETE, NULL, 0)))

/* This refcounted he structure is used for storing the hints used for lexical
   pragmas. Without threads, it's basically struct he + refcount.
   With threads, life gets more complex as the structure needs to be shared
   between threads (because it hangs from OPs, which are shared), hence the
   alternate definition and mutex.  */

struct refcounted_he;

/* flags for the refcounted_he API */
#define REFCOUNTED_HE_KEY_UTF8		0x00000001
#ifdef PERL_CORE
# define REFCOUNTED_HE_EXISTS		0x00000002
#endif

#ifdef PERL_CORE

/* Gosh. This really isn't a good name any longer.  */
struct refcounted_he {
    struct refcounted_he *refcounted_he_next;	/* next entry in chain */
#ifdef USE_ITHREADS
    U32                   refcounted_he_hash;
    U32                   refcounted_he_keylen;
#else
    HEK                  *refcounted_he_hek;	/* hint key */
#endif
    union {
	IV                refcounted_he_u_iv;
	UV                refcounted_he_u_uv;
	STRLEN            refcounted_he_u_len;
	void		 *refcounted_he_u_ptr;	/* Might be useful in future */
    } refcounted_he_val;
    U32	                  refcounted_he_refcnt;	/* reference count */
    /* First byte is flags. Then NUL-terminated value. Then for ithreads,
       non-NUL terminated key.  */
    char                  refcounted_he_data[1];
};

/*
=for apidoc m|SV *|refcounted_he_fetch_pvs|const struct refcounted_he *chain|const char *key|U32 flags

Like L</refcounted_he_fetch_pvn>, but takes a literal string instead of
a string/length pair, and no precomputed hash.

=cut
*/

#define refcounted_he_fetch_pvs(chain, key, flags) \
    Perl_refcounted_he_fetch_pvn(aTHX_ chain, STR_WITH_LEN(key), 0, flags)

/*
=for apidoc m|struct refcounted_he *|refcounted_he_new_pvs|struct refcounted_he *parent|const char *key|SV *value|U32 flags

Like L</refcounted_he_new_pvn>, but takes a literal string instead of
a string/length pair, and no precomputed hash.

=cut
*/

#define refcounted_he_new_pvs(parent, key, value, flags) \
    Perl_refcounted_he_new_pvn(aTHX_ parent, STR_WITH_LEN(key), 0, value, flags)

/* Flag bits are HVhek_UTF8, HVhek_WASUTF8, then */
#define HVrhek_undef	0x00 /* Value is undef. */
#define HVrhek_delete	0x10 /* Value is placeholder - signifies delete. */
#define HVrhek_IV	0x20 /* Value is IV. */
#define HVrhek_UV	0x30 /* Value is UV. */
#define HVrhek_PV	0x40 /* Value is a (byte) string. */
#define HVrhek_PV_UTF8	0x50 /* Value is a (utf8) string. */
/* Two spare. As these have to live in the optree, you can't store anything
   interpreter specific, such as SVs. :-( */
#define HVrhek_typemask 0x70

#ifdef USE_ITHREADS
/* A big expression to find the key offset */
#define REF_HE_KEY(chain)						\
	((((chain->refcounted_he_data[0] & 0x60) == 0x40)		\
	    ? chain->refcounted_he_val.refcounted_he_u_len + 1 : 0)	\
	 + 1 + chain->refcounted_he_data)
#endif

#  ifdef USE_ITHREADS
#    define HINTS_REFCNT_LOCK		MUTEX_LOCK(&PL_hints_mutex)
#    define HINTS_REFCNT_UNLOCK		MUTEX_UNLOCK(&PL_hints_mutex)
#  else
#    define HINTS_REFCNT_LOCK		NOOP
#    define HINTS_REFCNT_UNLOCK		NOOP
#  endif
#endif

#ifdef USE_ITHREADS
#  define HINTS_REFCNT_INIT		MUTEX_INIT(&PL_hints_mutex)
#  define HINTS_REFCNT_TERM		MUTEX_DESTROY(&PL_hints_mutex)
#else
#  define HINTS_REFCNT_INIT		NOOP
#  define HINTS_REFCNT_TERM		NOOP
#endif

/* Hash actions
 * Passed in PERL_MAGIC_uvar calls
 */
#define HV_DISABLE_UVAR_XKEY	0x01
/* We need to ensure that these don't clash with G_DISCARD, which is 2, as it
   is documented as being passed to hv_delete().  */
#define HV_FETCH_ISSTORE	0x04
#define HV_FETCH_ISEXISTS	0x08
#define HV_FETCH_LVALUE		0x10
#define HV_FETCH_JUST_SV	0x20
#define HV_DELETE		0x40
#define HV_FETCH_EMPTY_HE	0x80 /* Leave HeVAL null. */

/* Must not conflict with HVhek_UTF8 */
#define HV_NAME_SETALL		0x02

/*
=for apidoc newHV

Creates a new HV.  The reference count is set to 1.

=cut
*/

#define newHV()	MUTABLE_HV(newSV_type(SVt_PVHV))

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
