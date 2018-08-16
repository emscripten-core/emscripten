// Library containing only single static constructor function.
// This is used to verify that the -Wl,-whole-archive works as expected

extern int foo;

void library_ctor() __attribute__ ((constructor));
void library_ctor() {
  foo = 42;
}
