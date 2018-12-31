#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

int main(void) {
  time_t t = time(NULL);
  struct tm *ptm = gmtime(&t);
  struct tm tmCurrent = *ptm;
  int hour = tmCurrent.tm_hour;

  t -= hour * 3600;  // back to midnight
  int yday = -1;
  for (hour = 0; hour < 24; hour++) {
    ptm = gmtime(&t);
    // tm_yday must be constant all day...
    printf("yday: %d, hour: %d\n", ptm->tm_yday, hour);
    if (yday == -1)
      yday = ptm->tm_yday;
    else
      assert(yday == ptm->tm_yday);
    t += 3600;  // add one hour
  }
  printf("ok!\n");
  return (0);
}
