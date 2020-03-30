//========================================================================
//
// This file is under the GPLv2 or later license
//
// Copyright (C) 2005-2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005, 2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011 Simon Kellner <kellner@kit.edu>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "PageLabelInfo.h"

/* http://mathworld.wolfram.com/RomanNumerals.html */

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

#ifdef TEST
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
#endif

PageLabelInfo::Interval::Interval(Object *dict, int baseA) {
  Object obj;

  style = None;
  if (dict->dictLookup("S", &obj)->isName()) {
    if (obj.isName("D")) {
      style = Arabic;
    } else if (obj.isName("R")) {
      style = UppercaseRoman;
    } else if (obj.isName("r")) {
      style = LowercaseRoman;
    } else if (obj.isName("A")) {
      style = UppercaseLatin;
    } else if (obj.isName("a")) {
      style = LowercaseLatin;
    }
  }
  obj.free();

  if (dict->dictLookup("P", &obj)->isString())
    prefix = obj.getString()->copy();
  else
    prefix = new GooString("");
  obj.free();

  if (dict->dictLookup("St", &obj)->isInt())
    first = obj.getInt();
  else
    first = 1;
  obj.free();

  base = baseA;
}

PageLabelInfo::Interval::~Interval() {
  delete prefix;
}

PageLabelInfo::PageLabelInfo(Object *tree, int numPages) {
  int i;
  Interval *interval, *next;

  parse(tree);

  for (i = 0; i < intervals.getLength(); i++) {
    interval = (Interval *) intervals.get(i);

    if (i + 1 < intervals.getLength()) {
      next = (Interval *) intervals.get(i + 1);
      interval->length = next->base - interval->base;
    } else {
      interval->length = numPages - interval->base;
    }
    if (interval->length < 0) interval->length = 0;
  }
}

PageLabelInfo::~PageLabelInfo() {
  int i;
  for (i = 0; i < intervals.getLength(); ++i) {
    delete (Interval*)intervals.get(i);
  }
}

void PageLabelInfo::parse(Object *tree) {
  Object nums, obj;
  Object kids, kid, limits, low, high;
  int i, base;
  Interval *interval;

  // leaf node
  if (tree->dictLookup("Nums", &nums)->isArray()) {
    for (i = 0; i < nums.arrayGetLength(); i += 2) {
      if (!nums.arrayGet(i, &obj)->isInt()) {
	obj.free();
	continue;
      }
      base = obj.getInt();
      obj.free();
      if (!nums.arrayGet(i + 1, &obj)->isDict()) {
	obj.free();
	continue;
      }

      interval = new Interval(&obj, base);
      obj.free();
      intervals.append(interval);
    }
  }
  nums.free();

  if (tree->dictLookup("Kids", &kids)->isArray()) {
    for (i = 0; i < kids.arrayGetLength(); ++i) {
      if (kids.arrayGet(i, &kid)->isDict())
	parse(&kid);
      kid.free();
    }
  }
  kids.free();
}

GBool PageLabelInfo::labelToIndex(GooString *label, int *index)
{
  Interval *interval;
  char *str = label->getCString(), *end;
  int prefixLength;
  int i, number;

  for (i = 0; i < intervals.getLength(); i++) {
    interval = (Interval *) intervals.get(i);
    const int base = interval->base;
    prefixLength = interval->prefix->getLength();
    if (label->cmpN(interval->prefix, prefixLength) != 0)
      continue;

    switch (interval->style) {
    case Interval::Arabic:
      number = strtol(str + prefixLength, &end, 10);
      if (*end == '\0' && number - interval->first < interval->length) {
	*index = base + number - interval->first;
	return gTrue;
      }
      break;
    case Interval::LowercaseRoman:
    case Interval::UppercaseRoman:
      number = fromRoman(str + prefixLength);
      if (number >= 0 && number - interval->first < interval->length) {
	*index = base + number - interval->first;
	return gTrue;
      }
      break;
    case Interval::UppercaseLatin:
    case Interval::LowercaseLatin:
      number = fromLatin(str + prefixLength);
      if (number >= 0 && number - interval->first < interval->length) {
	*index = base + number - interval->first;
	return gTrue;
      }
      break;
    case Interval::None:
      break;
    }
  }

  return gFalse;
}

GBool PageLabelInfo::indexToLabel(int index, GooString *label)
{
  char buffer[32];
  int i, base, number;
  Interval *interval;
  GooString number_string;

  base = 0;
  interval = NULL;
  for (i = 0; i < intervals.getLength(); i++) {
    interval = (Interval *) intervals.get(i);
    if (base <= index && index < base + interval->length)
      break;
    base += interval->length;
  }

  if (i == intervals.getLength())
    return gFalse;

  number = index - base + interval->first;
  switch (interval->style) {
  case Interval::Arabic:
    snprintf (buffer, sizeof(buffer), "%d", number);
    number_string.append(buffer);
    break;
  case Interval::LowercaseRoman:
    toRoman(number, &number_string, gFalse);
    break;
  case Interval::UppercaseRoman:
    toRoman(number, &number_string, gTrue);
    break;
  case Interval::UppercaseLatin:
  case Interval::LowercaseLatin:
    number = 0;
    break;
  case Interval::None:
    break;
  }

  label->clear();
  label->append(interval->prefix);
  if (label->hasUnicodeMarker()) {
      int i, len;
      char ucs2_char[2];

      /* Convert the ascii number string to ucs2 and append. */
      len = number_string.getLength ();
      ucs2_char[0] = 0;
      for (i = 0; i < len; ++i) {
	  ucs2_char[1] = number_string.getChar(i);
	  label->append(ucs2_char, 2);
      }
      ucs2_char[1] = 0;
      label->append(ucs2_char, 2);
  } else {
      label->append(&number_string);
  }

  return gTrue;
}

#ifdef TEST
int main(int argc, char *argv[])
{
  {
    GooString str;
    toRoman(177, &str, gFalse);
    assert (str.cmp("clxxvii") == 0);
  }

  {
    GooString roman("clxxvii");
    assert (fromRoman(roman.getCString()) == 177);
  }

  {
    GooString str;
    toLatin(54, &str, gFalse);
    assert (str.cmp("bbb") == 0);
  }

  {
    GooString latin("ddd");
    assert (fromLatin(latin.getCString()) == 56);
  }
}
#endif
