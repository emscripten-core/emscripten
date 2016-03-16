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


void test_1() {
  int a[] = {1, 2, 3};

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "a", 100)["int [3]"];
    test_assert("[0]", decoded["0"] == 1);
    test_assert("[1]", decoded["1"] == 2);
    test_assert("[2]", decoded["2"] == 3);
    test_assert("Size == 3", Object.keys(decoded).length == 3);
  }, &a);

  int b[3][5] =
    {
      { 1, 2, 3, 4, 5 },
      { 6, 7, 8, 9, 10 },
      { 11, 12, 13, 14, 15 }
    };

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "b", 100)["int [3][5]"];
    test_assert("[0:0]", decoded["0:0"] == 1);
    test_assert("[1:3]", decoded["1:3"] == 9);
    test_assert("[2:2]", decoded["2:2"] == 13);
    test_assert("Size == 3", Object.keys(decoded).length == 15);
  }, &b);
}

int main(int argc, char *argv[]) {
  EM_ASM(init_cd_test("test_arrays"));
  test_1();
}
