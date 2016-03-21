#include <vector>
#include <set>
#include <map>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emscripten.h>

union TestBase {
    char c;
    int i;
    float f;
};

void test_1() {
  TestBase tb;
  tb.f = 13.37;

  EM_ASM_INT({
    var decoded = Module['emdebugger_heap_printer'].decode_from_stack($0, "tb", 100)["union TestBase"];
    console.log(JSON.stringify(decoded, null, "\t"));
  }, &tb);

  tb.i = 1337;

  EM_ASM_INT({
    var decoded = Module['emdebugger_heap_printer'].decode_from_stack($0, "tb", 1)["union TestBase"];
    console.log(JSON.stringify(decoded, null, "\t"));
  }, &tb);

}

int main(int argc, char *argv[]) {
  EM_ASM(init_cd_test("test_union"));

  test_1();
}
