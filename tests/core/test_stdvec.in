#include <vector>
#include <stdio.h>

struct S {
  int a;
  float b;
};

void foo(int a, float b) { printf("%d:%.2f\n", a, b); }

int main(int argc, char *argv[]) {
  std::vector<S> ar;
  S s;

  s.a = 789;
  s.b = 123.456f;
  ar.push_back(s);

  s.a = 0;
  s.b = 100.1f;
  ar.push_back(s);

  foo(ar[0].a, ar[0].b);
  foo(ar[1].a, ar[1].b);
}
