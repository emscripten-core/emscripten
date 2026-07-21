/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "config.h"
#include "driver.h"
#include "exec.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <sys/param.h>
#else
#include <unistd.h>
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define UNREACHABLE() __assume(0)
#else
#define UNREACHABLE() __builtin_unreachable()
#endif

namespace fs = std::filesystem;

using namespace emscripten;

namespace {

template<typename... Args> void errlog(Args&&... args) {
  (std::cerr << ... << std::forward<Args>(args)) << std::endl;
}

bool is_cxx_driver_name(std::string_view exe_name) {
  return exe_name.find("em++") != std::string_view::npos;
}

fs::path get_self_executable_path() {
#if defined(_WIN32)
  std::vector<wchar_t> buf(MAX_PATH);
  DWORD len = GetModuleFileNameW(NULL, buf.data(), static_cast<DWORD>(buf.size()));
  while (len >= buf.size()) {
    buf.resize(buf.size() * 2);
    len = GetModuleFileNameW(NULL, buf.data(), static_cast<DWORD>(buf.size()));
  }
  if (len == 0) {
    errlog("emcc_native: error: GetModuleFileNameW failed");
    std::exit(1);
  }
  return fs::path(buf.data());
#elif defined(__linux__)
  std::error_code ec;
  fs::path proc_path = fs::read_symlink("/proc/self/exe", ec);
  if (ec || proc_path.empty()) {
    errlog("emcc_native: error: reading /proc/self/exe failed: ", ec.message());
    std::exit(1);
  }
  return proc_path;
#elif defined(__APPLE__)
  uint32_t size = 1024;
  std::vector<char> buf(size);
  if (_NSGetExecutablePath(buf.data(), &size) != 0) {
    buf.resize(size);
    if (_NSGetExecutablePath(buf.data(), &size) != 0) {
      errlog("emcc_native: error: _NSGetExecutablePath failed");
      std::exit(1);
    }
  }
  return fs::path(buf.data());
#else
#error "Unsupported platform for get_self_executable_path"
#endif
}

fs::path find_emscripten_root(const fs::path& exe_path) {
  fs::path p = fs::weakly_canonical(exe_path);

  // Executable is in <root>/bin, so root is one level up from bin (parent of
  // parent)
  if (p.has_parent_path() && p.parent_path().has_parent_path()) {
    fs::path root = p.parent_path().parent_path();
    if (fs::exists(root / "emcc.py")) {
      return root;
    }
  }

  return "";
}

void log_decision(const DriverDecision& decision) {
  if (decision.use_fallback) {
    errlog("emcc_native: falling back to python driver (", decision.reason, ")");
  } else {
    errlog("emcc_native: native launcher executing clang directly");
  }

  std::string full_cmd = decision.target_binary;
  for (const auto& arg : decision.target_args) {
    full_cmd += " " + quote_for_windows(arg);
  }

  errlog("emcc_native: exec: ", full_cmd);
}

} // namespace

int main(int argc, char** argv) {
  assert(argc >= 1);

  fs::path exe_path = get_self_executable_path();
  fs::path emscripten_root = find_emscripten_root(exe_path);
  if (emscripten_root.empty()) {
    errlog("emcc_native: error: could not locate Emscripten root directory "
           "(emcc.py not found relative to launcher binary at ",
           exe_path.generic_string(),
           ")");
    return 1;
  }
  Config config = load_config(emscripten_root);

  bool is_cxx = is_cxx_driver_name(argv[0]);

  std::vector<std::string> user_args;
  for (int i = 1; i < argc; ++i) {
    user_args.push_back(argv[i]);
  }

  auto decision = analyze_request(is_cxx, emscripten_root, user_args, config);

  const char* native_env = std::getenv("EMCC_NATIVE");
  if (decision.use_fallback && native_env && std::string(native_env) == "1") {
    errlog("emcc_native: error: falling back to python driver with EMCC_NATIVE=1 (",
           decision.reason,
           ")");
    return 1;
  }

  const char* emcc_debug = std::getenv("EMCC_DEBUG");
  if (emcc_debug && emcc_debug[0] != '\0') {
    log_decision(decision);
  }

  exec_process(decision.target_binary, decision.target_args);
  UNREACHABLE();
}
