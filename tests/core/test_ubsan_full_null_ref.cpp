#include <stdlib.h>

int main(void) {
  char options[] = "UBSAN_OPTIONS=color=never";
  putenv(options);

  int *p = nullptr;
  int &a = *p;
  auto &b = *p;
  auto &&c = *p;
}
