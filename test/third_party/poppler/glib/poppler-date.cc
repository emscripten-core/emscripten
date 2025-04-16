/* poppler-date.cc: glib interface to poppler
 *
 * Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <DateInfo.h>

#include "poppler-date.h"

/**
 * poppler_date_parse:
 * @date: string to parse
 * @timet: an uninitialized #time_t
 *
 * Parses a PDF format date string and converts it to a #time_t. Returns #FALSE
 * if the parsing fails or the input string is not a valid PDF format date string
 *
 * Return value: #TRUE, if @timet was set
 *
 * Since: 0.12
 **/
gboolean
poppler_date_parse (const gchar *date,
		    time_t      *timet)
{
  gint year, mon, day, hour, min, sec, tz_hour, tz_minute;
  gchar tz;
  struct tm time;
  time_t retval;
  
  /* See PDF Reference 1.3, Section 3.8.2 for PDF Date representation */
  // TODO do something with the timezone information
  if (!parseDateString (date, &year, &mon, &day, &hour, &min, &sec, &tz, &tz_hour, &tz_minute))
    return FALSE;
	
  time.tm_year = year - 1900;
  time.tm_mon = mon - 1;
  time.tm_mday = day;
  time.tm_hour = hour;
  time.tm_min = min;
  time.tm_sec = sec;
  time.tm_wday = -1;
  time.tm_yday = -1;
  time.tm_isdst = -1; /* 0 = DST off, 1 = DST on, -1 = don't know */
 
  /* compute tm_wday and tm_yday and check date */
  retval = mktime (&time);
  if (retval == (time_t) - 1)
    return FALSE;
    
  *timet = retval;

  return TRUE;	
}
