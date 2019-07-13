struct A {
  char base;
  int : 4;
  int x : 7;
};

__attribute__ ((noinline)) int f(void *p) {
  return ((struct A *)p)->x;
}

int main(void) {
  char a = 0;
  return f(&a);
}
