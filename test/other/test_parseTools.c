#include <limits.h>
#include <stdio.h>
#include <inttypes.h>
#include <emscripten/emscripten.h>

void test_makeGetValue(int64_t* ptr);
void test_makeSetValue(int64_t* ptr);
int  test_receiveI64ParamAsI53(int64_t arg1, int64_t arg2);
int  test_receiveI64ParamAsDouble(int64_t arg1, int64_t arg2);

#define MAX_SAFE_INTEGER (1ll << 53)
#define MIN_SAFE_INTEGER (-MAX_SAFE_INTEGER)

EMSCRIPTEN_KEEPALIVE void printI64(int64_t* ptr) {
  printf("printI64: 0x%llx\n", *ptr);
}

EMSCRIPTEN_KEEPALIVE void clearI64(int64_t* ptr) {
  *ptr = 0;
}

int main() {
  int rtn;
  printf("MAX_SAFE_INTEGER: %lld\n", MAX_SAFE_INTEGER);
  printf("MIN_SAFE_INTEGER: %lld\n", MIN_SAFE_INTEGER);

  int64_t num = -0x0000aabb12345678ll;
  printI64(&num);
  test_makeGetValue(&num);

  test_makeSetValue(&num);

  rtn = test_receiveI64ParamAsI53(42, -42);
  printf("rtn = %d\n", rtn);

  rtn = test_receiveI64ParamAsI53(MAX_SAFE_INTEGER, MIN_SAFE_INTEGER);
  printf("rtn = %d\n", rtn);

  rtn = test_receiveI64ParamAsI53(MAX_SAFE_INTEGER + 1, 0);
  printf("rtn = %d\n", rtn);

  rtn = test_receiveI64ParamAsI53(MIN_SAFE_INTEGER - 1, 0);
  printf("rtn = %d\n", rtn);

  rtn = test_receiveI64ParamAsDouble(42, -42);
  printf("rtn = %d\n", rtn);

  rtn = test_receiveI64ParamAsDouble(MAX_SAFE_INTEGER, MIN_SAFE_INTEGER);
  printf("rtn = %d\n", rtn);

  rtn = test_receiveI64ParamAsDouble(MAX_SAFE_INTEGER + 1, 0);
  printf("rtn = %d\n", rtn);

  rtn = test_receiveI64ParamAsDouble(MIN_SAFE_INTEGER - 1, 0);
  printf("rtn = %d\n", rtn);

  printf("\ndone\n");
  return 0;
}
