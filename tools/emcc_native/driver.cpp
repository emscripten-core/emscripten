/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "diagnostics.h"
#include "driver.h"
#include "exec.h"
#include "generated_settings.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace fs = std::filesystem;

namespace emscripten {

using std::string_view;

namespace {

constexpr string_view get_extension(string_view path) {
  size_t idx = path.rfind('.');
  if (idx == string_view::npos || idx == 0) {
    return {};
  }
  size_t slash = path.find_last_of("/\\");
  if (slash != string_view::npos && slash > idx) {
    return {};
  }
  return path.substr(idx);
}

// Flags that require fallback to the Python driver because they represent
// complex operations or standing system commands.
// NOTE: Keep in sync with system options in tools/cmdline.py and complex
// options in tools/compile.py.
const std::unordered_set<std::string_view> COMPLEX_OR_SYSTEM_FLAGS = {
  "--clear-cache",
  "--clear-ports",
  "--build",
  "--generate-config",
  "--compiler-wrapper",
  "--tracing",
  "--memoryprofiler",
  "--em-config",
};

// Subset of LINK_ONLY_FLAGS that take a value argument when passed without '='.
const std::unordered_set<std::string_view> LINK_ONLY_FLAGS_WITH_ARGS = {
  "--closure",
  "--embed-file",
  "--exclude-file",
  "--extern-post-js",
  "--extern-pre-js",
  "--js-library",
  "--js-transform",
  "--oformat",
  "--output-eol",
  "--output_eol",
  "--post-js",
  "--pre-js",
  "--preload-file",
  "--shell-file",
  "--source-map-base",
};

// Default LLVM backend arguments injected during compilation.
// NOTE: Keep in sync with llvm_backend_args() in tools/building.py.
const std::vector<std::string> DEFAULT_LLVM_BACKEND_FLAGS = {
  "-mllvm",
  "-combiner-global-alias-analysis=false",
  "-mllvm",
  "-enable-emscripten-sjlj",
  "-mllvm",
  "-disable-lsr",
};

void create_python_command(const fs::path& script_path,
                           const std::vector<std::string>& user_args,
                           DriverDecision& decision) {
  decision.target_binary = get_python_executable();
  decision.target_args.push_back("-E");
#ifdef _WIN32
  decision.target_args.push_back("-X");
  decision.target_args.push_back("utf8");
#endif
  decision.target_args.push_back(script_path.generic_string());
  for (const auto& arg : user_args) {
    decision.target_args.push_back(arg);
  }
}

// Process SIMD/SSE/NEON feature flags and inject corresponding macro
// definitions. NOTE: Keep in sync with get_cflags() in tools/compile.py and
// SIMD_INTEL_FEATURE_TOWER / SIMD_NEON_FLAGS in tools/cmdline.py.
void handle_simd_flags(const std::vector<string_view>& filtered_user_args,
                       DriverDecision& decision) {
  bool has_simd = false;
  bool has_sse = false, has_sse2 = false, has_sse3 = false, has_ssse3 = false;
  bool has_sse4_1 = false, has_sse4_2 = false, has_avx = false,
       has_avx2 = false;
  bool has_fma = false, has_neon = false;
  bool has_intel_simd = false;

  for (string_view arg : filtered_user_args) {
    if (arg == "-msimd128" || arg == "-mrelaxed-simd") {
      has_simd = true;
    } else if (arg == "-msse") {
      has_sse = true;
      has_intel_simd = true;
    } else if (arg == "-msse2") {
      has_sse = has_sse2 = true;
      has_intel_simd = true;
    } else if (arg == "-msse3") {
      has_sse = has_sse2 = has_sse3 = true;
      has_intel_simd = true;
    } else if (arg == "-mssse3") {
      has_sse = has_sse2 = has_sse3 = has_ssse3 = true;
      has_intel_simd = true;
    } else if (arg == "-msse4.1") {
      has_sse = has_sse2 = has_sse3 = has_ssse3 = has_sse4_1 = true;
      has_intel_simd = true;
    } else if (arg == "-msse4.2" || arg == "-msse4") {
      has_sse = has_sse2 = has_sse3 = has_ssse3 = has_sse4_1 = has_sse4_2 =
        true;
      has_intel_simd = true;
    } else if (arg == "-mavx") {
      has_sse = has_sse2 = has_sse3 = has_ssse3 = has_sse4_1 = has_sse4_2 =
        has_avx = true;
      has_intel_simd = true;
    } else if (arg == "-mavx2") {
      has_sse = has_sse2 = has_sse3 = has_ssse3 = has_sse4_1 = has_sse4_2 =
        has_avx = has_avx2 = true;
      has_intel_simd = true;
    } else if (arg == "-mfma") {
      has_sse = has_sse2 = has_sse3 = has_ssse3 = has_sse4_1 = has_sse4_2 =
        has_avx = has_avx2 = has_fma = true;
      has_intel_simd = true;
    } else if (arg == "-mfpu=neon" || arg == "-mneon") {
      has_neon = true;
    }
  }

  if ((has_intel_simd || has_neon) && !has_simd) {
    std::cerr << "emcc: error: passing any of -msse, -msse2, -msse3, -mssse3, "
                 "-msse4.1, -msse4.2, -msse4, -mavx, -mavx2, -mfma, -mfpu=neon "
                 "flags also requires passing -msimd128 (or -mrelaxed-simd)!"
              << std::endl;
    std::exit(1);
  }

  if (has_sse || has_neon) decision.target_args.push_back("-D__SSE__=1");
  if (has_sse2) decision.target_args.push_back("-D__SSE2__=1");
  if (has_sse3) decision.target_args.push_back("-D__SSE3__=1");
  if (has_ssse3) decision.target_args.push_back("-D__SSSE3__=1");
  if (has_sse4_1) decision.target_args.push_back("-D__SSE4_1__=1");
  if (has_sse4_2) decision.target_args.push_back("-D__SSE4_2__=1");
  if (has_avx) decision.target_args.push_back("-D__AVX__=1");
  if (has_avx2) decision.target_args.push_back("-D__AVX2__=1");
  if (has_fma) decision.target_args.push_back("-D__FMA__=1");
  if (has_neon) decision.target_args.push_back("-D__ARM_NEON__=1");
}

// Construct the native Clang/Clang++ binary path and compiler argument vector.
// NOTE: Keep in sync with get_clang_flags() and get_cflags() in
// tools/compile.py.
void create_clang_command(bool is_cxx,
                          bool is_wasm64,
                          bool is_asm_only,
                          const std::vector<string_view>& filtered_user_args,
                          const Config& config,
                          DriverDecision& decision) {
  std::string clang_name = is_cxx ? "clang++" : "clang";
#ifdef _WIN32
  clang_name += ".exe";
#endif

  if (!config.llvm_root.empty()) {
    decision.target_binary = (fs::path(config.llvm_root) / clang_name).generic_string();
  } else {
    decision.target_binary = clang_name;
  }

  // Target flags
  std::string target_triple =
    is_wasm64 ? "wasm64-unknown-emscripten" : "wasm32-unknown-emscripten";
  decision.target_args.push_back("-target");
  decision.target_args.push_back(target_triple);

  if (!is_asm_only) {
    // Frontend exceptions flag
    bool has_exceptions = false;
    for (string_view arg : filtered_user_args) {
      if (arg == "-fexceptions" || arg == "-fwasm-exceptions" ||
          arg == "-fno-ignore-exceptions") {
        has_exceptions = true;
        break;
      }
    }
    if (!has_exceptions) {
      decision.target_args.push_back("-fignore-exceptions");
    } else {
      decision.target_args.push_back("-mllvm");
      decision.target_args.push_back("-enable-emscripten-cxx-exceptions");
    }

    // Backend flags
    for (const auto& flag : DEFAULT_LLVM_BACKEND_FLAGS) {
      decision.target_args.push_back(flag);
    }

    // Sysroot
    fs::path sysroot = fs::path(config.em_cache) / "sysroot";
    decision.target_args.push_back("--sysroot=" + sysroot.generic_string());

    // Handle SIMD flags
    handle_simd_flags(filtered_user_args, decision);

    // Check user args for special flags
    bool nostdinc = false;
    bool has_fpic = false;
    bool has_fvisibility = false;
    bool has_pthread = false;
    bool has_explicit_pthread = false;

    for (string_view arg : filtered_user_args) {
      if (arg == "-nostdinc") {
        nostdinc = true;
      } else if (arg == "-fPIC") {
        has_fpic = true;
      } else if (arg.starts_with("-fvisibility")) {
        has_fvisibility = true;
      } else if (arg == "-pthread") {
        has_pthread = true;
        has_explicit_pthread = true;
      } else if (arg == "-fopenmp" || arg == "-fopenmp=libomp") {
        has_pthread = true;
      }
    }

    if (has_pthread) {
      decision.target_args.push_back("-D__EMSCRIPTEN_SHARED_MEMORY__=1");
      if (!has_explicit_pthread) {
        decision.target_args.push_back("-pthread");
      }
    }

    if (has_fpic && !has_fvisibility) {
      decision.target_args.push_back("-fvisibility=default");
    }

    if (!nostdinc) {
      decision.target_args.push_back("-Xclang");
      decision.target_args.push_back("-iwithsysroot/include/fakesdl");
      decision.target_args.push_back("-Xclang");
      decision.target_args.push_back("-iwithsysroot/include/compat");
    }
  }

  for (string_view arg : filtered_user_args) {
    decision.target_args.push_back(std::string(arg));
  }
}

bool is_upper_identifier(string_view s) {
  if (s.empty() || std::isdigit(static_cast<unsigned char>(s[0]))) {
    return false;
  }
  bool has_upper = false;
  for (char c : s) {
    unsigned char uc = static_cast<unsigned char>(c);
    if (std::isupper(uc)) {
      has_upper = true;
    } else if (!std::isdigit(uc) && uc != '_') {
      return false;
    }
  }
  return has_upper;
}

bool is_dash_s_setting(const std::vector<std::string>& user_args,
                       size_t i,
                       string_view& setting_key,
                       bool& ate_next) {
  string_view arg = user_args[i];
  ate_next = false;
  string_view val;
  if (arg == "-s") {
    if (i + 1 >= user_args.size())
      return false;
    val = user_args[i + 1];
    ate_next = true;
  } else if (arg.starts_with("-s")) {
    val = arg.substr(2);
  } else {
    return false;
  }

  size_t eq = val.find('=');
  if (eq != string_view::npos) {
    setting_key = val.substr(0, eq);
  } else {
    setting_key = val;
  }
  return is_upper_identifier(setting_key);
}

bool is_assembly_only(const std::vector<std::string>& user_args) {
  static const std::unordered_set<string_view> ASM_EXTS = {".s", ".S"};
  static const std::unordered_set<string_view> C_EXTS = {
    ".c", ".i", ".cppm", ".pcm", ".cpp", ".cxx", ".cc", ".c++",
    ".CPP", ".CXX", ".C", ".CC", ".C++", ".ii", ".m", ".mi", ".mm", ".mii",
    ".bc", ".ll"
  };

  bool has_asm = false;
  bool has_c_source = false;

  for (size_t i = 0; i < user_args.size(); ++i) {
    const std::string& arg = user_args[i];
    if (arg.empty() || arg[0] == '-') {
      if ((arg == "-o" || arg == "-I" || arg == "-L" || arg == "-include" ||
           arg == "-isystem" || arg == "-MF" || arg == "-MT" || arg == "-MQ" ||
           arg == "-x") && i + 1 < user_args.size()) {
        ++i;
      }
      continue;
    }
    string_view ext = get_extension(arg);
    if (ASM_EXTS.contains(ext)) {
      has_asm = true;
    } else if (C_EXTS.contains(ext)) {
      has_c_source = true;
    }
  }

  return has_asm && !has_c_source;
}

// Check if any input argument is a header file (via extension) or if an explicit
// header language flag (e.g. -xc++-header) is specified. Compiling header inputs
// is a compile-only operation that generates precompiled headers (.pch / .gch).
// NOTE: Keep in sync with HEADER_EXTENSIONS and phase_setup() in emcc.py.
bool has_header_inputs(const std::vector<std::string>& user_args) {
  static const std::unordered_set<std::string_view> HEADER_EXTS = {
    ".h", ".hxx", ".hpp", ".hh", ".H", ".HXX", ".HPP", ".HH"
  };

  for (size_t i = 0; i < user_args.size(); ++i) {
    const std::string& arg = user_args[i];
    if (arg.empty()) {
      continue;
    }

    if (arg == "-x") {
      if (i + 1 < user_args.size() && user_args[i + 1].find("header") != std::string::npos) {
        return true;
      }
      if (i + 1 < user_args.size()) {
        ++i;
      }
      continue;
    }
    if (arg.starts_with("-x") && arg.find("header") != std::string::npos) {
      return true;
    }

    if (arg[0] == '-') {
      if ((arg == "-o" || arg == "-I" || arg == "-L" || arg == "-include" ||
           arg == "-isystem" || arg == "-MF" || arg == "-MT" || arg == "-MQ") &&
          i + 1 < user_args.size()) {
        ++i;
      }
      continue;
    }

    std::string_view ext = get_extension(arg);
    if (HEADER_EXTS.contains(ext)) {
      return true;
    }
  }

  return false;
}

std::string get_tool_name(std::string_view driver_arg0) {
  if (driver_arg0.empty()) {
    std::cerr << "emcc_native: error: empty command name (argv[0])" << std::endl;
    std::exit(1);
  }
  std::string tool = fs::path(driver_arg0).stem().string();
  if (tool.empty()) {
    std::cerr << "emcc_native: error: unable to determine tool name from command '"
              << driver_arg0 << "'" << std::endl;
    std::exit(1);
  }
  return tool;
}

fs::path find_script_for_tool(const fs::path& emscripten_root,
                              const fs::path& exe_path,
                              string_view tool) {
  static const char* const SEARCH_SUBDIRS[] = {"", "tools", "test"};

  for (const char* subdir : SEARCH_SUBDIRS) {
    fs::path candidate = emscripten_root / subdir / (std::string(tool) + ".py");
    if (fs::exists(candidate)) {
      return candidate;
    }
  }

  if (exe_path.has_parent_path()) {
    fs::path exe_dir = exe_path.parent_path();
    for (const char* subdir : SEARCH_SUBDIRS) {
      fs::path candidate = exe_dir / subdir / (std::string(tool) + ".py");
      if (fs::exists(candidate)) {
        return candidate;
      }
    }
  }

  std::cerr << "emcc_native: error: python script for tool '" << tool << "' not found" << std::endl;
  std::exit(1);
}

DriverDecision make_fallback_decision(string_view tool,
                                      const fs::path& emscripten_root,
                                      const fs::path& exe_path,
                                      const std::vector<std::string>& user_args,
                                      std::string reason) {
  fs::path script_path = find_script_for_tool(emscripten_root, exe_path, tool);

  DriverDecision decision;
  decision.use_fallback = true;
  decision.reason = std::move(reason);
  create_python_command(script_path, user_args, decision);
  return decision;
}

std::optional<std::string> check_system_environment(const Config& config) {
  const char* native_env = std::getenv("EMCC_NATIVE");
  if (native_env && std::string(native_env) == "0") {
    return "EMCC_NATIVE set to disable native launcher";
  }

  const char* compiler_wrapper = std::getenv("EM_COMPILER_WRAPPER");
  if (compiler_wrapper && compiler_wrapper[0] != '\0') {
    return "EM_COMPILER_WRAPPER configured";
  }

  if (config.failure) {
    return config.failure_reason;
  }

  std::error_code ec;
  fs::path sysroot = fs::path(config.em_cache) / "sysroot";
  fs::path sysroot_stamp = fs::path(config.em_cache) / "sysroot_install.stamp";
  if (!fs::exists(sysroot, ec) || ec || !fs::exists(sysroot_stamp, ec) || ec) {
    return "Emscripten sysroot not installed in cache: " + sysroot.string();
  }

  return std::nullopt;
}

struct FilterArgsResult {
  std::vector<string_view> args;
  std::string failure_reason;
};

FilterArgsResult filter_compiler_args(const std::vector<std::string>& user_args) {
  FilterArgsResult result;

  for (size_t i = 0; i < user_args.size(); ++i) {
    string_view arg = user_args[i];

    string_view arg_base = arg;
    size_t eq_pos = arg_base.find('=');
    if (eq_pos != string_view::npos) {
      arg_base = arg_base.substr(0, eq_pos);
    }

    if (COMPLEX_OR_SYSTEM_FLAGS.contains(arg_base)) {
      result.failure_reason =
        "Contains Emscripten system or complex flag: " + std::string(arg);
      return result;
    }

    if (is_emscripten_only_warning(arg)) {
      continue;
    }

    string_view setting_key;
    bool ate_next = false;
    if (is_dash_s_setting(user_args, i, setting_key, ate_next)) {
      if (setting_key == "STRICT") {
        if (ate_next) {
          ++i;
        }
        continue;
      }
      if (COMPILE_TIME_SETTINGS.contains(setting_key)) {
        result.failure_reason =
          "Contains Emscripten compile-time setting: -s" + std::string(setting_key);
        return result;
      } else {
        // Linker-only setting: warn and ignore during compilation
        emit_unused_warning("linker setting ignored during compilation: '" +
                            std::string(setting_key) + "'");
        if (ate_next) {
          ++i;
        }
        continue;
      }
    }

    // Check for .bc output file suffix without -flto or -emit-llvm
    if (arg == "-o" && i + 1 < user_args.size()) {
      string_view out_path = user_args[i + 1];
      if (get_extension(out_path) == ".bc") {
        bool has_lto_or_emit_llvm = false;
        for (const auto& a : user_args) {
          string_view sv_a(a);
          if (sv_a.starts_with("-flto") || sv_a == "-emit-llvm") {
            has_lto_or_emit_llvm = true;
            break;
          }
        }
        if (!has_lto_or_emit_llvm) {
          result.failure_reason =
            ".bc output file suffix used without -flto or -emit-llvm";
          return result;
        }
      }
    }

    if (arg == "-g4") {
      result.failure_reason = "Contains deprecated debug flag: -g4";
      return result;
    }

    // Check if arg is a debug flag that Clang doesn't accept directly
    if (arg == "-g1" || arg == "-g2") {
      result.args.push_back("-g0");
      continue;
    }
    if (arg == "-gsource-map" || arg == "-gsource-map=inline" ||
        arg.starts_with("-gseparate-dwarf")) {
      result.args.push_back("-g");
      continue;
    }

    // Check if arg is a link-only flag (e.g. --js-library or
    // --js-library=lib.js)
    string_view flag_name = arg;
    size_t eq = flag_name.find('=');
    bool has_eq = (eq != string_view::npos);
    if (has_eq) {
      flag_name = flag_name.substr(0, eq);
    }

    if (LINK_ONLY_FLAGS.contains(flag_name)) {
      emit_unused_warning("linker flag ignored during compilation: '" +
                          std::string(arg) + "'");
      if (!has_eq && LINK_ONLY_FLAGS_WITH_ARGS.contains(flag_name) &&
          i + 1 < user_args.size() && !user_args[i + 1].starts_with("-")) {
        ++i;
      }
      continue;
    }

    result.args.push_back(arg);
  }

  return result;
}

} // namespace

std::string get_python_executable() {
  const char* env_python = std::getenv("EMSDK_PYTHON");
  if (env_python && env_python[0] != '\0') {
    return env_python;
  }
#ifdef _WIN32
  return "python.exe";
#else
  return "python3";
#endif
}

// Analyze user arguments to determine if native compilation is supported or if
// fallback to Python is required.
DriverDecision analyze_request(string_view driver_arg0,
                               const fs::path& emscripten_root,
                               const fs::path& exe_path,
                               const std::vector<std::string>& user_args,
                               const Config& config) {
  std::string tool = get_tool_name(driver_arg0);

  auto fallback = [&](string_view reason) {
    return make_fallback_decision(tool, emscripten_root, exe_path, user_args, std::string(reason));
  };

  bool is_compiler = (tool == "emcc" || tool == "em++");
  if (!is_compiler) {
    return fallback("Tool " + tool + " runs via Python script");
  }

  if (auto reason = check_system_environment(config)) {
    return fallback(*reason);
  }

  parse_warning_flags(user_args);

  bool compile_only = has_header_inputs(user_args);
  bool is_wasm64 = false;

  // Response files (@file) require complex tokenization (handling shell quoting,
  // escaping, character encodings like UTF-8 with BOM, and recursive response
  // file expansion). In LLVM/Clang, this is handled by llvm::cl::ExpandResponseFiles
  // and llvm::cl::TokenizeGNUCommandLine / TokenizeWindowsCommandLine. Because
  // emcc_native is a standalone executable without LLVM library dependencies,
  // we fall back to Python (which uses shlex.split() in response_file.py)
  // rather than maintaining a custom cross-platform tokenizer and encoding parser.
  // TODO: Implement native response file expansion if we add LLVM dependencies
  // or a robust lightweight tokenizer.
  for (const auto& arg : user_args) {
    if (arg.starts_with("@")) {
      return fallback("Response files (@file) not yet supported by native launcher");
    }
    if (arg == "-c" || arg == "-S" || arg == "-E" || arg == "-M" ||
        arg == "-MM" || arg == "-fsyntax-only") {
      compile_only = true;
    } else if (arg == "-m64" || arg == "-sMEMORY64" ||
               arg == "-sMEMORY64=1" || arg == "-sMEMORY64=2") {
      is_wasm64 = true;
    }
  }

  // Pure compile step requires -c, -S, -E, -M, -MM, -fsyntax-only, or header compilation
  if (!compile_only) {
    return fallback("No compile-only flag (-c, -S, -E) or header input found; defaulting to link phase");
  }

  FilterArgsResult filter_result = filter_compiler_args(user_args);
  if (!filter_result.failure_reason.empty()) {
    return fallback(filter_result.failure_reason);
  }

  DriverDecision decision;
  bool is_cxx = (tool == "em++");
  bool is_asm_only = is_assembly_only(user_args);
  create_clang_command(is_cxx, is_wasm64, is_asm_only,
                       filter_result.args, config, decision);

  // Fall back if total command line length exceeds platform limits
  size_t total_cmd_len = decision.target_binary.size();
  for (const auto& a : decision.target_args) {
    total_cmd_len += a.size() + 1;
  }
#ifdef _WIN32
  constexpr size_t MAX_CMD_LEN = 8192;
#else
  constexpr size_t MAX_CMD_LEN = 32768;
#endif
  if (total_cmd_len > MAX_CMD_LEN) {
    std::string reason = "Command line length (" + std::to_string(total_cmd_len) +
                         " chars) exceeds limit (" + std::to_string(MAX_CMD_LEN) +
                         "); falling back to Python driver for response file handling";
    return fallback(reason);
  }

  return decision;
}

void apply_ccache_wrapper(DriverDecision& decision) {
  const char* emcc_ccache = std::getenv("_EMCC_CCACHE");
  if (!emcc_ccache || emcc_ccache[0] == '\0') {
    return;
  }
  unsetenv("_EMCC_CCACHE");

  decision.target_args.insert(decision.target_args.begin(), decision.target_binary);
#ifdef _WIN32
  decision.target_binary = "ccache.exe";
#else
  decision.target_binary = "ccache";
#endif
}

} // namespace emscripten
