// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string>
#include <time.h>
#include <cstring>
#include <cstdio>

typedef struct tm TimeStruct;

static std::string formatTM(TimeStruct const& tm) {
  char actualTimeBuffer[256];
  sprintf(actualTimeBuffer, "%04d-%02d-%02dT%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1,
    tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  return actualTimeBuffer;
}

static bool isTimeStructEqual(const TimeStruct& LHS, const TimeStruct& RHS) {
  return LHS.tm_year == RHS.tm_year && LHS.tm_mon == RHS.tm_mon && LHS.tm_mday == RHS.tm_mday &&
         LHS.tm_hour == RHS.tm_hour && LHS.tm_min == RHS.tm_min && LHS.tm_sec == RHS.tm_sec;
}

bool parseAndCompare(const std::string& time, std::string expectedTime) {
  TimeStruct tm;
  memset(&tm, 0, sizeof(TimeStruct));
  static const char* format = "%FT%T%z";
  strptime(time.c_str(), format, &tm);

  std::string actualTime = formatTM(tm);
  printf("%s: %s == %s, %s\n", time.c_str(), actualTime.c_str(), expectedTime.c_str(),
    actualTime == expectedTime ? "true" : "false");

  return actualTime == expectedTime;
}

// Ensure that the %c specifier can roundtrip. Meaning we use the same format
// for strftime and strptime.
bool roundTripCSpecifier() {
  TimeStruct tm;
  time_t t = time(NULL);
  memcpy(&tm, localtime(&t), sizeof(TimeStruct));

  char timeBuffer[256];
  if (strftime(timeBuffer, sizeof(timeBuffer), "%c", &tm) == NULL) {
    printf("Failed to format current time\n");
    return false;
  }
  const std::string formattedValue = timeBuffer;

  TimeStruct newTm;
  const char* lastParsed;
  if ((lastParsed = strptime(timeBuffer, "%c", &newTm)) == NULL) {
    printf("Failed to parse time string '%s'\n", timeBuffer);
    return false;
  }

  return isTimeStructEqual(tm, newTm);
}

int main(int argc, char** argv) {
  bool testPassed = parseAndCompare("2018-03-27T19:33:09+0000", "2018-03-27T19:33:09") &&
                    parseAndCompare("2018-03-27T19:33:09-0735", "2018-03-27T19:33:09") &&
                    parseAndCompare("2018-03-27T19:33:09+1043", "2018-03-27T19:33:09") &&
                    parseAndCompare("1900-01-01T00:00:00+0000", "1900-01-01T00:00:00") &&
                    parseAndCompare("2018-12-31T23:59:59+0000", "2018-12-31T23:59:59") &&
                    roundTripCSpecifier();

  if (testPassed) {
    printf("TEST PASSED\n");
  } else {
    printf("TEST FAILED\n");
  }

  return 0;
}