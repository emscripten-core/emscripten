These files are from compiler-rt.

Last Changed Rev: 351636
Last Changed Date: 2019-01-19

===========================================================================

* `ubsan_minimal_handlers.cpp` -- changed to use
  `emscripten_return_address` as `__builtin_return_address` is not yet
  available in clang target `wasm-unknown-emscripten`.
* `sanitizer_atomic.h` -- based on `sanitizer_atomic.h`,
  `sanitizer_atomic_clang.h` and `sanitizer_atomic_clang_other.h` in
  upstream, combined into one to avoid pulling in and porting the entire
  `sanitizer_common` directory from upstream.
