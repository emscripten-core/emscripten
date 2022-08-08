#include <unicode/ucnv.h>

int main() {
  UErrorCode e;
  UConverter *p = ucnv_open("UTF8", &e);
  if (p) {
    ucnv_close(p);
  }
  return 0;
}
