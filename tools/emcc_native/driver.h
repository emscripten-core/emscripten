/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef EMCC_NATIVE_DRIVER_H
#define EMCC_NATIVE_DRIVER_H

#include "config.h"
#include <string>
#include <vector>

namespace emscripten {

struct DriverDecision {
  bool use_fallback = false;
  std::string target_binary;
  std::vector<std::string> target_args;
  std::string reason;
};

// Get the Python executable path (from EMSDK_PYTHON or default).
std::string get_python_executable();

DriverDecision analyze_request(std::string_view driver_arg0,
                               const fs::path& emscripten_root,
                               const fs::path& exe_path,
                               const std::vector<std::string>& user_args,
                               const Config& config);

// If _EMCC_CCACHE is set, unsets it and transforms target_binary to ccache.
void apply_ccache_wrapper(DriverDecision& decision);

} // namespace emscripten

#endif // EMCC_NATIVE_DRIVER_H
