int main(int argc, char **argv) {
  int k = 0x7fffffff;
  k += argc;
  #pragma clang loop unroll(disable)
  for (int i = 0; i < 100; ++i) {
    k = 0x7fffffff;
    k += argc;
  }
  return 0;
}
