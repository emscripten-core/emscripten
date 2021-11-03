#include <locale.h>
#include <stdio.h>

void test(const char* name) {
  locale_t loc = newlocale(LC_ALL_MASK, name, 0);
  if (loc)
    printf("newlocale '%s' succeeded\n", name);
  else
    printf("newlocale '%s' failed\n", name);
  freelocale(loc);
}

int main(int argc, char* argv[]) {
  test("C");
  test("waka");
  return 0;
}
