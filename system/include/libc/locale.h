/*
	locale.h
	Values appropriate for the formatting of monetary and other
	numberic quantities.
*/

#ifndef _LOCALE_H_
#define _LOCALE_H_

#include "_ansi.h"

#ifndef NULL
#define NULL    0
#endif

#define LC_ALL	    0
#define LC_COLLATE  1
#define LC_CTYPE    2
#define LC_MONETARY 3
#define LC_NUMERIC  4
#define LC_TIME     5
#define LC_MESSAGES 6

/* XXX Emscripten: add masks */
#define LC_ALL_MASK      (1 << LC_ALL)
#define LC_COLLATE_MASK  (1 << LC_COLLATE)
#define LC_CTYPE_MASK    (1 << LC_CTYPE)
#define LC_MONETARY_MASK (1 << LC_MONETARY)
#define LC_NUMERIC_MASK  (1 << LC_NUMERIC)
#define LC_TIME_MASK     (1 << LC_TIME)
#define LC_MESSAGES_MASK (1 << LC_MESSAGES)

_BEGIN_STD_C

struct lconv
{
  char *decimal_point;
  char *thousands_sep;
  char *grouping;
  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;
  char p_cs_precedes;
  char p_sep_by_space;
  char n_cs_precedes;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
  char int_n_cs_precedes;
  char int_n_sep_by_space;
  char int_n_sign_posn;
  char int_p_cs_precedes;
  char int_p_sep_by_space;
  char int_p_sign_posn;
};

#ifndef _REENT_ONLY
char *_EXFUN(setlocale,(int category, const char *locale));
struct lconv *_EXFUN(localeconv,(void));
#endif

struct _reent;
char *_EXFUN(_setlocale_r,(struct _reent *, int category, const char *locale));
struct lconv *_EXFUN(_localeconv_r,(struct _reent *));

/* XXX Emscripten */
typedef void* locale_t;
locale_t newlocale(int category_mask, const char *locale, locale_t base);
void freelocale(locale_t locobj);
locale_t uselocale(locale_t newloc);

_END_STD_C

#endif /* _LOCALE_H_ */
