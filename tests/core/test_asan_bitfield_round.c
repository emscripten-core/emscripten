struct A {
  char base;
  int : 8;
  int x : 8;
};

__attribute__ ((noinline)) int f(void *p) {
  return ((struct A *)p)->x;
}

int main(void) {
  char a = 0;
  return f(&a);
}
