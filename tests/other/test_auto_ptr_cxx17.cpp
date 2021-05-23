#include <cstdio>
#include <memory>

struct Foo {
  int x = 1;
};

int test_auto_ptr() {
  std::auto_ptr<Foo> p(new Foo);
  int ret = p.get()->x;
  p.release();
  return ret;
}

int main() {
  printf("c++17 std::auto_ptr: %d\n", test_auto_ptr());
  return 0;
}
