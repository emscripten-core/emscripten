# Native Clang Frontend Launcher (`emcc_native`)

`emcc_native` is a high-performance C++ launcher for Emscripten's compiler
drivers (`emcc` and `em++`). It drastically reduces compiler startup latency for
compile-only invocations (`-c`, `-S`, `-E`, `-M`, `-MM`) by bypassing Python and directly
executing `clang` / `clang++`.

## Overview & Architecture

When running large CMake or Ninja builds, `emcc` is launched hundreds or
thousands of times. Executing the Python interpreter for each single compile
unit adds overhead (especially on Windows).

`emcc_native` provides native executables (`bin/emcc`, `bin/em++`) that:
1. **Directly invoke Clang** for pure compilation steps (`-c`, `-S`, `-E`, `-M`, `-MM`), injecting:
   - Target triple (`-target wasm32-unknown-emscripten` or `wasm64-unknown-emscripten`)
   - Frontend exceptions flag (`-fignore-exceptions`)
   - Default LLVM backend flags (e.g. `-mllvm -enable-emscripten-sjlj`)
   - Emscripten sysroot (`--sysroot=<cache>/sysroot`)
   - Clang sysroot include paths (e.g. `-Xclang -iwithsysroot/include/compat`)
   - SIMD/SSE/NEON preprocessor macros (`-D__SSE__=1`, `-D__SSE2__=1`,
     `-D__ARM_NEON__=1`, etc.) when architecture flags are specified
   - Visibility flag (`-fvisibility=default` when `-fPIC` is passed without
     `-fvisibility`)
2. **Ignore compile-unused linker flags**: Link-only flags (`--js-library`,
   `--embed-file`, etc.) and linker settings (`-sEXPORTED_FUNCTIONS`, etc.) are
   ignored during compilation (with diagnostic warnings matching `emcc.py`),
   allowing compile steps with link flags to run natively.
3. **Fall back to Python** (`emcc.py` / `em++.py`) when link-phase invocations
   are run, or when compile-time `-s` settings or system flags (`--clear-cache`,
   `--build`, `--tracing`, etc.) are present.

## Building

Building requires CMake 3.20+ and a C++20 compiler.

```bash
cmake -B out/build_emcc_native -S tools/emcc_native
cmake --build out/build_emcc_native
cmake --install out/build_emcc_native
```

The output executables (`emcc`, `em++`) will be installed in `./bin`.

## Running Tests

To run the unit and integration tests:

```bash
ctest --test-dir out/build_emcc_native --output-on-failure
```

## Code Generation

Compile-time settings, link-only flags, and Emscripten warning options are
generated in `generated_settings.h`. To update this header from Python
definitions, run:

```bash
./tools/emcc_native/gen_settings.py
```

To verify whether `generated_settings.h` is up to date:

```bash
./tools/emcc_native/gen_settings.py --check
```

## Configuration & Environment Variables

- `EMCC_NATIVE`:
  - Set to `0` to disable the native driver and unconditionally fall back to `emcc.py`.
  - Set to `1` to force strict native mode; if an invocation requires falling back to Python, `emcc_native` will print the fallback reason and exit with an error (useful for debugging).
- `EMCC_DEBUG`: When set (e.g. `EMCC_DEBUG=1`), logs launcher decision details (whether direct Clang execution or Python fallback was selected, reason, target binary, and command arguments).
- `EMSDK_PYTHON`: Path to the Python executable (defaults to `python3` or `python.exe` on Windows).
- `EM_CACHE`: Path to Emscripten cache directory (defaults to `<emscripten_root>/cache`).
- `EM_CONFIG`: Path to `.emscripten` configuration file (reads `LLVM_ROOT` and `CACHE`).
- `EM_LLVM_ROOT`: Environment variable override for the directory containing LLVM binaries (`clang`, `clang++`).

## CI Benchmark Results

Compile-time performance is automatically benchmarked on CI across Linux,
macOS, and Windows (`embuilder build libc --force` compiling 1,075 files
sequentially with `EMCC_CORES=1`, `EMCC_USE_NINJA=0`, and
`EMCC_BATCH_BUILD=0`).

| Platform    | Before (Python Baseline) | After (Native Launcher) | Improvement                |  Speedup  |
| :---------: | :----------------------: | :---------------------: | :------------------------: | :-------: |
| **Linux**   | 181.96 s (169.3 ms/file) |  64.82 s (60.3 ms/file) | -117.14 s (-109.0 ms/file) | **2.81x** |
| **Windows** | 343.90 s (319.9 ms/file) | 105.12 s (97.8 ms/file) | -238.78 s (-222.1 ms/file) | **3.27x** |
| **macOS**   | 162.08 s (150.6 ms/file) |  64.73 s (60.1 ms/file) |  -97.35 s (-90.5 ms/file)  | **2.50x** |

As expected, because process creation and `python.exe` startup carry
significantly higher overhead on Windows than on POSIX systems, the speedup on
Windows CI (**3.27x**, saving over 222 ms per invocation) is even larger than on
Linux and macOS.
