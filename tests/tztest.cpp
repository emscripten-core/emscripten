#include <time.h>
#include <stdio.h>

int main() {
  tzset();
  printf("timezone=%ld", timezone);
  return 0;
}
