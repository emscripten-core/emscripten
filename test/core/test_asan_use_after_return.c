const char *__asan_default_options() {
  return "detect_stack_use_after_return=1";
}

__attribute__((noinline)) volatile int* f() {
  int val;
  return &val;
}

int main() {
  *f() = 1;
}
