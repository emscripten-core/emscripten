// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

extern "C" {
  int js_library_function();
}

int cpp_library_function()
{
	return js_library_function();
}
