// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

extern "C" {
  void lib_function();
  void lib_function2();
}

int cpp_library_function();

int main()
{
  lib_function();
  lib_function2();
  cpp_library_function();
}
