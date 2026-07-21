/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "config.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace emscripten {

namespace {

std::string get_env(const char* name) {
  const char* val = std::getenv(name);
  return val ? std::string(val) : std::string();
}

std::string trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t\r\n");
  return str.substr(first, (last - first + 1));
}

std::string strip_quotes(const std::string& str) {
  std::string s = trim(str);
  if (s.size() >= 2 && ((s.front() == '\'' && s.back() == '\'') ||
                        (s.front() == '"' && s.back() == '"'))) {
    return s.substr(1, s.size() - 2);
  }
  return s;
}

void set_env_var(const std::string& key, const std::string& val) {
#ifdef _WIN32
  _putenv_s(key.c_str(), val.c_str());
#else
  setenv(key.c_str(), val.c_str(), 1);
#endif
}

std::string expand_user(const std::string& path) {
  if (path.empty())
    return path;
  if (path[0] == '~') {
    std::string home = get_env("HOME");
    if (home.empty()) {
      home = get_env("USERPROFILE");
    }
    if (!home.empty()) {
      return home + path.substr(1);
    }
  }
  return path;
}

std::string expand_vars(const std::string& input) {
  std::string s = expand_user(input);
  if (s.empty())
    return s;

  std::string result;
  size_t i = 0;
  while (i < s.size()) {
    if (s[i] == '$') {
      if (i + 1 < s.size() && s[i + 1] == '{') {
        size_t end = s.find('}', i + 2);
        if (end != std::string::npos) {
          std::string var_name = s.substr(i + 2, end - (i + 2));
          result += get_env(var_name.c_str());
          i = end + 1;
          continue;
        }
      } else {
        size_t start = i + 1;
        size_t end = start;
        while (end < s.size() &&
               (std::isalnum(static_cast<unsigned char>(s[end])) || s[end] == '_')) {
          ++end;
        }
        if (end > start) {
          std::string var_name = s.substr(start, end - start);
          result += get_env(var_name.c_str());
          i = end;
          continue;
        }
      }
    }
#ifdef _WIN32
    else if (s[i] == '%') {
      size_t end = s.find('%', i + 1);
      if (end != std::string::npos && end > i + 1) {
        std::string var_name = s.substr(i + 1, end - (i + 1));
        result += get_env(var_name.c_str());
        i = end + 1;
        continue;
      }
    }
#endif

    result += s[i];
    ++i;
  }

  return result;
}

} // namespace

// Search order for the config file (must match find_config_file() in tools/config.py):
// 1. Specified via EM_CONFIG environment variable
// 2. Local .emscripten file in emscripten_root (<root>/.emscripten)
// 3. Embedded config file two levels above emscripten_root, as used by
//    `emsdk --embedded` (<root>/../../.emscripten)
// 4. User home directory config (~/.emscripten)
fs::path find_config_file(const fs::path& emscripten_root) {
  std::string env_config = get_env("EM_CONFIG");
  if (!env_config.empty() && fs::exists(env_config)) {
    return fs::path(env_config);
  }

  fs::path root_config = emscripten_root / ".emscripten";
  if (fs::exists(root_config)) {
    return root_config;
  }

  // Look two levels up for emsdk --embedded compatibility
  // (e.g. emsdk/upstream/emscripten or emsdk/emscripten/x.y.z -> emsdk)
  fs::path emsdk_embedded_config =
    emscripten_root.parent_path().parent_path() / ".emscripten";
  if (fs::exists(emsdk_embedded_config)) {
    return emsdk_embedded_config;
  }

  std::string home = get_env("HOME");
  if (home.empty())
    home = get_env("USERPROFILE");
  if (!home.empty()) {
    fs::path home_config = fs::path(home) / ".emscripten";
    if (fs::exists(home_config)) {
      return home_config;
    }
  }

  return "";
}

Config load_config(const fs::path& emscripten_root) {
  Config config;

  fs::path config_file = find_config_file(emscripten_root);
  if (!config_file.empty() && fs::exists(config_file)) {
    set_env_var("CFGDIR", config_file.parent_path().string());
    std::ifstream in(config_file);
    std::string line;
    while (std::getline(in, line)) {
      size_t comment = line.find('#');
      if (comment != std::string::npos) {
        line = line.substr(0, comment);
      }
      std::string tline = trim(line);
      if (tline.empty())
        continue;

      size_t eq = tline.find('=');
      if (eq != std::string::npos) {
        std::string key = trim(tline.substr(0, eq));
        if (key == "LLVM_ROOT" || key == "CACHE") {
          std::string raw_val = trim(tline.substr(eq + 1));
          if (raw_val.find('+') != std::string::npos ||
              (!raw_val.empty() && raw_val.front() != '\'' &&
               raw_val.front() != '"' && raw_val.front() != '$')) {
            config.failure = true;
            config.failure_reason =
              "Complex expression in config file for " + key + ": " + raw_val;
            continue;
          }
          std::string val = expand_vars(strip_quotes(raw_val));
          if (key == "LLVM_ROOT") {
            config.llvm_root = val;
          } else if (key == "CACHE") {
            config.em_cache = val;
          }
        }
      }
    }
  }

  // Override with environment variables if present
  std::string env_llvm = get_env("EM_LLVM_ROOT");
  if (!env_llvm.empty()) {
    config.llvm_root = env_llvm;
  }

  std::string env_cache = get_env("EM_CACHE");
  if (!env_cache.empty()) {
    config.em_cache = env_cache;
  }

  // Apply defaults
  if (config.em_cache.empty()) {
    config.em_cache = (emscripten_root / "cache").string();
  }

  return config;
}

} // namespace emscripten
