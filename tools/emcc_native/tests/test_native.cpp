/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "config.h"
#include "driver.h"
#include "exec.h"

#undef NDEBUG
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace emscripten;

namespace {

void set_env(const std::string& name, const std::string& value) {
#ifdef _WIN32
  _putenv_s(name.c_str(), value.c_str());
#else
  setenv(name.c_str(), value.c_str(), 1);
#endif
}

void unset_env(const std::string& name) {
#ifdef _WIN32
  _putenv_s(name.c_str(), "");
#else
  unsetenv(name.c_str());
#endif
}

} // namespace

void test_config_parsing() {
  fs::path temp_dir = fs::temp_directory_path() / "emcc_native_test";
  fs::create_directories(temp_dir);

  fs::path config_file = temp_dir / ".emscripten";
  {
    std::ofstream out(config_file);
    out << "# Test config\n";
    out << "LLVM_ROOT = '/custom/llvm/bin' # trailing comment\n";
    out << "CACHE = '/custom/cache'\n";
  }

  set_env("EMSDK_PYTHON", "python3.11");

  assert(load_config(temp_dir).llvm_root == "/custom/llvm/bin");
  assert(load_config(temp_dir).em_cache == "/custom/cache");
  assert(get_python_executable() == "python3.11");

  // Test two-levels-up embedded config file lookup
  fs::path emsdk_root = temp_dir / "emsdk";
  fs::path nested_dir = emsdk_root / "upstream" / "emscripten";
  fs::create_directories(nested_dir);
  fs::path embedded_config = emsdk_root / ".emscripten";
  {
    std::ofstream out(embedded_config);
    out << "LLVM_ROOT = '/embedded/llvm/bin'\n";
  }
  assert(find_config_file(nested_dir) == embedded_config);
  assert(load_config(nested_dir).llvm_root == "/embedded/llvm/bin");

  // Test environment variable expansion (including $CFGDIR, $VAR, ${VAR})
  fs::path var_dir = temp_dir / "var_test";
  fs::create_directories(var_dir);
  fs::path var_config = var_dir / ".emscripten";
  set_env("TEST_LLVM_DIR", "/env_llvm");
  {
    std::ofstream out(var_config);
    out << "LLVM_ROOT = '$CFGDIR/bin'\n";
    out << "CACHE = '${TEST_LLVM_DIR}/cache'\n";
  }
  assert(load_config(var_dir).llvm_root == (var_dir / "bin").string());
  assert(load_config(var_dir).em_cache == "/env_llvm/cache");

  // Test Python expression evaluation (emsdk_path, os.path.dirname, os.path.abspath, +)
  fs::path py_dir = temp_dir / "py_test";
  fs::create_directories(py_dir);
  fs::path py_config = py_dir / ".emscripten";
  {
    std::ofstream out(py_config);
    out << "import os\n";
    out << "emsdk_path = os.path.dirname(os.path.abspath(__file__))\n";
    out << "LLVM_ROOT = emsdk_path + '/upstream/bin'\n";
    out << "CACHE = emsdk_path + '/cache'\n";
  }
  assert(load_config(py_dir).failure == true);
  assert(load_config(py_dir).failure_reason.find("Complex expression in config file") != std::string::npos);

  unset_env("EMSDK_PYTHON");

  fs::remove_all(temp_dir);
  std::cout << "[PASS] test_config_parsing" << std::endl;
}

std::string get_test_cache() {
  fs::path p = fs::temp_directory_path() / "emcc_native_test_cache";
  fs::create_directories(p / "sysroot");
  std::ofstream stamp(p / "sysroot_install.stamp");
  return p.string();
}

