/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef EMCC_NATIVE_EXEC_H
#define EMCC_NATIVE_EXEC_H

#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
inline int unsetenv(const char* name) {
  return _putenv_s(name, "");
}
#endif

namespace emscripten {

using std::string_view;

// Quote a command line argument for Windows _spawnvp / CreateProcess.
std::string quote_for_windows(string_view arg);

// Execute the specified binary with args, replacing the current process or
// exiting with the child's return code. Does not return.
[[noreturn]] void exec_process(const std::string& binary,
                               const std::vector<std::string>& args);

} // namespace emscripten

#endif // EMCC_NATIVE_EXEC_H
