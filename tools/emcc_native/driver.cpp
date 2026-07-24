/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "driver.h"
#include "generated_settings.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <unordered_set>

namespace fs = std::filesystem;

namespace emscripten {

namespace {

// Flags that require fallback to the Python driver because they represent
// complex operations or standing system commands.
// NOTE: Keep in sync with system options in tools/cmdline.py and complex
// options in tools/compile.py.
const std::unordered_set<std::string> COMPLEX_OR_SYSTEM_FLAGS = {
  "--clear-cache",
  "--clear-ports",
  "--build",
  "--generate-config",
  "--compiler-wrapper",
  "--tracing",
  "--memoryprofiler",
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

// Warning state for diagnostics
bool g_warn_unused = true;
bool g_error_unused = false;

void parse_warning_flags(const std::vector<std::string>& user_args) {
  g_warn_unused = true;
  g_error_unused = false;
  for (const auto& arg : user_args) {
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

void emit_unused_warning(const std::string& msg) {
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

void create_fallback_command(bool is_cxx,
                             const fs::path& emscripten_root,
                             const std::vector<std::string>& user_args,
                             DriverDecision& decision) {
  decision.target_binary = get_python_executable();
  fs::path script = emscripten_root / (is_cxx ? "em++.py" : "emcc.py");
  decision.target_args.push_back(script.generic_string());
  for (const auto& arg : user_args) {
    decision.target_args.push_back(arg);
  }
}

// Process SIMD/SSE/NEON feature flags and inject corresponding macro
// definitions. NOTE: Keep in sync with get_cflags() in tools/compile.py and
// SIMD_INTEL_FEATURE_TOWER / SIMD_NEON_FLAGS in tools/cmdline.py.
void handle_simd_flags(const std::vector<std::string>& filtered_user_args,
                       DriverDecision& decision) {
  bool has_simd = false;
  bool has_sse = false, has_sse2 = false, has_sse3 = false, has_ssse3 = false;
  bool has_sse4_1 = false, has_sse4_2 = false, has_avx = false,
       has_avx2 = false;
  bool has_fma = false, has_neon = false;
  bool has_intel_simd = false;

  for (const auto& arg : filtered_user_args) {
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
                          const std::vector<std::string>& filtered_user_args,
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
    for (const auto& arg : filtered_user_args) {
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

    for (const auto& arg : filtered_user_args) {
      if (arg == "-nostdinc") {
        nostdinc = true;
      } else if (arg == "-fPIC") {
        has_fpic = true;
      } else if (arg.rfind("-fvisibility", 0) == 0) {
        has_fvisibility = true;
      } else if (arg == "-pthread" || arg == "-fopenmp" ||
                 arg == "-fopenmp=libomp") {
        has_pthread = true;
      }
    }

    if (has_pthread) {
      decision.target_args.push_back("-D__EMSCRIPTEN_SHARED_MEMORY__=1");
      bool pthread_in_args = false;
      for (const auto& arg : filtered_user_args) {
        if (arg == "-pthread") {
          pthread_in_args = true;
          break;
        }
      }
      if (!pthread_in_args) {
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

  for (const auto& arg : filtered_user_args) {
    decision.target_args.push_back(arg);
  }
}

bool is_upper_identifier(std::string_view s) {
  if (s.empty())
    return false;
  for (char c : s) {
    if (!std::isupper(static_cast<unsigned char>(c)) && c != '_') {
      return false;
    }
  }
  return true;
}

bool is_dash_s_setting(const std::vector<std::string>& user_args,
                       size_t i,
                       std::string& setting_key,
                       bool& ate_next) {
  const std::string& arg = user_args[i];
  ate_next = false;
  std::string_view val;
  if (arg == "-s") {
    if (i + 1 >= user_args.size())
      return false;
    val = user_args[i + 1];
    ate_next = true;
  } else if (arg.starts_with("-s")) {
    val = std::string_view(arg).substr(2);
  } else {
    return false;
  }

  size_t eq = val.find('=');
  if (eq != std::string_view::npos) {
    setting_key = std::string(val.substr(0, eq));
  } else {
    setting_key = std::string(val);
  }
  return is_upper_identifier(setting_key);
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
  return EMSCRIPTEN_ONLY_WARNINGS.count(std::string(name)) > 0;
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

bool is_assembly_only(const std::vector<std::string>& user_args) {
  static const std::unordered_set<std::string> ASM_EXTS = {".s", ".S"};
  static const std::unordered_set<std::string> C_EXTS = {
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
    fs::path p(arg);
    std::string ext = p.extension().string();
    if (ASM_EXTS.count(ext)) {
      has_asm = true;
    } else if (C_EXTS.count(ext)) {
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
  static const std::unordered_set<std::string> HEADER_EXTS = {
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

    fs::path p(arg);
    std::string ext = p.extension().string();
    if (HEADER_EXTS.count(ext)) {
      return true;
    }
  }

  return false;
}

bool has_emmaken_env_var() {
  return std::getenv("EMMAKEN_CFLAGS") || std::getenv("EMMAKEN_COMPILER");
}

// Analyze user arguments to determine if native compilation is supported or if
// fallback to Python is required.
// NOTE: Keep flag filtering and fallback conditions in sync with option parsing
// in tools/cmdline.py and tools/compile.py.
DriverDecision analyze_request(bool is_cxx,
                               const fs::path& emscripten_root,
                               const std::vector<std::string>& user_args,
                               const Config& config) {
  DriverDecision decision;

  // Environment variable override to disable native driver
  const char* native_env = std::getenv("EMCC_NATIVE");
  if (native_env && std::string(native_env) == "0") {
    decision.use_fallback = true;
    decision.reason = "EMCC_NATIVE set to disable native launcher";
  }

  if (has_emmaken_env_var()) {
    decision.use_fallback = true;
    decision.reason = "Contains EMMAKEN_ environment variable";
  }

  const char* compiler_wrapper = std::getenv("EM_COMPILER_WRAPPER");
  if (compiler_wrapper && compiler_wrapper[0] != '\0') {
    decision.use_fallback = true;
    decision.reason = "EM_COMPILER_WRAPPER configured";
  }

  if (config.failure) {
    decision.use_fallback = true;
    decision.reason = config.failure_reason;
  }

  std::error_code ec;
  fs::path sysroot = fs::path(config.em_cache) / "sysroot";
  fs::path sysroot_stamp = fs::path(config.em_cache) / "sysroot_install.stamp";
  if (!fs::exists(sysroot, ec) || ec || !fs::exists(sysroot_stamp, ec) || ec) {
    decision.use_fallback = true;
    decision.reason =
      "Emscripten sysroot not installed in cache: " + sysroot.string();
  }

  parse_warning_flags(user_args);

  bool compile_only = false;
  bool is_wasm64 = false;

  if (!decision.use_fallback) {
    if (has_header_inputs(user_args)) {
      compile_only = true;
    }
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
        decision.use_fallback = true;
        decision.reason =
          "Response files (@file) not yet supported by native launcher";
        break;
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
    if (!decision.use_fallback && !compile_only) {
      decision.use_fallback = true;
      decision.reason =
        "No compile-only flag (-c, -S, -E) or header input found; defaulting to link phase";
    }
  }

  std::vector<std::string> filtered_user_args;

  if (!decision.use_fallback) {
    for (size_t i = 0; i < user_args.size(); ++i) {
      const std::string& arg = user_args[i];

      std::string arg_base = arg;
      size_t eq_pos = arg_base.find('=');
      if (eq_pos != std::string::npos) {
        arg_base = arg_base.substr(0, eq_pos);
      }

      if (COMPLEX_OR_SYSTEM_FLAGS.count(arg) ||
          COMPLEX_OR_SYSTEM_FLAGS.count(arg_base)) {
        decision.use_fallback = true;
        decision.reason = "Contains Emscripten system or complex flag: " + arg;
        break;
      }

      if (arg == "-pthreads") {
        decision.use_fallback = true;
        decision.reason = "Invalid option -pthreads";
        break;
      }

      if (is_emscripten_only_warning(arg)) {
        continue;
      }

      std::string setting_key;
      bool ate_next = false;
      if (is_dash_s_setting(user_args, i, setting_key, ate_next)) {
        if (setting_key == "STRICT") {
          if (ate_next) {
            ++i;
          }
          continue;
        }
        if (COMPILE_TIME_SETTINGS.count(setting_key)) {
          decision.use_fallback = true;
          decision.reason =
            "Contains Emscripten compile-time setting: -s" + setting_key;
          break;
        } else {
          // Linker-only setting: warn and ignore during compilation
          emit_unused_warning("linker setting ignored during compilation: '" +
                              setting_key + "'");
          if (ate_next) {
            ++i;
          }
          continue;
        }
      }

      // Check for .bc output file suffix without -flto or -emit-llvm
      if (arg == "-o" && i + 1 < user_args.size()) {
        const std::string& out_path = user_args[i + 1];
        if (fs::path(out_path).extension() == ".bc") {
          bool has_lto_or_emit_llvm = false;
          for (const auto& a : user_args) {
            if (a.starts_with("-flto") || a == "-emit-llvm") {
              has_lto_or_emit_llvm = true;
              break;
            }
          }
          if (!has_lto_or_emit_llvm) {
            decision.use_fallback = true;
            decision.reason =
              ".bc output file suffix used without -flto or -emit-llvm";
            break;
          }
        }
      }

      if (arg == "-g4") {
        decision.use_fallback = true;
        decision.reason = "Contains deprecated debug flag: -g4";
        break;
      }

      // Check if arg is a debug flag that Clang doesn't accept directly
      if (arg == "-g1" || arg == "-g2") {
        filtered_user_args.push_back("-g0");
        continue;
      }
      if (arg == "-gsource-map" || arg == "-gsource-map=inline" ||
          arg.starts_with("-gseparate-dwarf")) {
        filtered_user_args.push_back("-g");
        continue;
      }

      // Check if arg is a link-only flag (e.g. --js-library or
      // --js-library=lib.js)
      std::string flag_name = arg;
      size_t eq = flag_name.find('=');
      bool has_eq = (eq != std::string::npos);
      if (has_eq) {
        flag_name = flag_name.substr(0, eq);
      }

      if (LINK_ONLY_FLAGS.count(flag_name)) {
        emit_unused_warning("linker flag ignored during compilation: '" +
                            arg + "'");
        if (!has_eq && i + 1 < user_args.size() &&
            !user_args[i + 1].starts_with("-")) {
          ++i;
        }
        continue;
      }

      filtered_user_args.push_back(arg);
    }
  }

  // Construct command vectors
  if (decision.use_fallback) {
    create_fallback_command(is_cxx, emscripten_root, user_args, decision);
  } else {
    bool is_asm_only = is_assembly_only(user_args);
    create_clang_command(
      is_cxx, is_wasm64, is_asm_only, filtered_user_args, config, decision);

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
      decision.target_args.clear();
      decision.use_fallback = true;
      decision.reason = "Command line length (" + std::to_string(total_cmd_len) +
                        " chars) exceeds limit (" + std::to_string(MAX_CMD_LEN) +
                        "); falling back to Python driver for response file handling";
      create_fallback_command(is_cxx, emscripten_root, user_args, decision);
    }
  }

  return decision;
}

} // namespace emscripten