void test_driver_decision_compile_only() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {"-c", "hello.c", "-o", "hello.o"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);

  assert(!dec.use_fallback);
  assert(dec.target_args[0] == "-target");
  assert(dec.target_args[1] == "wasm32-unknown-emscripten");
  assert(dec.target_args[2] == "-fignore-exceptions");
  assert(dec.target_args[9] == "--sysroot=" + cfg.em_cache + "/sysroot");
  assert(dec.target_args[14] == "-c");
  assert(dec.target_args[15] == "hello.c");
  assert(dec.target_args[16] == "-o");
  assert(dec.target_args[17] == "hello.o");

  std::cout << "[PASS] test_driver_decision_compile_only" << std::endl;
}

void test_driver_decision_cxx_and_wasm64() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {"-c", "hello.cpp", "-o", "hello.o", "-m64"};
  DriverDecision dec = analyze_request(true, "/emsdk/emscripten", args, cfg);

  assert(!dec.use_fallback);
  assert(dec.target_args[0] == "-target");
  assert(dec.target_args[1] == "wasm64-unknown-emscripten");

  std::cout << "[PASS] test_driver_decision_cxx_and_wasm64" << std::endl;
}

void test_driver_decision_header_and_syntax_only() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args1 = {"-xc++-header", "header.h", "-o", "header.h.gch"};
  DriverDecision dec1 = analyze_request(true, "/emsdk/emscripten", args1, cfg);
  assert(!dec1.use_fallback);

  std::vector<std::string> args2 = {"header.hpp", "-o", "header.hpp.pch"};
  DriverDecision dec2 = analyze_request(true, "/emsdk/emscripten", args2, cfg);
  assert(!dec2.use_fallback);

  std::vector<std::string> args3 = {"-fsyntax-only", "test.cpp"};
  DriverDecision dec3 = analyze_request(true, "/emsdk/emscripten", args3, cfg);
  assert(!dec3.use_fallback);

  std::cout << "[PASS] test_driver_decision_header_and_syntax_only" << std::endl;
}

void test_driver_decision_fallback_link() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {"hello.o", "-o", "hello.js"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);

  assert(dec.use_fallback);
  assert(dec.target_binary == "python3");
  assert(dec.target_args[0] == "/emsdk/emscripten/emcc.py");

  std::cout << "[PASS] test_driver_decision_fallback_link" << std::endl;
}

void test_driver_decision_ignore_linker_flags_and_settings_during_compile() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {"-c",
                                   "hello.c",
                                   "-o",
                                   "hello.o",
                                   "--js-library",
                                   "lib.js",
                                   "--embed-file=file",
                                   "-sEXPORTED_FUNCTIONS=['_main']"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);

  assert(!dec.use_fallback);
  assert(dec.target_args[0] == "-target");
  assert(dec.target_args[1] == "wasm32-unknown-emscripten");
  assert(dec.target_args[14] == "-c");
  assert(dec.target_args[15] == "hello.c");
  assert(dec.target_args[16] == "-o");
  assert(dec.target_args[17] == "hello.o");

  std::cout
    << "[PASS] "
       "test_driver_decision_ignore_linker_flags_and_settings_during_compile"
    << std::endl;
}

void test_driver_decision_fallback_compile_time_setting() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {
    "-c", "hello.c", "-o", "hello.o", "-sDISABLE_EXCEPTION_CATCHING=0"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);

  assert(dec.use_fallback);
  assert(dec.target_binary == "python3");

  std::cout << "[PASS] test_driver_decision_fallback_compile_time_setting"
            << std::endl;
}

void test_driver_decision_wno_unused_command_line_argument() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {"-c",
                                   "hello.c",
                                   "-o",
                                   "hello.o",
                                   "--js-library",
                                   "lib.js",
                                   "-Wno-unused-command-line-argument"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);

  assert(!dec.use_fallback);
  assert(dec.target_args[14] == "-c");
  assert(dec.target_args[15] == "hello.c");
  assert(dec.target_args[16] == "-o");
  assert(dec.target_args[17] == "hello.o");
  assert(dec.target_args[18] == "-Wno-unused-command-line-argument");

  std::cout << "[PASS] test_driver_decision_wno_unused_command_line_argument"
            << std::endl;
}

