void *global_ptr;

void f(void) {
  void *local_ptr = new short[21];
}

int main(int argc, char **argv) {
  global_ptr = new char[1337];
  global_ptr = 0;
  f();
  new long long[256];
}
