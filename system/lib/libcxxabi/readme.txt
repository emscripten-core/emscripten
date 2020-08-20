These files are from libc++, release branch 10.x.

tag: llvmorg-10.0.0
git: d32170dbd5b0d54436537b6b75beaf44324e0c28

Update Instructions
-------------------

Run `system/lib/update_libcxxabi.py path/to/llvm-root`

Local Modification
------------------

Local modifications are marked with the comment: 'XXX EMSCRIPTEN'

1. Add __cxa_can_catch and __cxa_is_pointer_type to private_typeinfo.cpp.

2. Duplicate __isOurExceptionClass in cxa_handlers.cpp since we don't compile
   cxa_exception.cpp in Emscripten EH mode.

The following changes are not marked with 'XXX EMSCRIPTEN'.

3. Replace throw() with _NOEXCEPT macro defined in libcxx/include/__config.

4. Wasm exception handling support code is added and guarded by
   '#ifdef __USING_WASM_EXCEPTIONS__'.
