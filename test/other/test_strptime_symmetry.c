// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// glibc requires _XOPEN_SOURCE to be defined in order to get strptime.
#define _XOPEN_SOURCE
#include <assert.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct tm TimeStruct;

static void formatTM(TimeStruct const tm, char* buffer) {
  sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1,
    tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

static bool isTimeStructEqual(const TimeStruct LHS, const TimeStruct RHS) {
  return LHS.tm_year == RHS.tm_year && LHS.tm_mon == RHS.tm_mon && LHS.tm_mday == RHS.tm_mday &&
         LHS.tm_hour == RHS.tm_hour && LHS.tm_min == RHS.tm_min && LHS.tm_sec == RHS.tm_sec;
}

bool parseAndCompare(const char* time, const char* expectedTime) {
  TimeStruct tm;
  memset(&tm, 0, sizeof(TimeStruct));
  assert(strptime(time, "%FT%T%z", &tm));

  char actualTime[256];
  formatTM(tm, actualTime);
  printf("input   : %s\n", time);
  printf("expected: %s\n", expectedTime);
  printf("actual  : %s\n", actualTime);
  bool isEqual = strcmp(actualTime, expectedTime) == 0;
  return isEqual;
}

// Ensure that the %c specifier can roundtrip. Meaning we use the same format
// for strftime and strptime.
bool roundTripCSpecifier() {
  TimeStruct tm;
  time_t t = time(NULL);
  memcpy(&tm, localtime(&t), sizeof(TimeStruct));

  char timeBuffer[256];
  if (strftime(timeBuffer, sizeof(timeBuffer), "%c", &tm) == 0) {
    printf("Failed to format current time\n");
    return false;
  }

  TimeStruct newTm;
  const char* lastParsed;
  if ((lastParsed = strptime(timeBuffer, "%c", &newTm)) == NULL) {
    printf("Failed to parse time string '%s'\n", timeBuffer);
    return false;
  }

  return isTimeStructEqual(tm, newTm);
}

int main(int argc, char** argv) {
  assert(parseAndCompare("2018-03-27T19:33:09+0000", "2018-03-27T19:33:09"));
  assert(parseAndCompare("2018-03-27T19:33:09-0735", "2018-03-27T19:33:09"));
  assert(parseAndCompare("2018-03-27T19:33:09+1043", "2018-03-27T19:33:09"));
  assert(parseAndCompare("1900-01-01T00:00:00+0000", "1900-01-01T00:00:00"));
  assert(parseAndCompare("2018-12-31T23:59:59+0000", "2018-12-31T23:59:59"));
  assert(roundTripCSpecifier());
  printf("TEST PASSED\n");
  return 0;
}
