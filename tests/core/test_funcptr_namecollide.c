#include <stdio.h>

void do_call(void (*puts)(const char *), const char *str);

void do_print(const char *str) {
  if (!str) do_call(NULL, "delusion");
  if ((int)str == -1) do_print(str + 10);
  puts("====");
  puts(str);
  puts("====");
}

void do_call(void (*puts)(const char *), const char *str) {
  if (!str) do_print("confusion");
  if ((int)str == -1) do_call(NULL, str - 10);
  (*puts)(str);
}

int main(int argc, char **argv) {
  for (int i = 0; i < argc; i++) {
    do_call(i != 10 ? do_print : NULL, i != 15 ? "waka waka" : NULL);
  }
  return 0;
}
