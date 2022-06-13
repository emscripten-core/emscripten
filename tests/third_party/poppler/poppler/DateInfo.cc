//========================================================================
//
// DateInfo.cc
//
// Copyright (C) 2008 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

//========================================================================
//
// Based on code from pdfinfo.cc
//
// Copyright 1998-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#include "DateInfo.h"

#include <stdio.h>
#include <string.h>

/* See PDF Reference 1.3, Section 3.8.2 for PDF Date representation */
GBool parseDateString(const char *dateString, int *year, int *month, int *day, int *hour, int *minute, int *second, char *tz, int *tzHour, int *tzMinute)
{
    if ( dateString == NULL ) return gFalse;
    if ( strlen(dateString) < 2 ) return gFalse;

    if ( dateString[0] == 'D' && dateString[1] == ':' )
        dateString += 2;

    *month = 1;
    *day = 1;
    *hour = 0;
    *minute = 0;
    *second = 0;
    *tz = 0x00;
    *tzHour = 0;
    *tzMinute = 0;

    if ( sscanf( dateString,
                 "%4d%2d%2d%2d%2d%2d%c%2d%*c%2d",
                 year, month, day, hour, minute, second,
                 tz, tzHour, tzMinute ) > 0 ) {
        /* Workaround for y2k bug in Distiller 3 stolen from gpdf, hoping that it won't
        * be used after y2.2k */
        if ( *year < 1930 && strlen (dateString) > 14)
        {
           int century, years_since_1900;
           if ( sscanf( dateString,
                        "%2d%3d%2d%2d%2d%2d%2d",
                        &century, &years_since_1900, month, day, hour, minute, second) == 7 )
           {
               *year = century * 100 + years_since_1900;
           }
           else
           {
               return gFalse;
           }
       }

       if (*year <= 0) return gFalse;

       return gTrue;
   }

   return gFalse;
}


GooString *timeToDateString(time_t *timet) {
  GooString *dateString;
  char s[5];
  struct tm *gt;
  size_t len;
  time_t timep = timet ? *timet : time(NULL);
  
#ifdef HAVE_GMTIME_R
  struct tm t;
  gt = gmtime_r (&timep, &t);
#else
  gt = gmtime (&timep);
#endif

  dateString = new GooString ("D:");

  /* Year YYYY */
  len = strftime (s, sizeof(s), "%Y", gt);
  dateString->append (s, len);

  /* Month MM */
  len = strftime (s, sizeof(s), "%m", gt);
  dateString->append (s, len);

  /* Day DD */
  len = strftime (s, sizeof(s), "%d", gt);
  dateString->append (s, len);

  /* Hour HH */
  len = strftime (s, sizeof(s), "%H", gt);
  dateString->append (s, len);

  /* Minute mm */
  len = strftime (s, sizeof(s), "%M", gt);
  dateString->append (s, len);

  /* Second SS */
  len = strftime (s, sizeof(s), "%S", gt);
  dateString->append (s, len);

  return dateString;
}

