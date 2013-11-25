//========================================================================
//
// This file is under the GPLv2 or later license
//
// Copyright (C) 2005-2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005, 2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011 Simon Kellner <kellner@kit.edu>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

/* http://mathworld.wolfram.com/RomanNumerals.html */

#include "goo/GooString.h"

static int fromRoman(const char *buffer) {
  int digit_value, prev_digit_value, value;
  int i;

  prev_digit_value = INT_MAX;
  value = 0;
  for (i = 0; buffer[i] != '\0'; i++) {
    switch (buffer[i]) {
    case 'm':
    case 'M':
      digit_value = 1000;
      break;
    case 'd':
    case 'D':
      digit_value = 500;
      break;
    case 'c':
    case 'C':
      digit_value = 100;
      break;
    case 'l':
    case 'L':
      digit_value = 50;
      break;
    case 'x':
    case 'X':
      digit_value = 10;
      break;
    case 'v':
    case 'V':
      digit_value = 5;
      break;
    case 'i':
    case 'I':
      digit_value = 1;
      break;
    default:
      return -1;
    }

    if (digit_value <= prev_digit_value)
      value += digit_value;
    else
      value += digit_value - prev_digit_value * 2;
    prev_digit_value = digit_value;
  }

  return value;
}

static void toRoman(int number, GooString *str, GBool uppercase) {
  static const char uppercaseNumerals[] = "IVXLCDM";
  static const char lowercaseNumerals[] = "ivxlcdm";
  int divisor;
  int i, j, k;
  const char *wh;

  if (uppercase)
    wh = uppercaseNumerals;
  else
    wh = lowercaseNumerals;

  divisor = 1000;
  for (k = 3; k >= 0; k--) {
    i = number / divisor;
    number = number % divisor;

    switch (i) {
    case 0:
      break;
    case 5:
      str->append(wh[2 * k + 1]);
      break;
    case 9:
      str->append(wh[2 * k + 0]);
      str->append(wh[ 2 * k + 2]);
      break;
    case 4:
      str->append(wh[2 * k + 0]);
      str->append(wh[2 * k + 1]);
      break;
    default:
      if (i > 5) {
       str->append(wh[2 * k + 1]);
       i -= 5;
      }
      for (j = 0; j < i; j++) {
       str->append(wh[2 * k + 0]);
      }
    }
       
    divisor = divisor / 10;
  }
}

static int fromLatin(const char *buffer)
{
  int count;
  const char *p;

  for (p = buffer; *p; p++) {
    if (*p != buffer[0])
      return -1;
  }

  count = p - buffer;
  if (buffer[0] >= 'a' && buffer[0] <= 'z')
    return 26 * (count - 1) + buffer[0] - 'a' + 1;
  if (buffer[0] >= 'A' && buffer[0] <= 'Z')
    return 26 * (count - 1) + buffer[0] - 'A' + 1;

  return -1;
}

static void toLatin(int number, GooString *str, GBool uppercase) {
  char base, letter;
  int i, count;

  if (uppercase)
    base = 'A';
  else
    base = 'a';

  count = (number - 1) / 26 + 1;
  letter = base + (number - 1) % 26;

  for (i = 0; i < count; i++)
    str->append(letter);
}
