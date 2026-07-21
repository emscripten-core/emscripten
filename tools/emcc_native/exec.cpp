/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "exec.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace emscripten {

// Quotes a command line argument for Windows CreateProcess / CommandLineToArgvW.
//
// Arguments that are empty or contain spaces, tabs, or double quotes must be
// wrapped in double quotes. According to standard Windows command-line parsing
// rules (CommandLineToArgvW):
// - 2N backslashes followed by a double quote produce N literal backslashes and
//   a string quote delimiter (start/end of quote).
// - (2N + 1) backslashes followed by a double quote produce N literal
//   backslashes and a literal double quote character (").
// - Backslashes not followed by a double quote are literal and are not doubled.
std::string quote_for_windows(const std::string& arg) {
  if (!arg.empty() && arg.find_first_of(" \t\"") == std::string::npos) {
    return arg;
  }
  std::string quoted = "\"";
  for (size_t i = 0; i < arg.size(); ++i) {
    size_t num_backslashes = 0;
    while (i < arg.size() && arg[i] == '\\') {
      num_backslashes++;
      i++;
    }
    if (i == arg.size()) {
      quoted.append(num_backslashes * 2, '\\');
      break;
    }
    if (arg[i] == '\"') {
      quoted.append(num_backslashes * 2 + 1, '\\');
      quoted.push_back('\"');
    } else {
      quoted.append(num_backslashes, '\\');
      quoted.push_back(arg[i]);
    }
  }
  quoted += "\"";
  return quoted;
}

[[noreturn]] void exec_process(const std::string& binary,
                               const std::vector<std::string>& args) {
#ifdef _WIN32
  std::string cmdline = quote_for_windows(binary);
  for (const auto& arg : args) {
    cmdline += " " + quote_for_windows(arg);
  }

  int wlen = MultiByteToWideChar(CP_UTF8, 0, cmdline.c_str(), -1, nullptr, 0);
  if (wlen == 0) {
    std::cerr << "emcc_native: error converting command line to UTF-16" << std::endl;
    std::exit(1);
  }
  std::vector<wchar_t> wcmdline(wlen);
  MultiByteToWideChar(CP_UTF8, 0, cmdline.c_str(), -1, wcmdline.data(), wlen);

  STARTUPINFOW si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcessW(nullptr, wcmdline.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
    DWORD err = GetLastError();
    std::cerr << "emcc_native: error executing " << binary
              << " (CreateProcessW failed: " << err << ")" << std::endl;
    std::exit(1);
  }

  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD exit_code = 0;
  GetExitCodeProcess(pi.hProcess, &exit_code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  std::exit(static_cast<int>(exit_code));
#else
  std::vector<const char*> argv;
  argv.reserve(args.size() + 2);
  argv.push_back(binary.c_str());
  for (const auto& arg : args) {
    argv.push_back(arg.c_str());
  }
  argv.push_back(nullptr);

  execvp(binary.c_str(), const_cast<char* const*>(argv.data()));

  std::cerr << "emcc_native: error executing " << binary << ": "
            << std::strerror(errno) << std::endl;
  std::exit(1);
#endif
}

} // namespace emscripten
