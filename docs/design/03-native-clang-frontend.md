# Design Doc: Native Launcher / Clang Frontend

- **Status**: Draft
- **Bug**: https://github.com/emscripten-core/emscripten/issues/26453

## Context

When running builds using build systems such as CMake, Make, or Ninja,
Emscripten's driver scripts (`emcc`, `em++`) are launched hundreds or thousands
of times. Today, `emcc` is a Python script (`emcc.py`) executed via Node or a
Python interpreter.

Launching Python for every compiler invocation introduces significant startup
overhead:
- On Linux, Python startup takes ~50ms to 100ms per invocation.
- On Windows, process launch and Python startup overhead can take up to
  ~1.5s to 2.4s per invocation.

For incremental compilation of single translation units (`emcc -c file.cpp -o
file.o`), the actual Clang compilation step may take only ~150ms, meaning Python
wrapper startup consumes 80–90% of total build time on Windows and a noticeable
percentage on Linux/macOS.

This document explores replacing or bypassing the Python wrapper for
compilation steps (`-c`, `-S`, `-E`) using a native C++ launcher/frontend
binary.

### Subsuming `pylauncher`

Currently, on Windows, Emscripten ships `pylauncher`
(`tools/pylauncher/pylauncher.c`), a minimal Win32 C executable (`emcc.exe`,
`em++.exe`) whose sole function is locating `python.exe` and spawning `emcc.py`.
On POSIX platforms, shell scripts (`emcc`, `run_python.sh`) perform a similar
wrapper role.

Both native C++ frontend designs presented in this document would completely
**subsume and replace `pylauncher`** (and Unix wrapper scripts). Instead of
`emcc.exe` unconditionally launching `python.exe emcc.py`, the new native `emcc`
binary becomes the primary entry point executable. It directly parses
arguments, handles compile-only commands (`-c`, `-S`, `-E`) natively, and only
spawns `python.exe emcc.py` as a fallback when linking or complex Python-driven
post-processing is required.


## Goals

- Drastically reduce compiler startup overhead for compile-only invocations
  (`-c`, `-S`, `-E`).
- Provide a fast, native C++ driver (`emcc` / `em++`) that directly targets
  `clang` / `clang++`.
- Maintain 100% compatibility with Emscripten header search paths, sysroots,
  target triples (`wasm32-unknown-emscripten`), macros, and flag translation.
- Seamlessly fall back to the existing `emcc.py` Python driver for linking
  steps, post-processing, JS library integration, HTML generation, or
  unhandled/complex flags.
- Keep build, packaging, and distribution within EMSDK simple across Linux,
  macOS, and Windows.

## Non-Goals

- Replacing `emcc.py` for the full linking phase (`-o app.js`, `-o app.html`,
  `-o app.wasm`), WebAssembly transformation passes (Binaryen/wasm-opt), JS glue
  generation, or `embuilder`.
- Rewriting Emscripten's complete Python test harness or configuration
  management in C++.

## Overview & Key Technical Challenges

1. **Emscripten Configuration Parsing**:
   Emscripten relies on `.emscripten` (or `EM_CONFIG`) to locate LLVM binaries
   (`LLVM_ROOT`), cache paths (`EM_CACHE`), and node paths. Because
   `.emscripten` is a Python script, a native binary needs a fast configuration
   reader (e.g. key-value file parser or environment variable inspection) with
   a fallback to invoking Python only when complex logic is present.

2. **Flag Construction & Sysroot Management**:
   The native launcher must build the exact `clang` invocation array,
   injecting:
   - `-target wasm32-unknown-emscripten` (or `wasm64-unknown-emscripten`).
   - `--sysroot=<path_to_emscripten_cache_or_system>`.
   - Built-in include paths (`system/include`, `system/lib/libc/musl/include`,
     etc.).
   - Preprocessor definitions (`__EMSCRIPTEN__`, etc.).

3. **Fallback Mechanism**:
   If the native driver detects linking actions (`.js`, `.html`, `.wasm` output
   without `-c`/`-S`/`-E`) or flags requiring Python post-processing (e.g.,
   `--js-library`, `--embed-file`), it immediately replaces itself with
   `emcc.py` via `execvp` / `CreateProcess`.

---

## Detailed Architectural Designs

We consider two main architectural approaches for the native C++ frontend.

### Design 1: Standalone C++ Launcher (`exec`ing `clang`)

In Design 1, the native C++ frontend is a small, lightweight executable. It
parses command-line arguments, reads config settings, constructs the target
`clang` command line (adding sysroot, include paths, and target triples), and
directly executes the `clang` binary as a child process (or replaces itself via
`execvp` on POSIX / `CreateProcess` on Windows).

```
   [User / CMake / Ninja]
             │
             ▼
     +---------------+
     |  emcc (C++)   |  <--- Fast argument check & sysroot setup (~2ms)
     +---------------+
       │           │
       │ (link or  │ (compile-only: -c)
       │ complex)  │
       ▼           ▼
  +---------+   +----------+
  | emcc.py |   |  clang   |  <--- Direct binary exec (fast)
  +---------+   +----------+
```

