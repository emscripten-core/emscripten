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


// Keep in mind that it is undefined behavior to read from the non active member of a union in C++
// Don't add tests that assign to 1 member but check another
void test_1() {
  TestBase tb;
  tb.c = 11;

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "tb", 100)["union TestBase"];
    test_assert("char == 11", decoded["char : c"] == 11);
  }, &tb);

  tb.i = 1337;

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "tb", 100)["union TestBase"];
    test_assert("int == 1337", decoded["int : i"] == 1337);
  }, &tb);

  tb.f = 10000;

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "tb", 100)["union TestBase"];
    test_assert("float == 10000", decoded["float : f"] == 10000);
  }, &tb);


}

int main(int argc, char *argv[]) {
  EM_ASM(init_cd_test("test_union"));

  test_1();
}
