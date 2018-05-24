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

struct TestBase {
  float a;
  char b;
  char *c;
  char *d;
  char **e;
  char ***f;
  short **g;

  // Test a reference
  int &h;
  TestBase(int& rt) : h(rt) {}
};

struct TinyStruct {
  short len;
  char * chars;
};

struct StructOfStructs {
  TestBase *tb;
  TinyStruct *ts;
};

void test_1() {
  int z = 42;
  TestBase tb(z);
  tb.a = 42.1f;
  tb.b = 'b';
  tb.c = nullptr;

  tb.d = static_cast<char *>(malloc(10));
  tb.d[0] = 'd';

  tb.e = static_cast<char**>(malloc(sizeof(char*)));
  tb.e[0] = static_cast<char*>(malloc(sizeof(char)));
  tb.e[0][0] = 'e';

  tb.f = static_cast<char***>(malloc(sizeof(char**)));
  tb.f[0] = static_cast<char**>(malloc(sizeof(char*)));
  tb.f[0][0] = static_cast<char*>(malloc(sizeof(char)));
  tb.f[0][0][0] = 'f';

  tb.g = static_cast<short**>(malloc(sizeof(short*)));
  tb.g[0] = static_cast<short*>(malloc(sizeof(short)));
  tb.g[0][0] = 12345;

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "tb", 100)["struct TestBase"];
    test_assert("Found the answer", decoded["float : a"] - 42 < 1.0);
    test_assert("Found 'b'", decoded["char : b"] == 98);
    test_assert("Found null", decoded["char * : c"] == "null");
    test_assert("Found 'd'", decoded["char * : d"] == 100);
    test_assert("Found 'e'", decoded["char * * : e"] == 101);
    test_assert("Found 'f'", decoded["char * * * : f"] == 102);
    test_assert("Found simple short", decoded["short * * : g"] == 12345);
    test_assert("Found the answer by reference", decoded["int & : h"] == 42);

  }, &tb);

}

void test_2() {
  StructOfStructs sos;
  int z = 50;
  sos.tb = new TestBase(z);
  sos.tb->a = 1337;

  sos.ts = static_cast<TinyStruct *>(malloc(sizeof(TinyStruct)));
  sos.ts->chars = "Hello in there";
  sos.ts->len = strlen(sos.ts->chars);

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "sos", 100)["struct StructOfStructs"];
    test_assert("Found the answer", decoded["struct TestBase * : tb"]["float : a"] == 1337);
  }, &sos);

}


int main(int argc, char *argv[]) {
  EM_ASM(init_cd_test("test_pointers"));

  test_1();
  test_2();
}
