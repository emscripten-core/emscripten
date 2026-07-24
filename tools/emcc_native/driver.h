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

// Analyze command line arguments and decide whether to handle natively or fall
// back to Python.
DriverDecision analyze_request(bool is_cxx,
                               const fs::path& emscripten_root,
                               const std::vector<std::string>& user_args,
                               const Config& config);

} // namespace emscripten

#endif // EMCC_NATIVE_DRIVER_H