void test_driver_decision_emcc_native_override() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  set_env("EMCC_NATIVE", "0");
  std::vector<std::string> args = {"-c", "hello.c", "-o", "hello.o"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(dec.use_fallback);
  assert(dec.reason == "EMCC_NATIVE set to disable native launcher");

  set_env("EMCC_NATIVE", "1");
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);

  unset_env("EMCC_NATIVE");

  std::cout << "[PASS] test_driver_decision_emcc_native_override" << std::endl;
}

void test_driver_decision_simd_flags() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {
    "-c", "hello.c", "-o", "hello.o", "-msimd128", "-msse2"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);

  assert(!dec.use_fallback);
  bool has_sse = false, has_sse2 = false;
  for (const auto& arg : dec.target_args) {
    if (arg == "-D__SSE__=1")
      has_sse = true;
    if (arg == "-D__SSE2__=1")
      has_sse2 = true;
  }
  assert(has_sse);
  assert(has_sse2);

  std::cout << "[PASS] test_driver_decision_simd_flags" << std::endl;
}

void test_driver_decision_pthread() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {
    "-c", "hello.c", "-o", "hello.o", "-pthread"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);

  assert(!dec.use_fallback);
  bool has_pthread = false;
  for (const auto& arg : dec.target_args) {
    if (arg == "-pthread")
      has_pthread = true;
  }
  assert(has_pthread);

  std::cout << "[PASS] test_driver_decision_pthread" << std::endl;
}

void test_driver_decision_emscripten_only_warning_flags() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {
    "-c", "hello.c", "-o", "hello.o", "-Wclosure", "-Wno-limited-postlink-optimizations", "-Werror"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);

  assert(!dec.use_fallback);
  bool has_closure = false, has_limited = false, has_error = false;
  for (const auto& arg : dec.target_args) {
    if (arg == "-Wclosure") has_closure = true;
    if (arg == "-Wno-limited-postlink-optimizations") has_limited = true;
    if (arg == "-Werror") has_error = true;
  }
  assert(!has_closure);
  assert(!has_limited);
  assert(has_error);

  std::cout << "[PASS] test_driver_decision_emscripten_only_warning_flags" << std::endl;
}

void test_driver_decision_cmdline_length_fallback() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {"-c", "hello.c", "-o", "hello.o"};
  std::string long_flag = "-I/path/to/very/long/include/directory/";
  for (int i = 0; i < 1500; ++i) {
    args.push_back(long_flag + std::to_string(i));
  }
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(dec.use_fallback);
  assert(dec.reason.find("Command line length") != std::string::npos);

  std::cout << "[PASS] test_driver_decision_cmdline_length_fallback" << std::endl;
}

void test_driver_decision_strict_setting() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {"-c", "hello.c", "-o", "hello.o", "-sSTRICT"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);

  args = {"-c", "hello.c", "-o", "hello.o", "-sSTRICT=1"};
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);

  args = {"-c", "hello.c", "-o", "hello.o", "-s", "STRICT"};
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);

  std::cout << "[PASS] test_driver_decision_strict_setting" << std::endl;
}

void test_driver_decision_lto() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {"-c", "hello.c", "-o", "hello.o", "-flto"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);
  bool has_flto = false;
  for (const auto& arg : dec.target_args) {
    if (arg == "-flto") has_flto = true;
  }
  assert(has_flto);

  args = {"-c", "hello.c", "-o", "hello.o", "-flto=thin"};
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);

  args = {"-c", "hello.c", "-o", "hello.o", "-fno-lto"};
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);

  args = {"-c", "hello.c", "-o", "hello.bc"};
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(dec.use_fallback);
  assert(dec.reason.find(
           ".bc output file suffix used without -flto or -emit-llvm") !=
         std::string::npos);

  args = {"-c", "hello.c", "-o", "hello.bc", "-flto"};
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);

  args = {"-c", "hello.c", "-o", "hello.bc", "-emit-llvm"};
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);

  std::cout << "[PASS] test_driver_decision_lto" << std::endl;
}

