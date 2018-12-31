#include <string>
#include <time.h>
#include <cstring>
#include <cstdio>

bool parseAndCompare(const std::string& time, std::string expectedTime) {
  char actualTimeBuffer[256];

  struct tm tm;
  memset(&tm, 0, sizeof(struct tm));
  static const char* format = "%FT%T%z";
  strptime(time.c_str(), format, &tm);
       
  sprintf(actualTimeBuffer,
    "%04d-%02d-%02dT%02d:%02d:%02d", 
    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
    tm.tm_hour, tm.tm_min, tm.tm_sec);

  std::string actualTime = actualTimeBuffer;
  printf("%s: %s == %s, %s\n", time.c_str(), actualTime.c_str(), expectedTime.c_str(),
    actualTime == expectedTime ? "true" : "false");

  return actualTime == expectedTime;
}

int main(int argc, char** argv) {
  bool testPassed = 
    parseAndCompare("2018-03-27T19:33:09+0000", "2018-03-27T19:33:09") &&
    parseAndCompare("2018-03-27T19:33:09-0735", "2018-03-27T19:33:09") &&
    parseAndCompare("2018-03-27T19:33:09+1043", "2018-03-27T19:33:09") &&
    parseAndCompare("1900-01-01T00:00:00+0000", "1900-01-01T00:00:00") &&
    parseAndCompare("2018-12-31T23:59:59+0000", "2018-12-31T23:59:59");

  if (testPassed) {
    printf("TEST PASSED\n");
  } else {
    printf("TEST FAILED\n");
  }

  return 0;
}