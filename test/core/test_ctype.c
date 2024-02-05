/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _BSD_SOURCE
#include <stdio.h>
#include <ctype.h>

int main() {
  printf("%c%c%c\n", tolower('a'), tolower('B'), tolower('1'));
  printf("%c%c%c\n", toupper('a'), toupper('B'), toupper('1'));

  /* These appear to require glibc headers and fail with newlib
  const int* tl = *__ctype_tolower_loc();
  printf("%c%c%c\n", (char)tl['c'], (char)tl['D'], (char)tl['2']);
  const int* tu = *__ctype_toupper_loc();
  printf("%c%c%c\n", (char)tu['c'], (char)tu['D'], (char)tu['2']);
  */

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
  const char* funcNames[] = {
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

  printf("\n");
  printf("isascii(-45): %d\n", isascii(-45) != 0);
  printf("isascii( 10): %d\n", isascii( 10) != 0);
  printf("isascii('a'): %d\n", isascii('a') != 0);
  printf("isascii('A'): %d\n", isascii('A') != 0);
  printf("isascii('5'): %d\n", isascii('5') != 0);
  printf("isascii('.'): %d\n", isascii('.') != 0);
  printf("isascii(183): %d\n", isascii(183) != 0);

  // These are manually preprocessed to use __ctype_b_loc. The flags combination
  // that the test runner uses seems to cause the macro to be ignored.
  printf("\n");
  printf("islower(-45): %d\n", islower(-45));
  printf("islower( 10): %d\n", islower(10));
  printf("islower('a'): %d\n", islower('a'));
  printf("islower('A'): %d\n", islower('A'));
  printf("islower('5'): %d\n", islower('5'));
  printf("islower('.'): %d\n", islower('.'));
  printf("islower(183): %d\n", islower(183));

  printf("\n");
  printf("isupper(-45): %d\n", isupper(-45));
  printf("isupper( 10): %d\n", isupper(10));
  printf("isupper('a'): %d\n", isupper('a'));
  printf("isupper('A'): %d\n", isupper('A'));
  printf("isupper('5'): %d\n", isupper('5'));
  printf("isupper('.'): %d\n", isupper('.'));
  printf("isupper(183): %d\n", isupper(183));

  printf("\n");
  printf("isupper(-45): %d\n", isupper(-45));
  printf("isupper( 10): %d\n", isupper(10));
  printf("isupper('a'): %d\n", isupper('a'));
  printf("isupper('A'): %d\n", isupper('A'));
  printf("isupper('5'): %d\n", isupper('5'));
  printf("isupper('.'): %d\n", isupper('.'));
  printf("isupper(183): %d\n", isupper(183));

  printf("\n");
  printf("isalpha(-45): %d\n", isalpha(-45));
  printf("isalpha( 10): %d\n", isalpha(10));
  printf("isalpha('a'): %d\n", isalpha('a'));
  printf("isalpha('A'): %d\n", isalpha('A'));
  printf("isalpha('5'): %d\n", isalpha('5'));
  printf("isalpha('.'): %d\n", isalpha('.'));
  printf("isalpha(183): %d\n", isalpha(183));

  printf("\n");
  printf("isdigit(-45): %d\n", isdigit(-45));
  printf("isdigit( 10): %d\n", isdigit(10));
  printf("isdigit('a'): %d\n", isdigit('a'));
  printf("isdigit('A'): %d\n", isdigit('A'));
  printf("isdigit('5'): %d\n", isdigit('5'));
  printf("isdigit('.'): %d\n", isdigit('.'));
  printf("isdigit(183): %d\n", isdigit(183));

  printf("\n");
  printf("isxdigit(-45): %d\n", isxdigit(-45));
  printf("isxdigit( 10): %d\n", isxdigit(10));
  printf("isxdigit('a'): %d\n", isxdigit('a'));
  printf("isxdigit('A'): %d\n", isxdigit('A'));
  printf("isxdigit('5'): %d\n", isxdigit('5'));
  printf("isxdigit('.'): %d\n", isxdigit('.'));
  printf("isxdigit(183): %d\n", isxdigit(183));

  printf("\n");
  printf("isalnum(-45): %d\n", isalnum(-45));
  printf("isalnum( 10): %d\n", isalnum(10));
  printf("isalnum('a'): %d\n", isalnum('a'));
  printf("isalnum('A'): %d\n", isalnum('A'));
  printf("isalnum('5'): %d\n", isalnum('5'));
  printf("isalnum('.'): %d\n", isalnum('.'));
  printf("isalnum(183): %d\n", isalnum(183));

  printf("\n");
  printf("ispunct(-45): %d\n", ispunct(-45));
  printf("ispunct( 10): %d\n", ispunct(10));
  printf("ispunct('a'): %d\n", ispunct('a'));
  printf("ispunct('A'): %d\n", ispunct('A'));
  printf("ispunct('5'): %d\n", ispunct('5'));
  printf("ispunct('.'): %d\n", ispunct('.'));
  printf("ispunct(183): %d\n", ispunct(183));

  printf("\n");
  printf("isspace(-45): %d\n", isspace(-45));
  printf("isspace( 10): %d\n", isspace(10));
  printf("isspace('a'): %d\n", isspace('a'));
  printf("isspace('A'): %d\n", isspace('A'));
  printf("isspace('5'): %d\n", isspace('5'));
  printf("isspace('.'): %d\n", isspace('.'));
  printf("isspace(183): %d\n", isspace(183));

  printf("\n");
  printf("isblank(-45): %d\n", isblank(-45));
  printf("isblank( 10): %d\n", isblank(10));
  printf("isblank('a'): %d\n", isblank('a'));
  printf("isblank('A'): %d\n", isblank('A'));
  printf("isblank('5'): %d\n", isblank('5'));
  printf("isblank('.'): %d\n", isblank('.'));
  printf("isblank(183): %d\n", isblank(183));

  printf("\n");
  printf("iscntrl(-45): %d\n", iscntrl(-45));
  printf("iscntrl( 10): %d\n", iscntrl(10));
  printf("iscntrl('a'): %d\n", iscntrl('a'));
  printf("iscntrl('A'): %d\n", iscntrl('A'));
  printf("iscntrl('5'): %d\n", iscntrl('5'));
  printf("iscntrl('.'): %d\n", iscntrl('.'));
  printf("iscntrl(183): %d\n", iscntrl(183));

  printf("\n");
  printf("isprint(-45): %d\n", isprint(-45));
  printf("isprint( 10): %d\n", isprint(10));
  printf("isprint('a'): %d\n", isprint('a'));
  printf("isprint('A'): %d\n", isprint('A'));
  printf("isprint('5'): %d\n", isprint('5'));
  printf("isprint('.'): %d\n", isprint('.'));
  printf("isprint(183): %d\n", isprint(183));

  printf("\n");
  printf("isgraph(-45): %d\n", isgraph(-45));
  printf("isgraph( 10): %d\n", isgraph(10));
  printf("isgraph('a'): %d\n", isgraph('a'));
  printf("isgraph('A'): %d\n", isgraph('A'));
  printf("isgraph('5'): %d\n", isgraph('5'));
  printf("isgraph('.'): %d\n", isgraph('.'));
  printf("isgraph(183): %d\n", isgraph(183));
 
  return 0;
}
