These files are from libc++, release branch 10.x.

tag: none
git: d2071b8fcdc5e8794d467cec9ebab735d09218bf

Update Instructions
-------------------

Run `system/lib/update_libcxx.py path/to/llvm-project`

Local Modification
------------------

Local modifications are marked with the comment: 'XXX EMSCRIPTEN'

1. Define _LIBCPP_OBJECT_FORMAT_ELF under __asmjs__ in libcxx/__config.

2. Define _LIBCPP_HAS_THREAD_API_PTHREAD in libcxx/__config./

3. Define _LIBCPP_ELAST in libcxx/include/config_elast.h

4. Set init_priority of __start_std_streams in libcxx/iostream.cpp
