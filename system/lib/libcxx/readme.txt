These files are from libc++, release 8.0.0.

tag: llvmorg-8.0.0
git: d2298e74235598f15594fe2c99bbac870a507c59

Update Instructions
-------------------

Run system/lib/update_libcxx.py.

Local Modification
------------------

Local modifications are marked with the comment: 'XXX EMSCRIPTEN'

1. Define _LIBCPP_OBJECT_FORMAT_ELF under __asmjs__ in libcxx/__config.

2. Define _LIBCPP_HAS_THREAD_API_PTHREAD in libcxx/__config./

3. Define _LIBCPP_ELAST in libcxx/include/config_elast.h
