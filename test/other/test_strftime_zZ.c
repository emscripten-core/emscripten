#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main() {
  void tzset(void);

  // Buffer to hold the current hour of the day.  Format is HH + nul
  // character.
  char hour[3];

  // Buffer to hold our ISO 8601 formatted UTC offset for the current
  // timezone.  Format is [+-]hhmm + nul character.
  char utcOffset[6];

  // Buffer to hold the timezone name or abbreviation.  Just make it
  // sufficiently large to hold most timezone names.
  char timezone[128];

  struct tm tm;

  // Get the current timestamp.
  const time_t now = time(NULL);

  // What time is that here?
  if (localtime_r(&now, &tm) == NULL) {
    const int error = errno;
    printf("Failed to get localtime for timestamp=%lld; errno=%d; %s", now, errno, strerror(error));
    return 1;
  }

  size_t result = 0;

  // Get the formatted hour of the day.
  if ((result = strftime(hour, 3, "%H", &tm)) != 2) {
    const int error = errno;
    printf("Failed to format hour for timestamp=%lld; result=%zu; errno=%d; %s\n",
           now, result, error, strerror(error));
    return 1;
  }
  printf("The current hour of the day is: %s\n", hour);

  // Get the formatted UTC offset in ISO 8601 format.
  if ((result = strftime(utcOffset, 6, "%z", &tm)) != 5) {
    const int error = errno;
    printf("Failed to format UTC offset for timestamp=%lld; result=%zu; errno=%d; %s\n",
           now, result, error, strerror(error));
    return 1;
  }
  printf("The current timezone offset is: %s\n", utcOffset);

  // Get the formatted timezone name or abbreviation.  We don't know how long
  // this will be, so just expect some data to be written to the buffer.
  if ((result = strftime(timezone, 128, "%Z", &tm)) == 0) {
    const int error = errno;
    printf("Failed to format timezone for timestamp=%lld; result=%zu; errno=%d; %s\n",
           now, result, error, strerror(error));
    return 1;
  }
  printf("The current timezone is: %s\n", timezone);

  printf("ok!\n");
  return 0;
}
