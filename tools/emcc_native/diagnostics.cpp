/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "diagnostics.h"
#include "generated_settings.h"

#include <cstdlib>
#include <iostream>

namespace emscripten {

namespace {

// Warning state for diagnostics
bool g_warn_unused = true;
bool g_error_unused = false;

} // namespace

void parse_warning_flags(const std::vector<std::string>& user_args) {
  g_warn_unused = true;
  g_error_unused = false;
  for (std::string_view arg : user_args) {
    if (arg == "-w") {
      g_warn_unused = false;
    } else if (arg == "-Werror") {
      g_error_unused = true;
    } else if (arg == "-Wno-error") {
      g_error_unused = false;
    } else if (arg == "-Wunused-command-line-argument") {
      g_warn_unused = true;
    } else if (arg == "-Wno-unused-command-line-argument") {
      g_warn_unused = false;
    } else if (arg == "-Werror=unused-command-line-argument") {
      g_warn_unused = true;
      g_error_unused = true;
    } else if (arg == "-Wno-error=unused-command-line-argument") {
      g_error_unused = false;
    }
  }
}

void emit_unused_warning(std::string_view msg) {
  if (!g_warn_unused) {
    return;
  }
  if (g_error_unused) {
    std::cerr << "emcc: error: " << msg
              << " [-Wunused-command-line-argument] [-Werror]" << std::endl;
    std::exit(1);
  } else {
    std::cerr << "emcc: warning: " << msg << " [-Wunused-command-line-argument]"
              << std::endl;
  }
}

bool is_emscripten_only_warning(std::string_view arg) {
  if (!arg.starts_with("-W")) {
    return false;
  }
  std::string_view name = arg.substr(2);
  if (name.starts_with("error=")) {
    name = name.substr(6);
  } else if (name.starts_with("no-error=")) {
    name = name.substr(9);
  } else if (name.starts_with("no-")) {
    name = name.substr(3);
  }
  return EMSCRIPTEN_ONLY_WARNINGS.contains(name);
}

} // namespace emscripten
