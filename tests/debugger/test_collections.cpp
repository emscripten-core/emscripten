#include <vector>
#include <emscripten.h>

using namespace std;

struct TinyStruct {
  short len;
  double value;
  TinyStruct(short l, double v):len(l), value(v) {}
};

void test_1() {
  vector<int> v = {7, 5, 16, 8};
  vector<vector<int>> v2 = { {1, 2, 3}, {9, 8, 7, 6, 5}, {4, 4, 4, 4} };

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "v", 100);
    decoded = decoded["class vector<int, std::__1::allocator<int> >"];
    test_assert("[0]", decoded[0] == 7);
    test_assert("[1]", decoded[1] == 5);
    test_assert("[2]", decoded[2] == 16);
    test_assert("[3]", decoded[3] == 8);
  }, &v);

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "v2", 100);
    decoded = decoded["class vector<std::__1::vector<int, std::__1::allocator<int> >, std::__1::allocator<std::__1::vector<int, std::__1::allocator<int> > > >"];
    test_assert("len([0])", decoded[0].length == 3);
    test_assert("len([1])", decoded[1].length == 5);
    test_assert("len([2])", decoded[2].length == 4);
  }, &v2);

}

void test_2() {
  vector<TinyStruct> v = {TinyStruct(100, 100), TinyStruct(5, 345)};

  EM_ASM_INT({
    var decoded = Module['cyberdwarf'].decode_from_stack($0, "v", 100);
    decoded = decoded["class vector<TinyStruct, std::__1::allocator<TinyStruct> >"];
    test_assert("[0][len])", decoded[0]["short : len"] == 100);
    test_assert("[0][value])", decoded[0]["double : value"] == 100);
    test_assert("[1][len])", decoded[1]["short : len"] == 5);
    test_assert("[1][value])", decoded[1]["double : value"] == 345);
  }, &v);
}


int main(int argc, char *argv[]) {
  EM_ASM(init_cd_test("test_collections"));

  test_1();
  test_2();
}
