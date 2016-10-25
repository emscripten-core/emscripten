This document describes changes between tagged Emscripten SDK versions.

Note that in the compiler, version numbering is used as the mechanism to invalidate internal compiler caches,
so version numbers do not necessarily reflect the amount of changes between versions.

To browse or download snapshots of old tagged versions, visit https://github.com/kripken/emscripten/releases .

Not all changes are documented here. In particular, new features, user-oriented fixes, options, command-line parameters, usage changes, deprecations, significant internal modifications and optimizations etc. generally deserve a mention. To examine the full set of changes between versions, visit the link to full changeset diff at the end of each section.

Current trunk code
------------------
 - Updated to libc++'s "v2" ABI, which provides better alignment for string data and other improvements. This is an ABI-incompatible change, so bitcode files from previous versions will not be compatible.
 - To see a list of commits in the active development branch 'incoming', which have not yet been packaged in a release, see
    - Emscripten: https://github.com/kripken/emscripten/compare/1.36.1...incoming
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.36.1...incoming
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.36.1...incoming

v1.36.1: 3/8/2016
-----------------
 - Fixed glfwSetWindowSizeCallback to conform to GLFW2 API.
 - Update OpenAL sources only when the browser window is visible to avoid occasional stuttering static glitches when the page tab is hidden. (#4107)
 - Implemented LLVM math intrinsics powi, trunc and floor.
 - Added support for SDL_GL_ALPHA_SIZE in GL context initialization. (#4125)
 - Added no-op stubs for several pthread functions when building without pthreads enabled (#4130)
 - Optimize glUniform*fv and glVertexAttrib*fv functions to generate less garbage and perform much faster (#4128)
 - Added new EVAL_CTORS optimization pass which evaluates global data initializer constructors at link time, which would improve startup time and reduce code size of these ctors.
 - Implemented support for OpenAL AL_PITCH option.
 - Implemented new build options -s STACK_OVERFLOW_CHECK=0/1/2 which adds runtime stack overrun checks. 0: disabled, 1: minimal, between each frame, 2: at each explicit JS side stack allocation call to allocate().
 - Fixed an issue with -s SPLIT_MEMORY mode where an unsigned 32-bit memory access would come out as signed. (#4150)
 - Fixed asm.js validation in call handlers to llvm_powi_f*.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.36.0...1.36.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.36.0...1.36.1
    - Emscripten-Clang: no changes.

v1.36.0: 2/23/2016
------------------
 - Fixed an OpenAL bug where OpenAL sources would not respect global volume setting.
 - Fixed an issue where alGetListenerf() with AL_GAIN would not return the correct value. (#4091)
 - Fixed an issue where setting alListenerf() with AL_GAIN would not set the correct value. (#4092)
 - Implemented new JS optimizer "Duplicate Function Elimination" pass which collapses identical functions to save code size.
 - Implemented the _Exit() function.
 - Added support for SSE3 and SSSE3 intrinsics (#4099) and partially for SSE 4.1 intrinsics (#4030, #4101)
 - Added support for -include-pch flag (#4086)
 - Fixed a regex syntax in ccall on Chrome Canary (#4111)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.23...1.36.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.23...1.36.0
    - Emscripten-Clang: no changes.

v1.35.23: 2/9/2016
------------------
 - Provide $NM environment variable to point to llvm-nm when running emconfigure, which helps e.g. libjansson to build (#4036)
 - Fixed glGetString(GL_SHADING_LANGUAGE_VERSION) to return appropriate result depending on if running on WebGL1 vs WebGL2, instead of hardcoding the result (#4040)
 - Fixed a regression with CMake try_run() possibly failing, caused by the addition of CMAKE_CROSSCOMPILING_EMULATOR in v1.32.3.
 - Fixed CMake to work in the case when NODE_JS is an array containing parameters to be passed to Node.js. (#4045)
 - Fixed a memory issue that caused the Emscripten memory initializer file (.mem.js) to be unnecessarily retained in memory during runtime (#4044)
 - Added support for complex valued mul and div ops.
 - Added new option "Module.environment" which allows overriding the runtime ENVIRONMENT_IS_WEB/ENVIRONMENT_IS_WORKER/ENVIRONMENT_IS_NODE/ENVIRONMENT_IS_SHELL fields.
 - Fixed an issue with SAFE_HEAP methods in async mode (#4046)
 - Fixed WebSocket constructor to work in web worker environment (#3849)
 - Fixed a potential issue with some browsers reporting gamepad axis values outside \[-1, 1\] (#3602)
 - Changed libcxxabi to be linked in last, so that it does not override weakly linked methods in libcxx (#4053)
 - Implemented new JSDCE code optimization pass which removes at JS link stage dead code that is not referenced anywhere (in addition to LLVM doing this for C++ link stage).
 - Fixed a Windows issue where embedding memory initializer as a string in JS code might cause corrupted output. (#3854)
 - Fixed an issue when spaces are present in directory names in response files (#4062)
 - Fixed a build issue when using --tracing and -s ALLOW_MEMORY_GROWTH=1 simultaneously (#4064)
 - Greatly updated Emscripten support for SIMD.js intrinsics (non-SSE or NEON)
 - Fixed an issue where compiler would not generate a link error when JS library function depended on a nonexisting symbol. (#4077)
 - Removed UTF16 and UTF32 marshalling code from being exported by default.
 - Removed the -s NO_BROWSER linker option and automated the detection of when that option is needed.
 - Removed the JS implemented C++ symbol name demangler, now always depend on the libcxxabi compiled one.
 - Fixed an issue where Emscripten linker would redundantly generate missing function stubs for some functions that do exist.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.22...1.35.23
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.22...1.35.23
    - Emscripten-Clang: no changes.

v1.35.22: 1/13/2016
-------------------
 - Updated to latest upstream LLVM trunk as of January 13th.
 - Bumped up the required LLVM version from LLVM 3.8 to LLVM 3.9.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.21...1.35.22
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.21...1.35.22
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.35.21...1.35.22

v1.35.21: 1/13/2016
-------------------
 - Improved support for handling GLFW2 keycodes.
 - Improved emranlib, system/bin/sdl-config and system/bin/sdl2-config to be executable in both python2 and python3.
 - Fixed build flags -s AGGRESSIVE_VARIABLE_ELIMINATION=1 and -s USE_PTHREADS=2 to correctly work when run on a browser that does not support pthreads.
 - Fixed a build issue that caused sequences of \r\r\n to be emitted on Windows.
 - Fixed an issue that prevented building LLVM on Visual Studio 2015 (emscripten-fastcomp-clang #7)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.20...1.35.21
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.20...1.35.21
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.35.20...1.35.21

v1.35.20: 1/10/2016
-------------------
 - Fixed -s USE_PTHREADS compilation mode to account that SharedArrayBuffer specification no longer allows futex waiting on the main thread. (#4024)
 - Added new python2 vs python3 compatibility wrappers for emcmake, emconfigure, emmake and emar.
 - Fixed atomicrmw i64 codegen (#4025)
 - Optimized codegen to simplify "x != 0" to just "x" when output is a boolean.
 - Fixed a compiler crash when generating atomics code in debug builds of LLVM.
 - Fixed a compiler crash when generating SIMD.js code that utilizes non-canonical length vectors (e.g. <float x 3>)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.19...1.35.20
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.19...1.35.20
    - Emscripten-Clang: no changes.

v1.35.19: 1/7/2016
------------------
 - Updated to latest upstream LLVM trunk as of January 7th.
 - Full list of changes:
    - Emscripten: no changes.
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.18...1.35.19
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.35.18...1.35.19

v1.35.18: 1/7/2016
------------------
 - Implemented getpeername() and fixed issues with handling getsockname() (#3997)
 - Fixed an issue with daylight saving time in mktime() (#4001)
 - Optimized pthreads code to avoid unnecessary FFI transitions (#3504)
 - Fixed issues with strftime() (#3993)
 - Deprecated memory growth support in asm.js.
 - Implemented llvm_bitreverse_i32() (#3976)
 - Fixed missing include header that affected building relooper on some compilers.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.17...1.35.18
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.17...1.35.18
    - Emscripten-Clang: no changes.

v1.35.17: 1/4/2016
------------------
 - Updated to latest upstream LLVM trunk as of January 4th.
 - Full list of changes:
    - Emscripten: no changes.
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.16...1.35.17
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp/compare/1.35.16...1.35.17

v1.35.16: 1/4/2016
------------------
 - Improved support for -s USE_PTHREADS=2 build mode and added support for Atomics.exchange().
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.15...1.35.16
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.15...1.35.16
    - Emscripten-Clang: no changes.

v1.35.15: 1/4/2016
------------------
 - Fixed an error with glClearbufferfv not working. (#3961)
 - Improved file packager code so that file:// URLs work in Chrome too (#3965)
 - Fixed issues with the --memoryprofiler UI.
 - Fixed a Windows issue when generating system libraries in cache (#3939)
 - Fixed a regression from v1.35.13 where GLES2 compilation would not work when -s USE_PTHREADS=1 was passed.
 - Added support for WebIDL arrays as input parameters to WebIDL binder.
 - Updated build support when using the LLVM wasm backend.
 - Added new linker option --threadprofiler which generates a threads dashboard on the generated page for threads status overview. (#3971)
 - Improved backwards compatibility of building on GCC 4.3 - 4.6.
 - Fixed an asm.js validation issue when building against updated SIMD.js specification. (#3986)
 - Improved Rust support.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.14...1.35.15
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.14...1.35.15
    - Emscripten-Clang: no changes.

v1.35.14: 12/15/2015
--------------------
 - Updated to latest upstream LLVM trunk as of December 15th.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.13...1.35.14
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.13...1.35.14
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.35.13...1.35.14

v1.35.13: 12/15/2015
--------------------
 - Updated -s USE_PTHREADS code generation to reflect that the SharedInt*Array hierarchy no longer exists in the SharedArrayBuffer spec.
 - Removed references to Atomic.fence() which no longer is part of the SharedArrayBuffer specification.
 - Fixed an issue where JS code minifiers might generate bad code for cwrap (#3945)
 - Updated compiler to issue a warning when --separate-asm is being used and output suffix is .js.
 - Added new build option -s ONLY_MY_CODE which aims to eliminate most of the Emscripten runtime and generate a very minimal compiler output.
 - Added new build option -s WASM_BACKEND=0/1 which controls whether to utilize the upstream LLVM wasm emitting codegen backend.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.12...1.35.13
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.12...1.35.13
    - Emscripten-Clang: no changes.

v1.35.12: 11/28/2015
--------------------
 - Update to latest upstream LLVM trunk as of November 28th.
 - Fix Emscripten to handle new style format outputted by llvm-nm.
 - Added new build option BINARYEN_METHOD to allow choosing which wasm generation method to use.
 - Updates to Binaryen support.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.11...1.35.12
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.11...1.35.12
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.35.11...1.35.12

v1.35.11: 11/27/2015
--------------------
 - Updated atomics test to stress 64-bit atomics better (#3892)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.10...1.35.11
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.10...1.35.11
    - Emscripten-Clang: no changes.

v1.35.10: 11/25/2015
--------------------
 - Integration with Binaryen.
 - Add a performance warning when multiple FS.syncfs() calls are in flight simultaneously.
 - Correctly pass GLFW_REPEAT when sending key press repeats.
 - Improved filesystem performance when building in multithreaded mode (#3923)
 - Improve error detection when data file fails to load.
 - Clarified that -s NO_DYNAMIC_EXECUTION=1 and -s RELOCATABLE=1 build modes are mutually exclusive.
 - Added new build option -s NO_DYNAMIC_EXECUTION=2 which demotes eval() errors to warnings at runtime, useful for iterating fixes in a codebase for multiple eval()s  (#3930)
 - Added support to Module.locateFile(filename) to locate the pthread-main.js file (#3500)
 - Changed -s USE_PTHREADS=2 and -s PRECISE_F32=2 to imply --separate-asm instead of requiring it, to be backwards compatible (#3829, #3933)
 - Fixed bad codegen for some 64-bit atomics (#3892, #3936)
 - When emitting NaN canonicalization warning, also print the location in code where it occurs.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.9...1.35.10
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.9...1.35.10
    - Emscripten-Clang: no changes.

v1.35.9: 11/12/2015
-------------------
 - Implement glfwSetInputMode when mode is GLFW_CURSOR and value is GLFW_CURSOR_NORMAL|GLFW_CURSOR_DISABLED
 - Add explicit abort() when dlopen() is called without linking support
 - Make emcc explicitly reinvoke itself from python2 if called from python3.
 - Optimize memory initializer to omit zero-initialized values (#3907)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.8...1.35.9
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.8...1.35.9
    - Emscripten-Clang: no changes.

v1.35.8: 11/10/2015
-------------------
 - Removed obsoleted EXPORTED_GLOBALS build option.
 - Export filesystem as global object 'FS' in Emscripten runtime.
 - Fixed realpath() function on directories.
 - Fixed round() and roundf() to work when building without -s PRECISE_F32=1 and optimize these to be faster (#3876)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.7...1.35.8
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.35.7: 11/4/2015
------------------
 - Updated to latest upstream LLVM trunk version as of November 4th.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.6...1.35.7
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.6...1.35.7
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.35.6...1.35.7

v1.35.6: 11/4/2015
------------------
 - This tag was created for technical purposes, and has no changes compared to v1.35.6.

v1.35.5: 11/4/2015
------------------
 - Removed Content-Length and Connection: close headers in POST requests.
 - Migrate to using the native C++11-implemented optimizer by default.
 - Fixed call to glDrawBuffers(0, *); (#3890)
 - Fixed lazy file system to work with closure (#3842)
 - Fixed gzip compression with lazy file system (#3837)
 - Added no-op gracefully failing stubs for process spawn functions (#3819)
 - Clarified error message that memory growth is not supported with shared modules (#3893)
 - Initial work on wasm support in optimizer
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.4...1.35.5
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.35.4: 10/26/2015
-------------------
 - Move to legalization in the JS backend.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.3...1.35.4
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.3...1.35.4
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.35.3...1.35.4

v1.35.3: 10/26/2015
-------------------
 - Ignore O_CLOEXEC on NODEFS (#3862)
 - Improved --js-library support in CMake by treating these as libraries (#3840)
 - Still support -Wno-warn-absolute-paths (#3833)
 - Add support to zext <4 x i1> to <4x i32>
 - Emit emscripten versions of llvm and clang in clang --version
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.2...1.35.3
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.2...1.35.3
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.35.2...1.35.3

v1.35.2: 10/20/2015
-------------------
 - Rebase against upstream LLVM "google/stable" branch, bringing us to LLVM 3.8.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.1...1.35.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.35.1...1.35.2
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.35.1...1.35.2

v1.35.1: 10/20/2015
-------------------
 - Fixed a bug where passing -s option to LLVM would not work.
 - Work around a WebAudio bug on WebKit "pauseWebAudio failed: TypeError: Not enough arguments" (#3861)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.35.0...1.35.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.35.0: 10/19/2015
-------------------
 - Fixed out of memory abort message.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.12...1.35.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.34.12: 10/13/2015
--------------------
 - Added new experimental build option -s SPLIT_MEMORY=1, which splits up the Emscripten HEAP to multiple smaller slabs.
 - Added SDL2_ttf to Emscripten ports.
 - Added support for building GLES3 code to target WebGL 2. (#3757, #3782)
 - Fixed certain glUniform*() functions to work properly when called in conjunction with -s USE_PTHREADS=1.
 - Fixed support for -l, -L and -I command line parameters to accept a space between the path, i.e. "-l SDL". (#3777)
 - Fixed SSE2 support in optimized builds.
 - Changed the default behavior of warning when absolute paths are passed to -I to be silent. To enable the absolute paths warning, pass "-Wwarn-absolute-paths" flag to emcc.
 - Added new linker option -s ABORTING_MALLOC=0 that can be used to make malloc() return 0 on failed allocation (Current default is to abort execution of the page on OOM) (#3822)
 - Removed the default behavior of automatically decoding all preloaded assets on page startup (#3785)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.11...1.34.12
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.34.11...1.34.12
    - Emscripten-Clang: no changes.

v1.34.11: 9/29/2015
-------------------
 - Fixed asm.js validation on autovectorized output
 - Fix an issue with printing to iostream in global ctors (#3824)
 - Added support for LLVM pow intrinsics with integer exponent.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.10...1.34.11
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.34.10...1.34.11
    - Emscripten-Clang: no changes.

v1.34.10: 9/25/2015
-------------------
 - Added wasm compressor/decompressor polyfill (#3766)
 - Added support for sRGB texture formats.
 - Removed the deprecated --compression option.
 - Fixed an issue with with asm.js validation for pthreads being broken since v1.34.7 (#3719)
 - Added built-in cpu performance profiler, which is enabled with linker flag --cpuprofiler. (#3781)
 - Added build-in memory usage profiler, which is enabled with linker flag --memoryprofiler. (#3781)
 - Fixed multiple arities per EM_ASM block (#3804)
 - Fixed issues with SSE2 an NaN bit patterns. (emscripten-fastcomp #116)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.9...1.34.10
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.34.9...1.34.10
    - Emscripten-Clang: no changes.

v1.34.9: 9/18/2015
------------------
 - Fixed an issue with --llvm-lto 3 builds (#3765)
 - Optimized LZ4 compression
 - Fixed a bug where glfwCreateWindow would return success even on failure (#3764)
 - Greatly optimized the -s SAFE_HEAP=1 linker flag option by executing the heap checks in asm.js side instead.
 - Fixed the return value of EM_ASM_DOUBLE (#3770)
 - Implemented getsockname syscall (#3769)
 - Don't warn on unresolved symbols when LINKABLE is specified.
 - Fixed various issues with SSE2 compilation in optimized builds.
 - Fixed a breakage with -s USE_PTHREADS=2 (#3774)
 - Added support for GL_HALF_FLOAT in WebGL 2. (#3790)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.8...1.34.9
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.34.8...1.34.9
    - Emscripten-Clang: no changes.

v1.34.8: 9/9/2015
-----------------
 - Fixed a race condition at worker startup (#3741)
 - Update emrun to latest, which improves unit test run automation with emrun.
 - Added support for LZ4 compressing file packages, used with the -s LZ4=1 linker flag. (#3754)
 - Fixed noisy build warning on "unexpected number of arguments in call to strtold" (#3760)
 - Added new linker flag --separate-asm that splits the asm.js module and the handwritten JS functions to separate files.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.7...1.34.8
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.34.7: 9/5/2015
-----------------
 - Fixed uses of i64* in side modules.
 - Improved GL support when proxying, and fake WebAudio calls when proxying.
 - Added new main loop timing mode EM_TIMING_SETIMMEDIATE for rendering with vsync disabled (#3717)
 - Updated emrun to latest version, adds --safe_firefox_profile option to run emrun pages in clean isolated environment.
 - Implemented glGetStringi() method for WebGL2/GLES3. (#3472, #3725)
 - Automatically emit loading code for EMTERPRETIFY_FILE if emitting html.
 - Added new build option -s USE_PTHREADS=2 for running pthreads-enabled pages in browsers that do not support SharedArrayBuffer.
 - Added support for building SSE2 intrinsics based code (emmintrin.h), when -msse2 is passed to the build.
 - Added exports for getting FS objects by their name (#3690)
 - Updated LLVM to latest upstream PNaCl version (Clang 3.7, July 29th).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.6...1.34.7
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.34.6...1.34.7
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.34.6...1.34.7

v1.34.6: 8/20/2015
------------------
 - Added new build option -s EMULATED_FUNCTION_POINTERS=2.
 - Fixed a bug with calling functions pointers that take float as parameter across dynamic modules.
 - Improved dynamic linking support with -s LINKABLE=1.
 - Added new build option -s MAIN_MODULE=2.
 - Cleaned up a few redundant linker warnings (#3702, #3704)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.5...1.34.6
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.34.5...1.34.6
    - Emscripten-Clang: no changes.

v1.34.5: 8/18/2015
------------------
 - Added Bullet physics, ogg and vorbis to emscripten-ports.
 - Added FreeType 2.6 to emscripten-ports.
 - Fixed CMake handling when building OpenCV.
 - Fixed and issue with exceptions being thrown in empty glBegin()-glEnd() blocks (#3693)
 - Improved function pointer handling between dynamically linked modules
 - Fixed some OpenAL alGetSource get calls (#3669)
 - Fixed issues with building the optimizer on 32-bit Windows (#3673)
 - Increased optimizer stack size on Windows to 10MB (#3679)
 - Added support for passing multiple input files to opt, to speed up optimization and linking in opt.  
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.4...1.34.5
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.34.4...1.34.5
    - Emscripten-Clang: no changes.

v1.34.4: 8/4/2015
-----------------
 - Add special handling support for /dev/null as an input file (#3552)
 - Added basic printf support in NO_FILESYSTEM mode (#3627)
 - Update WebVR support to the latest specification, and add support for retrieving device names
 - Improved --proxy-to-worker build mode with proxying (#3568, #3623)
 - Generalized EXPORT_FS_METHODS to EXPORT_RUNTIME_METHODS
 - Added node externs for closure
 - Fixed a memory allocation bug in pthreads code (#3636)
 - Cleaned up some debug assertion messages behind #ifdef ASSERTIONS (#3639)
 - Fixed umask syscall (#3637)
 - Fixed double alignment issue with formatStrind and emscripten_log (#3647)
 - Added new EXTRA_EXPORTED_RUNTIME_METHODS build option
 - Updated emrun to latest version
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.3...1.34.4
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.34.3...1.34.4
    - Emscripten-Clang: no changes.

v1.34.3: 7/15/2015
------------------
 - Move libc to musl+syscalls
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.2...1.34.3
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.34.2: 7/14/2015
------------------
 - Upgrade to new SIMD.js polyfill version and improved SIMD support.
 - Improved WebGL support in --proxy-to-worker mode (#3569)
 - Removed warning on unimplemented JS library functions
 - Fix WebGL 2 support with closure compiler
 - Fixed an issue with WebRTC support (#3574)
 - Fixed emcc to return a correct error process exit code when invoked with no input files
 - Fixed a compiler problem where global data might not get aligned correctly for SIMD.
 - Fixed a LLVM backend problem which caused recursive stack behavior when linking large codebases, which was seen to cause a stack overflow crash on Windows.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.1...1.34.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.34.1...1.34.2
    - Emscripten-Clang: no changes.

v1.34.1: 6/18/2015
------------------
 - Fixed an issue with resize canvas not working with GLFW.
 - Fixed handling of empty else blocks.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.34.0...1.34.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.34.0: 6/16/2015
------------------
 - Fixed an issue when generating .a files from object files that reside on separate drives on Windows (#3525).
 - Added a missing dependency for GLFW (#3530).
 - Removed the Emterpreter YIELDLIST option.
 - Added support for enabling memory growth before the runtime is ready.
 - Added a new feature to store the memory initializer in a string literal inside the generated .js file.
 - Fixed a code miscompilation issue with a constexpr in fcmp.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.33.2...1.34.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.33.2...1.34.0
    - Emscripten-Clang: no changes.

v1.33.2: 6/9/2015
-----------------
 - Added support for OpenAL Extension AL_EXT_float32 (#3492).
 - Added support for handling command line flags -M and -MM (#3518).
 - Fixed a code miscompilation issue with missing ';' character (#3520).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.33.1...1.33.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.33.1...1.33.2
    - Emscripten-Clang: no changes.

v1.33.1: 6/3/2015
-----------------
 - Added support for multithreading with the POSIX threads API (pthreads), used when compiling and linking with the -s USE_PTHREADS=1 flag (#3266).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.33.0...1.33.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.33.0...1.33.1
    - Emscripten-Clang: no changes.

v1.33.0: 5/29/2015
------------------
 - Fix an issue with writing to /dev/null (#3454).
 - Added a hash to objects inside .a files to support to linking duplicate symbol names inside .a files (#2142).
 - Provide extensions ANGLE_instanced_arrays and EXT_draw_buffers as aliases to the WebGL ones.
 - Fixed LLVM/Clang to build again on Windows after previous LLVM upgrade.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.32.4...1.33.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.32.4...1.33.0
    - Emscripten-Clang: no changes.

v1.32.4: 5/16/2015
------------------
 - Update LLVM and Clang to PNaCl's current 3.7 merge point (April 17 2015)
 - Added libpng to Emscripten-ports.
 - Added intrinsic llvm_fabs_f32.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.32.3...1.32.4
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.32.3...1.32.4
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.32.3...1.32.4

v1.32.3: 5/15/2015
------------------
 - Improved dynamic linking support.
 - Added new option to file_packager.py to store metadata externally.
 - Improved CMake support with CMAKE_CROSSCOMPILING_EMULATOR (#3447).
 - Added support for sysconf(_SC_PHYS_PAGES) (#3405, 3442).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.32.2...1.32.3
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.32.2...1.32.3
    - Emscripten-Clang: no changes.

v1.32.2: 5/8/2015
-----------------
 - Removed a (name+num)+num -> name+newnum optimization, which caused heavy performance regressions in Firefox when the intermediate computation wraps around the address space (#3438).
 - Improved dynamic linking support.
 - Improved emterpreter when doing dynamic linking.
 - Fixed an issue with source maps debug info containing zeroes as line numbers.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.32.1...1.32.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.32.1...1.32.2
    - Emscripten-Clang: no changes.

v1.32.1: 5/2/2015
-----------------
 - Removed old deprecated options -s INIT_HEAP, MICRO_OPTS, CLOSURE_ANNOTATIONS, INLINE_LIBRARY_FUNCS, SHOW_LABELS, COMPILER_ASSERTIONS and COMPILER_FASTPATHS.
 - Added support for dynamic linking and dlopen().
 - Fixed a compilation issue that affected -O2 builds and higher (#3430).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.32.0...1.32.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.32.0...1.32.1
    - Emscripten-Clang: no changes.

v1.32.0: 4/28/2015
------------------
 - Compile .i files properly as C and not C++ (#3365).
 - Removed old deprecated options -s PRECISE_I32_MUL, CORRECT_ROUNDINGS, CORRECT_OVERFLOWS, CORRECT_SIGNS, CHECK_HEAP_ALIGN, SAFE_HEAP_LINES, SAFE_HEAP >= 2, ASM_HEAP_LOG, SAFE_DYNCALLS, LABEL_DEBUG, RUNTIME_TYPE_INFO and EXECUTION_TIMEOUT, since these don't apply to fastcomp, which is now the only enabled compilation mode.
 - Preliminary work towards supporting dynamic linking and dlopen().
 - Fixed an issue where emrun stripped some characters at output (#3394).
 - Fixed alignment issues with varargs.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.31.3...1.32.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.31.3...1.32.0
    - Emscripten-Clang: no changes.

v1.31.3: 4/22/2015
------------------
 - Improved support for -E command line option (#3365).
 - Removed the old optimizeShifts optimization pass that was not valid for asm.js code.
 - Fixed an issue when simultaneously using EMULATE_FUNCTION_POINTER_CASTS and EMULATED_FUNCTION_POINTERS.
 - Fixed an issue with -s PRECISE_I64_MATH=2 not working (#3374).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.31.2...1.31.3
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.31.2...1.31.3
    - Emscripten-Clang: no changes.

v1.31.2: 4/20/2015
------------------
 - Added support for file suffixes .i and .ii (#3365).
 - Fixed an issue with embind and wide strings (#3299).
 - Removed more traces of the old non-fastcomp compiler code.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.31.1...1.31.2
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.31.1: 4/17/2015
------------------
 - Added support for unicode characters in EM_ASM() blocks (#3348).
 - Removed the pointer masking feature as experimental and unsupported.
 - Fixed an issue where exit() did not terminate execution of Emterpreter (#3360).
 - Removed traces of the old non-fastcomp compiler code.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.31.0...1.31.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.31.0...1.31.1
    - Emscripten-Clang: no changes.

v1.31.0: 4/14/2015
------------------
 - Remove references to unsupported EMCC_FAST_COMPILER mode, fastcomp is always enabled (#3347).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.30.6...1.31.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.30.6...1.31.0
    - Emscripten-Clang: no changes.

v1.30.6: 4/14/2015
------------------
 - Removed support for the deprecated jcache functionality (#3313).
 - Added support to emscripten_GetProcAddress() to fetch symbols with the ANGLE suffix (#3304, #3315).
 - Added immintrin.h header file to include all SSE support.
 - Added an async option to ccall (#3307).
 - Stopped from using 0 as a valid source ID for OpenAL (#3303).
 - When project has disabled exception catching, build an exceptions-disabled version of libcxx.
 - Split libcxx into two parts to optimize code size for projects that only need small amount of libcxx (#2545, #3308).
 - Avoid fprintf usage in emscripten_GetProcAddress() to allow using it with -s NO_FILESYSTEM=1 (#3327).
 - Removed old deprecated functionalities USE_TYPED_ARRAYS, FHEAP, GC emulation and non-asmjs-emscripten ABI.
 - Don't refer to prefixed GL extensions when creating a GL context (#3324).
 - Removed support code for x86_fp80 type (#3341).
 - Optimize EM_ASM() calls even more (#2596).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.30.5...1.30.6
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.30.5...1.30.6
    - Emscripten-Clang: no changes.

v1.30.5: 4/7/2015
-----------------
 - Fixed WebIDL operation when closure is enabled after the previous EM_ASM() optimizations.
 - Optimized jsCall() to handle variadic cases of number of arguments faster (#3290, #3305).
 - Removed support for the getwd() function (#1115, #3309).
 - Fixed a problem with -s IGNORED_FUNCTIONS and -s DEAD_FUNCTIONS not working as expected (#3239).
 - Fixed an issue with -s EMTERPRETIFY_ASYNC=1 and emscripten_sleep() not working (#3307).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.30.4...1.30.5
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.30.4...1.30.5
    - Emscripten-Clang: no changes.

v1.30.4: 4/3/2015
-----------------
 - Optimized the performance and security of EM_ASM() blocks by avoiding the use of eval() (#2596).
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.30.3...1.30.4
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.30.3...1.30.4
    - Emscripten-Clang: no changes.

v1.30.3: 4/3/2015
-----------------
 - Improved error handling in library_idbstore.js.
 - Fixed an asm.js validation issue with EMULATE_FUNCTION_POINTER_CASTS=1 feature (#3300).
 - Fixed Clang build by adding missing nacltransforms project after latest LLVM/Clang upstream merge.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.30.2...1.30.3
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.30.2...1.30.3
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.30.2...1.30.3

v1.30.2: 4/1/2025
-----------------
 - Added support to writing to mmap()ed memory by implementing msync() (#3269).
 - Updated SDL2 port to version 7.
 - Exported new singleton function Module.createContext() for creating a GL context from SDL2.
 - Added support for asm.js/Emscripten arch in Clang.
 - Finished LLVM 3.6 upgrade merge.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.30.1...1.30.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.30.1...1.30.2
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.30.1...1.30.2

v1.30.1: 3/24/2015
------------------
 - Upgraded LLVM+Clang from vrsion 3.5 to version 3.6.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.30.0...1.30.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.30.0...1.30.1
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.30.0...1.30.1

v1.30.0: 3/24/2015
------------------
 - Fixed a bug where html5.h API would not remove event handlers on request.
 - Fixed a regression issue that broke building on Windows when attempting to invoke tools/gen_struct_info.py.
 - Improved memory growth feature to better handle growing to large memory sizes between 1GB and 2GB (#3253).
 - Fixed issues with emrun with terminating target browser process, managing lingering sockets and command line quote handling.
 - Fixed a bug where unsigned integer return values in embind could be returned as signed (#3249).
 - Improved handling of lost GL contexts.
 - Changed malloc to be fallible (return null on failure) when memory growth is enabled (#3253).
 - Fixed a bug with WebIDL not being able to handle enums (#3258).
 - Updated POINTER_MASKING feature to behave as a boolean rather than a mask (#3240).
 - Improved "emcmake cmake" on Windows to automatically remove from path any entries that contain sh.exe in them, which is not supported by CMake.
 - Fixed an issue with symlink handling in readlink (#3277).
 - Updated SDL2 port to version 6.
 - Removed the obsolete FAST_MEMORY build option.
 - Added reciprocalApproximation and reciprocalSqrtApproximation SIMD intrinsics.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.12...1.30.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.29.12...1.30.0
    - Emscripten-Clang: no changes.

v1.29.12: 3/15/2015
-------------------
 - Fix a bug where SDL_malloc and SDL_free were not available. (#3247)
 - Fix various issues with emrun usage. (#3234)
 - Fixed a off-by-one memory access in native optimizer.
 - Improve emterpreter support.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.11...1.29.12
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.29.11: 3/11/2015
-------------------
 - Remove the requirement to pass -s PRECISE_F32=1 manually when building with SIMD support.
 - Fix a temp directory leak that could leave behind empty directories in the temp directory after build (#706)
 - Improve support for growable Emscripten heap in asm.js mode.
 - Added a warning message when generating huge asset bundles with file packager.
 - Fixed a bug where emscripten_get_gamepad_status might throw a JS exception if called after a gamepad was disconnected.
 - Improve emterpreter sleep support.
 - Optimize code generation when multiple consecutive bitshifts are present.
 - Optimize redundant stack save and restores, and memcpy/memsets.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.10...1.29.11
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.29.10...1.29.11
    - Emscripten-Clang: no changes.

v1.29.10: 2/19/2015
-------------------
 - Add a warning message when generating code that has a very large number of variables, which optimization flags could remove.
 - Improve support for SIMD casts and special loads.
 - Fix the process return code when using EMCONFIGURE_JS=1.
 - Improved the error message in abort().
 - Fix main loop handling during emterpreter sync save/load.
 - Handle emscripten_async_call and friends during sleep, by pausing all safeSet*() operations.
 - Add support for Google WTF when building with --tracing.
 - Improve emterpreter stability with fuzzing.
 - Add an option to load the memory initializer file from a typed array (#3187)
 - Remove linker warning message when linking to -lm, since Emscripten includes musl that implements the math libraries built-in.
 - Add support for SDL_WM_SetCaption(), which calls to Module['setWindowTitle'], or if not present, sets the web page title. (#3192)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.9...1.29.10
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.29.9...1.29.10
    - Emscripten-Clang: no changes.

v1.29.9: 2/9/2015
-------------------
 - Documented FORCE_ALIGNED_MEMORY to be no longer supported.
 - Fixes issues with native optimizer handling of "if () else {}" statements. (#3129)
 - Improved cross-browser support for EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST. (#3165)
 - Added new linker option --profiling-funcs, which generates output that is otherwise minified, except that function names are kept intact, for use in profilers and getting descriptive call stacks.
 - The Module object is no longer written in global scope. (#3167)
 - Added new emscripten_idb_* API. (#3169)
 - Added new function emscripten_wget_data().
 - Add support for GL_RED with GLES3/WebGL2. (#3176)
 - Added basic WebVR support. (#3177)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.8...1.29.9
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.29.8: 1/31/2015
-------------------
 - Fix a temp file leak with emterpreter. (#3156)
 - Fix a typo that broke glBlitFramebuffer. (#3159)
 - Added scandir() and alphasort() from musl. (#3161)
 - Add a warning if multiple .a files with same basename are being linked together. (#2619)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.7...1.29.8
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.29.7...1.29.8
    - Emscripten-Clang: no changes.

v1.29.7: 1/28/2015
-------------------
 - Fixed an issue with backwards compatibility in emscripten-ports. (#3144)
 - Warn on duplicate entries in archives. (#2619)
 - Removed the MAX_SETJMPS limitation to improve setjmp/longjpmp support. (#3151)
 - Improve the native optimizer to not emit empty if clauses in some cases. (#3154)
 - Optimize Math.clz32, Math.min, NaN, and inf handling in asm.js.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.6...1.29.7
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.29.6...1.29.7
    - Emscripten-Clang: no changes.

v1.29.6: 1/23/2015
-------------------
 - Fixed an issue where calling glGen*() when the GL context was lost might throw a JS exception, instead a GL_INVALID_OPERATION is now recorded.
 - Improve label handling in native optimizer.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.5...1.29.6
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.29.5: 1/23/2015
-------------------
 - Enable compiling source files with the extension ".c++".
 - Enable versioning of the emscripten ports so that older Emscripten versions can keep using older versions of the ports (#3144)
 - Added a whitelist option to emterpreter, a linker flag of form -s EMTERPRETIFY_WHITELIST=["symbol1","symbol2"]. (#3129)
 - Improved emscripten_get_pointerlock_status() to always fill the output structure even when pointer lock is not supported.
 - Added an environment variable EMCC_NO_OPT_SORT=0/1 option to configure whether the generated output should have the functions sorted by length, useful for debugging.
 - Added new tool tools/merge_pair.py which allows bisecting differences between two output files to find discrepancies.
 - Improved parsing in cashew.
 - Improved output message from emconfigure and emmake when inputs are unexpected.
 - Added built-in asm handler for LLVM fabs operation.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.4...1.29.5
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.29.4...1.29.5
    - Emscripten-Clang: no changes.

v1.29.4: 1/21/2015
-------------------
 - Added new C <-> JS string marshalling functions asciiToString(), stringToAscii(), UTF8ToString(), stringToUTF8() that can be used to copy strings across the JS and C boundaries. (#2363)
 - Added new functions lengthBytesUTF8(), lengthBytesUTF16() and lengthBytesUTF32() to allow computing the byte lengths of strings in different encodings. (#2363)
 - Upgraded SDL2 port to version 4.
 - Add support for saving the emterpreter stack when there are functions returning a value on the stack (#3129)
 - Notice async state in emterpreter trampolines (#3129)
 - Optimize SDL1 pixel copying to the screen.
 - Fixed an issue with emterpreter parsing. (#3141)
 - Fixed an issue with native optimizer and -s PPRECISE_F32=1.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.3...1.29.4
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.29.3...1.29.4
    - Emscripten-Clang: no changes.

v1.29.3: 1/16/2015
-------------------
 - Fixed a bug with OpenGL context initialization enableExtensionsByDefault. (#3135)
 - Fixed an issue with nested if parsing in native optimizer.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.2...1.29.3
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.29.2: 1/16/2015
-------------------
 - Fixed an issue with embind compilation in LLVM 3.5.
 - Fixed an issue with SDL audio queueing stability, which would queue audio too eagerly and cause stutter in some applications (#3122, #3124)
 - Enabled native JS optimizer to be built automatically on Windows, requires VS2012 or VS2013. 
 - Improve error message to reflect the fact that DLOPEN_SUPPORT is currently not available (#2365)
 - Improve SIMD load and store support.
 - Upgraded SDL2 port to version 3.
 - Fix a bug with native JS optimizer and braces in nested ifs.
 - Improved emterpreter support.
 - Fixed LLVM 3.5 to build with Visual Studio on Windows (emscripten-fastcomp #61)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.1...1.29.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.29.1...1.29.2
    - Emscripten-Clang: no changes.

v1.29.1: 1/7/2015
-------------------
 - Migrated to upstream PNaCl LLVM+Clang 3.5 from the previous 3.4.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.29.0...1.29.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.29.0...1.29.1
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.29.0...1.29.1

v1.29.0: 1/7/2015
-------------------
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.28.3...1.29.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.28.3...1.29.0
    - Emscripten-Clang: no changes.

v1.28.3: 1/4/2015
-------------------
 - embuilder.py tool
 - Many fixes for native optimizer on Windows
 - Perform LLVM LTO in a separate invocation of opt, so that it does not mix with legalization and other stuff we do at link time
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.28.2...1.28.3
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.28.2...1.28.3
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.28.2...1.28.3

v1.28.2: 12/17/2014
-------------------
 - Enable native optimizer by default
 - Disable slow2asm legacy testing (asm.js mode in pre-fastcomp)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.28.1...1.28.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.28.1...1.28.2
    - Emscripten-Clang: no changes.

v1.28.1: 12/15/2014
-------------------
 - Use a lot more MUSL math functions
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.28.0...1.28.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.28.0...1.28.1
    - Emscripten-Clang: no changes.

v1.28.0: 12/12/2014
-------------------
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.27.2...1.28.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.27.2...1.28.0
    - Emscripten-Clang: no changes.

v1.27.2: 12/10/2014
-------------------
 - Added more complete support for SSE1 SIMD intrinsics API. (#2792)
 - Fixed an issue with glTexImage2D on GL_LUMINANCE + GL_FLOAT textures. (#3039)
 - Use the cashew asm.js parser in native optimizer.
 - Fixed issues with IE when running closure minified pages. (#3012)
 - Enabled asm.js validation for SIMD compilation.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.27.1...1.27.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.27.1...1.27.2
    - Emscripten-Clang: no changes.

v1.27.1: 11/20/2014
-------------------
 - Migrated to upstream PNaCl LLVM+Clang 3.4 from the previous 3.3.
 - Added a FindOpenGL.cmake to support find_package() for OpenGL in CMake scripts.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.27.0...1.27.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.27.0...1.27.1
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.27.0...1.27.1

v1.27.0: 11/20/2014
-------------------
 - Added new work in progress option -s NATIVE_OPTIMIZER=1 that migrates optimizer code from JS to C++ for better performance.
 - Fixed an embind issue when compiling with closure (#2974)
 - Fixed an embind issue with unique_ptr (#2979)
 - Fixed a bug with new GL context initialization in proxy to worker mode.
 - Fixed an issue where GL context event handlers would leak after a GL context has been freed.
 - Optimized embind operation in Chrome by avoiding using Function.prototype.bind().
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.26.1...1.27.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.26.1...1.27.0
    - Emscripten-Clang: no changes.

v1.26.1: 11/7/2014
------------------
 - Fixed emscripten::val handle for special js values (#2930)
 - Implemented SDL 1.2 SDL_SetClipRect / SDL_GetClipRect (#2931)
 - Added support for building zlib from Emscripten Ports with linker flag -s USE_ZLIB=1.
 - Improved experimental GLES3 support.
 - Fixed issues with llseek (#2945)
 - Enable using emscripten_get_now() in web workers (#2953)
 - Added stricter input data validation in GL code.
 - Added new HTML5 C API for managing fullscreen mode transitions to resolve cross-browser issue #2556 (#2975)
 - Fixed an issue with using structs in va_args (#2923)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.26.0...1.26.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.26.0...1.26.1
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.26.0...1.26.1

v1.26.0: 10/29/2014
-------------------
 - Fixed an issue where emar would forward --em-config to llvm-ar (#2886)
 - Added a new "emterpreter" feature that allows running Emscripten compiled code in interpreted form until asm.js compilation is ready (-s EMTERPRETIFY=1).
    - For more information, see https://groups.google.com/d/msg/emscripten-discuss/vhaPL9kULxk/_eD2G06eucwJ
 - Added new "Emscripten Ports" architecture that enables building SDL2 with -s USE_SDL=2 command line flag.
 - Added support for SDL 1.2 SDL_CreateRGBSurfaceFrom() function.
 - Improved experimental SIMD support.
 - Use only minimum necessary digits to print floating point literals in generated JS code for smaller code output.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.25.2...1.26.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.25.2...1.26.0
    - Emscripten-Clang: no changes.

v1.25.2: 10/16/2014
-------------------
 - Fixed a bug in tmpfile() function not allocating the mode argument correctly.
 - Fixed a bug with handling empty files in IDBFS (#2845)
 - Added an implementation of the utimes() function (#2845)
 - Added experimental WebGL 2.0 support with the linker flag -s USE_WEBGL2=1. (#2873)
 - Fixed a UnboundTypeError occurring in embind (#2875)
 - Fixed an error "IndexSizeError: Index or size is negative or greater than the allowed amount" being thrown by Emscripten SDL 1.2 surface blit code. (#2879)
 - Fixed a JS minifier issue that generated "x--y from x - -y" (#2869)
 - Added a new emcc command line flag "--cache <dir>" to control the location of the Emscripten cache directory (#2816)
 - Implemented SDL_ConvertSurface() and added support for SDL_SRCALPHA in SDL_SetAlpha (#2871)
 - Fixed issues with the GL library handling of invalid input values.
 - Optimized SDL copyIndexedColorData function (#2890)
 - Implemented GLES3 emulation for glMapBufferRange() for upcoming WebGL 2 support, using the -s FULL_ES3=1 linker option.
 - Fixed a bug where setting up and cancelling the main loop multiple times would stack up the main loop to be called too frequently (#2839)
 - Introduced a new API emscripten_set_main_loop_timing() for managing the Emscripten main loop calling frequency (#2839)
 - Added new optimization flags SDL.discardOnLock and SDL.opaqueFrontBuffer to Emscripten SDL 1.2 SDL_LockSurface() and SDL_UnlockSurface() (#2870)
 - Fixed a bug with glfwGetProcAddress().
 - Added option to customize GLOBAL_BASE (the starting address of global variables in the Emscripten HEAP).
 - Added the ability to register mouseover and mouseout events from the HTML5 API.
 - Improved experimental SIMD support.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.25.1...1.25.2
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.25.1: 10/1/2014
------------------
 - Updated heap resize support code when -s ALLOW_MEMORY_GROWTH=1 is defined.
 - Updated libc++ to new version from upstream svn revision 218372, 2014-09-24.
 - Fixed a bug where building on Windows might generate output JS files with incorrect syntax (emscripten-fastcomp #52)
 - Improved experimental SIMD support.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.25.0...1.25.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.25.0...1.25.1
    - Emscripten-Clang: no changes.


v1.25.0: 9/30/2014
------------------
 - Fixed a warning message with -s EXPORTED_FUNCTIONS.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.24.1...1.25.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.24.1: 9/27/2014
------------------
 - Fixed issues with the tmpnam and tmpfile functions (#2797, 2798)
 - Fixed CMake package find code to not search any system directories, because Emscripten is a cross-compiler.
 - Improved support for the proposed solution for heap resizing.
 - Fixed an issue where one could not run a main loop without having first a GL context created when -s FULL_ES2 or -s LEGACY_GL_EMULATION were set.
 - For compatibility, Emscripten will no longer warn about missing library files for -lGL, -lGLU and -lglut libraries, since Emscripten provides the implementation for these without having to explicitly link to anything.
 - Added support for readonly (const) attributes and automatically call Pointer_stringify on DOMStrings in WebIDL.
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Added support for GLFW 3.0.
 - Added new Emscripten HTML 5 functions emscripten_set_mouseenter_callback() and emscripten_set_mouseleave_callback().
 - Emscripten now recognizes an environment variable EMCC_JSOPT_BLACKLIST=a,b,c,d which can be used to force-disable Emscripten to skip running specific JS optimization passes. This is intended as a debugging aid to help zoom in on JS optimizer bugs when compiling with -O1 and greater. (#2819)
 - Fixed a bug where Module['TOTAL_STACK'] was ignored (#2837).
 - Improved SIMD support for the experimental Ecmascript SIMD spec. Preliminary asm.js validation.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.24.0...1.24.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.24.0...1.24.1
    - Emscripten-Clang: no changes.

v1.24.0: 9/16/2014
------------------
 - Renamed the earlier Module.locateFilePackage() to Module.locateFile() added in v1.22.2 to better reflect its extended usage.
 - Improved exceptions support with exception_ptr.
 - Fixed a bug where restoring files from IDBFS would not preserve their file modes.
 - Fixed and issue where one could not pass a null pointer to strftime() function.
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.23.5...1.24.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.23.5...1.24.0
    - Emscripten-Clang: no changes.

v1.23.5: 9/12/2014
------------------
 - Added new functions emscripten_get_device_pixel_ratio(), emscripten_set_canvas_css_size() and emscripten_get_canvas_css_size() which allow handling High DPI options from C code.
 - Fixed bugs with timzone-related functions in the JS-implemented C standard library.
 - Implemented clock_gettime(CLOCK_MONOTONIC) and added a new function emscripten_get_now_is_monotonic() to query whether the JS-provided timer is monotonic or not.
 - Fixed an issue where the user could not pass --llvm-opts=xxx when also specifying --llvm-lto=2.
 - Renamed the linker option -profiling to --profiling for consistency. The old form is still supported.
 - Formalized the set of valid characters to be used in files passed to the file_packager.py (#2765).
 - Implemented SDL function SDL_BlitScaled.
 - Fixed a bug with right modifier keys in SDL.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.23.4...1.23.5
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.23.4: 9/7/2014
------------------
 - Implemented new targetX and targetY fields for native HTML5 mouse and touch events (#2751)
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.23.3...1.23.4
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.23.3...1.23.4
    - Emscripten-Clang: no changes.

v1.23.3: 9/7/2014
------------------
 - Removed the scons-tools SCons build system as unused.
 - Fixed an issue where applications could not handle WebGL context creation failures gracefully.
 - Fixed a bug where the stringToC function in ccall/cwrap might not allocate enough space to hold unicode strings.
 - Removed CMake from attempting to link to library -ldl when building projects, by unsetting CMAKE_DL_LIBS.
 - Fixed a bug where write_sockaddr might return undefined data in its output structure.
 - Added a new _experimental_ -s POINTER_MASKING=1 linker option that might help JS VMs to optimize asm.js code.
 - Added first version of a memory tracing API to profile memory usage in Emscripten applications.
 - Added functions glob and globfree from musl regex library.
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.23.2...1.23.3
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.23.2...1.23.3
    - Emscripten-Clang: no changes.

v1.23.2: 9/2/2014
------------------
 - Adjusted the process and group ids reported by the stub library functions to be closer to native unix values.
 - Set stack to be aligned to 16 bytes. (#2721)
 - Fixed a compiler error "unresolved symbol: __cxa_decrement_exception_refcount" (#2715)
 - Added a new warning message that instructs that building .so, .dll and .dylib files is not actually supported, and is faked for compatibility reasons for existing build chains. (#2562)
 - Fixed problems with SDL mouse scrolling (#2643)
 - Implemented OpenAL function alSourceRewind.
 - Removed several old header files from the Emscripten repository that had been included for emulation purposes (zlib.h, png.h, tiff.h, tiffio.h), but their implementation is not included.
 - Work around an issue in d8 with binary file reading that broke e.g. printf when running in d8. (#2731)
 - Rigidified the semantics of Module.preRun and Module.postRun: These must always be JS arrays, single functions are not allowed (#2729)
 - Improved compiler warning diagnostics when generating output that will not validate as asm.js (#2737)
 - Updated to latest emrun version to enable support for passing arguments with hyphens to the program. (#2742)
 - Added Bessel math functions of the first kind  (j0, j1, jn) from musl.
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.23.1...1.23.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.23.1...1.23.2
    - Emscripten-Clang: no changes.

v1.23.1: 8/26/2014
------------------
 - Add support for the Chrome variant of the Gamepad API.
 - Updates to SIMD.js support.
 - Implemented glutSetCursor function.
 - Added new link-time options -s NO_FILESYSTEM=1 and -s NO_BROWSER=1 to enable reducing output file sizes when those functionalities are not necessary.
 - Added a new option --closure 2 to allow running closure even on the asm.js output.
 - Fixed a regression bug that broke the use of emscripten_set_socket_error_callback() in emscripten.h
 - Removed the support for old discontinued Mozilla Audio Data API in src/library_sdl.js.
 - Removed the support for using Web Audio ScriptProcessorNode to stream audio.
 - Improved SDL audio streaming by using the main rAF() callback instead of a separate setTimeout() callback to schedule the audio data.
 - Deprecated compiling without typed arrays support. 
 - Migrated to using musl PRNG functions. Fixes reported bugs about the quality of randomness (#2341)
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.23.0...1.23.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.23.0...1.23.1
    - Emscripten-Clang: no changes.

v1.23.0: 8/21/2014
------------------
 - Added support for array attributes in WebIDL bindings.
 - Allow cloning pointers that are scheduled for deletion in embind, and add support for null in embind_repr().
 - Fixed possible issues with rounding and flooring operations.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.22.2...1.23.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.22.2: 8/19/2014
------------------
 - Adds stack overflow checks when building with the link flag -s ASSERTIONS=1.
 - Fix an issue where EM_ASM was not usable with closure when closure removed the Module object (#2639)
 - The locale "POSIX" is now recognized (#2636)
 - Fixed a problem with embind on IE11.
 - Added OpenAL functions alSource3i, alListener3f, alGetEnumValue and alSpeedOfSound and also recognize ALC_MAX_AUXILIARY_SENDS.
 - Fixed an issue where emcc would create .o files in the current directory when compiling multiple code files simultaneously (#2644)
 - The -s PROXY_TO_WORKER1= option now looks for a GET option "?noProxy" in the page URL to select at startup time whether proxying should be on or off.
 - Added new functions emscripten_yield, emscripten_coroutine_create and emscripten_coroutine_next which implement coroutines when building with the -s ASYNCIFY=1 option.
 - Optimized the size of intermediate generated .o files by omitting LLVM debug info from them when not needed. (#2657)
 - Fixed WebSocket connection URLs to allow a port number in them, e.g. "server:port/addr" (2610)
 - Added support for void* to the WebIDL binder, via the identifier VoidPtr.
 - Optimize emcc to not copy bitcode files around redundantly.
 - Fix stat() to correctly return ENOTDIR when expected (#2669).
 - Fixed issues with nested exception catching (#1714).
 - Increased the minimum size of the Emscripten HEAP to 64k instead of a previous 4k.
 - The {{{ cDefine('name') }}} macros now raise a compile-time error if the define name is not found, instead of hiding the error message inside the compiled output (#2672)
 - Fixed an issue where --emrun parameter was not compatible with the -s PROXY_TO_WORKER=1 option.
 - Improved WebGL support when compiling with the PROXY_TO_WORKER=1 option.
 - Fixed a regression issue with the handling of running dtors of classes that use virtual inheritance. (#2682)
 - Added an option Module.locateFilePackage() as a means to customize where data files are found in relative to the running page (#2680). NOTE: This parameter was later renamed to Module.locateFile() instead in release 1.24.0.
 - Fixed a bug where OpenAL sources would not properly delete.
 - Fixed a bug with upstream libc++ on std::map, std::multimap and std::unordered_map self-assignment (http://llvm.org/bugs/show_bug.cgi?id=18735)
 - Allow using __asm__ __volatile__("": : :"memory") as a compile-time reordering barrier (#2647)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.22.1...1.22.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.22.1...1.22.2
    - Emscripten-Clang: no changes.

v1.22.1: 8/7/2014
------------------
 - Added support for prefixing functions with '$' in JS libraries, in order to cause them not be prefixed with '_' when compiling.
 - Improved WebIDL compiler to support enums.
 - Fixed a bug with emscripten_force_exit() that would throw an exception (#2629).
 - Fixed setlocale() when setting a bad locale. (#2630)
 - Fixed a compiler miscompilation bug when optimizing loops. (#2626)
 - Fixed an issue with rethrowing an exception (#2627)
 - Fixed a bug where malloc()ing from JS code would leak memory if the C/C++ side does not use malloc() (#2621)
 - Removed an unnecessary assert() in glReadPixels, and improved it to support more texture pixel types.
 - Fixed a bug with std::locale accepting unknown locale names (#2636)
 - Added support for WebIDL binder to work with Closure (#2620)
 - Added no-op SDL IMG_Quit() and TTF_Quit() symbols.
 - Migrated to building libcxx and libcxxapi with -Oz optimization flags.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.22.0...1.22.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.22.0: 8/5/2014
------------------
 - Added support to emrun to dump files to the local filesystem for debugging purposes.
 - Implemented emscripten_wget in ASYNCIFY mode.
 - Improved extension catching support (#2616)
 - Fixed .a link groups to also work when linking to bitcode. (#2568)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.10...1.22.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.21.10...1.22.0
    - Emscripten-Clang: no changes.

v1.21.10: 7/29/2014
-------------------
 - Fixed a Windows-specific issue where the generated output files might contain line endings of form \r\r\n. This caused browser debuggers to get confused with line numbers. (#2133)
 - Improved the node.js workaround introduced in v1.21.8.
 - Implemented new HTML5 API for direct WebGL context creation, emscripten_webgl_*().
 - Fixed a bug when loading in node.js and loaded by another module (#2586)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.9...1.21.10
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.9: 7/28/2014
------------------
 - Fixed issues with exception catching. (#2531)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.8...1.21.9
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.8: 7/28/2014
------------------
 - Fixed an issue when using --embed-file to embed very large files.
 - Worked around a Windows node.js bug where the compiler output might get cut off when the compilation ends in an error. (https://github.com/joyent/node/issues/1669)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.7...1.21.8
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.21.7...1.21.8
    - Emscripten-Clang: no changes.

v1.21.7: 7/25/2014
------------------
 - Added new link option -s EMCC_ONLY_FORCED_STDLIBS which can be used to restrict to only linking to the chosen set of Emscripten-provided libraries. (See also -s EMCC_FORCE_STDLIBS)
 - Adjusted argv[0] and environment variables USER, HOME, LANG and _ to report a more convenient set of default values. (#2565)
 - Fixed an issue where the application could not use environ without also referring to getenv() (#2557)
 - Fixed an issue with IDBFS running in web workers.
 - Print out an error if IDBFS is used without IDB support.
 - Fixed calling Runtime.getFuncWrapper() when -s ALIASING_FUNCTION_POINTERS=1 (#2010)
 - Fixed an issue where deleting files during directory iteration would produce incorrect iteration results (#2528)
 - Fixed support for strftime with %z and %Z (#2570)
 - Fixed a bug with truncate() throwing an exception (#2572)
 - Improved the linker to generate warning messages if user specifies -s X=Y linker flags that do not exist (#2579)
 - Fixed an issue with creating read-only files (#2573)
 - Added first implementation for the ASYNCIFY option, which splits up synchronous blocking loops to asynchronous execution. For more information on this approach, see https://github.com/kripken/emscripten/wiki/Asyncify
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.6...1.21.7
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.21.6...1.21.7
    - Emscripten-Clang: no changes.

v1.21.6: 7/22/2014
------------------
 - Separated OpenAL AL and ALC errors to properly separate fields.
 - When using EGL to initialize a GL context, initialize a stencil buffer to the context as well, since proper EGL context choosing is not yet implemented.
 - Added new linker flag -s DEMANGLE_SUPPORT to choose whether to compile the application with libcxxabi-provided demangling support ___cxa_demangle().
 - Fixed a problem where calling stat() on a nonexisting file in the runtime VFS would result in an exception being thrown. (#2552)
 - When using the -v flag, no longer retain intermediate compilation files. To preserve the intermediate files, set the EMCC_DEBUG=1 environment variable. (#2538)
 - Added a new HTML setting Module.memoryInitializerPrefixURL which specifies a prefix for where the memory initializer file .mem.js should be loaded from (#2542)
 - Implemented eglReleaseThread to work according to spec.
 - Implemented a new function emscripten_force_exit() which immediately shuts down the C runtime.
 - Fixed a bug with exception handling that resulted in an error unresolved symbol: _ZTISt13bad_exception (#2560)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.5...1.21.6
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.5: 7/21/2014
------------------
 - Added support for glDrawBuffers with the WEBGL_draw_buffers extension.
 - Added stub implementation for eglReleaseThread.
 - Fixed a bug where passing -E to emcc used the system include headers instead of the built-in ones. (#2534)
 - Fixed the stacktrace() function to work on MSIE as well.
 - Removed the zlib.h header file from system include directory, since Emscripten does not provide an implementation of zlib built-in.
 - Added support for __cxa_bad_typeid (#2547)
 - Fixed an internal compiler crash with a certain pattern involving optimized builds and int64_t (#2539)
 - Fixed an issue with -s EXCEPTION_CATCHING_WHITELIST handling where an extension that was a substring of another might get erroneously handled.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.4...1.21.5
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.21.4...1.21.5
    - Emscripten-Clang: no changes.

v1.21.4: 7/17/2014
------------------
 - Implemented the getsockopt() function.
 - Added new event callback functions emscripten_set_socket_xx_callback() that allow listening to WebSocket events in an asynchronous manner.
 - Greatly improved CMake support, now various forms of configure-time test builds are supported, and the default extension is set to ".js"
 - Prohibit the virtual filesystem from creating files with name '.' or '..' at runtime.
 - Have runtime mkdir() function call normalize the path to be created before creation.
 - Fixed an issue with omitting the third paramter in cwrap() call (#2511).
 - Fixed an issue where mouse event handling would throw an exception if the page did not contain a canvas object.
 - Fixed a GL initialization problem when user has extended Array with custom functions (#2514)
 - Added new compiler defines __EMSCRIPTEN_major__, __EMSCRIPTEN_minor__ and __EMSCRIPTEN_tiny__ which communicate the compiler version major.minor.tiny to compiled applications (#2343)
 - Fixed a bug where emrun did not properly capture the exit code when exit runtime via not calling exit().
 - Fixed an error message when symlinkin invalid filenams at runtime.
 - Fixed a bug in EGL context creation that parsed the input context creation parameters with wrong terminator.
 - Improved ffdb.py to be smarter when to attempt port forwarding to connect to a FFOS device DevTools port.
 - Implemented strsignal() function (#2532)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.3...1.21.4
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.3: 7/10/2014
------------------
 - Added implementations for SDL function SDL_AudioQuit and SDL_VideoQuit.
 - Fix an issue with the optimizeShifts optimization enabled in previous version.
 - Fixed the -s RELOOPER command line parameter to work.
 - Fixed a bug where building the system libc migt result in a compiler deadlock on Windows.
 - Removed emcc from trying to link in .dll files as static libraries on Windows.
 - Added support for GL_HALF_FLOAT_OES.
 - Fixed a bug where emcmake did not work on Windows.
 - Use multithreaded compilation to build libc.
 - Fixed an issue where the GL interop library could throw an exception in an error condition, instead of raising a GL error.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.2...1.21.3
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.2: 7/5/2014
------------------
 - Improved the checks that detect that code is run only while the runtime is initialized.
 - The memory initializer file (.mem.js) is now emitted by default when compiling with at least -O2 optimization level.
 - Fixed a performance issue where built-in math functions (Math.sqrt, etc.) took a slightly slower path (#2484).
 - Added support for the ffs libc function.
 - Re-enabled optimizeShifts optimization when not compiling for asm.js (#2481)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.1...1.21.2
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.1: 7/3/2014
------------------
 - Fixed an issue where wrong python interpreter could get invoked on Windows when both native and cygwin python were installed.
 - Updated musl from version 0.9.13 to version 1.0.3.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.21.0...1.21.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.0: 7/2/2014
------------------
 - Enable memory init files (.mem) by default in optimized builds (-O2+), as if  --memory-init-file 1  is specified. This makes the default behavior on optimized builds emit smaller and faster-to-load code, but does require that you ship both a .js and a .mem file (if you prefer not to, can use  --memory-init-file 1  ).
 - Implemented new SDL 1.2 functions SDL_GetRGB, SDL_GetRGBA and SDL_putenv.
 - Added support for /dev/random, /dev/urandom and C++11 std::random_device, which will use cryptographically secure random api if available. (#2447)
 - Added support for CMake find_path() directive.
 - Added support for std::unique_ptr in embind.
 - Improved Windows support for ffdb.py.
 - Implemented the clip_rect structure for created SDL surfaces.
 - Fixed a regression with SDL touch events (#2466)
 - Added support for C++11 std::thread::hardware_concurrency which backs to navigator.hardwareConcurrency. See http://wiki.whatwg.org/wiki/Navigator_HW_Concurrency (#2456)
 - Optimized embind code generation with constexprs.
 - Enabled the use of Runtime.add&removeFunction when closure minification is active (#2446)
 - Implemented support for accessing WebGL when building via the proxy to worker architecture.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.20.0...1.21.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.20.0: 6/13/2014
------------------
 - Optimize in-memory virtual filesystem performance when serialized to an IndexedDB.
 - Fixed memcpy regression with ta0 and ta1 modes.
 - Fixed an issue with line numbers being messed up when generating source maps (#2410)
 - Fixed an ffdb logging bug that could cause it to drop messages if they were being received too fast. Added support getting memory and system descriptions with ffdb.
 - Added a new extension to SDL "emscripten_SDL_SetEventHandler()" which enabled application to perform SDL event handling inside a JS event handler to overcome browser security restrictions. (#2417)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.19.2...1.20.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.19.2: 6/9/2014
------------------
 - Updated CMake support for response file handling.
 - Fixed issues with glfwGetProcAddress and glfwSetWindowSizeCallback.
 - Fixed an issue with regexes that caused issues on IE11 runtime (#2400)
 - Added a new functions emscripten_get_preloaded_image_data() and emscripten_get_preloaded_image_data_from_FILE() to obtain pixel data of preloaded images.
 - Greatly improved ffdb capabilities to operate a FFOS device.
 - Fixed a Windows-specific bug where the user temp directory was littered with temporary .rsp files that did not get cleaned up.
 - Improved SIMD support.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.19.1...1.19.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.19.1...1.19.2
    - Emscripten-Clang: no changes.

v1.19.1: 6/3/2014
------------------
 - Migrate to using musl sscanf and sprintf and the family that writes to memory, and not directly to the filesystem.
 - Improve the error messages from -s SAFE_HEAP_ACCESS=1 runtime checks.
 - Added new linker flag -s NO_DYNAMIC_EXECUTION=1 which removes the use of eval() and new Function() in the generated output. For more information, see "Eval and related functions are disabled" in https://developer.chrome.com/extensions/contentSecurityPolicy .
 - Fixed a compiler issue when very large double constants are present. (#2392)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.19.0...1.19.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.19.0: 5/29/2014
------------------
 - Added an error message to signal that linkable modules are not supported in fastcomp.
 - Fixed a miscompilation issue that resulted in an error "SyntaxError: invalid increment operand" and a statement +(+0) being generated (#2314)
 - Make optimized compiler output smaller by running the shell code through uglify when not using closure.
 - Fixed a crash in SDL audio loading code introduced in v1.18.3
 - Fixed an issue where glTex(Sub)Image2D might throw an exception on error, instead of setting glGetError().
 - Added new typedefs emscripten_align1_short, emscripten_align{1/2}_int, emscripten_align{1/2}_float and emscripten_align{1/2/4}_double to ease signaling the compiler that unaligned data is present. (#2378)
 - Fixed an embind issue with refcount tracking on smart pointers.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.18.4...1.19.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.18.4...1.19.0
    - Emscripten-Clang: no changes.

v1.18.4: 5/27/2014
------------------
 - Fixed error message on unsupported linking options (#2365)
 - Updated embind to latest version from IMVU upstream.
 - Fixed an issue where source maps did not load properly in Firefox.
 - Added a more descriptive error message to fastcomp when MAX_SETJMPS limit is violated. (#2379)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.18.3...1.18.4
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.18.3...1.18.4
    - Emscripten-Clang: no changes.

v1.18.3: 5/21/2014
------------------
 - Added support to emcc command line for "archive groups": -Wl,--start-group and -Wl,--end-group
 - Greatly optimized ccall and cwrap implementations.
 - Added new support for SDL_Mix backend to use WebAudio to play back audio clips.
 - Fixed a registerizeHarder issue with elimination of conditional expressions.
 - Migrated single-character standard C functions (islower, tolower, and the family) to use musl implementations.
 - Updated relooper to not optimize out breaks if it causes excessive nesting.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.18.2...1.18.3
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.18.2...1.18.3
    - Emscripten-Clang: no changes.

v1.18.2: 5/19/2014
------------------
 - Fixed a problem which blocked user applications from handling WebGL context loss events themselves.
 - Added a new HTML5 api function emscripten_is_webgl_context_lost() which allows polling for context loss in addition to receiving events.
 - Improved async wget progress events to work better across browsers.
 - Improved WebIDL binder support.
 - Added new typeof() function to emscripten::val.
 - Added support for SDL window events SDL_WINDOWEVENT_FOCUS_GAINED, SDL_WINDOWEVENT_FOCUS_LOST, SDL_WINDOWEVENT_SHOWN, SDL_WINDOWEVENT_HIDDEN.
 - Fixed a compiler miscompilation on unsigned i1 bitcasts (#2350)
 - Fixed a compiler bug where doubles in varargs might not get 8-byte aligned (#2358)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.18.1...1.18.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.18.1...1.18.2
    - Emscripten-Clang: no changes.

v1.18.1: 5/12/2014
------------------
 - Fixed an issue where the mouse wheel scroll did not work with SDL.
 - Fixed an issue with emscripten_async_wget, which undesirably expected that the string pointer passed to it stayed alive for the duration of the operation (#2349)
 - Emscripten now issues a warning message when the EXPORTED_FUNCTIONS list contains invalid symbol names (#2338)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.18.0...1.18.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.18.0: 5/10/2014
------------------
 - Enable support for low-level C<->JS interop to marshall 64 bit integers from C to JS.
 - Fixed an issue that caused some programs to immediately run out of memory "(cannot enlarge memory arrays)" at startup. (#2334)
 - Fixed a crash issue with generated touch events that didn't correspond to a real touch.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.17.0...1.18.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.17.0...1.18.0
    - Emscripten-Clang: no changes.

v1.17.0: 5/6/2014
------------------
 - Enabled asm.js compilation and -s PRECISE_F32 support when using embind.
 - Improved relooper to emit switches in many-entried blocks.
 - Fixed a GLFW bug where mouse wheel direction was reversed.
 - Fixed glfwGetKey to work even when no callback is registered with glfwGetKeyCallback (#1320)
 - Added a new tool 'webidl_binder' that generates C <-> JS interop code from WebIDL descriptions.
 - Fix emscripten compilation to work on pages that don't contain a HTML canvas.
 - Added a new error message to default shell when an uncaught exception is thrown.
 - Improved error diagnostics reported by -s SAFE_HEAP=1.
 - Added support for registering callbacks hook to VFS file open, write, move, close and delete.
 - Added embind support to std::basic_string<unsigned char>
 - By default, the C runtime will no longer exit after returning from main() when safeSetTimeout() or safeSetInterval() is used.
 - Fixed an issue with sscanf formatting (#2322)
 - Fixed an issue where precompiled headers were given a wrong output filename (#2320)
 - Enabled registerizeHarder optimization pass to work when outlining is enabled.
 - Fixed an issue with strptime month handling (#2324)
 - Added an initial implementation of a new tool 'ffdb' which can be used to operate a Firefox OS phone from the command line.
 - Fixed a compiler crash on assertion failure '!contains(BranchesOut, Target)' (emscripten-fastcomp #32)
 - Added a new ABI to Clang that targets Emscripten specifically. Stop aligning member functions to save some space in the function table array.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.16.0...1.17.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.16.0...1.17.0
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.16.0...1.17.0

v1.16.0: 4/16/2014
------------------
 - Removed browser warnings message in VFS library about replacing __proto__ performance issue. 
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.15.1...1.16.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.15.1...1.16.0

v1.15.1: 4/15/2014
------------------
 - Added support for SDL2 touch api.
 - Added new user-controllable emdind-related define #define EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES, which allows optimizing embind for minimal size when std::type_info is not needed. 
 - Fixed issues with CMake support where CMAKE_AR and CMAKE_RANLIB were not accessible from CMakeLists.txt files.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.15.0...1.15.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.15.0: 4/11/2014
------------------
 - Fix outlining feature for functions that return a double (#2278)
 - Added support for C++11 atomic constructs (#2273)
 - Adjusted stdout and stderr stream behavior in the default shell.html to always print out to both web page text log box, and the browser console.
 - Fixed an issue with loop variable optimization.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.14.1...1.15.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.14.1...1.15.0
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.14.1...1.15.0

v1.14.1: 4/8/2014
------------------
 - Added new command line utility 'emcmake', which can be used to call emconfigure for cmake.
 - Added a new emcc command line parameter '--valid-abspath', which allows selectively suppressing warning messages that occur when using absolute path names in include and link directories.
 - Added a new emcc linker command line parameter '--emit-symbol-map', which will save a map file between minified global names and the original function names.
 - Fixed an issue with --default-object-ext not always working properly.
 - Added optimizations to eliminate redundant loop variables and redundant self-assignments.
 - Migrated several libc functions to use compiled code from musl instead of handwritten JS implementations.
 - Improved embind support.
 - Renamed the EM_ASM_() macro to the form EM_ASM_ARGS().
 - Fixed mouse button ordering issue in glfw.
 - Fixed an issue when creating a path name that ends in a slash (#2258, #2263)
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.14.0...1.14.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.14.0...1.14.1
    - Emscripten-Clang: no changes.

v1.14.0: 3/25/2014
------------------
 - Added new emcc linker command line option '-profiling', which defaults JS code generation options suited for benchmarking and profiling purposes.
 - Implemented the EGL function eglWaitGL().
 - Fixed an issue with the HTML5 API that caused the HTML5 event listener unregistration to fail.
 - Fixed issues with numpad keys in SDL support library.
 - Added a new JS optimizer pass 'simplifyIfs', which is run when -s SIMPLIFY_IFS=1 link flag is set and -g is not specified. This pass merges multiple nested if()s together into single comparisons, where possible.
 - Removed false positive messages on missing internal "emscripten_xxx" symbols at link stage.
 - Updated to latest relooper version.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.13.2...1.14.0
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.13.2...1.14.0
    - Emscripten-Clang: no changes.

v1.13.2: 3/15/2014
------------------
 - Fixed issues with SDL audio on Safari.
 - Fixed issues with HTML5 API mouse scroll events on Safari.
 - Fixed issues with HTML5 fullscreen requests in IE11.
 - Enabled support for emscripten_get_callstack on IE10+.
 - Fixed issues with Closure symbol minification.
 - Further improved em_asm()-related error messages.
 - Updated to latest relooper version.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.13.1...1.13.2
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.13.1...1.13.2
    - Emscripten-Clang: no changes.

v1.13.1: 3/10/2014
------------------
 - Disallow C implicit function declarations by making it an error instead of a warning by default. These will not work with Emscripten, due to strict Emscripten signature requirements when calling function pointers (#2175).
 - Allow transitioning to full screen from SDL as a response to mouse press events.
 - Fixed a bug in previous 1.13.0 release that broke fullscreen transitioning from working.
 - Fixed emscripten/html5.h to be used in C source files.
 - Fix an issue where extraneous system libraries would get included in the generated output (#2191).
 - Added a new function emscripten_async_wget2_data() that allows reading from an XMLHTTPRequest directly into memory while supporting advanced features.
 - Fixed esc key code in GLFW.
 - Added new emscripten_debugger() intrinsic function, which calls into JS "debugger;" statement to break into a JS debugger.
 - Fixed varargs function call alignment of doubles to 8 bytes.
 - Switched to using default function local stack alignment to 16 bytes to be SIMD-friendly.
 - Improved error messages when user code has a syntax error in em_asm() statements.
 - Switched to using a new custom LLVM datalayout format for Emscripten. See https://github.com/kripken/emscripten-fastcomp/commit/65405351ba0b32a8658c65940e0b65ceb2601ad4
 - Optimized function local stack space to use fewer temporary JS variables.
 - Full list of changes:
    - Emscripten: https://github.com/kripken/emscripten/compare/1.13.0...1.13.1
    - Emscripten-LLVM: https://github.com/kripken/emscripten-fastcomp/compare/1.13.0...1.13.1
    - Emscripten-Clang: https://github.com/kripken/emscripten-fastcomp-clang/compare/1.13.0...1.13.1

v1.13.0: 3/3/2014
------------------
 - Fixed the deprecated source mapping syntax warning.
 - Fixed a buffer overflow issue in emscripten_get_callstack (#2171).
 - Added support for -Os (optimize for size) and -Oz (aggressively optimize for size) arguments to emcc.
 - Fixed a typo that broko the call signature of glCompressedTexSubImage2D() function (#2173).
 - Added new browser fullscreen resize logic that always retains aspect ratio and adds support for IE11.
 - Improve debug messaging with bad function pointer calls when -s ASSERTIONS=2 is set.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.12.3...1.13.0

v1.12.3: 2/27/2014
------------------
 - Fixed alcOpenDevice on Safari.
 - Improved the warning message on missing symbols to not show false positives (#2154).
 - Improved EmscriptenFullscreenChangeEvent HTML5 API structure to return information about HTML element and screen sizes for convenience.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.12.2...1.12.3

v1.12.2: 2/25/2014
------------------
 - Added better warning message if Emscripten, LLVM and Clang versions don't match.
 - Introduced the asmjs-unknown-emscripten target triple that allows specializing LLVM codegen for Emscripten purposes.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.12.1...1.12.2

v1.12.1: 2/25/2014
------------------
 - TURNED ON FASTCOMP BY DEFAULT. This means that you will need to migrate to fastcomp-clang build. Either use an Emscripten SDK distribution, or to build manually, see http://kripken.github.io/emscripten-site/docs/building_from_source/LLVM-Backend.html for info.
 - Migrate to requiring Clang 3.3 instead of Clang 3.2. The fastcomp-clang repository by Emscripten is based on Clang 3.3.
 - Deprecated old Emscripten libgc implementation.
 - asm.js will now be always enabled, even in -O0 builds in fastcomp.
 - Remove support for -s RUNTIME_TYPE_INFO, which is unsupported in fastcomp.
 - Added a new "powered by Emscripten" logo.
 - Updated default shell.html graphical layout.
 - Added new macro EM_ASM_, which allows sending values to JS without returning anything.
 - Deprecated the jcache compiler option. It should not be needed anymore.
 - Added support for fetching callstack column information in Firefox 30 in emscripten_get_callstack.
 - Fix issues with missing exceptions-related symbols in fastcomp.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.12.0...1.12.1

v1.12.0: 2/22/2014
------------------
 - Improved the runtime abort error message when calling an invalid function pointer if compiled with -s ASSERTIONS=1 and 2. This allows the developer to better deduce errors with bad function pointers or function pointers casted and invoked via a wrong signature.
 - Added a new api function emscripten_set_main_loop_arg, which allows passing a userData pointer that will be carried via the function call, useful for object-oriented encapsulation purposes (#2114).
 - Fixed CMake MinSizeRel configuration type to actually optimize for minimal size with -Os.
 - Added support for GLES2 VAO extension OES_vertex_array_object for browsers that support it.
 - Fix issues with emscripten/html5.f when compiled with the SAFE_HEAP option.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.11.1...1.12.0

v1.11.1: 2/19/2014
------------------
 - Improved eglSwapBuffers to be spec-conformant.
 - Fixed an issue with asm.js validation and va_args (#2120).
 - Fixed asm.js validation issues found with fuzzing.
 - Added new link-time compiler flag -s RETAIN_COMPILER_SETTINGS=1, which enables a runtime API for querying which Emscripten settings were used to compile the file.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.11.0...1.11.1

v1.11.0: 2/14/2014
------------------
 - Implemented some new SDL library functions.
 - Renamed standard file descriptors to have handles 0, 1 and 2 rather than 1, 2 and 3 to coincide with unix numbering.
 - Improved embind support with smart pointers and mixins.
 - Improved the registerization -O3 optimization pass around switch-case constructs.
 - Upper-case files with suffix .C are now also recognized (#2109).
 - Fixed an issue with glGetTexParameter (#2112).
 - Improved exceptions support in fastcomp.
 - Added new linker option -s NO_EXIT_RUNTIME=1, which can be used to set a default value for the Module["noExitRuntime"] parameter at compile-time.
 - Improved SDL audio buffer queueing when the sample rate matches the native web audio graph sample rate.
 - Added an optimization that removes redundant Math.frounds in -O3.
 - Improved the default shell.html file.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.10.4...1.11.0

v1.10.4: 2/10/2014
------------------
 - Added support for legacy GL emulation in fastcomp.
 - Deprecated the --split-js compiler option. This is not supported in fastcomp. 
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.10.3...1.10.4

v1.10.3: 2/9/2014
------------------
 - Work on supporting GL/EGL GetProcAddress.
 - Fixed issues with shared lib linking support.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.10.2...1.10.3

v1.10.2: 2/7/2014
------------------
 - Added basic FS unmount support.
 - Improved screen orientation lock API to return a success code.
 - Added PRECISE_F32 support to fastcomp.
 - Fixed issues in fastcomp related to special floating point literal serialization.
 - Improved SDL audio buffer queueing.
 - Added new link-time option -s WARN_UNALIGNED=1 to fastcomp to report compiler warnings about generated unaligned memory accesses, which can hurt performance.
 - Optimized libc strcmp and memcmp with the implementations from musl libc.
 - Optimized libc memcpy and memset to back to native code for large buffer sizes.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.10.1...1.10.2

v1.10.1: 1/31/2014
------------------
 - Improve srand() and rand() to be seedable and use a Linear Congruential Generator (LCG) for the rng generation for performance.
 - Improved OpenAL library support.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.10.0...1.10.1

v1.10.0: 1/29/2014
------------------
 - Improved C++ exception handling.
 - Improved OpenAL library support.
 - Fixed an issue where loading side modules could try to allocate from sealed heap (#2060).
 - Fixed safe heap issues (2068).
 - Added new EM_ASM variants that return a value but do not receive any inputs (#2070).
 - Add support for simultaneously using setjmp and C++ exceptions in fastcomp.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.9.5...1.10.0

v1.9.5: 1/25/2014
------------------
 - Added a spinner logo to default html shell.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.9.4...1.9.5

v1.9.4: 1/24/2014
------------------
 - Add support for Ninja and Eclipse+Ninja builds with Emscripten+CMake.
 - Fixed regressions with GL emulation.
 - Added support for #if !X in .js library preprocessor.
 - Make the syntax EM_ASM("code"); not silently fail. Note that the proper form is EM_ASM(code); without double-quotes.
 - Optimize generated code size by minifying loop labels as well.
 - Revised the -O3 optimization level to mean "safe, but very slow optimizations on top of -O2", instead of the old meaning "unsafe optimizations". Using -O3 will now only do safe optimizations, but can be very slow compared to -O2.
 - Implemented a new registerization optimization pass that does extra variable elimination in -O3 and later to reduce the number of local variables in functions.
 - Implemented a new emscripten/html5.h interface that exposes common HTML5 APIs directly to C code without having to handwrite JS wrappers.
 - Improved error messages reported on user-written .js libraries containing syntax errors (#2033).
 - Fixed glBufferData() function call signature with null data pointer.
 - Added new option Module['filePackagePrefixURL'] that allows customizing the URL where the VFS package is loaded from.
 - Implemented glGetTexEnviv and glGetTexEnvfv in GL emulation mode.
 - Optimized the size of large memory initializer sections.
 - Fixed issues with the safe heap compilation option.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.9.3...1.9.4

v1.9.3: 1/17/2014
------------------
 - re-merge split blocks in multiples
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.9.2...1.9.3

v1.9.2: 1/16/2014
------------------
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.9.1...1.9.2

v1.9.1: 1/16/2014
------------------
 - Optimize desktop GL fixed function pipeline emulation texture load instruction counts when GL_COMBINE is used.
 - fix Math_floor coercion in unrecommended codegen modes
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.9.0...1.9.1

v1.9.0: 1/16/2014
------------------
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.14...1.9.0

v1.8.14: 1/15/2014
------------------
 - add musl fputws and fix vswprintf.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.13...1.8.14

v1.8.13: 1/15/2014
------------------
 - remove musl use of fwritex
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.12...1.8.13

v1.8.12: 1/15/2014
------------------
 - Added new GLEW 1.10.0 emulation support.
 - Fixed an issue where the runtime could start more than once when run in a browser (#1992)
 - Fix a regression in wprintf.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.11...1.8.12

v1.8.11: 1/15/2014
------------------
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.10...1.8.11

v1.8.10: 1/14/2014
------------------
 - Update libc implementation from musl libc.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.9...1.8.10

v1.8.9: 1/14/2014
------------------
 - add fputwc, which enables wprintf.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.8...1.8.9

v1.8.8: 1/14/2014
------------------
 - Update to latest libcxx and libcxxabi libraries.
 - Fix handling of floating point negative zero (#1898)
 - Fixed a memory leak in relooper in previous release.
 - Fixed an issue in previous release with VBO handling in GL optimizations.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.7...1.8.8

v1.8.7: 1/13/2014
------------------
 - Added support to numpad keycodes in glut support library.
 - Fix SIMD support with fastcomp.
 - Fixed a compiler error 'ran out of names' that could occur with too many minified symbol names.
 - Work around webkit imul bug https://bugs.webkit.org/show_bug.cgi?id=126345 (#1991)
 - Optimized desktop GL fixed function pipeline emulation path for better performance.
 - Added support for exceptions when building with fastcomp.
 - Fix and issue where the run() function could be called multiple times at startup (#1992)
 - Removed a relooper limitation with fixed buffer size.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.6...1.8.7

v1.8.6: 1/8/2014
------------------
 - Added support for the libuuid library, see http://linux.die.net/man/3/libuuid.
 - Fixed .js file preprocessor to preprocess recursively (#1984).
 - Fixed a compiler codegen issue related to overflow arithmetic (#1975)
 - Added new link-time optimization flag -s AGGRESSIVE_VARIABLE_ELIMINATION=1 that enables the aggressiveVariableElimination js optimizer pass, which tries to remove temporary variables in generated JS code at the expense of code size.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.5...1.8.6

v1.8.5: 1/7/2014
------------------
 - Fixed compiler issues when used with LLVM 3.4.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.4...1.8.5

v1.8.4: 1/6/2014
------------------
 - Added support to Return and Backspace keys to glut
 - Fixed compiler issues when used with LLVM 3.4.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.3...1.8.4

v1.8.3: 1/5/2014
------------------
 - Improved SDL and page scroll pos handling support for IE10 and IE11.
 - Optimized SDL_UnlockSurface performance.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.2...1.8.3

v1.8.2: 1/4/2014
------------------
 - Fixed glGetFramebufferAttachmentParameteriv and an issue with glGetXXX when the returned value was null.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.1...1.8.2

v1.8.1: 1/3/2014
------------------
 - Added support for WebGL hardware instancing extension.
 - Improved fastcomp native LLVM backend support.
 - Added support for #include filename.js to JS libraries.
 - Deprecated --compression emcc command line parameter that manually compressed output JS files, due to performance issues. Instead, it is best to rely on the web server to serve compressed JS files.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.8.0...1.8.1

v1.8.0: 12/28/2013
------------------
 - Fix two issues with function outliner and relooper.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.9...1.8.0

v1.7.9: 12/27/2013
------------------
 - Added new command line parameter --em-config that allows specifying a custom location for the .emscripten configuration file.
 - Reintroduced relaxed asm.js heap sizes, which no longer need to be power of 2, but a multiple of 16MB is sufficient.
 - Added emrun command line tool that allows launching .html pages from command line on desktop and Android as if they were native applications. See https://groups.google.com/forum/#!topic/emscripten-discuss/t2juu3q1H8E . Adds --emrun compiler link flag.
 - Began initial work on the "fastcomp" compiler toolchain, a rewrite of the previous JS LLVM AST parsing and codegen via a native LLVM backend.
 - Added --exclude-file command line flag to emcc and a matching --exclude command line flag to file packager, which allows specifying files and directories that should be excluded while packaging a VFS data blob.
 - Improved GLES2 and EGL support libraries to be more spec-conformant.
 - Optimized legacy GL emulation code path. Added new GL_FFP_ONLY optimization path to fixed function pipeline emulation.
 - Added new core functions emscripten_log() and emscripten_get_callstack() that allow printing out log messages with demangled and source-mapped callstack information.
 - Improved BSD Sockets support. Implemented getprotobyname() for BSD Sockets library.
 - Fixed issues with simd support.
 - Various bugfixes: #1573, #1846, #1886, #1908, #1918, #1930, #1931, #1942, #1948, ..
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.8...1.7.9

v1.7.8: 11/19/2013
------------------
 - Fixed an issue with -MMD compilation parameter.
 - Added EM_ASM_INT() and EM_ASM_DOUBLE() macros. For more information, read https://groups.google.com/forum/#!topic/emscripten-discuss/BFGTJPCgO6Y .
 - Fixed --split parameter to also work on Windows.
 - Fixed issues with BSD sockets accept() call.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.7...1.7.8

v1.7.7: 11/16/2013
------------------
 - Improve SDL audio buffer queue timing support.
 - Improved default precision of clock_gettime even when not using CLOCK_REALTIME.
 - Optimize and fix issues with LLVM IR processing.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.6...1.7.7

v1.7.6: 11/15/2013
------------------
 - Added regex implementation from musl libc.
 - The command line parameter -s DEAD_FUNCTIONS=[] can now be used to explicitly kill functions coming from built-in library_xx.js.
 - Improved EGL support and GLES2 spec conformance.
 - Reverted -s TOTAL_MEMORY=x to require pow2 values, instead of the relaxed 'multiples of 16MB'. This is because the relaxed rule is released only in Firefox 26 which which is currently in Beta and ships on the week of December 10th (currently in Beta). As of writing, current stable Firefox 25 does not yet support these.
 - Adjusted the default linker behavior to warn about all missing symbols, instead of silently ignoring them. Use -s WARN_ON_UNDEFINED_SYMBOLS=0 to suppress these warnings if necessary.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.5...1.7.6

v1.7.5: 11/13/2013
------------------
 - Fix issues with the built-in C++ function name demangler.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.4...1.7.5

v1.7.4: 11/12/2013
------------------
 - Fixed issues with BSD sockets code and SDL joystick implementation.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.3...1.7.4

v1.7.3: 11/12/2013
------------------
 - Added support for generating single-precision floating point instructions.
    - For more information, read https://blog.mozilla.org/javascript/2013/11/07/efficient-float32-arithmetic-in-javascript/
 - Made GLES2 support library more spec-conformant by throwing fewer exceptions on errors. Be sure to build with -s GL_ASSERTIONS=1, remember to use glGetError() and check the browser console to best detect WebGL rendering errors.
 - Converted return value of emscripten_get_now() from float to double, to not lose precision in the function call.
 - Added support for joysticks in SDL via the Gamepad API
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.2...1.7.3
 
v1.7.2: 11/9/2013
------------------
 - The compiler now always generates a .js file that contains the generated source code even when compiling to a .html file.
    - Read https://groups.google.com/forum/#!topic/emscripten-discuss/EuHMwqdSsEs
 - Implemented depth+stencil buffer choosing behavior in GLUT, SDL and GLFW.
 - Fixed memory leaks generated by glGetString and eglGetString.
 - Greatly optimized startup times when virtual filesystems with a large amount of files in them.
 - Added some support for SIMD generated by LLVM.
 - Fixed some mappings with SDL keyboard codes.
 - Added a new command line parameter --no-heap-copy to compiler and file packager that can be used to optimize VFS memory usage at startup.
 - Updated libcxx to revision 194185, 2013-11-07.
 - Improvements to various library support. 
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.1...1.7.2

v1.7.1: 10/24/2013
------------------
 - Remove old call to Runtime.warn in file packager code
 - Fix bug with parsing of empty types.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.7.0...1.7.1

v1.7.0: 10/23/2013
------------------
 - Adds mouse wheel events support in GLUT library.
 - Adds support for a new link parameter -s CASE_INSENSITIVE_VFS=1 to enable Emscripten virtual filesystem to search files ignoring case.
 - *Numerous* optimizations in both compilation and runtime stages.
 - Remove unnecessary whitespace, compact postSets function, and other optimizations in compilation output to save on generated file size.
 - Fixes float parsing from negative zero.
 - Removes the -s EMIT_GENERATED_FUNCTIONS link parameter as unneeded.
 - Fixes an issue where updating subranges of GL uniform arrays was not possible.
 - asm.js heap size (-s TOTAL_MEMORY=x) no longer needs to be a power of 2. As a relaxed rule, choosing any multiple of 16MB is now possible.
 - O1 optimization no longer runs the 'simplifyExpressions' optimization pass. This is to improve build iteration times when using -O1. Use -O2 to run that pass.
 - EM_ASM() can now be used even when compiling to asm.js.
 - All currently specified non-debugging-related WebGL 1 extensions are now enabled by default on startup, no need to ctx.getExtension() manually to enable them.
 - Improve readability of uncaught JavaScript exceptions that are thrown all the way up to the web console by printing out the stack trace of where the throw occurred.
 - Fix an issue when renaming a directory to a subdirectory.
 - Several compiler stability fixes.
 - Adds a JavaScript implementation of cxa_demangle function for demangling call stack traces at runtime for easier debugging.
 - GL context MSAA antialising is now DISABLED by default, to make the GL behavior consistent with desktop usage.
 - Added support to SDL, GLUT and GLFW libraries to specify MSAA on/off at startup.
 - Implemented glColor4ubv in GL emulation mode.
 - Fix an issue with LLVM keyword __attribute__ ((__constructor__)) (#1155).
 - Fix an issue with va_args and -s UNALIGNED_MEMORY=1 (#1705).
 - Add initial support code for LLVM SIMD constructs and a JavaScript SIMD polyfill implementation from https://github.com/johnmccutchan/ecmascript_simd/ .
 - Fixed support for node.js native filesystem API NODEFS on Windows.
 - Optimize application startup times of Emscripten-compiled programs by enabling the virtual filesystem XHR and asm.js compilation to proceed in parallel when opening a page.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.6.4...1.7.0

v1.6.4: 9/30/2013
------------------
 - Implements a new preprocessor tool for preparsing C struct definitions (#1554), useful for Emscripten support library implementors.
 - Fix parsing issue with sscanf (#1668).
 - Improved the responsiveness of compiler print output on Windows.
 - Improved compilation times at link stage.
 - Added support for new "NODEFS" filesystem that directly accesses files on the native filesystem. Only usable with node.js when compiling to JS.
 - Added support for new IDBFS filesystem for accessing files in IndexedDB storage (#1601.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.6.3...1.6.4

v1.6.3: 9/26/2013
------------------
 - Emscripten CMake toolchain now generates archive files with .a suffix when project target type is static library, instead of generatic .bc files (#1648).
 - Adds iconv library from the musl project to implement wide functions in C library (#1670).
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.6.2...1.6.3

v1.6.2: 9/25/2013
------------------
 - Added support for dprintf() function (#1250).
 - Fixes several compiler stability issues (#1637, #1166, #1661, #1651 and more).
 - Enables support for WEBGL_depth_texture.
 - Adds support for new link flag -s GL_ASSERTIONS=1 which can be used to add extra validation layer to the Emscripten GL library to catch code issues.
 - Adds support to Web Audio API in SDL audio backend so that SDL audio now works in Chrome and new Opera as well.
 - Fixes an alpha blending issue with SDL_SetAlpha.
 - Implemented locale-related code in C library.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.6.1...1.6.2

v1.6.1: 9/22/2013
------------------
 - Several optimizations to compiler link stage.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.6.0...1.6.1

v1.6.0: 9/21/2013
------------------
 - Enable support for %[] pattern in scanf.
 - Added dependency tracking support to linked .js files in CMake toolchain.
 - The hex prefix 0x is now properly handled in sscanf (#1632).
 - Simplify internal compiler operations by removing the internal framework.js. 
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.9...1.6.0

v1.5.9: 9/15/2013
------------------
 - Add support for SDL_Delay in web workers.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.8...1.5.9

v1.5.8: 9/14/2013
------------------
 - Add support for the GCC -E compiler flag.
 - Update Emscripten libc headers to musl-0.9.13.
 - Added new utility function emscripten_async_load_script() to asynchronously load a new .js script URL.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.7...1.5.8

v1.5.7: 8/30/2013
------------------
 - The script tag in default shell.html is now marked 'async', which enables loading the JS script code asynchronously in Firefox without making the main thread unresponsive.
 - Implemented new utility function emscripten_get_canvas_size() which returns the current Module <canvas> element size in pixels.
 - Optimize code size in compiled side modules.
 - Optimize startup memory usage by avoiding unnecessary copying of VFS data at startup.
 - Add support for SDL_WM_ToggleFullScreen().
 - Add support for emscripten_get_now() when running in SpiderMonkey shell.
 - Added new environment variable EM_BUILD_VERBOSE=0,1,2,3 to set an extra compiler output verbosity level for debugging.
 - Added better support for dlopen() to simulate dynamic library loading in JavaScript.
 - Improved support for BSD sockets and networking.
 - Added new SOCKFS filesystem, which reads files via a network connection.
 - Avoid issues with long command line limitations in CMake toolchain by using response files.
 - Fix issues with client-side vertex data rendering in GL emulation mode.
 - Improved precision of clock_gettime().
 - Improve function outlining support.
 - Added support for using NMake generator with CMake toolchain.
 - Improved support for flexible arrays in structs (#1602).
 - Added ability to marshal UTF16 and UTF32 strings between C++ <-> JS code.
 - Added a new commandline tool validate_asms.py to help automating asm.js validation testing.
 - Improved stability with inline asm() syntax.
 - Updated libc headers to new version.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.6...1.5.7

v1.5.6: 8/17/2013
------------------
 - Improved BSD sockets support.
 - Added touch events support to GLUT library.
 - Added new --js-opts=0/1 command line option to control whether JS optimizer is run or not.
 - Improved OpenAL support.
 - Added new command line tool tools/find_bigvars.py which can be used on a output file to detect large functions and needs for outlining.
 - Merged link flags -s FORCE_GL_EMULATION and -s DISABLE_GL_EMULATION to a single opt-in flag -s LEGACY_GL_EMULATION=0/1 to control whether GL emulation is active.
 - Improved SDL input support.
 - Several stability-related compiler fixes.
 - Fixed source mapping generation support on Windows.
 - Added back the EMSCRIPTEN_KEEPALIVE attribute qualifier to help prevent inlining and to retain symbols in output without dead code elimination occurring.
 - Fix issues when marshalling UTF8 strings between C<->JS.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.5...1.5.6

v1.5.5: 8/9/2013
------------------
 - Update libcxx to revision 187959, 2013-08-08.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.4...1.5.5

v1.5.4: 8/9/2013
------------------
 - Fixed multiple issues with C stdlib support.
 - Fix audio buffer queueing issues with OpenAL.
 - Improved BSD sockets support.
 - Added a new compile+link time command line option -Wno-warn-absolute-paths to hide the emscripten compiler warning when absolute paths are passed into the compiler.
 - Added new link flag -s STB_IMAGE=0/1 and integrate it to SDL image loading to enable synchronous image loading support with SDL.
 - Several improvements on function outlining support.
 - Fix issues with GLES2 interop support.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.3...1.5.4

v1.5.3: 6/28/2013
------------------
 - Added new optimization level --llvm-lto 3 to run even more aggressive LTO optimizations.
 - Improve optimizations for libc and other libraries.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.2...1.5.3

v1.5.2: 6/27/2013
------------------
 - Added support for generating source maps along the built application when -g is specified. This lets the browser show original .cpp sources when debugging.
 - GLUT and SDL improvements.
 - Added new link option -g<level> where level=0-4, which allows controlling various levels of debuggability added to the output.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.1...1.5.2

v1.5.1: 6/22/2013
------------------
 - File packager now skips all directories and files starting with '.', and hidden files on Windows.
 - Fix issues with strnlen, memmove, LDBL_ constants, va_args, float.h, and others.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.5.0...1.5.1

v1.5.0: 6/17/2013
------------------
 - Several compiler optimizations.
 - Improve SDL key events support.
 - Increase debug logging when specifying emcc -v.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.4.9...1.5.0

v1.4.9: 6/8/2013
------------------
 - Several compiler optimizations.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.4.8...1.4.9

v1.4.8: 6/6/2013
------------------
 - Add support for webrtc-based sockets.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.4.7...1.4.8

v1.4.7: 6/2/2013
------------------
 - Remove more unneeded break and continue statements in relooper.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.4.6...1.4.7

v1.4.6: 6/2/2013
------------------
 - Improve relooper code.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.4.5...1.4.6

v1.4.5: 6/1/2013
------------------
 - Improve relooper code.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.4.4...1.4.5

v1.4.4: 6/1/2013
------------------
 - Add support for symlinks in source files.
 - Fix various issues with SDL.
 - Added -s FORCE_ALIGNED_MEMORY=0/1 link time flag to control whether all loads and stores are assumed to be aligned.
 - Fix file packager to work with closure.
 - Major improvements to embind support, and optimizations.
 - Improve GL emulation.
 - Optimize VFS usage.
 - Allow emscripten to compile .m and .mm files.
 - Added new syntax --preload-file src@dst to file packager command line to allow placing source files to custom destinations in the FS.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.4.3...1.4.4

v1.4.3: 5/8/2013
------------------
 - Fix issue with strcat.
 - Major embind improvements.
 - Switch to le32-unknown-nacl LLVM target triple as default build option instead of the old i386-pc-linux-gnu target triple.
 - Improve compiler logging behavior.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.4.2...1.4.3

v1.4.2: 5/3/2013
------------------
 - Fix issues with le32-unknown-nacl LLVM target triple.
 - Add some GLEW support.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.4.1...1.4.2

v1.4.1: 4/28/2013
------------------
 - Implement support for le32-unknown-nacl LLVM target triple.
 - Added new cmdline option -s ERROR_ON_UNDEFINED_SYMBOLS=0/1 to give compile-time error on undefined symbols at link time. Default off.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.3.8...1.4.1

v1.3.8: 4/29/2013
------------------
 - Improved 64-bit integer ops codegen.
 - Added Indexed DB support to vfs.
 - Improve warning message on dangerous function pointer casts when compiling in asm.js mode.
 - Added --use-preload-cache command line option to emcc, to be used with the file packager.
 - Fixes to libcextra.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.3.7...1.3.8

v1.3.7: 4/24/2013
------------------
 - Merge IMVU implementation of embind to emscripten trunk. Embind allows high-level C++ <-> JS types interop.
 - Enable asm.js compilation in -O1 and higher by default. Fix issues when compiling to asm.js.
 - Improve libc support with Emscripten with the musl libc headers.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.3.6...1.3.7

v1.3.6: 4/2/2013
------------------
 - Fix hang issue with strtof.
 - Update libcxx to upstream r178253 from March 29, 2013.
 - Fix issues with GL emulation.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.3.5...1.3.6

v1.3.5: 3/25/2013
------------------
 - Get exceptions working as they did before.
 - Remove symbol removing hack.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.3.4...1.3.5

v1.3.4: 3/24/2013
------------------
 - Update to new libcxx and libcxxabi versions from upstream.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.3.3...1.3.4

v1.3.3: 3/23/2013
------------------
 - Remove unneeded check from relooper.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.3.2...1.3.3

v1.3.2: 3/22/2013
------------------
 - Fix issues with fgets. 
 - Add support for non-fullscreen pointer lock.
 - Improve OpenAL support.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.3.1...1.3.2

v1.3.1: 3/19/2013
------------------
 - Improve SDL audio and mixer support.
 - Add GLES2 emulation features when -s FULL_ES2=1 is specified.
 - Add support for OpenAL.
 - Add new -s OPENAL_DEBUG=0/1 link command line option.
 - Fixed an issue with mouse coordinate being offset with canvas.
 - Removed -s UTF_STRING_SUPPORT=0 parameter, this is now always on.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.3.0...1.3.1

v1.3.0: 3/11/2013
------------------
 - Improve GLES2 emulation with -s FULL_ES2=1.
 - Deprecated -s USE_TYPED_ARRAYS=1 and -s QUANTUM_SIZE=1.
 - Implement a minifier infrastructure when compiling for asm.js.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.9...1.3.0

v1.2.9: 3/7/2013
------------------
 - Improved canvas behavior when transitioning between fullscreen.
 - Added support for getopt().
 - Fixed several libc issues.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.8...1.2.9

v1.2.8: 3/6/2013
------------------
 - Remove unnecessary recursion in relooper RemoveUnneededFlows.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.7...1.2.8

v1.2.7: 3/6/2013
------------------
 - Added SDL_Mixer support.
 - Implemented stubs for several Unix and threading-related functions.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.6...1.2.7

v1.2.6: 3/5/2013
------------------
 - Relooper updates.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.5...1.2.6

v1.2.5: 3/5/2013
------------------
 - Greatly improve GL emulation support.
 - Handle %c in sscanf.
 - Improve compilation times by optimizing parallel execution in the linker.
 - Improve several compiler stability issues detected from fuzzing tests.
 - Implemented emscripten_jcache_printf.
 - Allow running emscripten.py outside emcc itself.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.4...1.2.5

v1.2.4: 2/2/2013
------------------
 - Work on adding support for asm.js compilation.
 - Improve EGL support.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.3...1.2.4

v1.2.3: 1/9/2013
------------------
 - Work on adding support for asm.js compilation.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.2...1.2.3

v1.2.2: 1/8/2013
------------------
 - Work on adding support for asm.js compilation.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.1...1.2.2

v1.2.1: 1/8/2013
------------------
 - Improvements to GLUT, SDL and BSD sockets headers.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.2.0...1.2.1

v1.2.0: 1/1/2013
------------------
 - Work on adding support for asm.js compilation.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.1.0...1.2.0

v1.1.0: 12/12/2012
------------------
 - Fix several issues with Windows support.
 - Added a standalone toolchain for CMake.
 - Added emscripten_run_script_string().
 - Optimize compilation times via threading.
 - Update to requiring Clang 3.2. Older versions may no longer work.
 - Several improvements to emscripten library support headers.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.0.1a...1.1.0

v1.0.1a: 11/11/2012
------------------
 - Add relooper code to repository.
 - Full list of changes: https://github.com/kripken/emscripten/compare/1.0.1...1.0.1a

v1.0.1: 11/11/2012
------------------
 - First commit that introduced versioning to the Emscripten compiler.
