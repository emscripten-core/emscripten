#define _BSD_SOURCE
#include <stdio.h>
#include <ctype.h>

int main() {
  printf("%c%c%c\n", tolower('a'), tolower('B'), tolower('1'));
  printf("%c%c%c\n", toupper('a'), toupper('B'), toupper('1'));

  const int* tl = *__ctype_tolower_loc();
  printf("%c%c%c\n", (char)tl['c'], (char)tl['D'], (char)tl['2']);
  const int* tu = *__ctype_toupper_loc();
  printf("%c%c%c\n", (char)tu['c'], (char)tu['D'], (char)tu['2']);

  printf("\n");

  printf("toascii(-45): %d\n", toascii(-45));
  printf("toascii( 10): %d\n", toascii(10));
  printf("toascii('a'): %d\n", toascii('a'));
  printf("toascii(183): %d\n", toascii(183));

  // These will be compiled as actual function calls.
  int (*funcs[])(int) = {
    isascii,
    islower,
    isupper,
    isupper,
    isalpha,
    isdigit,
    isxdigit,
    isalnum,
    ispunct,
    isspace,
    isblank,
    iscntrl,
    isprint,
    isgraph
  };
  char* funcNames[] = {
    "isascii",
    "islower",
    "isupper",
    "isupper",
    "isalpha",
    "isdigit",
    "isxdigit",
    "isalnum",
    "ispunct",
    "isspace",
    "isblank",
    "iscntrl",
    "isprint",
    "isgraph"
  };
  for (int i = 0; i < sizeof funcs / sizeof funcs[0]; i++) {
    printf("\n");
    printf("%s/call(-45): %d\n", funcNames[i], funcs[i](-45) != 0);
    printf("%s/call( 10): %d\n", funcNames[i], funcs[i](10) != 0);
    printf("%s/call('a'): %d\n", funcNames[i], funcs[i]('a') != 0);
    printf("%s/call('A'): %d\n", funcNames[i], funcs[i]('A') != 0);
    printf("%s/call('5'): %d\n", funcNames[i], funcs[i]('5') != 0);
    printf("%s/call('.'): %d\n", funcNames[i], funcs[i]('.') != 0);
    printf("%s/call(183): %d\n", funcNames[i], funcs[i](183) != 0);
  }

  // These will be compiled as macros that use __ctype_b_loc.
  printf("\n");
  printf("isascii(-45): %d\n", isascii(-45) != 0);
  printf("isascii( 10): %d\n", isascii( 10) != 0);
  printf("isascii('a'): %d\n", isascii('a') != 0);
  printf("isascii('A'): %d\n", isascii('A') != 0);
  printf("isascii('5'): %d\n", isascii('5') != 0);
  printf("isascii('.'): %d\n", isascii('.') != 0);
  printf("isascii(183): %d\n", isascii(183) != 0);

  printf("\n");
  printf("islower(-45): %d\n", islower(-45) != 0);
  printf("islower( 10): %d\n", islower( 10) != 0);
  printf("islower('a'): %d\n", islower('a') != 0);
  printf("islower('A'): %d\n", islower('A') != 0);
  printf("islower('5'): %d\n", islower('5') != 0);
  printf("islower('.'): %d\n", islower('.') != 0);
  printf("islower(183): %d\n", islower(183) != 0);

  printf("\n");
  printf("isupper(-45): %d\n", isupper(-45) != 0);
  printf("isupper( 10): %d\n", isupper( 10) != 0);
  printf("isupper('a'): %d\n", isupper('a') != 0);
  printf("isupper('A'): %d\n", isupper('A') != 0);
  printf("isupper('5'): %d\n", isupper('5') != 0);
  printf("isupper('.'): %d\n", isupper('.') != 0);
  printf("isupper(183): %d\n", isupper(183) != 0);

  printf("\n");
  printf("isupper(-45): %d\n", isupper(-45) != 0);
  printf("isupper( 10): %d\n", isupper( 10) != 0);
  printf("isupper('a'): %d\n", isupper('a') != 0);
  printf("isupper('A'): %d\n", isupper('A') != 0);
  printf("isupper('5'): %d\n", isupper('5') != 0);
  printf("isupper('.'): %d\n", isupper('.') != 0);
  printf("isupper(183): %d\n", isupper(183) != 0);

  printf("\n");
  printf("isalpha(-45): %d\n", isalpha(-45) != 0);
  printf("isalpha( 10): %d\n", isalpha( 10) != 0);
  printf("isalpha('a'): %d\n", isalpha('a') != 0);
  printf("isalpha('A'): %d\n", isalpha('A') != 0);
  printf("isalpha('5'): %d\n", isalpha('5') != 0);
  printf("isalpha('.'): %d\n", isalpha('.') != 0);
  printf("isalpha(183): %d\n", isalpha(183) != 0);

  printf("\n");
  printf("isdigit(-45): %d\n", isdigit(-45) != 0);
  printf("isdigit( 10): %d\n", isdigit( 10) != 0);
  printf("isdigit('a'): %d\n", isdigit('a') != 0);
  printf("isdigit('A'): %d\n", isdigit('A') != 0);
  printf("isdigit('5'): %d\n", isdigit('5') != 0);
  printf("isdigit('.'): %d\n", isdigit('.') != 0);
  printf("isdigit(183): %d\n", isdigit(183) != 0);

  printf("\n");
  printf("isxdigit(-45): %d\n", isxdigit(-45) != 0);
  printf("isxdigit( 10): %d\n", isxdigit( 10) != 0);
  printf("isxdigit('a'): %d\n", isxdigit('a') != 0);
  printf("isxdigit('A'): %d\n", isxdigit('A') != 0);
  printf("isxdigit('5'): %d\n", isxdigit('5') != 0);
  printf("isxdigit('.'): %d\n", isxdigit('.') != 0);
  printf("isxdigit(183): %d\n", isxdigit(183) != 0);

  printf("\n");
  printf("isalnum(-45): %d\n", isalnum(-45) != 0);
  printf("isalnum( 10): %d\n", isalnum( 10) != 0);
  printf("isalnum('a'): %d\n", isalnum('a') != 0);
  printf("isalnum('A'): %d\n", isalnum('A') != 0);
  printf("isalnum('5'): %d\n", isalnum('5') != 0);
  printf("isalnum('.'): %d\n", isalnum('.') != 0);
  printf("isalnum(183): %d\n", isalnum(183) != 0);

  printf("\n");
  printf("ispunct(-45): %d\n", ispunct(-45) != 0);
  printf("ispunct( 10): %d\n", ispunct( 10) != 0);
  printf("ispunct('a'): %d\n", ispunct('a') != 0);
  printf("ispunct('A'): %d\n", ispunct('A') != 0);
  printf("ispunct('5'): %d\n", ispunct('5') != 0);
  printf("ispunct('.'): %d\n", ispunct('.') != 0);
  printf("ispunct(183): %d\n", ispunct(183) != 0);

  printf("\n");
  printf("isspace(-45): %d\n", isspace(-45) != 0);
  printf("isspace( 10): %d\n", isspace( 10) != 0);
  printf("isspace('a'): %d\n", isspace('a') != 0);
  printf("isspace('A'): %d\n", isspace('A') != 0);
  printf("isspace('5'): %d\n", isspace('5') != 0);
  printf("isspace('.'): %d\n", isspace('.') != 0);
  printf("isspace(183): %d\n", isspace(183) != 0);

  printf("\n");
  printf("isblank(-45): %d\n", isblank(-45) != 0);
  printf("isblank( 10): %d\n", isblank( 10) != 0);
  printf("isblank('a'): %d\n", isblank('a') != 0);
  printf("isblank('A'): %d\n", isblank('A') != 0);
  printf("isblank('5'): %d\n", isblank('5') != 0);
  printf("isblank('.'): %d\n", isblank('.') != 0);
  printf("isblank(183): %d\n", isblank(183) != 0);

  printf("\n");
  printf("iscntrl(-45): %d\n", iscntrl(-45) != 0);
  printf("iscntrl( 10): %d\n", iscntrl( 10) != 0);
  printf("iscntrl('a'): %d\n", iscntrl('a') != 0);
  printf("iscntrl('A'): %d\n", iscntrl('A') != 0);
  printf("iscntrl('5'): %d\n", iscntrl('5') != 0);
  printf("iscntrl('.'): %d\n", iscntrl('.') != 0);
  printf("iscntrl(183): %d\n", iscntrl(183) != 0);

  printf("\n");
  printf("isprint(-45): %d\n", isprint(-45) != 0);
  printf("isprint( 10): %d\n", isprint( 10) != 0);
  printf("isprint('a'): %d\n", isprint('a') != 0);
  printf("isprint('A'): %d\n", isprint('A') != 0);
  printf("isprint('5'): %d\n", isprint('5') != 0);
  printf("isprint('.'): %d\n", isprint('.') != 0);
  printf("isprint(183): %d\n", isprint(183) != 0);

  printf("\n");
  printf("isgraph(-45): %d\n", isgraph(-45) != 0);
  printf("isgraph( 10): %d\n", isgraph( 10) != 0);
  printf("isgraph('a'): %d\n", isgraph('a') != 0);
  printf("isgraph('A'): %d\n", isgraph('A') != 0);
  printf("isgraph('5'): %d\n", isgraph('5') != 0);
  printf("isgraph('.'): %d\n", isgraph('.') != 0);
  printf("isgraph(183): %d\n", isgraph(183) != 0);

  return 0;
}
