These files are from libcxxabi, release 8.0.0.

tag: llvmorg-8.0.0
git: d2298e74235598f15594fe2c99bbac870a507c59

Update Instructions
-------------------

Run system/lib/update_libcxxabi.py.

Local Modification
------------------

Local modifications are marked with the comment: 'XXX EMSCRIPTEN'

1. Add __cxa_can_catch and __cxa_is_pointer_type to private_typeinfo.cpp.

2. Duplicate __isOurExceptionClass in cxa_handlers.cpp since we don't compile
   cxa_exception.cpp.

3. Use arm-style ABI for __cxa_guard_acquire/__cxa_guard_release arguments
