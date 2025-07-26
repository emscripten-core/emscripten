// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#pragma once

#include <cstdlib>

#ifndef NDEBUG
// In debug builds show a message.
namespace wasmfs {
[[noreturn]] void
handle_unreachable(const char* msg, const char* file, unsigned line);
}
#define WASMFS_UNREACHABLE(msg)                                                \
  wasmfs::handle_unreachable(msg, __FILE__, __LINE__)
#else
// In release builds trap in a compact manner.
#define WASMFS_UNREACHABLE(msg) __builtin_trap()
#endif