#### Key Implementation Details
- Written in clean, portable C++17 with zero external dependencies (standard C++
  library only).
- Parses `.emscripten` using simple string parsing (looking for assignments
  like `LLVM_ROOT = ...`). If `.emscripten` contains complex executable Python
  code, it falls back to `emcc.py`.
- Determines if the command is a pure compile step (`-c`, `-S`, or `-E`
  passed, no linking flags).
- Constructs argument vector for `clang` / `clang++`.
- Replaces process with `clang` (`execv`) or spawns `clang` process.

#### Pros
- **Simplicity**: Easy to implement, audit, and debug. Very small codebase
  (~1,000–2,000 lines of C++).
- **Fast Build & Easy Packaging**: Compiles in milliseconds using any host C++
  compiler or host `clang`. No dependencies on LLVM header files or libraries.
- **Low Startup Latency**: Execution overhead of launcher is < 2ms. Total time
  is dominated solely by Clang's native binary execution (~20–30ms on Linux,
  ~150ms on Windows).
- **Decoupled from LLVM Versions**: Independent of LLVM C++ API/ABI changes;
  works across any LLVM release shipped in EMSDK.

#### Cons
- Incurs one process execution cost (`clang` child process launch).
- Cannot inspect or manipulate Clang internal AST/driver structures directly
  in C++.

---

### Design 2: LLVM-Native C++ Program (Linking `libclang` / LLVM Libraries)

In Design 2, the C++ frontend is built against LLVM/Clang libraries (e.g.
`libclang` or Clang's C++ driver/compiler interface `clangDriver` /
`clangFrontend`). Instead of executing an external `clang` process, it calls
directly into the Clang compiler routines in-process.

```
   [User / CMake / Ninja]
             │
             ▼
     +---------------+
     |  emcc (C++)   |
     | +-----------+ |
     | | libclang  | |  <--- In-process Clang driver & compiler invocation
     | +-----------+ |
     +---------------+
       │
       │ (fallback for link phase)
       ▼
  +---------+
  | emcc.py |
  +---------+
```

#### Key Implementation Details
- Links against `libclang.so` / `libclang.dll` or static LLVM/Clang libraries
  (`clangFrontend`, `clangDriver`, `clangBasic`, etc.).
- Initializes Clang `CompilerInstance` / `Driver` in-process.
- Passes options programmatically to Clang's diagnostic and compilation
  pipelines.

#### Pros
- **Zero Process Spawn Overhead**: No subprocess `fork`/`exec` or `CreateProcess`
  needed to start Clang.
- **Tighter Toolchain Integration**: Direct programmatic control over Clang
  options, diagnostics, target settings, and ASTs.
- **Potential Server/Daemon Mode**: Enables future long-running compilation
  daemon mode (re-using compiler state across multiple files in a single
  process).

#### Cons
- **High Build & Packaging Complexity**: Requires linking against LLVM/Clang C++
  libraries. Significantly increases binary size and EMSDK build/packaging
  requirements.
- **Version Lock-in & Maintenance**: Closely tied to specific LLVM API versions;
  breaking changes in LLVM/Clang internals require updates to `emcc`.
- **Process State Cleanup**: In-process Clang compilation must properly reset
  global LLVM signal handlers, memory allocators, and global state between runs
  if re-used.

---

## Comparison & Evaluation

| Metric / Consideration | Design 1: Standalone `exec()` Launcher | Design 2: LLVM-Native (`libclang` / LLVM) |
| :--- | :--- | :--- |
| **Startup Overhead** | Minimal (~2ms launcher + native `clang` exec) | Zero process spawn overhead |
| **Code Complexity** | Low (~1,500 lines of standard C++) | High (requires LLVM driver integration) |
| **Dependencies** | Standard C++ library only | `libclang` / LLVM C++ libraries |
| **EMSDK Packaging Impact** | Minimal (tiny standalone binary) | High (large binary/library size) |
| **LLVM Version Stability** | Unaffected by LLVM API changes | Must update code for LLVM API changes |
| **Fallback to `emcc.py`** | Instant (`execvp("python3", ...)`) | Instant (`execvp("python3", ...)`) |

---

## Recommendation & Phased Approach

We recommend a **phased implementation**:

1. **Phase 1 (Design 1 - Standalone C++ Launcher)**:
   Implement a standalone C++ launcher binary (`emcc-native`) that parses
   arguments, locates `clang`, adds Emscripten sysroot/target flags, and
   `exec()`s `clang`. This delivers 90–95% of the performance gains immediately
   with minimal maintenance overhead and zero extra LLVM library dependencies.

2. **Phase 2 (Evaluation of Design 2 / Daemon Mode)**:
   If further optimization is required for massive builds or IDE integration,
   evaluate linking against `libclang` or introducing a resident compilation
   daemon service.
