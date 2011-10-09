#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  printf("List:\n");
  for (char** str = environ; *str; str++) {
    printf("%s\n", *str);
  }
  printf("\n");

  printf("getenv(PATH): %s\n", getenv("PATH"));
  printf("getenv(NONEXISTENT): %s\n", getenv("NONEXISTENT"));

  printf("setenv/0(PATH) ret: %d\n", setenv("PATH", "test", 0));
  printf("getenv(PATH) after setenv/0: %s\n", getenv("PATH"));
  printf("setenv/1(PATH) ret: %d\n", setenv("PATH", "test2", 1));
  printf("getenv(PATH) after setenv/1: %s\n", getenv("PATH"));

  printf("setenv(SET_VALUE) ret: %d\n", setenv("SET_VALUE", "test3", 0));
  printf("setenv(SET_VALUE2) ret: %d\n", setenv("SET_VALUE2", "test4", 1));
  printf("getenv(SET_VALUE): %s\n", getenv("SET_VALUE"));
  printf("getenv(SET_VALUE2): %s\n", getenv("SET_VALUE2"));

  char buffer[] = "PUT_VALUE=test5";
  printf("putenv(PUT_VALUE) ret: %d\n", putenv(buffer));
  printf("getenv(PUT_VALUE): %s\n", getenv("PUT_VALUE"));
  // NOTE: This should change the value. In our implementation, it doesn't.
  buffer[10] = 'Q';
  printf("getenv(PUT_VALUE) after alteration: %s\n", getenv("PUT_VALUE"));

  printf("unsetenv(PUT_VALUE) ret: %d\n", unsetenv("PUT_VALUE"));
  printf("getenv(PUT_VALUE) after unsetenv: %s\n", getenv("PUT_VALUE"));

  printf("setenv(0) ret: %d\n", setenv(0, "foo", 1));
  printf("setenv('') ret: %d\n", setenv("", "foo", 1));
  printf("setenv(X=Y) ret: %d\n", setenv("X=Y", "foo", 1));
  printf("unsetenv(0) ret: %d\n", unsetenv(0));
  printf("unsetenv('') ret: %d\n", unsetenv(""));
  printf("unsetenv(X=Y) ret: %d\n", unsetenv("X=Y"));

  return 0;
}
