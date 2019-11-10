/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <langinfo.h>

int main() {
  printf("%16s: \"%s\"\n", "CODESET", nl_langinfo(CODESET));
  printf("%16s: \"%s\"\n", "D_T_FMT", nl_langinfo(D_T_FMT));
  printf("%16s: \"%s\"\n", "D_FMT", nl_langinfo(D_FMT));
  printf("%16s: \"%s\"\n", "T_FMT", nl_langinfo(T_FMT));
  printf("%16s: \"%s\"\n", "T_FMT_AMPM", nl_langinfo(T_FMT_AMPM));
  printf("%16s: \"%s\"\n", "AM_STR", nl_langinfo(AM_STR));
  printf("%16s: \"%s\"\n", "PM_STR", nl_langinfo(PM_STR));
  printf("%16s: \"%s\"\n", "DAY_1", nl_langinfo(DAY_1));
  printf("%16s: \"%s\"\n", "DAY_2", nl_langinfo(DAY_2));
  printf("%16s: \"%s\"\n", "DAY_3", nl_langinfo(DAY_3));
  printf("%16s: \"%s\"\n", "DAY_4", nl_langinfo(DAY_4));
  printf("%16s: \"%s\"\n", "DAY_5", nl_langinfo(DAY_5));
  printf("%16s: \"%s\"\n", "DAY_6", nl_langinfo(DAY_6));
  printf("%16s: \"%s\"\n", "DAY_7", nl_langinfo(DAY_7));
  printf("%16s: \"%s\"\n", "ABDAY_1", nl_langinfo(ABDAY_1));
  printf("%16s: \"%s\"\n", "ABDAY_2", nl_langinfo(ABDAY_2));
  printf("%16s: \"%s\"\n", "ABDAY_3", nl_langinfo(ABDAY_3));
  printf("%16s: \"%s\"\n", "ABDAY_4", nl_langinfo(ABDAY_4));
  printf("%16s: \"%s\"\n", "ABDAY_5", nl_langinfo(ABDAY_5));
  printf("%16s: \"%s\"\n", "ABDAY_6", nl_langinfo(ABDAY_6));
  printf("%16s: \"%s\"\n", "ABDAY_7", nl_langinfo(ABDAY_7));
  printf("%16s: \"%s\"\n", "MON_1", nl_langinfo(MON_1));
  printf("%16s: \"%s\"\n", "MON_2", nl_langinfo(MON_2));
  printf("%16s: \"%s\"\n", "MON_3", nl_langinfo(MON_3));
  printf("%16s: \"%s\"\n", "MON_4", nl_langinfo(MON_4));
  printf("%16s: \"%s\"\n", "MON_5", nl_langinfo(MON_5));
  printf("%16s: \"%s\"\n", "MON_6", nl_langinfo(MON_6));
  printf("%16s: \"%s\"\n", "MON_7", nl_langinfo(MON_7));
  printf("%16s: \"%s\"\n", "MON_8", nl_langinfo(MON_8));
  printf("%16s: \"%s\"\n", "MON_9", nl_langinfo(MON_9));
  printf("%16s: \"%s\"\n", "MON_10", nl_langinfo(MON_10));
  printf("%16s: \"%s\"\n", "MON_11", nl_langinfo(MON_11));
  printf("%16s: \"%s\"\n", "MON_12", nl_langinfo(MON_12));
  printf("%16s: \"%s\"\n", "ABMON_1", nl_langinfo(ABMON_1));
  printf("%16s: \"%s\"\n", "ABMON_2", nl_langinfo(ABMON_2));
  printf("%16s: \"%s\"\n", "ABMON_3", nl_langinfo(ABMON_3));
  printf("%16s: \"%s\"\n", "ABMON_4", nl_langinfo(ABMON_4));
  printf("%16s: \"%s\"\n", "ABMON_5", nl_langinfo(ABMON_5));
  printf("%16s: \"%s\"\n", "ABMON_6", nl_langinfo(ABMON_6));
  printf("%16s: \"%s\"\n", "ABMON_7", nl_langinfo(ABMON_7));
  printf("%16s: \"%s\"\n", "ABMON_8", nl_langinfo(ABMON_8));
  printf("%16s: \"%s\"\n", "ABMON_9", nl_langinfo(ABMON_9));
  printf("%16s: \"%s\"\n", "ABMON_10", nl_langinfo(ABMON_10));
  printf("%16s: \"%s\"\n", "ABMON_11", nl_langinfo(ABMON_11));
  printf("%16s: \"%s\"\n", "ABMON_12", nl_langinfo(ABMON_12));
  printf("%16s: \"%s\"\n", "ERA", nl_langinfo(ERA));
  printf("%16s: \"%s\"\n", "ERA_D_FMT", nl_langinfo(ERA_D_FMT));
  printf("%16s: \"%s\"\n", "ERA_D_T_FMT", nl_langinfo(ERA_D_T_FMT));
  printf("%16s: \"%s\"\n", "ERA_T_FMT", nl_langinfo(ERA_T_FMT));
  printf("%16s: \"%s\"\n", "ALT_DIGITS", nl_langinfo(ALT_DIGITS));
  printf("%16s: \"%s\"\n", "RADIXCHAR", nl_langinfo(RADIXCHAR));
  printf("%16s: \"%s\"\n", "THOUSEP", nl_langinfo(THOUSEP));
  printf("%16s: \"%s\"\n", "YESEXPR", nl_langinfo(YESEXPR));
  printf("%16s: \"%s\"\n", "NOEXPR", nl_langinfo(NOEXPR));
  printf("%16s: \"%s\"\n", "CRNCYSTR", nl_langinfo(CRNCYSTR));

  printf("%16s: \"%s\"\n", "(bad value)", nl_langinfo(123));

  return 0;
}
