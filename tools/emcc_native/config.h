/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef EMCC_NATIVE_CONFIG_H
#define EMCC_NATIVE_CONFIG_H

#include <filesystem>
#include <string>
#include <unordered_map>

namespace emscripten {

namespace fs = std::filesystem;

struct Config {
  // There are more possible settings in an emscripten config
  // but the native launcher only cares about these two.
  std::string llvm_root;
  std::string em_cache;
  bool failure = false;
  std::string failure_reason;
};

// Find the config file (.emscripten) location.
fs::path find_config_file(const fs::path& emscripten_root);

// Parse configuration file and environment variables.
Config load_config(const fs::path& emscripten_root);

} // namespace emscripten

#endif // EMCC_NATIVE_CONFIG_H
