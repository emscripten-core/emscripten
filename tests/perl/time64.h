#include <time.h>
#include "time64_config.h"

#ifndef TIME64_H
#    define TIME64_H


/* Set our custom types */
typedef INT_64_T        Int64;
typedef Int64           Time64_T;
typedef I32             Year;


/* A copy of the tm struct but with a 64 bit year */
struct TM64 {
        int     tm_sec;
        int     tm_min;
        int     tm_hour;
        int     tm_mday;
        int     tm_mon;
        Year    tm_year;
        int     tm_wday;
        int     tm_yday;
        int     tm_isdst;

#ifdef HAS_TM_TM_GMTOFF
        long    tm_gmtoff;
#endif

#ifdef HAS_TM_TM_ZONE
#  ifdef __GLIBC__
        const char    *tm_zone;
#  else
        char    *tm_zone;
#  endif
#endif
};


/* Decide which tm struct to use */
#ifdef USE_TM64
#define TM      TM64
#else
#define TM      tm
#endif


/* Declare functions */
static struct TM *S_gmtime64_r    (const Time64_T *, struct TM *);
static struct TM *S_localtime64_r (const Time64_T *, struct TM *);
static Time64_T   S_timegm64      (struct TM *);


/* Not everyone has gm/localtime_r(), provide a replacement */
#ifdef HAS_LOCALTIME_R
#    define LOCALTIME_R(clock, result) (L_R_TZSET localtime_r(clock, result))
#else
#    define LOCALTIME_R(clock, result) (L_R_TZSET S_localtime_r(clock, result))
#endif
#ifdef HAS_GMTIME_R
#    define GMTIME_R(clock, result)    gmtime_r(clock, result)
#else
#    define GMTIME_R(clock, result)    S_gmtime_r(clock, result)
#endif

#endif