void test_driver_decision_response_files() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();

  std::vector<std::string> args = {"-c", "hello.c", "-o", "hello.o", "@args.rsp"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(dec.use_fallback);
  assert(dec.reason == "Response files (@file) not yet supported by native launcher");

  std::cout << "[PASS] test_driver_decision_response_files" << std::endl;
}

void test_driver_decision_config_failure() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  cfg.em_cache = get_test_cache();
  cfg.failure = true;
  cfg.failure_reason =
    "Complex expression in config file for LLVM_ROOT: emsdk_path + '/upstream/bin'";

  std::vector<std::string> args = {"-c", "hello.c"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(dec.use_fallback);
  assert(dec.reason.find("Complex expression in config file") != std::string::npos);

  std::cout << "[PASS] test_driver_decision_config_failure" << std::endl;
}

void test_quote_for_windows() {
  assert(quote_for_windows("hello") == "hello");
  assert(quote_for_windows("hello world") == "\"hello world\"");
  assert(quote_for_windows("foo\\bar") == "foo\\bar");
  assert(quote_for_windows("foo\\bar\\ baz") == "\"foo\\bar\\ baz\"");
  assert(quote_for_windows("foo\\bar\\") == "foo\\bar\\");
  assert(quote_for_windows("foo bar\\") == "\"foo bar\\\\\"");
  assert(quote_for_windows("foo \"bar\"") == "\"foo \\\"bar\\\"\"");
  assert(quote_for_windows("foo \\\"bar\"") == "\"foo \\\\\\\"bar\\\"\"");
  assert(quote_for_windows("") == "\"\"");
  std::cout << "[PASS] test_quote_for_windows" << std::endl;
}

void test_driver_decision_missing_sysroot() {
  Config cfg;
  cfg.llvm_root = "/emsdk/llvm/bin";
  fs::path p = fs::temp_directory_path() / "emcc_native_test_cache_missing";
  fs::remove_all(p);
  fs::create_directories(p);
  cfg.em_cache = p.string();

  std::vector<std::string> args = {"-c", "hello.c", "-o", "hello.o"};
  DriverDecision dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(dec.use_fallback);
  assert(dec.reason.find("Emscripten sysroot not installed in cache") != std::string::npos);

  fs::create_directories(p / "sysroot");
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(dec.use_fallback);
  assert(dec.reason.find("Emscripten sysroot not installed in cache") != std::string::npos);

  std::ofstream stamp(p / "sysroot_install.stamp");
  dec = analyze_request(false, "/emsdk/emscripten", args, cfg);
  assert(!dec.use_fallback);

  fs::remove_all(p);
  std::cout << "[PASS] test_driver_decision_missing_sysroot" << std::endl;
}

int main() {
  std::cout << "Running emcc-native unit tests..." << std::endl;
  test_config_parsing();
  test_quote_for_windows();
  test_driver_decision_compile_only();
  test_driver_decision_cxx_and_wasm64();
  test_driver_decision_header_and_syntax_only();
  test_driver_decision_fallback_link();
  test_driver_decision_ignore_linker_flags_and_settings_during_compile();
  test_driver_decision_fallback_compile_time_setting();
  test_driver_decision_strict_setting();
  test_driver_decision_lto();
  test_driver_decision_wno_unused_command_line_argument();
  test_driver_decision_simd_flags();
  test_driver_decision_pthread();
  test_driver_decision_emscripten_only_warning_flags();
  test_driver_decision_cmdline_length_fallback();
  test_driver_decision_emcc_native_override();
  test_driver_decision_response_files();
  test_driver_decision_config_failure();
  test_driver_decision_missing_sysroot();
  std::filesystem::remove_all(fs::temp_directory_path() / "emcc_native_test_cache");
  std::cout << "All emcc-native tests passed successfully!" << std::endl;
  return 0;
}
