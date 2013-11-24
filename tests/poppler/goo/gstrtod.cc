/* This file is part of Libspectre.
 * 
 * Copyright (C) 2007, 2012 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2007 Carlos Garcia Campos <carlosgc@gnome.org>
 *
 * Libspectre is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Libspectre is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/* This function comes from spectre-utils from libspectre */

#include "gstrtod.h"

#include <locale.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define ascii_isspace(c) \
  (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#define ascii_isdigit(c) \
  (c >= '0' && c <= '9')
  
double gatof(const char *nptr)
{
  return gstrtod(nptr, NULL);
}

double gstrtod(const char *nptr, char **endptr)
{
  char *fail_pos;
  double val;
  struct lconv *locale_data;
  const char *decimal_point;
  int decimal_point_len;
  const char *p, *decimal_point_pos;
  const char *end = NULL; /* Silence gcc */
  int strtod_errno;

  fail_pos = NULL;

  locale_data = localeconv ();
  decimal_point = locale_data->decimal_point;
  decimal_point_len = strlen (decimal_point);

  decimal_point_pos = NULL;
  end = NULL;

  if (decimal_point[0] != '.' || decimal_point[1] != 0) {
    p = nptr;
    /* Skip leading space */
    while (ascii_isspace (*p))
      p++;
    
    /* Skip leading optional sign */
    if (*p == '+' || *p == '-')
      p++;
    
    if (ascii_isdigit (*p) || *p == '.') {
      while (ascii_isdigit (*p))
        p++;
      
      if (*p == '.')
        decimal_point_pos = p++;

      while (ascii_isdigit (*p))
        p++;

      if (*p == 'e' || *p == 'E')
        p++;
      if (*p == '+' || *p == '-')
        p++;
      while (ascii_isdigit (*p))
        p++;

      end = p;
    }
    /* For the other cases, we need not convert the decimal point */
  }

  if (decimal_point_pos) {
    char *copy, *c;
    
    /* We need to convert the '.' to the locale specific decimal point */
    copy = (char *) malloc (end - nptr + 1 + decimal_point_len);
    
    c = copy;
    memcpy (c, nptr, decimal_point_pos - nptr);
    c += decimal_point_pos - nptr;
    memcpy (c, decimal_point, decimal_point_len);
    c += decimal_point_len;
    memcpy (c, decimal_point_pos + 1, end - (decimal_point_pos + 1));
    c += end - (decimal_point_pos + 1);
    *c = 0;

    errno = 0;
    val = strtod (copy, &fail_pos);
    strtod_errno = errno;

    if (fail_pos) {
      if (fail_pos - copy > decimal_point_pos - nptr)
        fail_pos = (char *)nptr + (fail_pos - copy) - (decimal_point_len - 1);
      else
        fail_pos = (char *)nptr + (fail_pos - copy);
    }

    free (copy);
  } else if (end) {
    char *copy;
    
    copy = (char *) malloc (end - (char *)nptr + 1);
    memcpy (copy, nptr, end - nptr);
    *(copy + (end - (char *)nptr)) = 0;
    
    errno = 0;
    val = strtod (copy, &fail_pos);
    strtod_errno = errno;

    if (fail_pos) {
      fail_pos = (char *)nptr + (fail_pos - copy);
    }

    free (copy);
  } else {
    errno = 0;
    val = strtod (nptr, &fail_pos);
    strtod_errno = errno;
  }

  if (endptr)
    *endptr = fail_pos;

  errno = strtod_errno;

  return val;
}
