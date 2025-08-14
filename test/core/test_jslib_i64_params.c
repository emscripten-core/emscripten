#include <assert.h>
#include <emscripten.h>
#include <stdio.h>

#define MAX_SAFE_INTEGER (1ll<<53)
#define MIN_SAFE_INTEGER (-MAX_SAFE_INTEGER)

#define ERROR_VALUE 42

int64_t jscall(int64_t arg);

EMSCRIPTEN_KEEPALIVE
void called_from_js(uint64_t arg) {
  printf("called_from_js with: %lld\n", arg);
}

void check_ok(int64_t val) {
  printf("checking: %lld\n", val);
  int64_t rtn = jscall(val);
  int64_t expected = val/2;
  printf("got:      %lld\n", rtn);
  printf("expected: %lld\n", expected);
  assert(rtn == expected);
}

void check_invalid(int64_t val) {
  printf("checking: %lld\n", val);
  int64_t rtn = jscall(val);
  printf("got:      %lld\n", rtn);
  assert(rtn == ERROR_VALUE);
}

int main() {
  check_ok(0);
  check_ok(1);
  check_ok(-1);
  check_ok(42);
  check_ok(-42);
  check_ok(MAX_SAFE_INTEGER/2);
  check_ok(MIN_SAFE_INTEGER/2);
  check_ok(MAX_SAFE_INTEGER);
  check_ok(MIN_SAFE_INTEGER);
  check_invalid(MAX_SAFE_INTEGER + 1);
  check_invalid(MIN_SAFE_INTEGER - 1);
  printf("done\n");
  return 0;
}
