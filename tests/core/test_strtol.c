/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void check_error() {
  if (errno) {
    if (errno == ERANGE)
      printf("ERR ERANGE\n");
    else
      printf("ERR %s\n", strerror(errno));
    errno = 0;
  }
}

void check(const char* input, int base) {
  char* endptr = NULL;
  long result = strtol(input, &endptr, base);
  printf("strtol(\"%s\", 0, %d) = %ld\n", input, base, result);
  printf("consumed %td bytes\n", endptr - input);
  check_error();
}

int main() {
  const char* test_values[] = {
    "-9223372036854775809",
    "-9223372036854775808",
    "-9223372036854775807",
    "-2147483649",
    "-2147483648",
    "-2147483647",
    "-5",
    "-1",
    "0",
    "1",
    "5",
    "2147483646",
    "2147483647",
    "2147483648",
    "4294967294",
    "4294967295",
    "4294967296",
    "18446744073709551614",
    "18446744073709551615",
    "18446744073709551616",
  };

  char* endptr;
  for (int i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++) {
    printf("strtol(\"%s\") = %ld\n", test_values[i], strtol(test_values[i], &endptr, 0));
    printf("consumed %td bytes\n", endptr - test_values[i]);
    check_error();
    printf("strtoll(\"%s\") = %lld\n", test_values[i], strtoll(test_values[i], &endptr, 0));
    printf("consumed %td bytes\n", endptr - test_values[i]);
    check_error();
    printf("strtoul(\"%s\") = %lu\n", test_values[i], strtoul(test_values[i], &endptr, 0));
    printf("consumed %td bytes\n", endptr - test_values[i]);
    check_error();
    printf("strtoull(\"%s\") = %llu\n", test_values[i], strtoull(test_values[i], &endptr, 0));
    printf("consumed %td bytes\n", endptr - test_values[i]);
    check_error();
    printf("\n");
  }

  check("0x12", 0);
  check("0x12", 10);
  check("012", 0);
  check("012", 10);
  check("0y12", 0);
  check("hello", 30);
  check("hello", 10);
  check("not-a-number", 0);
  check(" ", 0);
  check("-", 0);
  check("  0x12end", 0);

  return 0;
}
