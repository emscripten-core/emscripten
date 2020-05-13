struct Foo {
  int counter = 5;
};

int main() {
  // libcxxabi will emit some checks for proper static initialization, which
  // can call abort_message()
  static Foo foo;
  return ++foo.counter;
}
