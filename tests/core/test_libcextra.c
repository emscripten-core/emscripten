#include <stdio.h>
#include <wchar.h>

int main() {
  const wchar_t* wstr = L"Hello";

  printf("wcslen: %d\n", wcslen(wstr));

  return 0;
}
