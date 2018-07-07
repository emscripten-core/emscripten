//clang --target=wasm32-unknown-unknown-wasm \
// -nostartfiles -nostdlib -Wl,--no-entry,--export=foo -g -o foo.wasm no_main.c
int foo()
{
  return 42;
}
