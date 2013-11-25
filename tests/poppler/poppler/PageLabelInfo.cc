//========================================================================
//
// This file is under the GPLv2 or later license
//
// Copyright (C) 2005-2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005, 2009, 2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011 Simon Kellner <kellner@kit.edu>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
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
#include "PageLabelInfo_p.h"

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
  case Interval::LowercaseLatin:
    toLatin(number, &number_string, gFalse);
    break;
  case Interval::UppercaseLatin:
    toLatin(number, &number_string, gTrue);
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
  } else {
      label->append(&number_string);
  }

  return gTrue;
}
