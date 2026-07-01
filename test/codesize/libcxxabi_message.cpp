struct Foo {
  int counter = 5;
};

int main() {
  // libc++abi will emit some checks for proper static initialization, which
  // can call abort_message(). the point of this test is to see that getting
  // that code does not lead to including printf logging which libc++abi
  // does by default. And in general, that a tiny program like this should
  // end up a tiny wasm file, with no other surprises.
  static Foo foo;
  return ++foo.counter;
}
