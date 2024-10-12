This document describes changes between tagged Emscripten SDK versions.

Note that version numbers do not necessarily reflect the amount of changes
between versions. A version number reflects a release that is known to pass all
tests, and versions may be tagged more or less frequently at different times.

Note that there is *no* ABI compatibility guarantee between versions - the ABI
may change, so that we can keep improving and optimizing it. The compiler will
automatically invalidate system caches when the version number updates, so that
libc etc. are rebuilt for you. You should also rebuild object files and
libraries in your project when you upgrade emscripten.

Not all changes are documented here. In particular, new features, user-oriented
fixes, options, command-line parameters, breaking ABI changes, deprecations,
significant internal modifications and optimizations etc. generally deserve a
mention. To examine the full set of changes between versions, you can use git
to browse the changes between the tags.

See docs/process.md for more on how version tagging works.

3.1.70 (in development)
-----------------------

3.1.69 - 10/12/24
-----------------
- The usage of `EM_BOOL` in the emscripten API has been replaced with C/C++
  bool.  This change should not be observable since `EM_BOOL` has been
  equivalent to `bool` since #22157. (#22155)
- Fix regression introduced in 3.1.67 (#22557) which broke webgpu / int64
  integration. (#22689)
- SDL2 port updated from 2.28.4 to 2.30.8. (#22697)
- embind no longer exports any library functions by default.  Previously we
  would export getInheritedInstanceCount, getLiveInheritedInstances,
  flushPendingDeletes and setDelayFunction.  If you need these library function
  exprted they can be added to `EXPORTED_RUNTIME_METHODS`. (#22705)

3.1.68 - 09/30/24
-----------------
- Added support for compiling 256-bit wide AVX intrinsics, emulated on top
  of 128-bit Wasm SIMD instruction set. (#22430). Pass `-msimd128 -mavx` to
  enable targeting AVX.
- Pthread-based programs no longer generates `.worker.js` file.  This file was
  made redundant back in 3.1.58 and now is completely removed. (#22598)
- The freetype port was updated from v2.6 to v2.13.3. (#22585)
- The number of arguments passed to Embind function calls is now only verified
  with ASSERTIONS enabled. (#22591)
- Optional arguments can now be omitted from Embind function calls. (#22591)
- Recent changes to Binaryen included in this version significantly improve
  the speed at which the post-link optimizations run for some large programs.

3.1.67 - 09/17/24
-----------------
- Add option `nonnull<ret_val>()` to Embind to omit `| null` from TS definitions
  for functions that return pointers.

3.1.66 - 09/10/24
-----------------
- The behaviour of the `pthread_kill` function was fixed to match the spec
  and will now run the designated handler on the target thread. (#22467)
- Added support for WebGL extensions EXT_clip_control, EXT_depth_clamp,
  EXT_polygon_offset_clamp and WEBGL_polygon_mode (#20841)
- New `emscripten_console_trace` and `emscripten_dbg_backtrace` APIs we were
  added to `console.h`.  The former simply maps directly to `console.trace`.
  The latter uses `dbg()` so it writes directly to stderr under node (better for
  multi-threaded apps).

3.1.65 - 08/22/24
-----------------
- A new `--emit-minification-map` command line flag was added, which can be used
  to emit a minifiction map in the case that import/export minification is
  performed (this happens at higher optimization levels). (#22428)
- Remove `Module['quit']` handling.  This could be used to override the internal
  method for shutting down the program, but it was neither documented nor
  tested.  Programs that want to intercept the shutting down of a program can
  use `Module['onExit']`. (#22371)
- The `NODEJS_CATCH_EXIT` setting is now disabled by default.  This setting
  is only useful under very specific circumstances, and has some downsides, so
  disabling it by default makes sense. (#22257)
- Add WebP (`.webp`) decoding support in file preloading. (#22282)

3.1.64 - 07/22/24
-----------------------
- Updated the SCons tool to not require the `EMSCRIPTEN_ROOT` environment
  variable, in which case it will assume that SCons will find the binaries in
  (its) `PATH`.
- Updated `emscons` to apply the `EMSCRIPTEN_ROOT`, `EMSCONS_PKG_CONFIG_LIBDIR`
  and `EMSCONS_PKG_CONFIG_PATH` environment variables. The SCons tool will use
  last two to set up `PKG_CONFIG_LIBDIR` and `PKG_CONFIG_PATH` respectively.

3.1.63 - 07/12/24
-----------------
- Fix html5 input event bug that was introduced in 3.1.62. (#22201)
- Fix webpack + pthreads bug that was introduced in 3.1.60. (#22165)

3.1.62 - 07/02/24
-----------------
- The `EM_BOOL` type changed from `int/u32` to `bool/u8`.  This changes the
  layout and size of some structs in the emscripten API. (#22157)
- The `EMSCRIPTEN_FETCH_WAITABLE` flag along with the `emscripten_fetch_wait`
  API were marked a deprecated.  These feature have not functions for several
  years now. (#22138)
- The internal `read_` function was removed.  We now just use `readBinary` or
  `readAsync`. (#22080)
- reference-types feature is now enabled by default in Emscripten, due to the
  upstream LLVM change (https://github.com/llvm/llvm-project/pull/93261).
- Emscripten now uses `strftime` from musl rather than using a custom
  JavaScript implementation. (#21379)
- Embind now supports return value policies for properties.

3.1.61 - 05/31/24
-----------------
- The internal `readAsync` function now returns a promise rather than accepting
  callback arguments.
- The JSPI feature now uses the updated browser API for JSPI (available in
  Chrome v126+). To support older versions of Chrome use Emscripten version
  3.1.60 or earlier.
- IDBFS mount has gained a new option { autoPersist: true }, which if passed,
  changes the semantics of the IDBFS mount to automatically persist any changes
  made to the filesystem. (#21938)

3.1.60 - 05/20/24
-----------------
- Under nodefs, symbolic links to files outside of mount locations no longer work.
  This reverts the previous behaviour added in #3277. (#21805)
- The `EXPORTED_FUNCTIONS` list can now include JS library symbols even if they
  have not been otherwise included (e.g. via `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE`).
  (#21867)
- Due to the upstream LLVM changes
  (https://github.com/llvm/llvm-project/pull/80923 and
  https://github.com/llvm/llvm-project/pull/90792), multivalue feature is now
  enabled by default in Emscripten. This only enables the language features and
  does not turn on the multivalue ABI.
- Embind now supports return value policies to better define object lifetimes.
  See https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#object-ownership for more information.

3.1.59 - 04/30/24
-----------------
- Fix the location of the dummy `.worker.js` file that is now generated as part
  of pthread builds so that is generated alongside the main JavaScript file.
  See #21701. ()
- `-sASYNCIFY=2` is setting now deprecated, use `-sJSPI` instead.

3.1.58 - 04/23/24
-----------------
- The `-sMAIN_MODULE=1` mode no longer exports all the main module symbols on
  `Module` object.  This saves a huge about of generated JS code due the fact
  that `-sMAIN_MODULE=1` includes *all* native symbols in your program as well
  is from the standard library.  The generated JS code for a simple program
  in this mode is reduced from from 3.3mb to 0.5mb.  The current implementation
  of this feature requires wasm-ld to be on the program twice which could have a
  noticeable effect on link times. (#21785)
- In `-sMODULARIZE` mode, the argument passed into the module constructor is
  no longer mutated in place.  The expectation is that the module instance will
  be available via the constructor return value.  Attempting to access methods
  on the object passed *into* the constructor will now abort. (#21775)
- Enable use of `::` to escape port option separator (#21710)
- In multi-threaded builds `--extern-pre-js` and `--extern-post-js` code is
  now only run on the main thread, and not on each of the workers. (#21750)
- Fix crash when throwing exceptions in dynamically linked int64 functions (#21759)
- Multi-threaded builds no depend on a separate `.worker.js` file.  This saves
  on code size and network requests.  In order to make this change go smoothly,
  without breaking build systems that expect a `worker.js`, emscripten will
  generate an empty `.worker.js` to give folks time to transition their
  deployment scripts.  In `-sSTRICT` mode, this empty file will not be
  generated. (#21701)

3.1.57 - 04/10/24
-----------------
- libcxx, libcxxabi, libunwind, and compiler-rt were updated to LLVM 18.1.2.
  (#21607, #21638, and #21663)
- musl libc updated from v1.2.4 to v1.2.5. (#21598)
- In `MODULARIZE` mode we no longer export the module ready promise as `ready`.
  This was previously exposed on the Module for historical reasons even though
  in `MODULARIZE` mode the only way to get access to the module is to wait on
  the promise returned from the factory function. (#21564)
- JS library code is now executed in its own context/scope, which limits how
  much of the compiler internals are accessible. If there are build time JS
  symbols that you are depending on, but that were not added to this scope,
  please file a bug and we can add more to this scope. (#21542)
- The JS functions for manipulating the native/shadow stack
  (`stackSave`/`stackRestore`/`stackAlloc`) are now just regular JS library
  function and as such are only included if you explicitly depend on them.  If
  you use these functions in your JS code you will need to depend on them via
  either:
  - The `EM_JS_DEPS` macro for `EM_ASM`/`EM_JS` code.
  - The `__deps` attribute for JS library functions
  - The `-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE` flag for `--pre-js`/`--post-js`
    code
  (#21555)
- TypeScript definitions for Wasm exports, runtime exports, and embind bindings
  can now be generated with `--emit-tsd`. The option `--embind-emit-tsd` has been
  deprecated, use `--emit-tsd` instead.
- Added the `ASYNCIFY_PROPAGATE_ADD` setting, to control whether the `ASYNCIFY_ADD`
  list propagates or not. By default this is enabled; as a result you may see larger
  ASYNCIFY builds as more of the function tree may be instrumented than you were
  previously manually specifying in `ASYNCIFY_ADD`. To stop propagation you can
  specify functions in the `ASYNCIFY_REMOVE` list, or to return to the previous
  behaviour, disable this setting (set `-sNO_ASYNCIFY_PROPAGATE_ADD`.) (#21672)
- ports changes:
  - Fixed transitive link dependencies (#21602)
  - Enable use of options in ports dependencies (#21629)

3.1.56 - 03/14/24
-----------------
- emscripten will now generate an `unused-command-line-argument` warning if
  a `-s` setting is specified more than once on the command line with
  conflicting values.  In this case the first setting is ignored. (#21464)

3.1.55 - 03/01/24
-----------------
- Update sdl2-mixer port from 2.6.0 to 2.8.0
- In `STRICT` mode the `HEAPXX` symbols (such as `HEAP8` and `HEAP32`) are now
  only exported on demand.  This means that they must be added to
  `EXPORTED_RUNTIME_METHODS` for them to appear on the `Module` object.  For
  now, this only effects users of `STRICT` mode. (#21439)
- Emscripten no longer supports `--memory-init-file` (i.e. extracting static
  data into an external .mem file).  This feature was only available under
  wasm2js (`-sWASM=0`) anyway so this change will only affect users of this
  setting. (#21217)
- `INITIAL_HEAP` setting is introduced to control the amount of initial
  memory available for dynamic allocation without capping it. If you are
  using `INITIAL_MEMORY`, consider switching to `INITIAL_HEAP`. Note that
  it is currently not supported in all configurations (#21071).

3.1.54 - 02/15/24
-----------------
- SDL2 port updated from v2.24.2 to v2.26.0. (#21337)
- The `DEMANGLE_SUPPORT` setting and the associated `demangle` function are
  now deprecated since Wasm stack traces always contain demangled symbols these
  days. (#21346)
- The type of `EMSCRIPTEN_WEBGL_CONTEXT_HANDLE` was changed to unsigned and
  the only valid error returned from `emscripten_webgl_create_context` is
  now zero.  This allows `EMSCRIPTEN_WEBGL_CONTEXT_HANDLE` to hold a pointer
  to memory even in 2GB+ mode.  Since `emscripten_webgl_create_context` never
  returns anything except zero for its errors today this change should not
  require any action. (#21268)
- Added `--use-port` option to `emcc`.  This option allows ports to be enabled
  by name and is designed to replace all existing `-sUSE_XXX` settings for
  ports. You can use `--show-ports` to get the list of available ports that
  can be used with this new option. (#21214)
- `--pre-js` and `--post-js` files can now opt into being run through the JS
  preprocessor. This change was originally landed in  #18525, but it got
  reverted in #19006.  Now it requires explicit opt-in by adding `#preprocess` to
  the top of the JS file.  This is useful as it allows things like `{{{
  POINTER_SIZE }}}` and `{{{ makeGetValue(..) }}}` to be used in pre/post JS
  files, just like they can be in JS library files. (#21227)
- Added concept of contrib ports which are ports contributed by the wider 
  community and supported on a "best effort" basis. See 
  `tools/ports/contrib/README.md` for details.A first contrib port is 
  available via `--use-port=contrib.glfw3`: an emscripten port of glfw written 
  in C++ with many features like support for multiple windows. (#21244 and 
  #21276)
- Added concept of external ports which live outside emscripten and are
  loaded on demand using the syntax `--use-port=/path/to/my_port.py` (#21316)
- `embuilder` can now build ports with options as well as external ports using
  the same syntax introduced with `--use-port`
  (ex: `embuilder sdl2_image:formats=png,jpg`) (#21345) 
- Allow comments in response files. Any line starting with `#` is now ignored.
  This is useful when listing exported symbols. (#21330)

3.1.53 - 01/29/24
-----------------
- The llvm version that emscripten uses was updated to 19.0.0 trunk. (#21165)

3.1.52 - 01/19/24
-----------------
- The core stack manipulation functions (`stackSave`, `stackRestore`,
  `stackAlloc`) are no longer exported by default.  Users of these function
  now need to depend on them explicitly (either via `__deps` attributes or via
  `-sEXPORTED_FUNCTIONS`). (#21075)
- Building with `pthreads+EXPORT_ES6` will now emit the worker file as
  `NAME.worker.mjs` rather than `.js`. This is a necessary breaking change to
  resolve other `pthreads+EXPORT_ES6` issues in Node.js (because Node.js is
  affected by the suffix in some cases). (#21041)
- Include paths added by ports (e.g. `-sUSE_SDL=2`) now use `-isystem` rather
  then `-I`.  This means that files in user-specified include directories will
  now take precedence over port includes. (#21014)
- Certain settings that only apply when generating JavaScript output will now
  trigger a warning if used when generating only Wasm.
- Fix bug where `main` was mistakenly included in debug builds but not in
  release builds. (#20971)
- Remove JAVA from the list of `.emscripten` config file settings.  In the
  past we used this to run the java version of closure compiler.  If there are
  folks who prefer to use the java version of closure compiler for some reason
  it should be possible by adding `--platform=java` to `--closure-args` or
  `EMCC_CLOSURE_ARGS` but emscripten will no longer do this automatically.
  (#20919)
- The WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG setting was
  removed.  This was a workaround from 2018 (#7459) that should no longer be
  needed. (#20925)
- The `--default-obj-ext` command line flag was removed. (#20917)
- emcc will now treat `.bc` files as source files.  These means that will get
  compiled by clang before being passed to the linker.  This matches the
  behaviour of clang. (#20922)
- Emscripten now only supports browsers going back to certain versions. The
  current set of minimum versions are: Chrome 32, Firefox 34, Safari 9.
  Attempting to targets version older this using, for example
  `MIN_CHROME_VERSION` will now result in build-time error.  All of these
  browser versions are at least 8 years old now so the hope is that nobody
  is intending to target them today.  (#20924)
- C++ objects passed into embind's val via constructors, methods, and call
  function will not be automatically destroyed after the function call. This
  makes the behavior consistent for invocations. 
- The `SUPPORT_ERRNO` setting is now deprecated as it only controlled setting
  errno from JS library functions and emscripten no longer requires this.
  (#21074)

3.1.51 - 12/13/23
-----------------
- Support for explicitly targeting the legacy Internet Explorer or EdgeHTML
  (edge version prior to the chromium-based edge) browsers via
  `-sMIN_EDGE_VERSION/-sMIN_IE_VERSION` was removed. (#20881)
- Emscripten is now more strict about handling unsupported shared library
  inputs.  For example, under the old behaviour if a system shared library
  such as `/usr/lib/libz.so` was passed to emscripten it would silently re-write
  this to `-lz`, which would then search this a libz in its own sysroot.  Now
  this file is passed though the linker like any other input file and you will
  see an `unknown file type` error from the linker (just like you would with the
  native clang or gcc toolchains). (#20886)
- Support for explicitly targeting the legacy EdgeHTML browser (edge version
  prior to the chromium-based edge) via `-sMIN_EDGE_VERSION` was removed.
  Using `-sLEGACY_VM_SUPPORT` should still work if anyone still wanted to target
  this or any other legacy browser.
- Breaking change: Using the `*glGetProcAddress()` family of functions now
  requires passing a linker flag -sGL_ENABLE_GET_PROC_ADDRESS. This prevents
  ports of native GL renderers from later accidentally attempting to activate
  "dormant" features if web browser implementations gain new WebGL extensions in
  the future, which `*glGetProcAddress()` is not able to support. (#20802)
- Added Hi DPI support to GLFW. When enabled, GLFW automatically accounts for
  the `devicePixelRatio` browser property and changes the size of the canvas
  accordingly (including dynamically if the canvas is moved from a 4K screen to
  a 2K screen and vice-versa). `glfwGetFramebufferSize` now properly returns the
  canvas size in pixels, while `glfwGetWindowSize` returns the canvas size is
  screen size. By default, this feature is disabled. You can enable it before
  creating a window by calling `glfwWindowHint(GLFW_SCALE_TO_MONITOR,
  GLFW_TRUE)`. You can also dynamically change it after the window has been
  created by calling `glfwSetWindowAttrib(window, GLFW_SCALE_TO_MONITOR,
  GLFW_TRUE)`. (#20584)
- Transpilation to support older environments/browsers is now performed by babel
  rather than closure compiler.  This means that folks targeting older browsers
  (e.g. `-sLEGACY_VM_SUPPORT`) do not need to ensure their code is closure
  compliant. (#20879)

3.1.50 - 11/29/23
-----------------
- Add a port of mimalloc, a fast and scalable multithreaded allocator. To use
  it, build with `-sMALLOC=mimalloc`. (#20651)
- When compiling, Emscripten will now invoke `clang` or `clang++` depending only
  on whether `emcc` or `em++` was run.  Previously it would determine which to
  run based on individual file extensions.  One side effect of this is that you
  may now see a clang warning when building `.c` source files using `em++`:
  `warning: treating 'c' input as 'c++' when in C++ mode`.  This also means that
  the `DEFAULT_TO_CXX` setting now only applies when linking and not when
  compiling. (#20712)
- JavaScript library code can now use the full range of ES6 features and we rely
  on closure compiler to transpile for ES5 when targeting older browsers.
  For those that would rather perform transpilation separately outside of
  emscripten you can use the `-sPOLYFILL=0` setting. (#20700)
- libcxx, libcxxabi, libunwind, and compiler-rt were updated to LLVM 17.0.4.
  (#20705, #20707, and #20708)
- Remove `BENCHMARK` setting. That has not been used by the benchmark suite for
  some time now (at least not by default), and is much less useful these days
  given lazy compilation in VMs (which makes it impossible to truly benchmark
  execution separately from compilation, which `BENCHMARK` hoped to do).
- Update GLFW handling of touch events to avoid sending duplicate mousedown and
  and mouseup events. Maps touchmove to mousemove events for a single primary
  touch. (#20805)

3.1.49 - 11/14/23
-----------------
- Many MEMORY64 fixes for browser and graphics APIs (#20678)
- The `glfwSetWindowSize` function no longer switches to fullscreen when the
  width/height provided as parameters match the screen size. This behavior
  now matches the behavior of SDL and glut. In order to switch to fullscreen,
  the client code should invoke `Module.requestFullscreen(...)` from a user 
  triggered event otherwise the browser raises an error. (#20600)

3.1.48 - 11/05/23
-----------------
- The JS `assert` function is no longer available in release builds when
  `-sSTRICT` is used.  This should only affect users with custom JS library code
  which doesn't use `#if ASSERTIONS` guards around their `assert` calls.  This
  behaviour matches that of `MINIMAL_RUNTIME`. (#20592)
- The minimum version of node required run the compiler was updated from
  10.19 to 16.20.  This does not effect the node requirements of the generated
  JavaScript code. (#20551)
- A new top-level `bootstrap` script was added.  This script is for emscripten
  developers and helps take a care of post-checkout tasks such as `npm install`.
  If this script needs to be run (e.g. because package.json was changed, emcc
  will exit with an error. (#19736)
- If exceptions are disabled, using `new` together with `std::nothrow` no
  longer aborts if the allocation fails. Instead `nullptr` is returned now.
  This does not change the behavior of regular usage of `new`.
- Default `MIN_CHROME_VERSION` was increased from 75 to 85 and default
  `MIN_FIREFOX_VERSION` was increased from 68 to 79 to allow Emscripten
  to use some ES2021 features for smaller JavaScript code size. (#20549)
- Emscripten now generates PowerShell wrappers for its CLIs. This allows to
  sidestep some of the issues with legacy cmd.exe, but developers must
  explicitly opt-in to running PowerShell scripts in system settings or
  via the `Set-ExecutionPolicy` command. (#20416)
- `emscripten::val` now supports C++20 `co_await` operator for JavaScript
  `Promise`s. (#20420)

3.1.47 - 10/09/23
-----------------
- The boost-headers port was updated to v1.83.0.
- The `noExitRuntime` global is now a JS library symbol that will only be
  included as needed.  User of `noExitRuntime` will now need to declare a
  dependency on it.  It will get implicitly included if `noExitRuntime` is part
  of `INCOMING_MODULE_JS_API.` (which it is by default), or it can be added to
  either `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE` or `noExitRuntime`. (#20336)
- The egl, html5, sdl and webgpu libraries now support basic functionality with
  `-sMEMORY64`. (#20276)
- Value types in `emscripten/html5.h` that correspond the WebIDL `long` type are
  now represented as `int` rather than `long` types so that they are 32-bit wide
  on both wasm32 and wasm64.  (#20290)
- `EM_JS` and `EM_ASM` that are present in side module now have their syntax
  validated at build time. (#20258)
- `MAIN_THREAD_EM_ASM_PTR` macro added for code that returns a pointer.  This
  mirrors the existing `EM_ASM_PTR`. (#20261)
- Emscripten now implements default POSIX signal handlers. These can
  terminate or abort the program in response to `raise` cals. (#20257)
- `emscripten::val` now prevents accidental access to the underlying JavaScript
  value from threads other than its owner. This already didn't work correctly
  in majority of cases, but now it will throw a clear assertion failure. (#20344)
- `emscripten::val` can now be iterated over with a C++ range-based for loop.
  (#20364)

3.1.46 - 09/15/23
-----------------
- The `wasmTable` global is now a JS library function that will only be included
  as needed.  Code that references `wasmTable` will no need to declare a
  dependency on it.  It can also be explicitly included using
  `-sEXPORTED_RUNTIME_METHODS=wasmTable`.
- libunwind updated to LLVM 16.0.6. (#20088)
- The `--minify=0` command line flag will now preserve comments as well as
  whitespace.  This means the resulting output can then be run though closure
  compiler or some other tool that gives comments semantic meaning. (#20121)
- `-sSTRICT` now implies `-sINCOMING_MODULE_JS_API=[]` which is generally good
  for code size.  If you `-sSTRICT` you now need to be explicit about the
  incoming module APIs you are supplying.  Users who supply symbols on the
  incoming module but forget to include them in `-sINCOMING_MODULE_JS_API`
  will see an error in debug builds so this change will not generate any
  silent failures.
- JS library decorators such as `__deps` and `__async` are now type checked so
  that errors are not silently ignored.
- The `USE_GLFW` settings now defaults to 0 rather than 2.  This matches other
  other settings such as `USE_SDL` that default to 0 these days and also matches
  the existing behaviour for `MINIMAL_RUNTIME` and `STRICT` mode.
  If you use GLFW you now need to explicitly opt into it using `-sUSE_GLFW` or
  `-lglfw`. (#19939)
- A new settings `TABLE_BASE` was introduced that can be used to place static
  function addresses (table slots) at a certain offset.  This defaults to 1
  which is the previously fixed value. (#20149)
- Clang's error detection of unused variables `-Wunused-variable` improved,
  which may require changes in user code (see #20169).

3.1.45 - 08/23/23
-----------------
- The function used to add symbols the JS library has been renamed from
  `mergeInto`, to the more specific `addToLibrary`.  This new function does not
  require the passing of `LibraryManager.library` as a first argument.  The old
  `mergeInto` continues to exist for backwards compat.
- The `--log_html` option was removed from `emrun`.  This option was already not
  working with python3.8 or above so we hope is safe to say that nobody was
  relying on it.

3.1.44 - 07/25/23
-----------------
- musl libc updated from v1.2.3 to v1.2.4. (#19812)
- The `EM_LOG_FUNC_PARAMS` flag to `emscripten_log`/`emscripten_get_callstack`
  has been deprecated and no longer has any effect.  It was based on a
  long-deprecated JS API. (#19820)
- The internal `read_` and `readAsync` functions no longer handle data URIs.
  (Higher-level functions are expected to handle that themselves, before calling.)
  This only effects builds that use `-sSINGLE_FILE` or `--memory-init-file`.
  (#19792)
- The `asm` property of the Module object (which held the raw exports of the
  wasm module) has been removed.  Internally, this is now accessed via the
  `wasmExports` global. If necessary, it is possible to export `wasmExports`
  on the Module object using `-sEXPORTED_RUNTIME_METHODS=wasmExports`. (#19816)
- Embind now supports generating TypeScript definition files using the
  `--embind-emit-tsd <filename>` option.

3.1.43 - 07/10/23
-----------------
- Handling i64 arguments and return values in JS functions is now much simpler
  with the new `__i53abi` decorator.  When this is set to true, i64 values are
  automatically converted to JS numbers (i53) at the JS boundary.  Parameters
  outside of the i53 will show up as NaN in the JS code (#19711)
- EM_JS functions are now supported in side modules (#19705)

3.1.42 - 06/22/23
-----------------
- The default minimum Node version of Emscripten output was bumped from 10.19 to
  16.0. To run the output JS in an older version of node, you can use e.g.
  `-sMIN_NODE_VERSION=101900` which will apply the previous minimum version of
  10.19.0. (#19192).
- The log message that emcc will sometime print (for example when auto-building
  system libraries) can now be completely suppressed by running with
  `EMCC_LOGGING=0`.
- Runtime dynamic linking symbols such as dlopen and dlsym will no longer cause
  a linker error when building without `-sMAIN_MODULE`.  Instead stub functions
  will be included that fail at runtime.  This matches the behaviour of other
  libc functions that we don't implement.  For those that prefer to get a linker
  error we have the `-sALLOW_UNIMPLEMENTED_SYSCALLS` settings. (#19527)
- The `modifyFunction` helper in `parseTools.js` was renamed to
  `modifyJSFunction` and its callback function no longer takes the name of the
  function being modified.  The name is not relevant for JS library functions
  and can be safely ignored.
- JS library functions can now be implemented using ES6 arrow notation, which
  can save to a few bytes on JS code size. (#19539)

3.1.41 - 06/06/23
-----------------
- A new setting (`CHECK_NULL_WRITES`) was added to disabled the checking of
  address zero that is normally done when `STACK_OVERFLOW_CHECK` is enabled.
  (#19487)
- compiler-rt updated to LLVM 16. (#19506)
- libcxx and libcxxabi updated to LLVM 16. (#)

3.1.40 - 05/30/23
-----------------
- The `_emscripten_out()`, `_emscripten_err()` and `_emscripten_dbg()` functions
  declared in `emscripten/console.h` no longer have the underscore prefix and
  are now documented. (#19445)

3.1.39 - 05/18/23
-----------------
- The JS `err()` function will now bind to `console.error` by default rather
  than `console.warning`.  For debugging/tracing/logging we recommend the
  `dbg()` function instead. (#19326)
- The `WASM2C` options has been removed. All known users are using upstream wabt
  these days anyhow.

3.1.38 - 05/10/23
-----------------
- The `dladdr` function will now always return an error rather than filling in
  dummy values. (#19319)
- The restriction preventing the use of dynamic linking in combination with
  `-sDYNAMIC_EXECUTION=0` was removed.  This restriction was being enforced
  unnecessarily since dynamic linking has not depended on `eval()` for a while
  now.
- Remove extra code for falling back to long-deprecated BlobBuilder browser API
  when Blob constructor is missing.  This was a fix for an issue that has long
  been fixed. (#19277)

3.1.37 - 04/26/23
-----------------
- The `EM_PYTHON_MULTIPROCESSING` environment variable no longer has any effect.
  This was added a temporary fallback but should no longer be needed. (#19224)
- The old reverse dependency system based on `tools/deps_info.py` has been
  removed and the existing `__deps` entries in JS library files can now be used
  to express JS-to-native dependencies.  As well being more precise, and
  extensible via user-supplied JS libraries, this also speeds up link times
  since we no longer need scan linker inputs using `llvm-nm`.  It also
  completely removes the need for the `REVERSE_DEPS` settings which has now
  been deprecated. (#18905)
- Bump the default minimum Firefox version from 65 to 68 (#19191).
- Background pthreads no longer prevent a Node.js app from exiting. (#19073)

3.1.36 - 04/16/23
-----------------
- The `USES_DYNAMIC_ALLOC` setting has been deprecated.  You can get the same
  effect from `-sMALLOC=none`. (#19164)

3.1.35 - 04/03/23
-----------------
- The following JavaScript runtime functions were converted to JavaScript
  library functions:
   - UTF8ArrayToString
   - UTF8ToString
   - stringToUTF8Array
   - stringToUTF8
   - lengthBytesUTF8
  
  If you use any of these functions in your JS code you will now need to include
  them explicitly in one of the following ways:
   - Add them to a `__deps` entry in your JS library file (with leading $)
   - Add them to `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE` (with leading $)
   - Add them to `EXPORTED_FUNCTIONS` (without leading $)
   - Set `-sLEGACY_RUNTIME` to include all of them at once.
- `FS.loadFilesFromDB` and `FS.saveFilesToDB` were removed.  We think it's
  unlikely there were any users of these functions since there is now a separate
  IDBFS filesystem for folks that want persistence. (#19049)
- `allocateUTF8` and `allocateUTF8OnStack` library function moved to
  `library_legacy.js`.  Prefer the more accurately named `stringToNewUTF8` and
  `stringToUTF8OnStack`. (#19089)
- `SDL_image` port was updated to version 2.6.0.
- `-z` arguments are now passed directly to wasm-ld without the need for the
  `-Wl,` prefix.  This matches the behaviour of both clang and gcc. (#18956)
- Reverted #18525 which runs the JS pre-processor over files passed via
  --pre-js and --post-js.  It turned out this change caused issue for several
  folks who had JS files with lines that start with `#` so can't be run through
  the pre-processor.  If folks want to re-enable this we can looks into ways to
  make it conditional/optional.
- The `{{{ cDefine('name') }}}` helper macro can now be simplified to just `{{{
  cDefs.name }}}`.

3.1.34 - 03/14/23
-----------------
- Fix for using `EM_JS` functions defined in other object files.  This was a bug
  that was introduced when `LLD_REPORT_UNDEFINED` was enabled by default back in
  3.1.28. (#18928)
- The preferred way to enable pthreads is now to just use the standard `-pthread`
  flag.  The `-sUSE_PTHREADS` setting still works but is marked as legacy and
  will generate a warning in `-sSTRICT` mode.
- When targeting node, and using `-sMODULARIZE`, we no longer internally catch
  unhandled promise rejections or exit status code. That is to say the,
  `NODEJS_CATCH_REJECTION` and `NODEJS_CATCH_EXIT` are no longer compatible
  with `-sMODULARIZE`.   

3.1.33 - 03/08/23
-----------------
- Initial support for C++20 modules.  We have added a very simple test in form
  of `other.test_cpp_module`. (#18915)
- Removed `sys/sysctl.h` compatibility header.  We don't implement the function
  it defines. (#18863)
- Update SDL2_ttf port to 2.20.2 (#18804)
- Update glfw header to 3.3.8 (#18826)
- The `LLD_REPORT_UNDEFINED` setting has been removed.  It's now essentially
  always enabled. (#18342)
- Added `-sEXPORT_KEEPALIVE` to export symbols. When using
  `MINIMAL_RUNTIME`, the option will be **disabled** by default.
  This option simply exports the symbols on the module object, i.e.,
  `Module['X'] = X;`
- The WasmFS OPFS backend is now faster in browsers that implement
  [`Atomics.waitAsync`](https://caniuse.com/mdn-javascript_builtins_atomics_waitasync).
  (#18861)
- The `emscripten_proxy_async_with_callback` API was replaced with a simpler
  `emscripten_proxy_callback` API that takes a second callback to be called if
  the worker thread dies before completing the proxied work.  

3.1.32 - 02/17/23
-----------------
- Added new linker option `-sEXCEPTION_STACK_TRACES` which will display a stack
  trace when an uncaught exception occurs. This defaults to true when
  `ASSERTIONS` is enabled. This option is mainly for the users who want only
  exceptions' stack traces without turning `ASSERTIONS` on. (#18642 and #18535)
- `SUPPORT_LONGJMP`'s default value now depends on the exception mode. If Wasm
  EH (`-fwasm-exception`) is used, it defaults to `wasm`, and if Emscripten EH
  (`-sDISABLE_EXCEPTION_CATCHING=0`) is used or no exception support is used, it
  defaults to `emscripten`. Previously it always defaulted to `emscripten`, so
  when a user specified `-fwasm-exceptions`, it resulted in Wasm EH + Emscripten
  SjLj, the combination we do not intend to support for the long term.
- Added support for Wasm-based AudioWorklets for realtime audio processing
  (#16449)
- Synchronous proxying functions in emscripten/proxying.h now return errors
  instead of hanging forever when the worker thread dies before the proxied work
  is finished.

3.1.31 - 01/26/23
-----------------
- Symbols that were previously exported from native code, but only for internal
  reasons, are no longer exported on the `Module` object by default.  For
  example, previously if you were using `Module._malloc` but not explicitly
  including `_malloc` in `EXPORTED_FUNCTIONS`, it might have been exported
  anyway due to internal use of `malloc` within the JS library code. (#18564)
- The `STACK_SIZE`, `STACK_ALIGN`, `POINTER_SIZE`, and `ASSERTIONS` JavaScript
  globals were removed by default.  In debug builds a clear error is shown if
  you try to use these. (#18503)
- --pre-js and --post-js files are now fed through the JS preprocessor, just
  like JS library files and the core runtime JS files.  This means they can
  now contain #if/#else/#endif blocks and {{{ }}} macro blocks. (#18525)
- `-sEXPORT_ALL` can now be used to export symbols on the `Module` object
  when used with `-sMINIMAL_RUNTIME` and `-sMODULARIZE` together. (#17911)
- The llvm version that emscripten uses was updated to 17.0.0 trunk.

3.1.30 - 01/11/23
-----------------
- The default pthread stack size will now be set to match `-sSTACK_SIZE` by
  default.  Set `DEFAULT_PTHREAD_STACK_SIZE` explicitly to override this.
  (#18479)
- The `buffer` JavaScript variable was removed.  This underlying buffer is
  still accessible via `wasmMemory.buffer` or `HEAPXX.buffer`.  In debug builds,
  a clear error is shown if you try to use it.  (#18454)
- The SDLv1 header directory is no longer added to the include path by default.
  This means if you include SDL headers without the explicit version in them
  (e.g. `SDL_events.h`) you will now need to add `-sUSE_SDL` explicitly at
  compile time.  If you include the SDL headers with the directory name included
  (e.g. `SDL/SDL_events.h`) you will not be affected by this change. (#18443)
- Significant improvements were made (in the version of LLVM associated with
  this emsdk release) to the quality of DWARF debug info when building in
  optimized mode. See https://reviews.llvm.org/D140373. Using the -O1 flag is
  recommended if a program is too large or slow to debug with -O0 (although
  -O0 is still better for debugging when feasible).

3.1.29 - 01/03/23
-----------------
- Fixed bug in `PROXY_TO_PTHREAD` whereby certain async operations on the main
  thread would cause the whole program to exit, even when the proxied main
  function was still running. (#18372)
- Added `Module.pthreadPoolReady` promise for the `PTHREAD_POOL_DELAY_LOAD`
  mode that allows to safely join spawned threads. (#18281)
- PThreads can now be safely spawned on-demand in Node.js even without a PThread
  pool (`PTHREAD_POOL_SIZE`) or proxying (`PROXY_TO_PTHREAD`) options. (#18305)

3.1.28 - 12/08/22
-----------------
- `LLD_REPORT_UNDEFINED` is now enabled by default.  This makes undefined symbol
  errors more precise by including the name of the object that references the
  undefined symbol. The old behaviour (of allowing all undefined symbols at
  wasm-ld time and reporting them later when processing JS library files) is
  still available using `-sLLD_REPORT_UNDEFINED=0`. (#16003)
- musl libc updated from v1.2.2 to v1.2.3. (#18270)
- The default emscripten config file no longer contains `EMSCRIPTEN_ROOT`.  This
  setting has long been completely ignored by emscripten itself. For
  applications that wish to know where emscripten is installed looking for
  `emcc` in the `PATH` has long been the recommended method (i.e. `which emcc`).
  (#18279)
- More accurate synchronisation of `emscripten_get_now` clocks between main
  thread and pthreads.
  This also changes the absolute value returned by the function, but it shouldn't
  affect correct usages as the function has always returned different values on
  different platforms, and is clearly documented as "only meaningful in
  comparison to other calls to this function". (#18267)
- Emscripten will now search your PATH for binaryen, llvm, and node if the
  corresponding config file settings (`BINARYEN_ROOT`, `LLVM_ROOT`, `NODE_JS`)
  are not set.  Allows emscripten to run with an empty config file given the
  right tools in the PATH. (#18289)

3.1.27 - 11/29/22
-----------------
- Add support for `-sEXPORT_ES6`/`*.mjs` on Node.js. (#17915)
- Idle workers in a PThread pool no longer prevent Node.js app from exiting. (#18227)
- The default `STACK_SIZE` was reduced from 5MB to 64KB.  Projects that use more
  than 64Kb of stack will now need specify `-sSTACK_SIZE` at link time.  For
  example, `-sSTACK_SIZE=5MB` can be used to restore the previous behaviour.
  To aid in debugging, as of #18154, we now also place the stack first in memory
  in debug builds so that overflows will be immediately detected, and result in
  runtime errors.  This change brings emscripten into line with `wasm-ld` and
  wasi-sdk defaults, and also reduces memory usage by default.  In general,
  WebAssembly stack usage should be lower than on other platforms since a lot of
  state normally stored on the stack is hidden within the runtime and does not
  occupy linear memory at all.  The default for `DEFAULT_PTHREAD_STACK_SIZE` was
  also reduced from 2MB to 64KB to match.
- Improved error messages for writing custom JS libraries. (#18266)
- EM_ASM code is now supported in side modules (#18228)

3.1.26 - 11/17/22
-----------------
- Inline with the recent changes to llvm and binaryen, emscripten will now, by
  default, enable the sign-extension and mutable-globals WebAssembly proposals.
  In order to do so the default minimum safari version (`MIN_SAFARI_VERSION`)
  was updated from 12.0 to 14.1, and support for the old EdgeHTML engine
  (`MIN_EDGE_VERSION`) was removed by default.  If you want to continue to
  support these older engines you can use these settings
  (`-sMIN_SAFARI_VERSION=120000` and/or `-sMIN_EDGE_VERSION=44`) to revert to
  the previous defaults, which will result in the new proposals being disabled.
  Note that in order to avoid support for the sign-extension emscripten uses
  a binaryen pass, so targeting older browsers requires the running of wasm-opt
  and is therefore incompatible with `ERROR_ON_WASM_CHANGES_AFTER_LINK` (i.e.
  fast linking). (#17690)
- Added `--reproduce` command line flag (or equivalently `EMCC_REPRODUCE`
  environment variable).  This options specifies the name of a tar file into
  which emscripten will copy all of the input files along with a response file
  that will allow the command to be replicated.  This can be useful for sharing
  reproduction cases with others (inspired by the lld option of the same name).
  (#18160)
- In non-optimizing builds emscripten will now place the stack first in memory,
  before global data.  This is to get more accurate stack overflow errors (since
  overflow will trap rather corrupting global data first).  This should not
  be a user-visible change (unless your program does something very odd such
  depending on the specific location of stack data in memory). (#18154)

3.1.25 - 11/08/22
-----------------
- The `TOTAL_STACK` setting was renamed to `STACK_SIZE`.  The old name will
  continue to work as an alias. (#18128)
- Exporting `print`/`printErr` via `-sEXPORTED_RUNTIME_METHODS` is deprecated in
  favor of `out`/`err`.  The former symbols are supposed to be used with
  `-sINCOMING_MODULE_JS_API` instead. (#17955)
- aio.h was removed from the sysroot.  Emscripten doesn't support any of the
  functions in this header.
- Clang's function pointer cast warnings (enabled with `-Wcast-function-type`)
  are now stricter. This warning is intended to help with CFI errors but also
  helps wasm builds since wasm traps on such type mismatches in indirect calls.
  We recommend that users enable it to prevent such errors (which can be hard to
  debug otherwise). The older (less strict) behavior is also still possible with
  `-Wcast-function-type -Wno-cast-function-type-strict` (or
  `-Wno-error=cast-function-type-strict` if you want the warnings to be visible
  but not errors). See https://reviews.llvm.org/D134831
- libcxx and libcxxabi updated to LLVM 15. (#18113)

3.1.24 - 10/11/22
-----------------
- In Wasm exception mode (`-fwasm-exceptions`), when `ASSERTIONS` is enabled,
  uncaught exceptions will display stack traces and what() message. (#17979 and
  #18003)
- It is now possible to specify indirect dependencies on JS library functions
  directly in C/C++ source code.  For example, in the case of a EM_JS or EM_ASM
  JavaScript function that depends on a JS library function.  See the
  `EM_JS_DEPS` macro in the `em_macros.h` header.  Adding dependencies in this
  way avoids the need to specify them on the command line with
  `-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE`. (#17854)

3.1.23 - 09/23/22
-----------------
- The `__EMSCRIPTEN_major__/minor__/tiny__` macros are no longer defined on the
  command line but require `<emscripten.h/>` (or just `<emscripten/version.h>`
  to be included. (#17883)
- Linking of bitcode files using `emcc -r` + `-flto` is no longer supported.
  `emcc -r` will now always use lld to link to an object file.  This matches the
  behavior of upstream llvm where bitcode linking using lld does not exist.
  The recommend way to combine bitcode input is to use library files (`ar`
  archives).  See #13492 for more details.

3.1.22 - 09/19/22
-----------------
- compiler-rt updated to LLVM 15. (#17802)
- Using `-Oz` or `-Os` will no longer pass `-fno-inline-functions` to clang and
  instead rely on clang's normal inline heuristics for these optimization
  levels.  `-fno-inline-functions` can be passed explicitly if needed.
- C++17 is now the default version of the C++ standard used by the compiler.
  This is due to an upstream change in llvm.  Use `-std=c++14` (or technically
  `-std=gnu++14`) to revert to the previous default.
- Closure warnings are now controlled via the standard `-Wclosure` warning flags
  rather than via a specific/custom `CLOSURE_WARNINGS` setting.  The old
  setting continues to work but will issue a deprecation warning.
  If you link with `-Werror` but you don't want closure warnings to be errors
  you can add `-Wno-error=closure` or `-Wno-closure`.

3.1.21 - 09/09/2022
-------------------
- Update SDL2 port to 2.24.0 (#17748)
- The `LEGACY_RUNTIME` setting is no longer enabled by default.  If you use any
  of these legacy runtime functions (except in library code with explicit
  dependencies) then you would need to set `LEGACY_RUNTIME` on the command line
  or add the ones you need to `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE`:
   - addFunction
   - removeFunction
   - allocate
   - AsciiToString
   - stringToAscii
   - UTF16ToString
   - stringToUTF16
   - lengthBytesUTF16
   - UTF32ToString
   - stringToUTF32
   - lengthBytesUTF32
   - allocateUTF8
   - allocateUTF8OnStack
   - writeStringToMemory
   - writeArrayToMemory
   - writeAsciiToMemory
   - intArrayFromString
   - intArrayToString
   - warnOnce
   - ccall
   - cwrap
  Although this is technically a breaking change for those who use these
  functions, there are assertion in debug builds that catch such usages and
  direct towards how to fix the issue.

3.1.20 - 08/24/2022
-------------------
- The `getTempRet0`/`setTempRet0` helper functions are now implemented directly
  in WebAssembly, rather than supplied by the JS host.  This simplifies the
  wasm/JS interface.  These function are no longer exported in all cases.  If
  your code directly calls these functions from JS, you can add them to
  `-sEXPORTED_RUNTIME_METHODS`.
- Several linux-specific headers were removed from the emscripten sysroot. None
  of the functionality in these headers was ever supported by emscripten. For
  example `sys/soundcard.h` and `sys/ptrace.h`. (#17704)

3.1.19 - 08/17/2022
-------------------
- Old method of metadata extraction via wasm-emscripten-finalize removed
  in favor of local python code. (#16529)

3.1.18 - 08/01/2022
-------------------
- The tests/ directory was renamed to just test/ to match other project close
  emscripten (llvm, wabt, binaryen).  This should not effect any users of
  emscripten, only developers. (#17502)
- The llvm version that emscripten uses was updated to 16.0.0 (#17534)
- worker.js now propagates unhandled promise rejections to the main thread the
  same way it propagates uncaught exceptions.

3.1.17 - 07/22/2022
-------------------
- Add support for dynamic linking with Asyncify. (#15893)
- A source map file and DWARF info in the wasm can now be emitted at the same if
  the user gives the both options: `-g -gsource-map`. (#17484)
- The `align` argument to the makeGetValue/makeSetValue JS library macros was
  removed (and replaced with an assert) as it had no uses internally and was
  removed (and replaced with an assert) as it had now uses internally and was
  untested.
- Removed unused `calledMain` global variable from the JS runtime.  Folks who
  want to know when main is done can use `Module[postRun]`.

3.1.16 - 07/14/2022
-------------------
- Warnings generated by the JS compiler (compiler.js) are now propagated back
  to the emcc compiler driver which means that `-Werror` builds will now fail
  in the presence of JS compiler warnings.  As usual `-Wno-js-compiler` and
  `-Wno-error=js-compiler)` can be used to control these warnings. (#17365)
- When JS library functions are included as part of `EXPORTED_RUNTIME_METHODS`
  it is no longer necessary to also add them to
  `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE`.  This change allows us to transition
  runtime functions to JS library functions without the need to folks to add
  `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE`. (#17369)
- The following function, which were previously part of the default runtime, are
  now JS library functions:
   - addFunction
   - removeFunction
   - allocate
   - AsciiToString
   - stringToAscii
   - UTF16ToString
   - stringToUTF16
   - lengthBytesUTF16
   - UTF32ToString
   - stringToUTF32
   - lengthBytesUTF32
   - allocateUTF8
   - allocateUTF8OnStack
   - writeStringToMemory
   - writeArrayToMemory
   - writeAsciiToMemory
   - intArrayFromString
   - intArrayToString
   - warnOnce
   - ccall
   - cwrap
  However, they all still available by default due to a new setting called
  `LEGACY_RUNTIME` which is enabled by default.  When `LEGACY_RUNTIME` is
  disabled (which it may be in the future) these symbols would only be included
  if there were explicitly exported via `EXPORTED_RUNTIME_METHODS` or added to
  `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE`.  `LEGACY_RUNTIME` is disabled by default
  in `STRICT` mode so this change only effects users of `STRICT` mode. (#17370,
  #17403)
- The `run` runtime function is no longer exported by default.  It can be added
  to `EXPORTED_RUNTIME_METHODS` if needed.
- The getWasmTableEntry/setWasmTableEntry library function are no longer
  included by default.  Add them to `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE` or
  `EXPORTED_RUNTIME_METHODS` if you want to use them outside of JS library code.
- The type of `time_t` was restored 64-bit after being converted to 32-bit in
  3.1.11. (#17401)

3.1.15 - 07/01/2022
-------------------
- The JS library helper function `hasExportedFunction` has been replaced with
  `hasExportedSymbol`, which takes an unmangled (no leading underscore) symbol
  name.
- Post-link metadata extraction switched from wasm-emscripten-finalize
  (binaryen) to python code within emscripten.  This change should have no
  semantic effect, but can temporarily be reverted by setting
  `EMCC_READ_METADATA=binaryen` in the environment. (#17337)
- Added a shim for `BigInt64Array` so `-sWASM_BIGINT` can be used in Safari
  v14. (#17103)

3.1.14 - 06/20/2022
-------------------
- emcc now accepts `-print-file-name` and reports the correct library paths in
  `-print-search-dirs`.
- `tools/file_packager` no longer generates (or requires) any "pre-js" code when
  running in `--embed-file` mode.  Instead the embedded files are loaded at
  static constructor time.
- Emscripten now knows what minimum browser versions the `WASM_BIGINT` feature
  requires and will automatically set the defaults accordingly. (#17163)
- Embind now supports binding to functions specified as `noexcept`. (#17140)
- Weak undefined symbols fixed in dynamic linking. (#17164)
- Internally, the name of `main` function now gets mangled (by clang) in the
  same way as with other wasm targets.  This means that within the wasm module
  the name of the main function can now be `__main_argc_argv`, but, since we
  still export this to JS as `_main`, this should not be a user-visible change.
- Use of pkg-config from cmake not longer causes the C++ include path to be
  broken. (#17137)
- `emscripten_runtime_keeplive_push()` and `emscripten_runtime_keeplive_push()`
  are now exposed to native code and can be used to keep the runtime alive
  without immediately unwinding the event loop (as
  `emscripten_exit_with_live_runtime()` does). (#17160)
- The file packager option `--use-preload-cache` now only invalidates the
  cache if the data contents has changed. (#16807)

3.1.13 - 06/02/2022
-------------------
- xlocale.h compatibility header was restored after being removed in 3.1.12.

3.1.12 - 05/25/2022
-------------------
- Fix crash, introduced in 3.1.11, which occurred when using pointer types
  (types ending in `*`) with getValue/setValue library functions. (#17028)
- Fix possible deadlock in multi-threaded builds that use EXIT_RUNTIME=0 with
  ASSERTIONS enabled. This was introduced in 3.1.3 as part of #16130. (#17044)

3.1.11 - 05/21/2022
-------------------
- The return value of `emscripten_get_compiler_setting` was changed from `int`
  to `long` so that it is compatible with `MEMORY64`.  The return value of this
  function sometimes contains a pointer value so `int` is not sufficiently
  wide under `wasm64`. (#16938)
- The `EM_BUILD_VERBOSE` environment variable only effects test code these days
  and therefore was renamed to `EMTEST_BUILD_VERBOSE`. (#16904)
- compiler-rt updated to LLVM 14. (#16991)
- libc++ updated to LLVM 14. (#17000)
- libc++abi updated to LLVM 14. (#16993)

3.1.10 - 05/02/2022
-------------------
- Bug fixes

3.1.9 - 04/21/2022
------------------
- The `-sSHELL_FILE` setting, which (unlike the --shell-file command line
  options) we believe was never tested or externally used, has been removed.
  (#16589)
- A warning is now issued when passing C++-only settings such
  `-sEXCEPTION_CATCHING_ALLOWED` when not linking as C++. (#16609)
- Introduce a new [proxying
  API](https://emscripten.org/docs/api_reference/proxying.h.html) for proxying
  work between threads.

3.1.8 - 03/24/2022
------------------
- Command line settings (`-s`) are now type checked.  For example, passing a
  string to a boolean setting will now generate an error (e.g.
  `-sEXIT_RUNTIME=foo`).  Previously, the value of `foo` would have have been
  interpreted as non-zero and accepted as valid. (#16539)
- A warning (limited-postlink-optimizations) was added that gets shown when
  binaryen optimizations are limited due to DWARF information being requested.
  Several binaryen passed are not compatible with the preservation of DWARF
  information. (#16428)
- Use normalized mouse wheel delta for GLFW 3 in `library_glfw.js`. This changes
  the vertical scroll amount for GLFW 3. (#16480)
- The emsdk binaries for macOS now require macOS 10.14 Mojave (or above).
  Prior versions of emsdk could run on 10.11 (or above), but supporting those
  older versions recently became harder.
- The SDL_mixer library can be configured to support MIDIs by passing "mid"
  to the SDL2_MIXER_FORMATS setting. It uses Timidity, and you must provide
  your own instrument sound files and mount them at "/etc/timidity". (#16556)

3.1.7 - 03/07/2022
-------------------
- Remove unmaintained ASMFS filesystem backend and associated `-sASMFS`
  settings.  The new wasmfs filesystem is far enough along that it seems clear
  that ASMFS will not need to be revived.
- Fix deadlock in `munmap` that was introduced in 3.1.5.  The deadlock would
  occur in multi-threaded programs when a partial unmap was requested (which
  emscripten does not support). (#16413)
- Added new compiler+linker option -sSHARED_MEMORY=1, which enables targeting
  a shared WebAssembly.Memory. (#16419)
- Added new API "Wasm Workers", which is an alternative to pthreads for building
  multithreaded applications, enabled via -sWASM_WORKERS=1 (#12833)

3.1.6 - 02/24/2022
------------------
- Remove support for deprecated `EMMAKEN_COMPILER`, `EMMAKEN_CFLAGS`, and
  `EMMAKEN_NO_SDK`  environment variables.  These are all legacy and redundant
  in the face of other settings/flags:
   - `EMMAKEN_COMPILER` -> `LLVM_ROOT` in the config settings
   - `EMMAKEN_CFLAGS` -> `EMCC_CFLAGS`
   - `EMMAKEN_NO_SDK` -> standard `-nostdlib` and `-nostdinc` flags
- emscripten will no longer automatically create a config file if it can't
  find one in the configured location.  Instead, it will error out and point the
  user to the `--generate-config` option, in case that is what they want.
  (#13962)

3.1.5 - 02/17/2022
------------------
- Emscripten no longer uses the `allocate()` runtime function.  For backwards
  compatibility with external JS code we still include this function by default
  but it will no longer be included in `-sSTRICT` mode.  Usages of this function
  are generally best replaced with `_malloc`, `stackAlloc` or `allocateUTF8`.

3.1.4 - 02/14/2022
------------------
- Due to an llvm change (https://reviews.llvm.org/D118573) some clang flags
  that did not previously have any effect are now honored (e.g.
  `-fnew-alignment` and `-fshort-wchar`).
- llvm dependency updated to 15.0.0 to match upstream. (#16178)
- The `__EMSCRIPTEN_major__`, `__EMSCRIPTEN_minor__` and `__EMSCRIPTEN_tiny__`
  macros are now available via the `emscripten/version.h` header file.  For the
  time being, unless you enable `-sSTRICT`, these are still also defined
  directly on the command line.  If you use these macros please make sure you
  include `emscripten/version.h` (or `emscripten.h` which indirectly includes
  it). (#16147)

3.1.3 - 01/31/2022
------------------
- The file packager now supports embedding files directly into wasm memory and
  `emcc` now uses this mode when the `--embed-file` option is used.  If you
  use `file_packager` directly it is recommended that you switch to the new mode
  by adding `--obj-output` to the command line. (#16050)
- The `--bind` flag used to enable embind has been deprecated in favor of
  `-lembind`.  The semantics have not changed and the old flag continues to
  work. (#16087)
- New setjmp/longjmp support using Wasm EH instructions is added, which is
  faster and reduces code size. You need a browser that supports Wasm EH spec to
  use it. The new SjLj support is enabled by `-sSUPPORT_LONGJMP=wasm`. This can
  be used with Wasm exception support (`-fwasm-exceptions`), but not with
  Emscripten exception support (`-fexceptions` or
  `-sDISABLE_EXCEPTION_CATCHING=0`). When using Wasm EH with Wasm SjLj, there is
  one restriction that you cannot directly call `setjmp` within a `catch`
  clause. (Calling another function that calls `setjmp` is fine.)
  (#14976 and #16072)

3.1.2 - 01/20/2022
------------------
- A new setting, `POLYFILL`, was added which is on by default but can be disabled
  (via `-sNO_POLYFILL`) to prevent emscripten from outputting needed polyfills.
  For default browser targets, no polyfills are needed so this option only has
  meaning when targeting older browsers.
- `EVAL_CTORS` has been rewritten and improved. The main differences from before
  are that it is much more capable (it can now eval parts of functions and not
  just all or nothing, and it can eval more wasm constructs like globals). It is
  no longer run by default, so to use it you should build with `-s EVAL_CTORS`.
  See `settings.js` for more details. (#16011)
- `wasmX` test suites that are defined in `test_core.py` have been renamed to
  `coreX` to better reflect where they are defined.  The old suite names such
  as `wasm2` will continue to work for now as aliases.

3.1.1 - 01/08/2022
------------------
- Happy new year!
- Updated SDL 2 port to upstream version 2.0.18 (from a patched 2.0.10). This
  includes all downstream patches and many upstream changes.
- libc++ library updated to llvm-13. (#15901)
- libc++-abi library updated to llvm-13. (#15904)
- compiler-rt library updated to llvm-13. (#15906)
- Added new internal/debugging related environment variable
  EM_FORCE_RESPONSE_FILES that can be set to 0 to force disable the use of
  response files, and to 1 to force enable response files. If not set,
  response files will be used if command lines are long (> 8192 chars). (#15973)

3.1.0 - 12/22/2021
------------------
- Emscripten in starting to use ES6 features in its core libraries (at last!).
  For most users targeting the default set of browsers this is a code size win.
  For projects targeting older browsers (e.g. `-sMIN_CHROME_VERSION=10`),
  emscripten will now run closure compiler in `WHITESPACE_ONLY` mode in order to
  transpile any ES6 down to ES5.  When this automatic transpilation is performed
  we generate a warning which can be disabled (using `-Wno-transpile`) or by
  explicitly opting in-to or out-of closure using `--closure=1` or
  `--closure=0`. (#15763).

3.0.1 - 12/17/2021
------------------
- Deprecate `EMMAKEN_CFLAGS` is favor of `EMCC_CFLAGS`.
- Fixed an issue where user provided --js-library directives would not be
  processed as the last item after all system provided JS libraries have been
  added to the build. This fix enables overriding WebGL 2 symbols from user JS
  libraries.

3.0.0 - 11/22/2021
------------------
- A set of internally-unused functions were removed from `parseTools.js`.  While
  emscripten no longer needs any of these functions, there is slim chance that
  some external JS library is depending on them.  Please file issues if any such
  library code is found.  The removed/unused functions are:
   `removePointing`, `pointingLevels`, `removeAllPointing`, `isVoidType`,
   `isStructPointerType`, `isArrayType`, `isStructType`, `isVectorType`,
   `isStructuralType` `getStructuralTypeParts`, `getStructuralTypePartBits`,
   `isFunctionDef`, `isPossiblyFunctionType`, `isFunctionType`, `getReturnType`,
   `splitTokenList`, `_IntToHex`, `IEEEUnHex`, `Compiletime.isPointerType`,
   `Compiletime.isStructType`, `Compiletime.INT_TYPES`, `isType`.
- The example `shell.html` and `shell_minimal.html` templates no longer override
  `printErr` on the module object.  This means error message from emscripten and
  stderr from the application will go to the default location of `console.warn`
  rather than `console.error`.  This only effects application that use the
  example shell html files.
- The version of musl libc used by emscripten was upgraded from v1.1.15 to
  v1.2.2.  There could be some minor size regressions (or gains) due to changes
  in upstream musl code but we don't expect anything major.  Since this is a
  fairly substantial change (at least internally) we are bumping the major
  version of Emscripten to 3. (#13006)
- Added support for specifying the text encoding to be used in response filenames
  by passing the encoding as a file suffix (e.g. "a.rsp.utf-8" or "a.rsp.cp1252").
  If not specified, the encoding is autodetected as either UTF-8 or Python
  default "locale.getpreferredencoding()". (#15406, #15292, #15426)

2.0.34 - 11/04/2021
-------------------
- Symbols marked as visibility hidden are no longer exported from C/C++
  code when building with `SIDE_MODULE`, `MAIN_MODULE` or `LINKABLE`.  If you
  need to export a hidden symbol you can still do so by adding it to
  EXPORTED_FUNCTIONS.

2.0.33 - 11/01/2021
-------------------
- Bug fixes

2.0.32 - 10/19/2021
-------------------
- Internal-only library functions can now be marked as `__internal: true` in JS
  system libraries.  Such symbols should not be used by external libraries and
  are subject to change.  As of now we generate warning when external libraries
  depend on the these symbols.
- Stub functions from `library_syscall.js` and `library.js` were replaced with
  native code stubs (See `system/lib/libc/emscripten_syscall_stubs.c`).  This
  should be better for wasm module portability as well as code size.  As part
  of this change the return value of `popen` was fixed to return NULL rather
  than -1 and the `getpwnam` family of functions were changed to return an
  error rather than throw a JavaScript exception (this behaviour matches what
  the other stub functions do).  As before, the `ALLOW_UNIMPLEMENTED_SYSCALLS`
  setting controls whether of not these stubs get included at link time, and
  `STRICT` disables this setting.
- Emscripten will now warn when linker-only flags are specified in
  compile-only (`-c`) mode.  Just like with clang itself, this warning can be
  disabled using the flag: `-Wno-unused-command-line-argument`.
- Internal symbol names for musl syscalls changed from number-based (e.g.
  `__syscall22`) to name-based (e.g. `__syscall_open`).  This should not be
  a visible change except for folks trying to intercept/implement syscalls
  in native code (#15202).
- Fixed launcher batch script issues on Windows, and added two env. vars
  EM_WORKAROUND_PYTHON_BUG_34780 and EM_WORKAROUND_WIN7_BAD_ERRORLEVEL_BUG that
  can be enabled to work around a Windows Python issue
  https://bugs.python.org/issue34780 , and a Windows 7 exit code issue (#15146)
- Support a new CMake property `EMSCRIPTEN_SYSTEM_PROCESSOR` which can be used
  to override the default value of `CMAKE_SYSTEM_PROCESSOR` set by the
  toolchain file.
- Remove support for the `EMIT_EMSCRIPTEN_METADATA` setting.  This setting has
  been deprecated for some time now and we don't know of any remaining reasons to
  keep it around.
- Add JavaScript API `Emval.{toHandle, toValue}` as well as a C++ method
  `val::as_handle()` to allow passing values between the `val` class and
  `EM_JS`/ `EM_ASM` JavaScript snippets. (#15279)
- Added SAFE_HEAP=2 option which tests safe heap behavior for wasm-only builds
  (allowing unaligned memory accesses, which would not work in Wasm2JS but in
   wasm would be correct but potentially slow).

2.0.31 - 10/01/2021
-------------------
- Bug fixes

2.0.30 - 09/14/2021
-------------------
- Bug fixes

2.0.29 - 08/26/2021
-------------------
- Bug fixes

2.0.28 - 08/23/2021
-------------------
- Added some support for signal handling libc functions (raise, kill,
  sigaction, sigpending, etc).  We still don't have a way to deliver signals from
  the outside but these at least now work for sending signals to the current
  thread (JS context) (#14883).
- Remove the workaround used in emcmake and emmake that removed directories
  with sh.exe from PATH on Windows when MinGW Makefiles generator was used.
  This was needed with CMake versions older than 3.17.0. If you get an error
  "sh.exe was found in your PATH" on Windows, you can either update to CMake
  3.17.0 or newer, or remove the offending directory from your PATH. See
  https://github.com/Kitware/CMake/commit/82ddcf0db1d220564145122c3cce25d25ee0e254
  for more information. (#14930)

2.0.27 - 08/12/2021
-------------------
- Added `EM_ASYNC_JS` macro - similar to `EM_JS`, but allows using `await`
  inside the JS block and automatically integrates with Asyncify without
  the need for listing the declared function in `ASYNCIFY_IMPORTS` (#9709).
- Errors that occur on pthreads (e.g. uncaught exception) will now get re-thrown
  on the main thread rather than simply being logged (#13666).

2.0.26 - 07/26/2021
-------------------
- When building ports with the `embuilder` tool some of the names of the
  libraries have changed (they now match the filenames in the `tools/ports/`
  directory). For example `sdl-image` is now `sdl_image` (#14737).
- Undefined data symbols (in static executables) are no longer silently ignored
  at link time.  The previous behaviour (which was to silently give all
  undefined data symbols address zero, which could lead to bugs)
  can be enabled by passing either `-Wl,--allow-undefined` or
  `-Wl,--unresolved-symbols=ignore-all`.
- The alignment of `long double`, which is a 128-bit floating-point value
  implemented in software, is reduced from 16 to 8. The lower alignment allows
  `max_align_t` to properly match the alignment we use for malloc, which is 8
  (raising malloc's alignment to achieve correctness the other way would come
  with a performance regression). (#10072)
- The `alignMemory` function is now a library function and therefore not
  included by default.  Debug builds will automatically abort if you try
  to use this function without including it.  The normal library `__deps`
  mechanism can be used to include it, or can be added to
  `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE`.
- dlopen can now load libraries at runtime from the web without preloading
  or embedding.  This features relies on `ASYNCIFY` to suspend execution until
  the library is loaded and then continue on as if dlopen was blocking.  For
  users who don't want to use `ASYNCIFY` (which has a size and runtime cost)
  there is a async (callback-based) version of the dlopen API available as
  `emscripten_dlopen()` declared in `emscropten/emscripten.h`.  See
  `docs/api_reference/emscripten.h.rst` (or the online version) for more
  details.
- Constructors, functions and methods bound with Embind can now be `await`ed.
  When Asyncify is used somewhere in the callstack, previously Embind would
  return `0` / `null` / `false` / instance with a NULL pointer, making it
  impossible to wait for the function to actually finish and retrieve its
  result. Now in those cases it will return a `Promise` instead that will
  resolve with the function's return value upon completion. (#11890)

2.0.25 - 06/30/2021
-------------------
- Support for the 'shell' environment is now disabled by default.  Running under
  `d8`, `js`, or `jsc` is not something that most emscripten users ever want to
  do, so including the support code is, more often than not, unnecessary.  Users
  who want shell support can enable it by including 'shell' in `-s ENVIRONMENT`
  (#14535).
- A new setting called `ALLOW_UNIMPLEMENTED_SYSCALLS` was added.  This setting
  is enabled by default but, if disabled, will generate link-time errors if
  a program references an unimplemented syscall.  This setting is disabled
  by default in `STRICT` mode.
- By default (unless `EXIT_RUNTIME=1` is specified) emscripten programs running
  under node will no longer call `process.exit()` on `exit()`.  Instead they
  will simply unwind the stack and return to the event loop, much like they do
  on the web.  In many cases the node process will then exit naturally if there
  is nothing keeping the event loop going.
  Note for users of node + pthreads: Because of the way that threads are
  implemented under node multi-threaded programs now require `EXIT_RUNTIME=1`
  (or call `emscripten_force_exit`) in order to actually bring down the process.
- Drop support for node versions older than v5.10.0.  We now assume the
  existence of `Buffer.from` which was added in v5.10.0.  If it turns out
  there is still a need to support these older node versions we can
  add a polyfil under LEGACY_VM_SUPPORT (#14447).

2.0.24 - 06/10/2021
-------------------
- Support `--preload-file` in Node.js. (#11785)
- System libraries are now passed to the linker internally via `-lfoo` rather
  than using their full path.  This is in line with how gcc and clang pass system
  libraries to the linker.  This should not effect any builds unless a project a
  happens to have, for example, a file called `libc.a` in one of its library
  paths.  This would have the effect of overriding the system library (as it
  would with gcc or clang) (#14342).
- CMake projects (those that either use emcmake or use Emscripten.cmake
  directly) are new configured to install (by default) directly into the
  emscripten sysroot.  This means that running `cmake --install` (or running the
  install target, via `make install` for example) will install resources into
  the sysroot such that they can later be found and used by `find_path`,
  `find_file`, `find_package`, etc.  Previously the default was to attempt to
  install into the host system (e.g `/usr/local`) which is almost always not
  desirable.  Folks that were previously using `CMAKE_INSTALL_PREFIX` to build
  their own secondary sysroot may be able to simplify their build system by
  removing this completely and relying on the new default.
- Reinstated the warning on linker-only `-s` settings passed when not linking
  (i.e. when compiling with `-c`).  As before this can be disabled with
  `-Wno-unused-command-line-argument` (#14182).
- Standalone wasm mode no longer does extra binaryen work during link. It used
  to remove unneeded imports, in hopes of avoiding nonstandard imports that
  could prevent running in WASI VMs, but that has not been needed any more. A
  minor side effect you might see from this is a larger wasm size in standalone
  mode when not optimizing (but optimized builds are unaffected). (#14338)
- You can now explicitly request that an environment variable remain unset by
  setting its value in `ENV` to `undefined`. This is useful for variables, such
  as `LANG`, for which Emscripten normally provides a default value.

2.0.23 - 05/26/2021
-------------------
- libcxxabi updated to llvm-12. (#14288)
- libcxx updated to llvm-12. (#14249)
- compiler-rt updated to llvm-12. (#14280)

2.0.22 - 05/25/2021
-------------------
- Fix a crash bug that was present in 2.0.21 with the use of `-g`.  See
  https://reviews.llvm.org/D102999.
- wasm-ld will now perform string tail merging in debug string sections as well
  as regular data sections.   This behaviour can be be disabled with `-Wl,-O0`.
  This should significantly reduce the size of dwarf debug information in the
  wasm binary.
- The experimental SPLIT_MODULE setting now expects the secondary module to be
  named `<module>.deferred.wasm` instead of `<module>.wasm.deferred`.
- sendfile.h header removed from musl. (#14248)

2.0.21: 05/18/2021
------------------
- Options such as EXPORTED_FUNCTIONS that can take a response file containing
  list of symbols can now use a simple one-symbol-per-line format.  This new
  format is much simpler and doesn't require commas between symbols, opening
  or closing braces, or any kind of escaping for special characters.
- The WebAssembly linker (`wasm-ld`) now performs string tail merging on any
  static string data in your program.   This has long been part of the native
  ELF linker and should not be observable in well-behaved programs.  This
  behavior can be disabled by passing `-Wl,-O0`.
- The functions `fork`, `vfork`, `posix_spawn` and `system` now fail with
  the errno value `ENOSYS` (52) rather than `EAGAIN` (6).  This is more
  correct, since they will never work and attempting to retry won't help.
- `EXPORT_ES6` will now emit static URLs for main WebAssembly file as well
  as for helper Worker used by `-pthread` that can be statically detected
  by modern bundlers at build time. In particular, you no longer have to set
  `Module.locateFile` hook and `Module.mainScriptUrlOrBlob` settings -
  both bundlers and browsers should pick up the required files automatically.
  Note: this doesn't yet cover other use-cases that emit external files,
  such as dynamic linking, `--proxy-to-worker`, external memory etc. (#14135)
- `EXPORT_ES6` can now be used in combination with `-o [filename].html`. (#14165)
- `EXPORT_ES6` no longer requires setting custom `EXPORT_NAME` too. (#14139)
- New diagnostics allow Emscripten to issue warnings when using Intel SIMD
  intrinsics (from xmmintrin.h) which have slow emulations rather than fast
  WebAssembly equivalents. To enable them, define WASM_SIMD_COMPAT_SLOW
  in the preprocessor (#14152)

2.0.20: 05/04/2021
------------------
- This ChangeLog and the `emscripten-version.txt` file that is checked into
  the repository now reflect the next, upcoming, release once a release is
  made.  Previously they would continue to reflect the old release until after
  we decide to cut the release.  Switching to this method allow for a slightly
  simpler release process that also allows us to tag a version that contains
  the correct version information.
- The version string reported by `-v`/`--version` now includes a `-git` suffix
  (e.g. `2.0.19-git`) when running from git checkout (to help distinguish
  unreleased git versions from official releases) (#14092).
- Temporarily back out new `-Wunused-command-line-argument` warnings introduced
  in 2.0.19.

2.0.19: 05/04/2021
------------------
- Emscripten will now warn when linker-only `-s` settings are specified in
  compile-only (`-c`) mode.  Just like with clang itself, this warning can be
  disabled using the flag: `-Wno-unused-command-line-argument`.
- When building with `-s MAIN_MODULE` emscripten will now error on undefined
  symbol by default.  This matches the behvious of clang/gcc/msvc.  This
  requires that your side modules be present on the command line.  If you do not
  specify your side modules on the command line (either directly or via
  `RUNTIME_LINKED_LIBS`) you may need to add `-s WARN_ON_UNDEFINED_SYMBOLS=0` to
  avoid errors about symbol that are missing at link time (but present in your
  side modules provided at runtime).  We hope that this case is not common and
  most users are building with side modules listed on the command line (#14060).
- The `RUNTIME_LINKED_LIBS` setting is now deprecated.  It's better to simply
  list dynamic library dependencies directly on the command line.

2.0.18: 04/23/2021
------------------
- The `makeBigInt` function was removed from the emscripten runtime since it
  had no internal users.
- Restored support for --cache command line flag to configure location of the
  Emscripten cache root directory.
- `EXTRA_EXPORTED_RUNTIME_METHODS` is deprecated in favor of just using
  `EXPORTED_RUNTIME_METHODS`.
- When building with `MAIN_MODULE=2` the linker will now automatically include
  any symbols required by side modules found on the command line.  This means
  that for many users of `MAIN_MODULE=2` it should no longer be necessary to
  list explicit `EXPORTED_FUNCTIONS`.  Also, users of `MAIN_MODULE=1` with
  dynamic linking (not dlopen) who list all side modules on the command line,
  should be able to switch to `MAIN_MODULE=2` and get a reduction in code size.
- When building with `MAIN_MODULE` it is now possible to warn or error on
  undefined symbols assuming all the side modules are passed at link time.  This
  means that for many projects it should now be possible to enable
  `ERROR_ON_UNDEFINED_SYMBOLS` along with `MAIN_MODULE`.

2.0.17: 04/10/2021
------------------
- Use of closure compiler (`--closure`) is now supported when using dynamic
  linking (building with `-s MAIN_MODULE`) (#13880)
- Specifying `EM_CONFIG` inline (python code in the environment variable itself)
  is no longer supported (#13855).  This has been long deprecated but finally
  completely removed.
- Deprecate `-g4`, which is a little confusing as it does not do more than `-g3`
  but instead emits source maps instead of DWARF. `-g4` will now warn. A new
  flag `-gsource-map` enables source maps without warning.
- In order to behave more like clang and gcc, emscripten no longer
  supports some nonstandard methods of library lookup (that worked
  unintentionally and were untested and not documented):
    1. Linking with `-llibc` rather than `-lc` will no longer work.
    2. Linking a library called `foo.a` via `-lfoo` will no longer work.
       (libraries found via `-l` have to start with `lib`)
- Use LLVM's new pass manager by default, as LLVM does. This changes a bunch of
  things about how LLVM optimizes and inlines, so it may cause noticeable
  changes in compile times, code size, and speed, either for better or for
  worse. You can use the old pass manager (until LLVM removes it) by passing
  `-flegacy-pass-manager` (and `-Wl,--lto-legacy-pass-manager` when doing LTO)
  (note however that neither workaround affects the building of system
  libraries, unless you modify emscripten or build them manually). (#13427)
- Removed use of Python multiprocessing library because of stability issues.
  Added a new environment variable `EM_PYTHON_MULTIPROCESSING=1` that can be set
  to revert back to using Python multiprocessing, in case there are reports of
  regressions (that variable is intended to be temporary). (#13493)
- Binaryen now always inlines single-use functions. This should reduce code size
  and improve performance. If you prefer the old default, you can get that with
  `-sBINARYEN_EXTRA_PASSES=--one-caller-inline-max-function-size=1` (#13744).
- Fix generating of symbol files with `--emit-symbol-map` for JS targets.
  When `-s WASM=2` is used. Two symbols are generated:
    - `[name].js.symbols` - storing Wasm mapping
    - `[name].wasm.js.symbols` - storing JS mapping
  In other cases a single `[name].js.symbols` file is created.

2.0.16: 03/25/2021
------------------
- Lists that are passed on the command line can now skip the opening an closing
  braces, allowing for simpler, more readable settings.  e.g.
    `-s EXPORTED_FUNCTIONS=foo,bar`
- Remove/deprecate no longer used `--llvm-opts` command line option.  Any
  arguments not processed by emcc will be passed through to clang directly
  these days.
- Values returned from `sysconf` now more closely match the definitions found in
  header files and in upstream musl (#13713).
- `DISABLE_EXCEPTION_CATCHING=2` is now deprecated since it can be inferred from
  the presence of the `EXCEPTION_CATCHING_ALLOWED` list.  This makes
  `DISABLE_EXCEPTION_CATCHING` a simple binary option (0 or 1) which defaults to
  0 which will be set to 1 internally if `EXCEPTION_CATCHING_ALLOWED` list is
  specified.
- Values returned from `pathconf` now match the definitions found in header files
  and/or upstream musl:
    _PC_LINK_MAX 3200 -> 8
    _PC_SYNC_IO -1 -> 1
    _PC_REC_INCR_XFER_SIZE -1 -> 4096
    _PC_REC_MAX_XFER_SIZE -1 -> 4096
    _PC_SYMLINK_MAX -1 -> 255
- Added support for wrapping emcc and em++ via ccache: install Emscripten port
  of ccache via emsdk, or from https://github.com/juj/ccache/tree/emscripten,
  and run explicitly with "ccache emcc ..." after installing, or automatically
  just with "emcc ..." after activating ccache via emsdk (#13498).
- Added support to use a custom set of substitution characters . # and ? to
  ease passing arrays of C symbols on the command line to ASYNCIFY_* settings.
  (#13477)
- In MINIMAL_RUNTIME build mode, errno support will now be disabled by default
  due to the code size that it adds. (MINIMAL_RUNTIME=1 implies SUPPORT_ERRNO=0
  by default) Pass -s SUPPORT_ERRNO=1 to enable errno support if necessary.
- Using EM_ASM and EM_JS in a side module will now result in an error (since
  this is not implemented yet).  This could effect users were previously
  inadvertently including (but not actually using) EM_ASM or EM_JS functions in
  side modules (#13649).
- Remove dependency on Uglify by finishing the rewrite of passes to acorn
 (#13636, #13621).
- Primary development branch switched from `master` to `main`.

2.0.15: 03/05/2021
------------------
- Calls to `newlocale` (and `new std::locale` in C++) with arbitrary names will
  now succeed.  This is the behaviour of musl libc which emscripten had
  previously inadvertently disabled.
- System libraries are now compiled with debug info (`-g`).  This doesn't
  affect release builds (builds without `-g`) but allows DWARF debugging of
  types defined in system libraries such as C++ STL types (#13078).
- uname machine field is now either wasm32 or wasm64 instead of x86-JS (#13440)
- Several pthreads exit-related fixes (#12985) (#10524).
- Fix IDBFS syncing with existing directories (#13574).
- Add libmodplug port and allow mod files to be played in SDL2 (#13478).
- `emscripten_GetProcAddress` is now part of `libGL`. Normally the change is not
  noticeable, unless you build in `STRICT` mode and do not already have `-lGL`
  to link in that library. If not, add `-lGL`. (#13524)

2.0.14: 02/14/2021
------------------
- Add new setting: `REVERSE_DEPS`. This can be used to control how emscripten
  decides which reverse dependencies to include.  See `settings.js` for more
  information.  The default setting ('auto') is the traditional way emscripten
  has worked in the past so there should be no change unless this options is
  actually used.  This option partially replaces the `EMCC_ONLY_FORCED_STDLIBS`
  environment variable which (among other things) essentially had the effect of
  setting `REVERSE_DEPS` to be 'all'.
- Clang now performs loop unrolling when targeting WebAssembly at -O2 and
  higher. It can be disabled using `-fno-unroll-loops`.

2.0.13: 01/19/2021
------------------
- Remove unused `Browser.safeSetInterval` and `Browser.safeCallback`.  These
  had no callers in emscripten itself or any testing.  If there are users of
  these functions we could re-enable them with some testing.
- Fix race condition when running many emcc processes after clearing the cache.
  The processes would race to run the sanity checks and could interfere with
  each other (#13299).
- Emscripten now builds a complete sysroot inside the EM_CACHE directory.
  This includes the system headers which get copied into place there rather
  than adding a sequence of extra include directories.
- Added support for -s MALLOC=emmalloc when -s MAXIMUM_MEMORY is more than 2GB.
  (#13258)
- Add back support for calling the legacy dynCall_sig() API to invoke function
  pointers to wasm functions from JavaScript. Pass -s DYNCALLS=1
  to include that functionality in the build. This fixes a regression that
  started in Aug 31st 2020 (Emscripten 2.0.2) in #12059. Also implement
  support for dynCall() in MINIMAL_RUNTIME builds. (#13296)
- `SDL2_ttf` now uses upstream compiled with `TTF_USE_HARFBUZ` flag.
- The system for linking native libraries on demand (based on the symbols
  present in input object files) has been removed.  Libraries such as libgl,
  libal, and libhtml5 are now included on the link line by default unless
  `-s AUTO_NATIVE_LIBRARIES=0` is used.  This should not effect most builds
  in any way since wasm-ld ignores unreferenced library files.  Only users
  of the `--whole-archive` linker flag (which is used when `MAIN_MODULE=1` is
  set) should be effected.

2.0.12: 01/09/2021
------------------
- `emscripten/vr.h` and other remnants of WebVR support removed. (#13210, which
  is a followup to #10460)
- Stop overriding CMake default flags based on build type. This will
  result in builds that are more like CMake does on other platforms. You
  may notice that `RelWithDebInfo` will now include debug info (it did not
  before, which appears to have been an error), and that `Release` will
  use `-O3` instead of `-O2` (which is a better choice anyhow). (#13083)

2.0.11: 12/17/2020
------------------
- `emcc -v` no longer forces the running the sanity checks.  Sanity checks
  are always run on first use or can be forced with `--check` or by setting
  `EMCC_DEBUG` is set in the environment.
- An upstream LLVM regression with global initializer linking has been fixed
  (#13038).
- Remove a racy unneeded assertion about async dynamic linking (#13060).

2.0.10: 12/04/2020
------------------
- Fix handling of exit() in pthreads. (#12933)
- Added support for C11 thread API. (#9243)
- The WebAssembly memory used by emscripten programs is now, by default, created
  in the wasm file and exported to JavaScript.  Previously we could create the
  memory in JavaScript and import it into the wasm file.  The new
  `IMPORTED_MEMORY` setting can be used to revert to the old behaviour.
  Breaking change: This new setting is required if you provide a runtime
  value for `wasmMemory` or `INITIAL_MEMORY` on the Module object.
- Internally, emscripten now uses the `--sysroot` argument to point clang at
  it headers.  This should not effect most projects but has a minor effect the
  order of the system include paths: The root include path
  (`<emscritpen_root>/system/include`) is now always last in the include path.
- Fix JS pthreads proxying + WASM_BIGINT (#12935)
- Optimize makeDynCall to use dynCall_xx function directly where needed (#12741)

2.0.9: 11/16/2020
-----------------
- dlopen, in conformace with the spec, now checks that one of either RTDL_LAZY
  or RTDL_NOW flags ar set.  Previously, it was possible set nether of these
  without generating an error.
- Allow `-lSDL2_mixer` to just work. (Others like `-lSDL2` always worked, but
  for `SDL2_mixer` things were broken because we build multiple variations of
  that library.) That link flag is now the same as `-s USE_SDL2_MIXER=2`.
- Stack state is no longer stored in JavaScript.  The following variables have
  been replaced with native functions in `<emscripten/stack.h>`:
  - STACK_BASE
  - STACK_MAX
  - STACKTOP
  - TOTAL_STACK
- The ABI used for importing symbol by address in dynamic linking (MAIN_MODULE +
  SIDE_MODULE) is now the same as the ABI used by llvm and wasm-ld.  That is,
  symbol addresses are imported from the 'GOT.mem' and 'GOT.func' pseudo
  modules.  As one side effect of this change it is now required that JavaScript
  functions that are imported by address are now required to have a `__sig`
  specified in the library JavaScript file.
- `MODULARIZE` + `WASM_ASYNC_COMPILATION=0`, that is, modularize mode but with
  async compilation turned off, so that startup is synchronous, now returns the
  Module object from the factory function (as it would not make sense to return
  a Promise without async startup). See #12647
- Added experimental support for using emscripten as a post link tool.  In this
  case the input to emscripten is a single wasm file (for example the output of
  `wasm-ld`).  When emcc is run with `--post-link` it will take a wasm file as
  input that perform all the normal post link steps such as finalizing and
  optimizing the wasm file and generating the JavaScript and/or html that will
  run it.
- Added emulation support and a build time warning for calling Wasm function
  pointers from JS library files via the old syntax
        {{{ makeDynCall('sig') }}} (ptr, arg1, arg2);
  that was broken on Aug 31st 2020 (Emscripten 2.0.2). A build warning will now
  be emitted if the old signature is used. Convert to using the new signature
        {{{ makeDynCall('sig', 'ptr') }}} (arg1, arg2);
  instead.

2.0.8: 10/24/2020
-----------------
- `-s ASSERTIONS=2` now implies `-s STACK_OVERFLOW_CHECK=2`. Previously only
  `-s ASSERTIONS=1` implied `-s STACK_OVERFLOW_CHECK=1`.
- Dynamic linking (MAIN_MODULE + SIDE_MODULE) now produces wasm binaries that
  depend on mutable globals.  Specifically the stack pointer global is mutable
  and shared between the modules. This is an ABI change for dynamic linking.
  (#12536)
- emcc now accepts `--arg=foo` as well as `--arg foo`.  For example
  `--js-library=file.js`.
- Reject promises returned from the factory function created by using the
  MODULARIZE build option if initialization of the module instance fails
  (#12396).
- emrun: Passing command line flags (arguments that start with `-`) to the
  program bring run now requires a `--` on the command line to signal the
  end of `emrun` arguments. e.g:
    `emrun filename.html -- --arg-for-page`
  This is standard behaviour for command line parsing and simplifies the
  emrun logic.

2.0.7: 10/13/2020
-----------------
- Don't run Binaryen postprocessing for Emscripten EH/SjLj. This lets us avoid
  running `wasm-emscripten-finalize` just for C++ exceptions or longjmp. This
  is an ABI change. (#12399)
- Run `SAFE_HEAP` on user JS code using a new Acorn pass, increasing the
  coverage of those tests to all JS in the output (#12450).
- `EM_LOG_DEMANGLE` is now deprecated.  Function names shown in wasm backtraces
  are never mangled (they are either missing or demangled already) so demangled
  is not possible anymore.
- In STRICT mode we no longer link in C++ mode by default.  This means if you
  are building a C++ program in STRICT mode you need to link via `em++` rather
  than `emcc`.  This matches the behaviour of gcc and clang.
- IDBFS now persists files whenever their timestamp changes; previously it acted
  on sync only if the timestamp increased and ignored the file changes otherwise.
- When `-s SUPPORT_LONGJMP=0` is passed to disable longjmp, do not run the LLVM
  wasm backend path that handles longjmp. Before this only affected linking, and
  now the flag gives you the ability to affect codegen at compile time too. This
  is necessary if one does not want any invokes generated for longjmp at all.
  (#12394)

2.0.6: 10/02/2020
-----------------
- Add new `COMPILER_WRAPPER` settings (with corresponding `EM_COMPILER_WRAPPER`
  environment variable.  This replaces the existing `EMMAKEN_COMPILER`
  environment variable which is deprecated, but still works for the time being.
  The main differences is that `EM_COMPILER_WRAPPER` only wraps the configured
  version of clang rather than replacing it.
- ASAN_SHADOW_SIZE is deprecated. When using AddressSanitizer, the correct
  amount of shadow memory will now be calculated automatically.

2.0.5: 09/28/2020
-----------------
- Fix a rare pthreads + exceptions/longjmp race condition (#12056).
- Add `WEBGL_multi_draw_instanced_base_vertex_base_instance` bindings (#12282).
- Fix a rare pthreads main thread deadlock (that worsened in 2.0.2, but existed
  before). (#12318)
- The WebAssembly table is now created and exported by the generated wasm
  module rather then constructed by the JS glue code.  This is an implementation
  detail that should not affect most users, but reduces code size. (#12296)
- Add `getentropy` in `sys/random.h`, and use that from libc++'s
  `random_device`. This is more efficient, see #12240.
- Fixed `ABORT_ON_WASM_EXCEPTIONS` to work with the recent dynCall changes where
  functions can be called via the WASM table directly, bypassing WASM exports
  (#12269).
- Add `ASYNCIFY_ADVISE` to output which functions have been instrumented for
  Asyncify mode, and why they need to be handled. (#12146)

2.0.4: 09/16/2020
-----------------
- First release with Bazel support.
- Stop including `malloc` and `free` by default. If you need access to them from
  JS, you must export them manually using
  `-s EXPORTED_FUNCTIONS=['_malloc', ..]`.
- Stop running Binaryen optimizations in `-O1`. This makes `-O1` builds a little
  larger but they compile a lot faster, which makes more sense in a "compromise"
  build (in between `-O0` and higher optimization levels suitable for release
  builds). (#12178)
- Add `ERROR_ON_WASM_CHANGES_AFTER_LINK` option that errors if we need to do
  any work in `wasm-emscripten-finalize` or `wasm-opt` after linking. This
  can verify the link is maximally fast and also does no DWARF rewriting.
  (#12173)

2.0.3: 09/10/2020
-----------------
- Breaking changes to calling Wasm function pointers from JavaScript:
  1. It is no longer possible to directly call dynCall_sig(funcPtr, param) to
    call a function pointer from JavaScript code. As a result, JavaScript code
    outside all JS libraries (pre-js/post-js/EM_ASM/EM_JS/external JS code) can no
    longer call a function pointer via static signature matching dynCall_sig, but
    must instead use the dynamic binding function

       dynCall(sig, ptr, args);

    This carries a significant performance overhead. The function dynCall is not
    available in -s MINIMAL_RUNTIME=1 builds.
  2. old syntax for calling a Wasm function pointer from a JS library file used
     to be

      {{{ makeDynCall('sig') }}} (ptr, arg1, arg2);

    This syntax will no longer work, and until Emscripten <2.0.9 causes
    a runtime error TypeError: WebAssembly.Table.get(): Argument 0 must be
    convertible to a valid number.

    New syntax for calling Wasm function pointers from JS library files is

      {{{ makeDynCall('sig', 'ptr') }}} (arg1, arg2);

  See PR #12059 for details.
- The native optimizer and the corresponding config setting
  (`EMSCRIPTEN_NATIVE_OPTIMIZER`) have been removed (it was only relevant to
  asmjs/fastcomp backend).
- Remove `ALLOC_DYNAMIC` and deprecate `dynamicAlloc`. (#12057, which also
  removes the internal `DYNAMICTOP_PTR` API.)
- Add `ABORT_ON_WASM_EXCEPTIONS` which will abort when an unhandled WASM exception
  is encountered. This makes the Emscripten program behave more like a native
  program where the OS would terminate the process and no further code can be
  executed when an unhandled exception (e.g. out-of-bounds memory access) happens.
  Once the program aborts any exported function calls will fail with a "program
  has already aborted" exception to prevent calls into code with a potentially
  corrupted program state.
- Use `__indirect_function_table` as the import name for the table, which is
  what LLVM does.
- Remove `BINARYEN_SCRIPTS` setting.
- The default output format is now executable JavaScript.  Previously we would
  default to output objecting files unless, for example, the output name ended
  in `.js`.  This is contrary to behaviour of clang and gcc.  Now emscripten
  will always produce and executable unless the `-c`, `-r` or `-shared` flags
  are given.  This is true even when the name of the output file ends in `.o`.
  e.g, `emcc foo.c -o foo.o` will produce a JavaScript file called `foo.o`.
  This might surprise some users (although it matches the behavior of existing
  toolchains) so we now produce a warning in this case.

2.0.2: 09/02/2020
-----------------
- Simplify Fetch C API error handling: we used to check if the error code was
  0 and switch that to 404, but that only really helps `file://` URLs, which
  are not very useful for testing anyhow for other reasons (like not working
  in chrome), and it made things more complex. The behavior has been changed
  to be simpler and just leave the browser's error code as it is.
- Enable `--no-heap-copy` file packager option by default, and remove the old
  default behavior entirely. That is the behavior we should have had from the
  beginning as it is more memory-efficient. (#12027)
- `--no-entry` is now required in `STANDALONE_WASM` mode when building a reactor
  (application without a main function).  Previously exporting a list of
  functions that didn't include `_main` would imply this.  Now the list of
  `EXPORTED_FUNCTIONS` is not relevant in the deciding the type of application
  to build. (#12020)
- Allow polymorphic types to be used without RTTI when using embind. (#10914)
- Do not remove `__original_main` using `--inline-main`. We used to do this
  so that it didn't show up in stack traces (which could be confusing because
  it is added by the linker - it's not in the source code). But this has had
  several downsides, so we are stopping that now. This does not affect program
  behavior, unless you look at the wasm internals. However, one noticeable
  effect is that if you use `ASYNCIFY_ADD` or `ASYNCIFY_ONLY` then you may need
  to add `__original_main` to there (since you are doing manual fine-tuning of
  the list of functions, which depends on the wasm's internals). Note that this
  should not matter in `-O2+` anyhow as normal inlining generally removes
  `__original_main`. (#11995)

2.0.1: 08/21/2020
-----------------
- Change the default value of `STACK_OVERFLOW_CHECK` in builds with `ASSERTIONS`
  from 2 to 1. This means that plain debug builds (`-O0`, which enables
  `ASSERTIONS`) do not have the most expensive stack checks on by default. You
  can still add them with `-s STACK_OVERFLOW_CHECK=2`.
- Remove the `RESERVED_FUNCTION_POINTERS` setting, which is no longer needed as
  we have `ALLOW_TABLE_GROWTH`. The old option allowed a fixed number of
  functions to be added to the table, while the new one allows an unlimited
  number. (We needed the old option for fastcomp, which could not support
  growth.) The old setting is mapped to the new one, so that building with
  `-s RESERVED_FUNCTION_POINTERS=K` for any `K > 0` will simply turn on
  table growth. The only noticeable effect of this is that you will be able to
  add an unlimited amount of functions and not just `K`.

2.0.0: 08/10/2020
-----------------
- First release that only supports the new upstream wasm backend (which has been
  the default for a long time) and no longer supports the old fastcomp backend.
  (#11319)
- Python2 is no longer supported by Emscripten.  Emsdk now includes a bundled
  copy of Python3 on both macOS and Windows.  This means that only non-emsdk
  users and linux users should be affected by this change.
- Store exceptions metadata in wasm memory instead of JS. This makes exception
  handling almost 100% thread-safe. (#11518)

1.40.1: 08/01/2020
------------------
- Last release that still has optional support for the old fastcomp backend.
  The new upstream backend, which has been the default for a long time, will
  be the only one supported from 2.0.0 and onward (#11319).
- Fix the WebGL2 regression in 1.40.0 due to #11738 (#11780).
- If input files don't have a known extension assume they are object files
  (linker inputs) rather then source files.  This matches gcc/clang behaviour.
  See #10560.

1.40.0: 07/30/2020
------------------
- This release contains a WebGL2 regression due to #11738.
- The `EM_CONFIG` environment variable and `--em-config` command line option no
  longer support a literal python string. Instead the name of a config file is
  required. Since all config file settings are individually override-able using
  `EM_FOO` this should be enough.
- Running emscripten under python2 is now deprecated.  It will show up as a
  warning (which can be disabled with `-Wno-deprecated`).  Please update to
  python3 as we hope to remove support completely in the next release.

1.39.20: 07/20/2020
-------------------
- Remove the `--save-bc` command line option.  This was specific to fastcomp,
  which is deprecated, and for debugging purposes we already have `EMCC_DEBUG`
  which saves all intermediate files.
- It is now an error if a function listed in the `EXPORTED_FUNCTIONS` list is
  missing from the build (can be disabled via `-Wno-undefined`)
  (ERROR_ON_UNDEFINED_SYMBOLS and WARN_ON_UNDEFINED_SYMBOLS no longer apply
  to these symbols which are explicly exported).
- Support for pthreads with wasm2js (`WASM=0`; #11505).
- Rename `emscripten/math.h` to `emscripten/em_math.h` because if a user adds
  `emscripten/` as an include path with `-I`, that can override libc math.h,
  which leads to very confusing errors.

1.39.19: 07/07/2020
-------------------
- In standalone mode make `main` mandatory by default (#11536). To build a
  library ("reactor"), use `--no-entry`. The compiler will suggest that if
  `main` is not present.
- Automatically resume AudioContexts on user input in SDL and OpenAL (#10843).
- Asyncify now does liveness analysis to find which locals to save
  (Binaryen#2890).
- Settings on the command line no longer require a space between the `-s` and
  the name of the setting.   For example, `-sEXPORT_ALL` is now equivalent to
  `-s EXPORT_ALL`.
- Rename `EXCEPTION_CATCHING_WHITELIST` to `EXCEPTION_CATCHING_ALLOWED`. The
  functionality is unchanged, and the old name will be allowed as an alias
  for a few releases to give users time to migrate.
- Add support for the new add-list in Asyncify and update existing list names
  following the updates in Binaryen, so that now we have `ASYNCIFY_ADD` to
  add a function, `ASYNCIFY_REMOVE` to remove one (previously this was
  called `ASYNCIFY_BLACKLIST`), and `ASYNCIFY_ONLY` to set a list of the
  only functions to instrument and no others (previously this was called
  `ASYNCIFY_WHITELIST`). The updated lists also handle indirect calls properly,
  so that if you use `ASYNCIFY_IGNORE_INDIRECT` and then add (using either the
  add-list or the only-list) all the functions that are on the stack when
  pausing, then things will work (for more, see
  https://github.com/WebAssembly/binaryen/pull/2913).

1.39.18: 06/12/2020
-------------------
- Disable `LIBCXX_ABI_OPTIMIZED_FUNCTION` which is an ABI change in libc++
  (changing the layout of the `std::function` object) (#11403).
- New `WASM2C` option that integrates with wabt's wasm2c tool in order to
  compile everything into a single C file (#11213).

1.39.17: 06/05/2020
-------------------
- Use Promise polyfill for MODULARIZE when supporting legacy browsers. (#11320)
- Fix minification of wasm2js output when using --emit-symbol-map. (#11279)
- On first use, emscripten creates a sample config file.  This config file
  is now created in the emscripten directory by default.  The traditional
  `~/.emscripten` config file in the `$HOME` directory is still supported and
  the sample config will still be written there in the case that the emscripten
  root is read-only.
- The default location for downloaded ports is now a directory called "ports"
  within the cache directory.  In practice these means by default they live
  in `cache/ports` inside the emscripten source directory.  This can be
  controlled by setting the location of the cache directory, or for even more
  fine grained control the `EM_PORTS` environment variable and the `PORTS`
  config setting can be used.
- Added support for compiling SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE 4.2 and
  128-bit wide AVX intrinsics, emulated on top of Wasm SIMD instruction set.
  (#11193, #11243, #11290, #11327). Pass -msimd128 -msse<version> to enable
  targeting SSE.
- Removed obsolete SIMD.js support (-s SIMD=1). Use -msimd128 to target Wasm
  SIMD. (#11180)
- Add warning about fastcomp deprecation (can be disabled via `-Wno-fastcomp`).
- The mmap method of JavaScript filesystem drivers (based on library_fs.js) no
  longer takes a target memory.  It's safer/cleaner/smaller to assume the target
  is the global memory buffer.
- Remove emterpreter and `EMTERPRETIFY` settings.  Emterpreter has largely
  been replaced by asyncify and is fastcomp only so due for removing in
  the near future anyway.
- Upgrade various musl string functions to 1.2 to fix aliasing issues. (#11215)

1.39.16: 05/15/2020
-------------------
- Add Math C API for direct access to JavaScript Math object (#11151).
- Address Sanitizer support now includes JavaScript as well, that is, memory
  access of HEAP\* arrays is checked by ASan. That allows errors to be found if
  JS glue code does something wrong like forget to shift a pointer. To use this,
  just build with ASan normally, `-fsanitize=address` at link (#11147).
- Fix embind string conversions in multithreaded builds (#10844).
- `ALLOW_MEMORY_GROWTH` used to silently disable `ABORTING_MALLOC`. It now
  just changes the default, which means you can pass `-s ABORTING_MALLOC=1` to
  override the default, which was not possible before. (If you pass the flag
  and don't want that behavior, stop passing the flag.) (#11131)
- Change the factory function created by using the `MODULARIZE` build option to
  return a Promise instead of the module instance. That is, beforehand

        Module()

  would return an instance (which was perhaps not ready yet if startup was
  async). In the new model, that returns a Promise which you can do `.then` or
  `await` on to get notified when the instance is ready, and the callback
  receives the instance. Note that both before and after this change
  doing `Module()` creates and runs an instance, so the only change is
  the return value from that call.
  This fixes some long-standing bugs with that option which have been reported
  multiple times, but is a breaking change - sorry about that. To reduce the
  risk of confusing breakage, in a build with `ASSERTIONS` we will show a clear
  warning on common errors. For more, see detailed examples for the current
  usage in `src/settings.js` on `MODULARIZE`. (#10697)
- A new `PRINTF_LONG_DOUBLE` option allows printf to print long doubles at full
  float128 precision. (#11130)
- `emscripten_async_queue_on_thread` has been renamed to
  `emscripten_dispatch_to_thread` which no longer implies that it is async -
  the operation is in fact only async if it is sent to another thread, while it
  is sync if on the same one. A new `emscripten_dispatch_to_thread_async`
  function is added which is always async.
- The emscripten cache now lives in a directory called `cache` at the root
  of the emscripten tree by default.  The `CACHE` config setting and the
  `EM_CACHE` environment variable can be used to override this (#11126).
- Honor `CACHE` setting in config file as an alternative to `EM_CACHE`
  environment variable.
- Remove `--cache` command line arg.  The `CACHE` config setting and the
  `EM_CACHE` environment variable can be used to control this.
- Compiling to a file with no suffix will now generate an executable (JS) rather
  than an object file.  This means simple cases like `emcc -o foo foo.c` do the
  expected thing and generate an executable.
- System libraries such as libc and libc++ are now included by default at
  link time rather than selectively included based on the symbols used in the
  input object files.  For small programs that don't use any system libraries
  this might result in slightly slower link times with the old fastcomp
  backend.  In order to exclude these libraries build with `-nostdlib` and/or
  `-nostdlib++`.

1.39.15: 05/06/2020
-------------------
- Add `--extern-pre-js` and `--extern-post-js` emcc flags. Files provided there
  are prepended/appended to the final JavaScript output, *after* all other
  work has been done, including optimization, optional `MODULARIZE`-ation,
  instrumentation like `SAFE_HEAP`, etc. They are the same as prepending/
  appending those files after `emcc` finishes running, and are just a convenient
  way to do that. (For comparison, `--pre-js` and `--post-js` optimize that code
  together with everything else, keep it in the same scope if running
  `MODULARIZE`, etc.).
- Stop defining `FE_INEXACT` and other floating point exception macros in libc,
  since we don't support them. That also prevents musl from including code using
  pragmas that don't make sense for wasm. Ifdef out other uses of those pragmas
  as well, as tip of tree LLVM now fails to compile them on wasm. (#11087)
- Update libcxx and libcxxabi to LLVM 10 release branch (#11038).
- Remove `BINARYEN_PASSES` setting (#11057). We still have
  `BINARYEN_EXTRA_PASSES` (the removed setting completely overrides the set
  of passes from the command line, which doesn't make much sense as some of
  them are mandatory like setting the sbrk ptr).
- Remove `MODULARIZE_INSTANCE` build option (#11037). This was a seldom used
  option that was complicating the logic for `MODULARIZE`. Module instances can
  be created by using `MODULARIZE` and calling the factory function explicitly.
  See the new `--extern-post-js` option added in this release, which can help
  code that used `MODULARIZE_INSTANCE` (you can add an extern post js which
  does `Module = Module();` for example).

1.39.14: 05/01/2020
-------------------
- Update SDL2 to latest in ports, which has recently been updated to include
  upstream 2.0.10.
- Add warning on use of `EMTERPRETIFY` which is soon to be removed.
- Emscripten can now compile assembly files in llvm's .s/.S file format.
- Remove test-only environment variable handling for `EMCC_LEAVE_INPUTS_RAW`.
  The two uses cases in our test code were covered by the `-nostdlib` option.
- Remove untested `CONFIGURE_CC`.  This could be used to override the underlying
  compiler used in emcc/em++ but only during configure tests.  There are other
  ways to control/fake the detected configure features that don't require such
  monkey patching. For example setting defaults via a site file:
  https://www.gnu.org/software/autoconf/manual/autoconf-2.67/html_node/Site-Defaults.html
- Remove undocumented and untested config settings: `COMPILER_OPTS`.  This was
  a global setting in the emscripten config file that would inject extra
  compiler options.
- Allow spaces in a path to Python interpreter when running emscripten from Unix
  shell (#11005).
- Support atexit() in standalone mode (#10995). This also fixes stdio stream
  flushing on exit in that mode.

v1.39.13: 04/17/2020
--------------------
- Support for WebAssembly BigInt integration with a new `WASM_BIGINT` flag. With
  that the VM will use a JS BigInt for a wasm i64, avoiding the need for JS
  legalization. See #10860.
- Add another value for ENVIRONMENT named 'webview' - it is a companion
  option for 'web' and enables some additional compatibility checks
  so that generated code works both in normal web and in a webview like Cordova.
  See #10846

v1.39.12: 04/09/2020
--------------------
- Pass linker flags directly to wasm-ld by default.  We still filter out certain
  flags explicitly.  If there are other flags that it would be useful for us
  to ignore we can add them to the list of ignored flags.
- Optionally support 2GB+ heap sizes. To do this we make the JS code have unsigned
  pointers (we need all 32 bits in them now), which can slightly increase code
  size (>>> instead of >>). This only happens when the heap size may be over
  2GB, which you must opt into explicitly, by setting `MAXIMUM_MEMORY` to a
  higher value (i.e. by default you do not get support for 2GB+ heaps).
  See #10601
- `--llvm-lto` flag is now ignored when using the upstream llvm backend.
  With the upstream backend LTO is controlled via `-flto`.
- Require format string for emscripten_log.
- Program entry points without extensions are now shell scripts rather than
  python programs. See #10729.  This means that `python emcc` no longer works.
  However `emcc`, `emcc.py` and `python emcc.py` all continue to work.
  The reason for this change is that `#!/usr/bin/env python` is no longer
  portable since the python symlink was dropped from Ubuntu 20.04.
- New EM_IMPORT macro to mark C/C++ symbols as imported from outside the module
  (i.e. imported from JS).  Currently we still default to assuming that *all*
  undefined symbols can come from JS, but in the future we hope to mark such
  symbols explicitly to allow the linker to report on genuinely undefined
  symbols.
- Dynamic linking optimizations: Stop emitting unnecessary `fp$` and `g$`
  accessors in main modules, possible in Binaryen thanks to ensuring function
  table indexes are unique (#10741).
- New `JS_MATH` option to use `Math.*` in JS instead of compiled musl (#10821).
- Pass `Module` to Module callback functions like `Module.preRun` (#10777).
- Support not having ports, for packagers of emscripten that don't want
  them (#10810).
- Rename syscalls to have meaningful names (#10750).

v1.39.11: 03/20/2020
--------------------
- The default c++ version is no longer fixed at c++03.  We now fall back to
  clang's default which is currently c++14.
- Remove arc4random function form library.js.  This is a BSD-only library
  function.  Anyone requiring BSD compat should be able to use something like
  https://libbsd.freedesktop.org/.
- Change the meaning of `ASYNCIFY_IMPORTS`: it now contains only new imports
  you add, and does not need to contain the list of default system imports like
  `emscripten_sleep`. There is no harm in providing them, though, so this
  is not a breaking change.
- Enable DWARF support: When compiling with `-g`, normal DWARF emitting happens,
  and when linking with `-g` we preserve that and update it. This is a change
  from before, where we assumed DWARF was unneeded and did not emit it, so this
  can increase the size of debug builds (i.e. builds compiling and/or linking
  with -g). This change is necessary for full debugging support, that is, to
  be able to build with `-g` and use a debugger. Before this change only the
  `-gforce_dwarf` flag enabled DWARF; that flag is now removed. If you want
  the old behavior, build your object files with `-gline-tables-only` (that will
  only add line table info, which is just enough for things like source maps and
  does not include full debug info). For more info and background see #10325.
- Remove hacks from `memset` handling, in particular, in the wasm backend,
  completely remove the JS version of memset from the JS library and from
  `DEFAULT_LIBRARY_FUNCS_TO_INCLUDE`. The regular C version will be linked in
  from compiler_rt normally. A noticeable difference you may see is that
  a JS library cannot add a `__dep` to `memset` - deps only work for JS
  library functions, but now we only have the regular C version. If you hit that
  issue, just add `_memset` to `EXPORTED_FUNCTIONS` (or adjust
  `deps_info.json`).
- Minimal runtime code size optimizations, see #10725, #10724, #10663.
- wasm2js fix for a long-existing but very rare correctness bug, see #10682.
- Use atomics in musl lock/unlock in pthreads builds, which may fix very rare
  pthreads + stdio issues (none have been reported though). See #10670.

v1.39.10: 03/09/2020
--------------------
- Fix a SIMD regression in 1.39.9 (#10658).
- Fix `emscripten_atomic_exchange_u8,16,32,64` (#10657).
- Switch bzip2 to an emscripten-ports mirror.

v1.39.9: 03/05/2020
-------------------
- Add support for -Wall, -Werror, -w, -Wno-error=, -Werror=, for controlling
  internal emscripten errors. The behavior of these flags matches the gcc/clang
  counterparts.
- Rename `TOTAL_MEMORY` to `INITIAL_MEMORY` and `WASM_MEM_MAX` to `MAXIMUM_MEMORY`,
  which are more accurate and match wasm conventions. The old names are still
  supported as aliases.
- Updated of libc++abi and libc++ to llvm 9.0.0 (#10510)
- Refactor syscall interface: Syscalls are no longer variadic (except those
  that are inherently such as open) and no longer take the syscall number as
  arg0.  This should be invisible to most users but will effect any external
  projects that try to implement/emulate the emscripten syscall interface.
  See #10474
- Removed src/library_vr.js, as it was outdated and nonfunctional, and the WebVR
  specification has been obsoleted in favor of the upcoming WebXR specification.
  (#10460)
- Deprecate `WASM_OBJECT_FILES` setting.  There are many standard ways to enable
  bitcode objects (-flto, -flto=full, -flto=thin, -emit-llvm).
- Removed EmscriptenWebGLContextAttributes::preferLowPowerToHighPerformance
  option that has become unsupported by WebGL. Access
  EmscriptenWebGLContextAttributes::powerPreference instead. (#10505)
- When implementing forwarding function aliases in JS libraries, either the
  alias or the target function must contain a signature annotation. (#10550)
- Add a check in Asyncify builds with `ASSERTIONS` that we do not have
  compiled code on the stack when starting to rewind, which is dangerous.
- Implement libc system() for node.js (#10547).
- Standalone mode improvements, time (#10530, #10536), sysconf (#10535),
  getpagesize (#10533), _Exit (#10534)
- Fix many closure compiler warnings (e.g. #10525).
- Avoid unnecessary syscall proxying (#10511).
- Added new link time command line option -jsDfoo=val to allow specifying
  custom preprocessor options to JS library files. (#10624, #10580)

v1.39.8: 02/14/2020
-------------------
- Add LLD_REPORT_UNDEFINED option that should allow for more detailed
  diagnostics when symbols are undefined at link time.  This currently has
  some limitations and is not enabled by default. For example, EM_JS symbols
  are reported as undefined at link time, as are `__invoke_*` functions.
- wasm2js optimizations. See binaryen#2623.
- WebGPU Compute fixes. Simple examples now work. See #10367.
- Many DWARF debug info fixes. Emitting of DWARF is correct as far as we know,
  including when optimizing (a few passes are disabled for now, but almost all
  work). We still only generate it behind the `-gforce_dwarf` flag for now,
  though (but that should be removed soon).

v1.39.7: 02/03/2020
-------------------
- The checked-in copy of the Closure compiler was removed in favor of getting it
  from npm.  This means that developers now need to run `npm install` after
  checking out emscripten if they want to use closure (--closure).  emsdk users
  are not effected because emsdk runs this as a post install step (#9989).
- Added support for specifying JSDoc minification annotations for Closure in
  JS library, pre and post files. See
  https://github.com/google/closure-compiler/wiki/Annotating-JavaScript-for-the-Closure-Compiler
  (#10272)
- Add new Fibers API for context switching, that supersedes the old coroutine
  API that only ran on fastcomp. See #9859
- Added new linker option -s WASM=2 which produces a dual Wasm+JS build, which
  falls back to using a JavaScript version if WebAssembly is not supported in
  target browser/shell. (#10118)
- Added new linker option -s CLOSURE_WARNINGS=quiet|warn|error that allows aborting
  the build if the Closure compiler produced any warnings.

v1.39.6: 01/15/2020
-------------------
- Development has switched from the "incoming" branch to "master".
- Added new system header <emscripten/heap.h>, which enables querying information
  about the current WebAssembly heap state.
- Reduced default geometric memory overgrowth rate from a very generous 2x factor
  to a more memory conserving +20% factor, and capped maximum reservation to 96MB
  at most.
- Added options MEMORY_GROWTH_GEOMETRIC_STEP and MEMORY_GROWTH_GEOMETRIC_CAP
  to allow customizing the heap growth rates.
- Renamed MEMORY_GROWTH_STEP option to MEMORY_GROWTH_LINEAR_STEP option.
- Added new linker option -s HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS=0
  (default enabled) to allow disabling support for deferred fullscreen mode and
  pointer lock requests for applications that do not need deferring support.

v1.39.5: 12/20/2019
-------------------
- Added support for streaming Wasm compilation in MINIMAL_RUNTIME (off by default)
- All ports now install their headers into a shared directory under
  `EM_CACHE`.  This should not really be a user visible change although one
  side effect is that once a given port is built, its headers are then
  universally accessible, just like the library is universally available as
  `-l<name>`.
- Removed `timestamp` field from mouse, wheel, devicemotion and
  deviceorientation events. The presence of a `timestamp` on these events was
  slightly arbitrary, and populating this field caused a small profileable
  overhead that all users might not care about. It is easy to get a timestamp of
  an event by calling `emscripten_get_now()` or `emscripten_performance_now()`
  inside the event handler function of any event.
- Add fine-grained options for specific legacy browser support,
  `MIN_FIREFOX_VERSION`, `MIN_SAFARI_VERSION`, `MIN_IE_VERSION`,
  `MIN_EDGE_VERSION`, `MIN_CHROME_VERSION`. The existing `LEGACY_VM_SUPPORT`
  option sets all of them to 0, that is, maximal backwards compatibility.
  Note that going forward, we will use these settings in more places, so if
  you do need very old legacy browser support, you may need to set either
  `LEGACY_VM_SUPPORT` or the fine-grained options. For more details see #9937
- Default `DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR` to 1. See #9895.
  With this change the old deprecated HTML5 API event target lookup behavior is
  disabled. There is no "Module.canvas" object, no magic "null" default handling,
  and DOM element 'target' parameters are taken to refer to CSS selectors, instead
  of referring to DOM IDs. For more information see:
  <https://groups.google.com/forum/#!msg/emscripten-discuss/xScZ_LRIByk/_gEy67utDgAJ>
- WASI API updated from `wasi_unstable` to `wasi_snapshot_preview1`. This
  is mostly an implementation detail, but if you use `STANDALONE_WASM` it means
  that the output of emscripten now requires a runtime with
  `wasi_snapshot_preview1` support.
- `SAFE_STACK` has been removed, as it overlaps with `STACK_OVERFLOW_CHECK`.
   Replace `SAFE_STACK=1` with `STACK_OVERFLOW_CHECK=2` (note the value is 2).
   This also has the effect of enabling stack checking on upstream builds when
   `ASSERTIONS` are enabled (as assertions enable `STACK_OVERFLOW_CHECK=2`).

v1.39.4: 12/03/2019
-------------------
- Remove deprecated `requestFullScreen` method from `library_browser.js`, please
  use `requestFullscreen` (without the capital S).
- Remove deprecated `requestFullScreen` and `cancelFullScreen` from `library_glut.js`
- Remove deprecated `requestFullScreen` and `cancelFullScreen`from `library_glfw.js`
- Fix SDL2_mixer support for ogg vorbis. See #9849
- Various source maps fixes, see #9926 #9882 #9837 #9814

v1.39.3: 11/14/2019
------------------

v1.39.2: 11/06/2019
------------------
 - Archives with missing indexes will now have ranlib run on them automatically
   at link time.  This avoids linker errors when using GNU ar to build archive
   files.
 - `ERROR_ON_MISSING_LIBRARIES` now also applies to internal symbols that start
   with `emscripten_`.  Prior to this change such missing symbols would result
   in a runtime error, now they are reported at compile time.
 - Pthread blocking on the main thread will now warn in the console. If
   `ALLOW_BLOCKING_ON_MAIN_THREAD` is unset then the warning is an error.
 - Add `pthread_tryjoin_np`, which is a POSIX API similar to `pthread_join`
   but without blocking.
 - New function `emscripten_has_asyncify()`.
 - Add support for pthreads in Node.js, using Node Workers. See #9745

v1.39.1: 10/30/2019
-------------------
 - Only MEMFS is included by default, others (NODEFS, IDBFS, WORKERFS, PROXYFS)
   must be linked in explicitly, using `-lnodefs.js`, `-lidbfs.js`',
   `-lworkerfs.js`, `-lproxyfs.js`. See #9645

v1.39.0: 10/18/2019
-------------------
 - The emsdk defaults to the upstream backend (instead of fastcomp) from this
   release onward (but both backends are still fully supported). See
   https://emscripten.org/docs/compiling/WebAssembly.html#backends
 - Add support for overriding `.emscripten` config variables using environment
   variables.  Any config variable `FOO` can be overridden by `EM_FOO` in the
   environment.
 - `-Werror` now also turns warnings in the python driver code into errors.
 - Internal settings have moved from `settings.js` to `settings_internal.js`.
   These are settings that are for internal use only and are not set-able from
   the command line.  If we misclassified any of these please open a bug.
 - `STANDALONE_WASM` mode now supports setting up argv via wasi APIs.
 - `STANDALONE_WASM` mode now supports running static constructors in `_start`.

v1.38.48: 10/11/2019
--------------------
 - Add support for `MAIN_THREAD_EM_ASM` in wasm backend. #9560
 - Add ability to disable FETCH worker in Fastcomp backend via
   `USE_FETCH_WORKER=0`.  This is useful for people who use FETCH, but don't
   perform any synchronous fetches on the main thread. #9567
 - Remove `EMCONFIGURE_JS`. Since #6269 we have set it to "2" which means never
   use native, always use JS.

v1.38.47: 10/02/2019
--------------------
 - Add support for FETCH API in WASM backend. This doesn't support FETCH in the
   main thread (`USE_FETCH_WORKER=0` is enforced). #9490
 - Redefine errno values to be consistent with wasi. This will let us avoid
   needing to convert the values back and forth as we use more wasi APIs.
   This is an ABI change, which should not be noticeable from user code
   unless you use errno defines (like EAGAIN) *and* keep around binaries
   compiled with an older version that you link against. In that case, you
   should rebuild them. See #9545.
 - Removed build option `-s ONLY_MY_CODE` as we now have much better solutions
   for that, like building to a wasm object file or using `STANDALONE_WASM`
   etc. (see
   https://github.com/emscripten-core/emscripten/wiki/WebAssembly-Standalone).
 - Emscripten now supports the config file (.emscripten) being placed in the
   emscripten directory rather that the current user's home directory.
   See #9543

v1.38.46: 09/25/2019
--------------------
 - Rename libpthreads to libpthread to match its normal name on other platforms.
   This change should be completely internal to emscripten.
 - Remove redundant `COMPILER_ENGINE` and `JS_ENGINE` options.  We only support
   node as the compiler engine so just use a single `NODE_JS` option for that.
 - Module.abort is no longer exported by default. It can be exported in the normal
   way using `EXTRA_EXPORTED_RUNTIME_METHODS`, and as with other such changes in
   the past, forgetting to export it will show a clear error in `ASSERTIONS` mode.
 - Remove `EMITTING_JS` flag, and replace it with `STANDALONE_WASM`. That flag indicates
   that we want the wasm to be as standalone as possible. We may still emit JS in
   that case, but the JS would just be a convenient way to run the wasm on the Web
   or in Node.js.
 - `ASYNCIFY_BLACKLIST` and `ASYNCIFY_WHITELIST` now support simple '\*' wildcard matching

v1.38.45: 09/12/2019
--------------------

v1.38.44: 09/11/2019
--------------------
 - Remove Binaryen from the ports system. This means that emscripten will
   no longer automatically build Binaryen from source. Instead, either use
   the emsdk (binaries are provided automatically, just like for LLVM), or
   build it yourself and point `BINARYEN_ROOT` in .emscripten to it. See #9409

v1.38.43: 08/30/2019
---------------------
 - noExitRuntime is no longer a property on the Module object. Use `noExitRuntime`
   instead of `Module.noExitRuntime`.

v1.38.42: 08/19/2019
--------------------
 - Add support for [address sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
   and standalone [leak sanitizer](https://clang.llvm.org/docs/LeakSanitizer.html)
   with multiple threads. (#9060, #9076)
 - Remove `ERROR_ON_MISSING_LIBRARIES` setting (it's always on now)
 - Remove the ability to use Python operators in flags that support KB/MB/GB/TB
   suffixes, e.g. `TOTAL_MEMORY`. This means that `-s TOTAL_MEMORY=1024*1024`
   will no longer work. This is done because the mechanism may result in
   execution of arbitrary code via command line flags.

v1.38.41: 08/07/2019
--------------------
 - Remove fastcomp's implementation of Asyncify. This has been deprecated for
   a long time, since we added Emterpreter-Async, and now we have a new Asyncify
   implementation in the upstream wasm backend. It is recommended to upgrade to
   the upstream backend and use Asyncify there if you need it. (If you do still
   need the older version, you can use 1.38.40.)
 - Drop ExitStatus from inheriting from Error(), as that could capture the whole
   global scope, preventing temporary variables at page startup from being garbage
   collected. (#9108)
 - `__builtin_return_address` now requires `-s USE_OFFSET_CONVERTER=1` to work. (#9073)
 - emrun now uses HTTP/1.1 instead of HTTP/1.0.
 - `callMain` is no longer exported by default on Module, to allow better JS
   minification. You must add it to `EXTRA_EXPORTED_RUNTIME_METHODS` if you want
   to call it on Module. (In assertions builds, an error with an explanation is
   shown.)
 - Allow expressions with side effects as `EM_ASM`'s arguments and prohibit
   non-arithmetic arguments (e.g. pointers, functions, arrays, objects). (#9054)
 - `emcc` on Windows now uses native newline byte sequence to get a line to
   print for parse error reporting. (#9088)
 - Internal API update: one can now specialize embind's (un)marshalling for a
   group of types via SFINAE, instead of a single type. (#9089)
 - Options passed on the `Module` object during startup, like `Module.arguments`,
   are now copied to a local (in order to avoid writing `Module.*` everywhere,
   which wastes space). You can still provide them as always, but you can't
   modify `Module.arguments` and other things *after* startup (which is now
   after we've finished processing them). In a build with assertions enabled you
   will get an error if you access those properties after startup. (#9072)

v1.38.40: 07/24/2019
--------------------
 - LLVM backend pthread builds no longer use external memory initialization
   files, replacing them with passive data segments.
 - LLVM backend now supports thread local storage via the C extension `__thread`
   and the C11/C++11 keyword `thread_local`. (#8976)
 - Internal API change: Move read, readAsync, readBinary, setWindowTitle from
   the Module object to normal JS variables. If you use those internal APIs,
   you must change `Module.readAsync()/Module['readAsync']()` to `readAsync()`.
   Note that read is also renamed to `read_` (since "`read`" is an API call in
   the SpiderMonkey shell). In builds with ASSERTIONS, an error message is
   shown about the API change. This change allows better JS minification
   (the names read, readAsync etc. can be minified, and if the variables are
   not used they can be removed entirely). Defining these APIs on Module
   (which was never documented or intended, but happened to work) is also
   no longer allowed (but you can override `read_` etc. from JS).

v1.38.39: 07/16/2019
--------------------
 - Add support for [address sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html). (#8884)
   - Currently, only supports one thread without dynamic linking.
 - Rename Bysyncify (the name used during development) to Asyncify. This keeps
   the name consistent with the old ASYNCIFY flag, no need for a new one, as
   they basically do the same thing.

v1.38.38: 07/08/2019
--------------------
 - Add support for standalone [leak sanitizer](https://clang.llvm.org/docs/LeakSanitizer.html). (#8711)

v1.38.37: 06/26/2019
--------------------
 - Set ENV['LANG'] following the user's preferred language (HTTP Accept-Language / navigator.languages[0])
 - `emscripten_run_script_string` now returns C `NULL` instead of the string `null`
   or `undefined` when the result of the `eval` is JavaScript `null` or `undefined`.
 - Add a new system for managing system libraries. (#8780)
   This may require minor changes when performing certain operations:
     - When using `embuilder.py` to build a specific library, the name may have
       changed: for consistency, all library names are prefixed with lib now.
     - `embuilder.py` now only builds the requested library, and not its dependencies
       and certain system libraries that are always built. For example, running
       `embuilder.py build libc` no longer builds `libcompiler_rt` if it hasn't be built.
     - When using `EMCC_FORCE_STDLIBS` with a list of libraries, you must now use
       the simplified names, for example, `libmalloc` and `libpthreads` instead of
       `libdlmalloc` or `libpthreads_stub`. These names will link in the correct
       version of the library: if the build is configured to use `emmalloc`, `libmalloc`
       will mean `libemmalloc`, and if thread support is disabled, `libpthreads` will
       mean `libpthreads_stub`. This allows you to say `libmalloc` or `libpthreads` without
       worrying about which implementation is supposed to be used, and avoid duplicate
       symbols if you used the wrong implementation.
 - LLVM wasm backend pthreads fixes, see #8811, #8718

v1.38.36: 06/15/2019
--------------------

v1.38.35: 06/13/2019
--------------------
 - Include some [waterfall fixes](https://github.com/WebAssembly/waterfall/pull/541)
   for the emsdk builds on linux regarding libtinfo.
 - NOTE: due to a CI failure, builds for mac and windows were not generated.

v1.38.34: 06/01/2019
--------------------
 - Add support for [undefined behavior sanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).
    - This allows `emcc -fsanitize=undefined` to work. (#8651)
    - The minimal runtime (`-fsanitize-minimal-runtime`) also works. (#8617)

v1.38.33: 05/23/2019
--------------------
 - First release to use the new chromium build infrastructure
   https://groups.google.com/forum/#!msg/emscripten-discuss/WhDtqVyW_Ak/8DfDnfk0BgAJ
 - Add `emscripten_return_address` which implements the functionality of
   gcc/clang's `__builtin_return_address`. (#8617)

v1.38.32: SKIPPED
-----------------
 - The transition from the old to the new CI occurred around here. To avoid
   ambiguity while both CIs were still generating builds, we just tagged a new
   one (1.38.33) on the new CI and skipped 1.38.32.
 - The transition also moves all builds and downloads away from the old
   mozilla-games infrastructure to the new chromium ones. As a result all links
   to *mozilla-games* URLs will not work (these were never documented, but could
   be seen from the internals of the emsdk; the new emsdk uses the proper new
   URLs, so you can either use the sdk normally or find the URLs from there).

v1.38.31: 04/24/2019
--------------------
 - Change `ino_t/off_t` to 64-bits. (#8467)
 - Add port for bzip2 library (`libbz2.a`). (#8349)
 - Add port for libjpeg library. (#8361)
 - Enable `ERROR_ON_MISSING_LIBRARIES` by default (#8461)

v1.38.30: 03/21/2019
--------------------
 - Remove Module.buffer which was exported by default unnecessarily. This was an
   undocumented internal detail, but in theory, code may have relied on it.
   (#8277)

v1.38.29: 03/11/2019
--------------------

v1.38.28: 02/22/2019
--------------------
 - Option `-s EMTERPRETIFY_WHITELIST` now accepts shell-style wildcards;
   this allows matching static functions with conflicting names that
   the linker distinguishes by appending a random suffix.
 - Normalize mouse wheel delta in `library_browser.js`. This changes the scroll
   amount in SDL, GLFW, and GLUT. (#7968)

v1.38.27: 02/10/2019
--------------------
 - Change how `EMCC_LOCAL_PORTS` works, to be more usable. See #7963
 - Remove deprecated Pointer_stringify (use UTF8ToString instead). See #8011
 - Added a new option `-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1` that
   changes the lookup semantics of DOM elements in html5.h event handler
   callbacks and WebGL context creation. The new behavior is to use CSS selector
   strings to look up DOM elements over the old behavior, which was somewhat
   ad hoc constructed rules around default Emscripten uses. The old behavior
   will be deprecated and removed in the future. Build with -s ASSERTIONS=1
   to get diagnostics messages related to this transition.
 - Breaking change with -s USE_PTHREADS=1 + -s FETCH=1: When building with
   -o a.html, the generated worker script is now named "a.fetch.js" according
   to the base name of the specified output, instead of having a fixed name
   "fetch-worker.js".

v1.38.26: 02/04/2019
--------------------
 - Fix some pthreads proxying deadlocks. See #7865

v1.38.25: 01/18/2019
--------------------
 - Move kripken/emscripten,emscripten-fastcomp,emscripten-fastcomp-clang to
   emscripten-core/\*

v1.38.24: 01/17/2019
--------------------
 - Perform JS static allocations at compile time (#7850)

v1.38.23: 01/10/2019
--------------------
 - Remove BINARYEN_METHOD: no more support for interpret modes, and if you want
   non-wasm, use WASM=0.
 - Support specifying multiple possible ENVIRONMENTs (#7809)

v1.38.22: 01/08/2019
--------------------
 - Add Regal port. See #7674
 - System libraries have been renamed to include the `lib` prefix.  If you use
   `EMCC_FORCE_STDLIBS` or `EMCC_ONLY_FORCED_STDLIBS` to select system libraries
   you may need to add the `lib` prefix.
 - Rename `pthread-main.js` to `NAME.worker.js`, where `NAME` is the main
   name of your application, that is, if you emit `program.js` then you'll get
   `program.worker.js` (this allows more than one to exist in the same
   directory, etc.).
 - Dynamic linker has been taught to handle library -> library dependencies.

v1.38.21: 11/30/2018
--------------------
 - fastcomp: Remove `runPostSets` function and replace with normal static
   constructor function. See #7579

v1.38.20: 11/20/2018
--------------------
 - Remove SPLIT_MEMORY option.
 - Move getTempRet0/setTempRet0 to be JS library functions rather than
   auto-generated by fastcomp.
 - Change `strptime()`'s handling of the "%c" to match that of `strftime()`.
   This is a breaking change for code which depends on the old definition of
   "%c".

v1.38.19: 11/15/2018
--------------------

v1.38.18: 11/08/2018
--------------------
 - Wasm dynamic linking: Rename `tableBase/memoryBase` to
   `__table_base/__memory_base` (#7467)

v1.38.17: 11/07/2018
--------------------
 - Minify wasm import and export names. This decreases JS and wasm size by
   minifying the identifiers where JS calls into wasm or vice versa, which
   are not minifiable by closure or other JS-only tools. This happens in
   -O3, -Os and above. See #7431

v1.38.16: 11/02/2018
--------------------
 - Breaking change: Do not automatically set EXPORT_ALL for MAIN_MODULES or
   SIDE_MODULES. This means that you must explicitly export things that will
   be called from outside (normally, on EXPORTED_FUNCTIONS), or
   you can manually enable EXPORT_ALL yourself (which returns to the exact
   same behavior as before). This change brings us in line with more standard
   dynamic linking, and will match what the LLVM wasm backend will have.
   See #7312.
 - Invalid -s flags on the command line are now treated as errors.
 - Remove BUILD_AS_SHARED_LIBRARY setting.

v1.38.15: 10/25/2018
--------------------

v1.38.14: 10/22/2018
--------------------
 - Errors are now reported when functions listed in EXPORTED_FUNCTIONS are not
   defined. This can be disabled via ERROR_ON_UNDEFINED_SYMBOLS=0. See #7311.

v1.38.13: 10/10/2018
--------------------
 - Support `-s NO_X=1` as an alias for `-s X=0` and vice versa, which
   simplifies current settings with `NO_`-prefixed names. See #7151.
 - Various `EMULATED_FUNCTION_POINTER` improvements. See #7108, #7128.
 - `ERROR_ON_UNDEFINED_SYMBOLS` is now the default.  See #7196

v1.38.12: 09/03/2018
--------------------
 - Update SDL2 to 2.0.7. See #7016.
 - Optionally build using native object files (wasm backend only).
   For now this is behind a new option flag: `-s WASM_OBJECT_FILES=1`.
   See #6875.

v1.38.11: 08/02/2018
--------------------
 - Support for loading wasm files in the same dir as the JS file, using
   node.js/Web-specific techniques as applicable. See #5368 and followups.
 - Add an API for async side module compilation in wasm. See #6663.
 - Remove builtin Crunch support. See #6827.

v1.38.10: 07/23/2018
--------------------
 - Change the type of `size_t` and friends from int to long. This may have
   noticeable effects if you depend on the name mangling of a function that uses
   `size_t` (like in `EXPORTED_FUNCTIONS`), and you must rebuild source files to
   bitcode (so your bitcode is in sync with the system libraries after they are
   rebuilt with this change). Otherwise this should not have any noticeable
   effects for users. See #5916.

v1.38.9: 07/22/2018
-------------------
 - Fix `Module.locateFile` to resolve relative paths to *.wasm, *.mem and other
   files relatively to the main JavaScript file rather than the current working
   directory (see #5368).
   - Add second argument `prefix` to `Module.locateFile` function that contains
     the path to the JavaScript file where files are loaded from by default.
   - Remove `Module.*PrefixURL` APIs (use `Module.locateFile` instead).

v1.38.8: 07/06/2018
-------------------
 - Fix a regression in 1.38.7 with binaryen no longer bundling binaryen.js
   (which emscripten doesn't need, that's just for handwritten JS users, but
   emscripten did check for its presence).

v1.38.7: 07/06/2018
-------------------
 - Correctness fix for stack handling in `invoke_*()s`. This may add noticeable
   overhead to programs using C++ exceptions and (less likely) setjmp/longjmp -
   please report any issues. See #6666 #6702
 - Deprecate Module.ENVIRONMENT: Now that we have a compile-time option to set
   the environment, also having a runtime one on Module is complexity that we
   are better off without. When Module.ENVIRONMENT is used with ASSERTIONS it
   will show an error to direct users to the new option (-s ENVIRONMENT=web , or
   node, etc., at compile time).
 - Breaking change: Do not export print/printErr by default. Similar to other
   similar changes (like getValue/setValue). We now use out() and err()
   functions in JS to print to stdout/stderr respectively. See #6756.

v1.38.6: 06/13/2018
-------------------

v1.38.5: 06/04/2018
-------------------
 - Update libc++ to 6.0, bringing c++17 support (std::byte etc.)

v1.38.4: 05/29/2018
-------------------
 - Fix asm.js validation regression from 1.38.2.

v1.38.3: 05/25/2018
-------------------
 - Upgrade to LLVM 6.0.1.

v1.38.2: 05/25/2018
--------------------
 - Add ENVIRONMENT option to specify at compile time we only need JS to support
   one runtime environment (e.g., just the web). When emitting HTML, set that to
   web so we emit web code only. #6565
 - Regression in asm.js validation due to cttz optimization #6547

v1.38.1: 05/17/2018
-------------------
 - Remove special-case support for `src/struct_info.compiled.json`: Make it a
   normal cached thing like system libraries, not something checked into the
   source tree.
 - Breaking change: Emit WebAssembly by default. Only the default is changed -
   we of course still support asm.js, and will for a very long time. But
   changing the default makes sense as the recommended output for most use cases
   should be WebAssembly, given it has shipped in all major browsers and
   platforms and is more efficient than asm.js. Build with `-s WASM=0` to
   disable wasm and use asm.js if you want that (or use `-s
   LEGACY_VM_SUPPORT=1`, which emits output that can run in older browsers,
   which includes a bunch of polyfills as well as disables wasm). (#6419)

v1.38.0: 05/09/2018
-------------------

v1.37.40: 05/07/2018
--------------------
 - Fix regression in 1.37.39 on  -s X=@file  parsing (see #6497, #6436)

v1.37.39: 05/01/2018
--------------------
 - Regression: Parsing of `-s X=@file`  broke if the file contains a newline
   (see #6436; fixed in 1.37.40)

v1.37.38: 04/23/2018
--------------------
 - Breaking change: Simplify exception handling, disabling it by default.
   Previously it was disabled by default in -O1 and above and enabled in -O0,
   which could be confusing. You may notice this change if you need exceptions
   and only run in -O0 (since if you test in -O1 or above, you'd see you need to
   enable exceptions manually), in which case you will receive an error at
   runtime saying that exceptions are disabled by default and that you should
   build with `-s DISABLE_EXCEPTION_CATCHING=0` to enable them.
 - Fix regression in 1.37.37 on configure scripts on MacOS (see #6456)

v1.37.37: 04/13/2018
--------------------
 - Regression: configure scripts on MacOS may be broken (see #6456; fixed in 1.37.38)

v1.37.36: 03/13/2018
--------------------

v1.37.35: 02/23/2018
--------------------
 - MALLOC option, allowing picking between dlmalloc (previous allocator and
   still the default) and emmalloc, a new allocator which is smaller and
   simpler.
 - Binaryen update that should fix all known determinism bugs.

v1.37.34: 02/16/2018
--------------------
 - `addFunction` is now supported on LLVM wasm backend, but when being used on
   the wasm backend, you need to provide an additional second argument, a Wasm
   function signature string. Each character within a signature string
   represents a type. The first character represents the return type of a
   function, and remaining characters are for parameter types.
    - 'v': void type
    - 'i': 32-bit integer type
    - 'j': 64-bit integer type (currently does not exist in JavaScript)
    - 'f': 32-bit float type
    - 'd': 64-bit float type
   For asm.js and asm2wasm you can provide the optional second argument, but it
   isn't needed. For that reason this isn't a breaking change, however,
   providing the second argument is recommended so that code is portable across
   all backends and modes.

v1.37.33: 02/02/2018
--------------------

v1.37.32: 01/31/2018
--------------------

v1.37.31: 01/31/2018
--------------------
 - LLVM and clang updates from upstream (5.0svn, close 5.0 release).

v1.37.30: 01/31/2018
--------------------

v1.37.29: 01/24/2018
--------------------

v1.37.28: 01/08/2018
--------------------
 - Breaking change: Don't export the `ALLOC_*` numeric constants by default. As
   with previous changes, a warning will be shown in `-O0` and when `ASSERTIONS`
   are on if they are used.
 - Breaking change: Don't export FS methods by default. As with previous
   changes, a warning will be shown in `-O0` and when `ASSERTIONS` are on, which
   will suggest either exporting the specific methods you need, or using
   `FORCE_FILESYSTEM` which will auto export all the main filesystem methods.
   Aside from using FS methods yourself, you may notice this change when using a
   file package created standalone, that is, by running the file packager
   directly and then loading it at run time (as opposed to telling `emcc` to
   package the files for you, in which case it would be aware of them at compile
   time); you should build with `FORCE_FILESYSTEM` to ensure filesystem support
   for that case.

v1.37.27: 12/24/2017
--------------------
 - Breaking change: Remove the `Runtime` object, and move all the useful methods
   from it to simple top-level functions. Any usage of `Runtime.func` should be
   changed to `func`.

v1.37.26: 12/20/2017
--------------------
 - Breaking change: Change `NO_EXIT_RUNTIME` to 1 by default. This means that by
   default we don't include code to shut down the runtime, flush stdio streams,
   run atexits, etc., which is better for code size. When `ASSERTIONS` is on, we
   warn at runtime if there is text buffered in the streams that should be
   flushed, or atexits are used.
 - Meta-DCE for JS+wasm: remove unused code between JS+wasm more aggressively.
   This should not break valid code, but may break code that depended on unused
   code being kept around (like using a function from outside the emitted JS
   without exporting it - only exported things are guaranteed to be kept alive
   through optimization).

v1.37.24: 12/13/2017
--------------------
 - Breaking change: Similar to the getValue/setValue change from before (and
   with the same `ASSERTIONS` warnings to help users), do not export the
   following runtime methods by default: ccall, cwrap, allocate,
   Pointer_stringify, AsciiToString, stringToAscii, UTF8ArrayToString,
   UTF8ToString, stringToUTF8Array, stringToUTF8, lengthBytesUTF8, stackTrace,
   addOnPreRun, addOnInit, addOnPreMain, addOnExit, addOnPostRun,
   intArrayFromString, intArrayToString, writeStringToMemory,
   writeArrayToMemory, writeAsciiToMemory.

v1.37.23: 12/4/2017
-------------------
 - Breaking change: Do not polyfill Math.{clz32, fround, imul, trunc} by
   default. A new `LEGACY_VM_SUPPORT` option enables support for legacy
   browsers. In `ASSERTIONS` mode, a warning is shown if a polyfill was needed,
   suggesting using that option.
 - Breaking change: Do not export getValue/setValue runtime methods by default.
   You can still use them by calling them directly in code optimized with the
   main file (pre-js, post-js, js libraries; if the optimizer sees they are
   used, it preserves them), but if you try to use them on `Module` then you
   must export them by adding them to `EXTRA_EXPORTED_RUNTIME_METHODS`. In `-O0`
   or when `ASSERTIONS` is on, a run-time error message explains that, if they
   are attempted to be used incorrectly.

v1.37.17: 7/25/2017
------------------
 - Updated to libc++'s "v2" ABI, which provides better alignment for string data
   and other improvements. This is an ABI-incompatible change, so bitcode files
   from previous versions will not be compatible.

v1.37.13: 5/26/2017
-------------------
 - Improved Android support for emrun.
 - Duplicate function elimination fixes (#5186)
 - Fix problem with embinding derived classes (#5193)
 - Fix CMake compiler detection when EMCC_SKIP_SANITY_CHECK=1 is used. (#5145)
 - Implemented GLFW Joystick API (#5175)
 - Fixed a bug with emcc --clear-ports command (#5248)
 - Updated Binaryen to version 33.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.12...1.37.13
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.37.12: 5/1/2017
------------------
 - Added emscripten-legalize-javascript-ffi option to LLVM to allow disabling JS FFI mangling
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.11...1.37.12
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.37.11...1.37.12
    - Emscripten-Clang: no changes.

v1.37.11: 5/1/2017
------------------
 - Added missing SIGSTKSZ define after musl 1.1.15 update (#5149)
 - Fix emscripten_get_mouse_status (#5152)
 - Fix `_mm_set_epi64x()` function (#5103)
 - Fix issue with number of gamepads connected at initial page load (#5169, #5170)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.10...1.37.11
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.37.10...1.37.11
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.37.10...1.37.11

v1.37.10: 4/20/2017
-------------------
 - Added stub for pthread_setcancelstate for singlethreaded runs.
 - Fixed an outlining bug on function returns (#5080)
 - Implemented new parallel test runner architecture (#5074)
 - Added Cocos2D to Emscripten ports. (-s USE_COCOS2D=1)
 - Updated Binaryen to version 32, which migrates Emscripten to use the new
   WebAssembly Names section. This is a forwards and backwards breaking change
   with respect to reading debug symbol names in Wasm callstacks. Use of the new
   Names section format first shipped in Emscripten 1.37.10, Binaryen version
   32, Firefox 55, Firefox Nightly 2017-05-18 and Chrome 59; earlier versions
   still used the old format. For more information, see
   https://github.com/WebAssembly/design/pull/984 and
   https://github.com/WebAssembly/binaryen/pull/933.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.9...1.37.10
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.37.9...1.37.10
    - Emscripten-Clang: no changes.

v1.37.9: 3/23/2017
------------------
 - Added new build feature -s GL_PREINITIALIZED_CONTEXT=1 which allows pages to
   manually precreate the GL context they use for customization purposes.
 - Added a custom callback hook Module.instantiateWasm() which allows user shell
   HTML file to manually perform Wasm instantiation for preloading and progress
   bar purposes.
 - Added a custom callback hook Module.getPreloadedPackage() to file preloader
   code to allow user shell HTML file to manually download .data files for
   preloading and progress bar purposes.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.8...1.37.9
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.37.8...1.37.9
    - Emscripten-Clang: no changes.

v1.37.8: 3/17/2017
------------------
 - Fixed a bug with robust_list initialization on pthreads build mode.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.7...1.37.8
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.37.7: 3/15/2017
------------------
 - Updated to LLVM 4.0.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.6...1.37.7
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.37.6...1.37.7
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.37.6...1.37.7

v1.37.6: 3/15/2017
------------------
 - Implemented readdir() function for WORKERFS.
 - Fixed bugs with Fetch API (#4995, #5027)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.5...1.37.6
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.37.5: 3/13/2017
------------------
 - Updated musl to version 1.1.15 from earlier version 1.0.5.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.4...1.37.5
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.37.4: 3/13/2017
------------------
 - Fixed glGetUniformLocation() to work according to spec with named uniform blocks.
 - Fixed WebAssembly Memory.grow() to work.
 - Switched to 16KB page size from earlier 64KB.
 - Optimize alBufferData() operation.
 - Fixed a resource lookup issue with multiple OpenAL audio contexts.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.3...1.37.4
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.37.3: 2/16/2017
------------------
 - Updated Binaryen to version 0x01. First official stable WebAssembly support version. (#4953)
 - Optimized memcpy and memset with unrolling and SIMD, when available.
 - Improved Emscripten toolchain profiler to track more hot code.
 - Added new linker flag -s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1 to allow
   simultaneously targeting WebGL 1 and WebGL 2.
 - Optimize Emscripten use of multiprocessing pools.
 - More WebGL 2 garbage free optimizations.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.2...1.37.3
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.37.2...1.37.3
    - Emscripten-Clang: no changes.

v1.37.2: 1/31/2017
------------------
 - Fixed a build error with boolean SIMD types.
 - Improved WebAssembly support, update Binaryen to version 22.
 - Update GL, GLES, GLES2 and GLES3 headers to latest upstream Khronos versions.
 - Implement support for new garbage free WebGL 2 API entrypoints which improve
   performance and reduce animation related stuttering.
 - Fixed a bug where -s USE_PTHREADS builds would not have correct heap size if
   -s TOTAL_MEMORY is not being used.
 - Fixed array type issue that prevented glTexImage3D() and glTexSubImage3D()
   from working.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.1...1.37.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.37.1...1.37.2
    - Emscripten-Clang: no changes.

v1.37.1: 12/26/2016
-------------------
 - Implemented new Fetch API for flexible multithreaded XHR and IndexedDB
   access.
 - Implemented initial version of new ASMFS filesystem for multithreaded
   filesystem operation.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.37.0...1.37.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.37.0: 12/23/2016
-------------------
 - Added support for LLVM sin&cos intrinsics.
 - Fix GLFW mouse button mappings (#4317, #4659)
 - Add support for --emit-symbol-map to wasm
 - Fixed handling of an invalid path in chdir (#4749)
 - Added new EMSCRIPTEN_STRICT mode, which can be enabled to opt in to removing
   support for deprecated behavior.
 - Remove references to Web Audio .setVelocity() function, which has been
   removed from the spec.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.14...1.37.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.36.14...1.37.0
    - Emscripten-Clang: no changes.

v1.36.14: 11/3/2016
-------------------
 - Added support to emscripten_async_wget() for relative paths.
 - Fixed FS.mkdirTree('/') to work.
 - Updated SDL 2 port to version 12.
 - Added more missing pthreads stubs.
 - Normalize system header includes to use the preferred form #include
   <emscripten/foo.h> to avoid polluting header include namespaces.
 - Fixed a bug where transitioning to fullscreen could cause a stack overflow in GLFW.
 - Added new system CMake option
   -DEMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES=ON to choose if static
   libraries should be LLVM bitcode instead of .a files.
 - Improved SIMD support to be more correct to the spec.
 - Updated Binaryen to version 18. (#4674)
 - Fixed dlopen with RTLD_GLOBAL parameter.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.13...1.36.14
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.36.13: 10/21/2016
--------------------
 - Pass optimization settings to asm2wasm.
 - Fix to exporting emscripten_builtin_malloc() and emscripten_builtin_free()
   when heap is split to multiple parts.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.12...1.36.13
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.36.12: 10/20/2016
--------------------
 - Improved Emscripten toolchain profiler with more data. (#4566)
 - Export dlmalloc() and dlfree() as emscripten_builtin_malloc() and
   emscripten_builtin_free() to allow user applications to hook into memory
   allocation (#4603)
 - Improved asm.js -s USE_PTHREADS=2 build mode compatibility when
   multithreading is not supported.
 - Improved WebGL support with closure compiler (#4619)
 - Improved Bianaryen WebAssembly support
 - Added support for GL_disjoint_timer_query extension (#4575)
 - Improved Emscripten compiler detection with CMake (#4129, #4314, #4318)
 - Added support for int64 in wasm.
 - Optimize small constant length memcpys in wasm.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.11...1.36.12
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.36.11...1.36.12
    - Emscripten-Clang: no changes.

v1.36.11: 9/24/2016
-------------------
 - Added new runtime functions
   emscripten_sync/async/waitable_run_in_main_runtime_thread() for proxying
   calls with pthreads (#4569)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.10...1.36.11
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.36.10: 9/24/2016
-------------------
 - Improved compiler logging print messages on first run experience. (#4501)
 - Fixed log printing in glFlushMappedBufferRange() and glGetInfoLog()
   functions. (#4521)
 - Added setjmp/longjmp handling for wasm.
 - Improved support for --proxy-to-worker build mode.
 - Improved GLES3 support for glGet() features that WebGL2 does not have. (#4514)
 - Added support for implementation defined glReadPixels() format.
 - Improved WebGL 2 support with closure compiler. (#4554)
 - Implemented support for nanosleep() when building in pthreads mode (#4578)
 - Added support for  llvm_ceil_f64 and llvm_floor_f64 intrinsics.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.9...1.36.10
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.36.9...1.36.10
    - Emscripten-Clang: no changes.

v1.36.9: 8/24/2016
------------------
 - Fixed glGet for GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING to work. (#1330)
 - Move the DYNAMICTOP variable from JS global scope to inside the heap so that
   the value is shared to multithreaded applications. This removes the global
   runtime variable DYNAMICTOP in favor of a new variable DYNAMICTOP_PTR.
   (#4391, #4496)
 - Implemented brk() system function.
 - Fixed --output-eol to work with --proxy-to-worker mode.
 - Improved reported error message when execution fails to stack overflow.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.8...1.36.9
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.36.8...1.36.9
    - Emscripten-Clang: no changes.

v1.36.8: 8/20/2016
------------------
 - Fixed a memory leak in ctor_evaller.py on Windows (#4446)
 - Migrate to requiring CMake 3.4.3 as the minimum version for Emscripten CMake
   build integration support.
 - Fixed an issue that prevented -s INLINING_LIMIT from working (#4471)
 - Fixed a bug with Building.llvm_nm interpretation of defined symbols (#4488)
 - Add support for DISABLE_EXCEPTION_CATCHING and EXCEPTION_CATCHING_WHITELIST
   options for wasm.
 - Added new emprofile.py script which can be used to profile toolchain wide
   performance. (#4491)
 - Added new linker flag --output-eol, which specifies what kind of line
   endings to generate to the output files. (#4492)
 - Fixed a Windows bug where aborting execution with Ctrl-C might hang
   Emscripten to an infinite loop instead. (#4494)
 - Implement support for touch events to GLUT (#4493)
 - Deprecated unsafe function writeStringToMemory() from src/preamble.js. Using
   stringToUTF8() is recommended instead. (#4497)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.7...1.36.8
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.36.7: 8/8/2016
-----------------
 - Updated to latest upstream LLVM 3.9.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.6...1.36.7
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.36.6...1.36.7
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.36.6...1.36.7

v1.36.6: 8/8/2016
-----------------
 - Fixed wheelDelta for MSIE (#4316)
 - Fixed inconsistencies in fullscreen API signatures (#4310, #4318, #4379)
 - Changed the behavior of Emscripten WebGL createContext() to not forcibly set
   CSS style on created canvases, but let page customize the style themselves
   (#3406, #4194 and #4350, #4355)
 - Adjusted the reported GL_VERSION field to adapt to the OpenGL ES
   specifications (#4345)
 - Added support for GLES3 GL_MAJOR/MINOR_VERSION fields. (#4368)
 - Improved -s USE_PTHREADS=1 and --proxy-to-worker linker options to be
   mutually compatible. (#4372)
 - Improved IDBFS to not fail on Safari where IndexedDB support is spotty
   (#4371)
 - Improved SIMD.js support when using Closure minifier. (#4374)
 - Improved glGetString to be able to read fields from WEBGL_debug_renderer_info
   extension. (#4381)
 - Fixed an issue with glFramebufferTextureLayer() not working correctly.
 - Fixed a bug with std::uncaught_exception() support (#4392)
 - Implemented a multiprocess lock to access the Emscripten cache. (#3850)
 - Implemented support for the pointerlockerror event in HTML5 API (#4373)
 - Report WebGL GLSL version number in GL_SHADING_LANGUAGE_VERSION string (#4365)
 - Optimized llvm_ctpop_i32() and conversion of strings from C to JS side
   (#4402, #4403)
 - Added support for the OffscreenCanvas proposal, and transferring canvases to
   offscreen in pthreads build mode, linker flag -s OFFSCREENCANVAS_SUPPORT=0/1
   (#4412)
 - Fixed an issue after updating to new LLVM version that response files passed
   to llvm-link must have forward slashes (#4434)
 - Fixed a memory leak in relooper in LLVM.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.5...1.36.6
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.36.5...1.36.6
    - Emscripten-Clang: no changes.

v1.36.5: 5/24/2016
------------------
 - Added support for passing custom messages when running in web worker.
 - Improved fp128 support when targeting WebAssembly.
 - Updated cpuprofiler.js to support tracing time spent in WebGL functions.
 - Fixed an issue with glFenceSync() function call signature (#4260, #4339)
 - Added missing zero argument version of emscripten_sync_run_in_main_thread().
 - Improves support for targeting pthreads when using Closure minifier (#4348).
 - Fixed an issue where pthreads enabled code did not correctly validate as asm.js
 - Fixed an issue with incorrect SIMD.js related imports (#4341)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.4...1.36.5
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.36.4...1.36.5
    - Emscripten-Clang: no changes.

v1.36.4: 5/9/2016
-----------------
 - Added EM_TRUE and EM_FALSE #defines to html5.h.
 - Fixed an issue with GLFW window and framebuffer size callbacks.
 - Added support for more missing WebGL 2 texture formats (#4277)
 - Added support for source files with no extension.
 - Updated emrun.py to latest version, adds support to precompressed content and
   running as just a web server without launching a browser.
 - Updated experimental WebAssembly support to generate 0xb version code.
 - Automatically build Binaryen when needed.
 - Updated libc++ to SVN revision 268153. (#4288)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.3...1.36.4
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.36.3: 4/27/2016
------------------
 - Fixed a deadlock bug with pthreads support.
 - Remove sources from temporary garbage being generated in OpenAL code (#4275)
 - Added support for calling alert() from pthreads code.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.2...1.36.3
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.36.2: 4/22/2016
------------------
 - Improve support for targeting WebAssembly with Binaryen.
 - Improve support for LLVM's WebAssembly backend (EMCC_WASM_BACKEND=1
   environment variable).
 - Separate out emscripten cache structure to asmjs and wasm directories.
 - Fix a bug where Emscripten would spawn too many unused python subprocesses (#4158)
 - Optimize Emscripten for large asm.js projects.
 - Added sdl2_net to Emscripten ports.
 - Updated to latest version of the SIMD polyfill (#4165)
 - Fixed an issue with missing texture formats support in GLES 3 (#4176)
 - Added a new WebAssembly linker option -s BINARYEN_IMPRECISE=1 (default=0)
   which mutes potential traps from WebAssembly int div/rem by zero and
   float-to-int conversions.
 - Added support for EXT_color_buffer_float extension.
 - Fixed behavior of SSE shift operations (#4165).
 - Fixed a bug where ctor_evaller.py (-Oz builds) would hang on Windows.
 - Fixed a bug where emscripten_set_main_loop() with EM_TIMING_SETTIMEOUT would
   incorrectly compute the delta times (#4200, #4208)
 - Update pthreads support to latest proposed spec version. (#4212, #4220)
 - Fixed an unresolved symbol linker error in embind (#4225)
 - Fix file_packager.py --use-preload-cache option to also work on Safari and
   iOS (#2977, #4253)
 - Added new file packager option --indexedDB-name to allow specifying the
   database name to use for the cache (#4219)
 - Added DWARF style debugging information.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.1...1.36.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.36.1...1.36.2
    - Emscripten-Clang: no changes.

v1.36.1: 3/8/2016
-----------------
 - Fixed glfwSetWindowSizeCallback to conform to GLFW2 API.
 - Update OpenAL sources only when the browser window is visible to avoid
   occasional stuttering static glitches when the page tab is hidden. (#4107)
 - Implemented LLVM math intrinsics powi, trunc and floor.
 - Added support for SDL_GL_ALPHA_SIZE in GL context initialization. (#4125)
 - Added no-op stubs for several pthread functions when building without
   pthreads enabled (#4130)
 - Optimize glUniform*fv and glVertexAttrib*fv functions to generate less
   garbage and perform much faster (#4128)
 - Added new EVAL_CTORS optimization pass which evaluates global data
   initializer constructors at link time, which would improve startup time and
   reduce code size of these ctors.
 - Implemented support for OpenAL AL_PITCH option.
 - Implemented new build options -s STACK_OVERFLOW_CHECK=0/1/2 which adds
   runtime stack overrun checks. 0: disabled, 1: minimal, between each frame, 2:
   at each explicit JS side stack allocation call to allocate().
 - Fixed an issue with -s SPLIT_MEMORY mode where an unsigned 32-bit memory
   access would come out as signed. (#4150)
 - Fixed asm.js validation in call handlers to `llvm_powi_f*`.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.36.0...1.36.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.36.0...1.36.1
    - Emscripten-Clang: no changes.

v1.36.0: 2/23/2016
------------------
 - Fixed an OpenAL bug where OpenAL sources would not respect global volume setting.
 - Fixed an issue where alGetListenerf() with AL_GAIN would not return the
   correct value. (#4091)
 - Fixed an issue where setting alListenerf() with AL_GAIN would not set the
   correct value. (#4092)
 - Implemented new JS optimizer "Duplicate Function Elimination" pass which
   collapses identical functions to save code size.
 - Implemented the `_Exit()` function.
 - Added support for SSE3 and SSSE3 intrinsics (#4099) and partially for SSE 4.1
   intrinsics (#4030, #4101)
 - Added support for -include-pch flag (#4086)
 - Fixed a regex syntax in ccall on Chrome Canary (#4111)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.23...1.36.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.23...1.36.0
    - Emscripten-Clang: no changes.

v1.35.23: 2/9/2016
------------------
 - Provide $NM environment variable to point to llvm-nm when running
   emconfigure, which helps e.g. libjansson to build (#4036)
 - Fixed glGetString(GL_SHADING_LANGUAGE_VERSION) to return appropriate result
   depending on if running on WebGL1 vs WebGL2, instead of hardcoding the result
   (#4040)
 - Fixed a regression with CMake try_run() possibly failing, caused by the
   addition of CMAKE_CROSSCOMPILING_EMULATOR in v1.32.3.
 - Fixed CMake to work in the case when NODE_JS is an array containing
   parameters to be passed to Node.js. (#4045)
 - Fixed a memory issue that caused the Emscripten memory initializer file
   (.mem.js) to be unnecessarily retained in memory during runtime (#4044)
 - Added support for complex valued mul and div ops.
 - Added new option "Module.environment" which allows overriding the runtime ENVIRONMENT_IS_WEB/ENVIRONMENT_IS_WORKER/ENVIRONMENT_IS_NODE/ENVIRONMENT_IS_SHELL fields.
 - Fixed an issue with SAFE_HEAP methods in async mode (#4046)
 - Fixed WebSocket constructor to work in web worker environment (#3849)
 - Fixed a potential issue with some browsers reporting gamepad axis values outside \[-1, 1\] (#3602)
 - Changed libcxxabi to be linked in last, so that it does not override weakly
   linked methods in libcxx (#4053)
 - Implemented new JSDCE code optimization pass which removes at JS link stage
   dead code that is not referenced anywhere (in addition to LLVM doing this for
   C++ link stage).
 - Fixed a Windows issue where embedding memory initializer as a string in JS
   code might cause corrupted output. (#3854)
 - Fixed an issue when spaces are present in directory names in response files
   (#4062)
 - Fixed a build issue when using --tracing and -s ALLOW_MEMORY_GROWTH=1
   simultaneously (#4064)
 - Greatly updated Emscripten support for SIMD.js intrinsics (non-SSE or NEON)
 - Fixed an issue where compiler would not generate a link error when JS library
   function depended on a nonexisting symbol. (#4077)
 - Removed UTF16 and UTF32 marshalling code from being exported by default.
 - Removed the -s NO_BROWSER linker option and automated the detection of when
   that option is needed.
 - Removed the JS implemented C++ symbol name demangler, now always depend on
   the libcxxabi compiled one.
 - Fixed an issue where Emscripten linker would redundantly generate missing
   function stubs for some functions that do exist.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.22...1.35.23
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.22...1.35.23
    - Emscripten-Clang: no changes.

v1.35.22: 1/13/2016
-------------------
 - Updated to latest upstream LLVM trunk as of January 13th.
 - Bumped up the required LLVM version from LLVM 3.8 to LLVM 3.9.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.21...1.35.22
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.21...1.35.22
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.35.21...1.35.22

v1.35.21: 1/13/2016
-------------------
 - Improved support for handling GLFW2 keycodes.
 - Improved emranlib, system/bin/sdl-config and system/bin/sdl2-config to be
   executable in both python2 and python3.
 - Fixed build flags -s AGGRESSIVE_VARIABLE_ELIMINATION=1 and -s USE_PTHREADS=2
   to correctly work when run on a browser that does not support pthreads.
 - Fixed a build issue that caused sequences of \r\r\n to be emitted on Windows.
 - Fixed an issue that prevented building LLVM on Visual Studio 2015
   (emscripten-fastcomp-clang #7)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.20...1.35.21
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.20...1.35.21
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.35.20...1.35.21

v1.35.20: 1/10/2016
-------------------
 - Fixed -s USE_PTHREADS compilation mode to account that SharedArrayBuffer
   specification no longer allows futex waiting on the main thread. (#4024)
 - Added new python2 vs python3 compatibility wrappers for emcmake, emconfigure, emmake and emar.
 - Fixed atomicrmw i64 codegen (#4025)
 - Optimized codegen to simplify "x != 0" to just "x" when output is a boolean.
 - Fixed a compiler crash when generating atomics code in debug builds of LLVM.
 - Fixed a compiler crash when generating SIMD.js code that utilizes
   non-canonical length vectors (e.g. <float x 3>)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.19...1.35.20
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.19...1.35.20
    - Emscripten-Clang: no changes.

v1.35.19: 1/7/2016
------------------
 - Updated to latest upstream LLVM trunk as of January 7th.
 - Full list of changes:
    - Emscripten: no changes.
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.18...1.35.19
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.35.18...1.35.19

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
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.17...1.35.18
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.17...1.35.18
    - Emscripten-Clang: no changes.

v1.35.17: 1/4/2016
------------------
 - Updated to latest upstream LLVM trunk as of January 4th.
 - Full list of changes:
    - Emscripten: no changes.
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.16...1.35.17
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.16...1.35.17

v1.35.16: 1/4/2016
------------------
 - Improved support for -s USE_PTHREADS=2 build mode and added support for Atomics.exchange().
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.15...1.35.16
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.15...1.35.16
    - Emscripten-Clang: no changes.

v1.35.15: 1/4/2016
------------------
 - Fixed an error with glClearbufferfv not working. (#3961)
 - Improved file packager code so that file:// URLs work in Chrome too (#3965)
 - Fixed issues with the --memoryprofiler UI.
 - Fixed a Windows issue when generating system libraries in cache (#3939)
 - Fixed a regression from v1.35.13 where GLES2 compilation would not work when
   -s USE_PTHREADS=1 was passed.
 - Added support for WebIDL arrays as input parameters to WebIDL binder.
 - Updated build support when using the LLVM wasm backend.
 - Added new linker option --threadprofiler which generates a threads dashboard
   on the generated page for threads status overview. (#3971)
 - Improved backwards compatibility of building on GCC 4.3 - 4.6.
 - Fixed an asm.js validation issue when building against updated SIMD.js specification. (#3986)
 - Improved Rust support.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.14...1.35.15
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.14...1.35.15
    - Emscripten-Clang: no changes.

v1.35.14: 12/15/2015
--------------------
 - Updated to latest upstream LLVM trunk as of December 15th.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.13...1.35.14
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.13...1.35.14
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.35.13...1.35.14

v1.35.13: 12/15/2015
--------------------
 - Updated -s USE_PTHREADS code generation to reflect that the `SharedInt*Array`
   hierarchy no longer exists in the SharedArrayBuffer spec.
 - Removed references to Atomic.fence() which no longer is part of the
   SharedArrayBuffer specification.
 - Fixed an issue where JS code minifiers might generate bad code for cwrap
   (#3945)
 - Updated compiler to issue a warning when --separate-asm is being used and
   output suffix is .js.
 - Added new build option -s ONLY_MY_CODE which aims to eliminate most of the
   Emscripten runtime and generate a very minimal compiler output.
 - Added new build option -s WASM_BACKEND=0/1 which controls whether to utilize
   the upstream LLVM wasm emitting codegen backend.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.12...1.35.13
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.12...1.35.13
    - Emscripten-Clang: no changes.

v1.35.12: 11/28/2015
--------------------
 - Update to latest upstream LLVM trunk as of November 28th.
 - Fix Emscripten to handle new style format outputted by llvm-nm.
 - Added new build option BINARYEN_METHOD to allow choosing which wasm
   generation method to use.
 - Updates to Binaryen support.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.11...1.35.12
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.11...1.35.12
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.35.11...1.35.12

v1.35.11: 11/27/2015
--------------------
 - Updated atomics test to stress 64-bit atomics better (#3892)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.10...1.35.11
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.10...1.35.11
    - Emscripten-Clang: no changes.

v1.35.10: 11/25/2015
--------------------
 - Integration with Binaryen.
 - Add a performance warning when multiple FS.syncfs() calls are in flight simultaneously.
 - Correctly pass GLFW_REPEAT when sending key press repeats.
 - Improved filesystem performance when building in multithreaded mode (#3923)
 - Improve error detection when data file fails to load.
 - Clarified that -s NO_DYNAMIC_EXECUTION=1 and -s RELOCATABLE=1 build modes are mutually exclusive.
 - Added new build option -s NO_DYNAMIC_EXECUTION=2 which demotes eval() errors
   to warnings at runtime, useful for iterating fixes in a codebase for multiple
   eval()s  (#3930)
 - Added support to Module.locateFile(filename) to locate the pthread-main.js file (#3500)
 - Changed -s USE_PTHREADS=2 and -s PRECISE_F32=2 to imply --separate-asm
   instead of requiring it, to be backwards compatible (#3829, #3933)
 - Fixed bad codegen for some 64-bit atomics (#3892, #3936)
 - When emitting NaN canonicalization warning, also print the location in code
   where it occurs.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.9...1.35.10
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.9...1.35.10
    - Emscripten-Clang: no changes.

v1.35.9: 11/12/2015
-------------------
 - Implement glfwSetInputMode when mode is GLFW_CURSOR and value is GLFW_CURSOR_NORMAL|GLFW_CURSOR_DISABLED
 - Add explicit abort() when dlopen() is called without linking support
 - Make emcc explicitly reinvoke itself from python2 if called from python3.
 - Optimize memory initializer to omit zero-initialized values (#3907)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.8...1.35.9
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.8...1.35.9
    - Emscripten-Clang: no changes.

v1.35.8: 11/10/2015
-------------------
 - Removed obsoleted EXPORTED_GLOBALS build option.
 - Export filesystem as global object 'FS' in Emscripten runtime.
 - Fixed realpath() function on directories.
 - Fixed round() and roundf() to work when building without -s PRECISE_F32=1 and
   optimize these to be faster (#3876)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.7...1.35.8
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.35.7: 11/4/2015
------------------
 - Updated to latest upstream LLVM trunk version as of November 4th.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.6...1.35.7
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.6...1.35.7
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.35.6...1.35.7

v1.35.6: 11/4/2015
------------------
 - This tag was created for technical purposes, and has no changes compared to
   v1.35.6.

v1.35.5: 11/4/2015
------------------
 - Removed Content-Length and Connection: close headers in POST requests.
 - Migrate to using the native C++11-implemented optimizer by default.
 - Fixed call to `glDrawBuffers(0, *);` (#3890)
 - Fixed lazy file system to work with closure (#3842)
 - Fixed gzip compression with lazy file system (#3837)
 - Added no-op gracefully failing stubs for process spawn functions (#3819)
 - Clarified error message that memory growth is not supported with shared modules (#3893)
 - Initial work on wasm support in optimizer
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.4...1.35.5
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.35.4: 10/26/2015
-------------------
 - Move to legalization in the JS backend.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.3...1.35.4
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.3...1.35.4
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.35.3...1.35.4

v1.35.3: 10/26/2015
-------------------
 - Ignore O_CLOEXEC on NODEFS (#3862)
 - Improved --js-library support in CMake by treating these as libraries (#3840)
 - Still support -Wno-warn-absolute-paths (#3833)
 - Add support to zext <4 x i1> to <4x i32>
 - Emit emscripten versions of llvm and clang in clang --version
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.2...1.35.3
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.2...1.35.3
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.35.2...1.35.3

v1.35.2: 10/20/2015
-------------------
 - Rebase against upstream LLVM "google/stable" branch, bringing us to LLVM 3.8.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.1...1.35.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.35.1...1.35.2
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.35.1...1.35.2

v1.35.1: 10/20/2015
-------------------
 - Fixed a bug where passing -s option to LLVM would not work.
 - Work around a WebAudio bug on WebKit "pauseWebAudio failed: TypeError: Not
   enough arguments" (#3861)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.35.0...1.35.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.35.0: 10/19/2015
-------------------
 - Fixed out of memory abort message.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.12...1.35.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.34.12: 10/13/2015
--------------------
 - Added new experimental build option -s SPLIT_MEMORY=1, which splits up the
   Emscripten HEAP to multiple smaller slabs.
 - Added SDL2_ttf to Emscripten ports.
 - Added support for building GLES3 code to target WebGL 2. (#3757, #3782)
 - Fixed certain glUniform*() functions to work properly when called in
   conjunction with -s USE_PTHREADS=1.
 - Fixed support for -l, -L and -I command line parameters to accept a space
   between the path, i.e. "-l SDL". (#3777)
 - Fixed SSE2 support in optimized builds.
 - Changed the default behavior of warning when absolute paths are passed to -I
   to be silent. To enable the absolute paths warning, pass
   "-Wwarn-absolute-paths" flag to emcc.
 - Added new linker option -s ABORTING_MALLOC=0 that can be used to make
   malloc() return 0 on failed allocation (Current default is to abort execution
   of the page on OOM) (#3822)
 - Removed the default behavior of automatically decoding all preloaded assets on page startup (#3785)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.11...1.34.12
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.34.11...1.34.12
    - Emscripten-Clang: no changes.

v1.34.11: 9/29/2015
-------------------
 - Fixed asm.js validation on autovectorized output
 - Fix an issue with printing to iostream in global ctors (#3824)
 - Added support for LLVM pow intrinsics with integer exponent.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.10...1.34.11
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.34.10...1.34.11
    - Emscripten-Clang: no changes.

v1.34.10: 9/25/2015
-------------------
 - Added wasm compressor/decompressor polyfill (#3766)
 - Added support for sRGB texture formats.
 - Removed the deprecated --compression option.
 - Fixed an issue with asm.js validation for pthreads being broken since v1.34.7 (#3719)
 - Added built-in cpu performance profiler, which is enabled with linker flag --cpuprofiler. (#3781)
 - Added build-in memory usage profiler, which is enabled with linker flag --memoryprofiler. (#3781)
 - Fixed multiple arities per EM_ASM block (#3804)
 - Fixed issues with SSE2 an NaN bit patterns. (emscripten-fastcomp #116)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.9...1.34.10
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.34.9...1.34.10
    - Emscripten-Clang: no changes.

v1.34.9: 9/18/2015
------------------
 - Fixed an issue with --llvm-lto 3 builds (#3765)
 - Optimized LZ4 compression
 - Fixed a bug where glfwCreateWindow would return success even on failure
   (#3764)
 - Greatly optimized the -s SAFE_HEAP=1 linker flag option by executing the heap
   checks in asm.js side instead.
 - Fixed the return value of EM_ASM_DOUBLE (#3770)
 - Implemented getsockname syscall (#3769)
 - Don't warn on unresolved symbols when LINKABLE is specified.
 - Fixed various issues with SSE2 compilation in optimized builds.
 - Fixed a breakage with -s USE_PTHREADS=2 (#3774)
 - Added support for GL_HALF_FLOAT in WebGL 2. (#3790)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.8...1.34.9
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.34.8...1.34.9
    - Emscripten-Clang: no changes.

v1.34.8: 9/9/2015
-----------------
 - Fixed a race condition at worker startup (#3741)
 - Update emrun to latest, which improves unit test run automation with emrun.
 - Added support for LZ4 compressing file packages, used with the -s LZ4=1 linker flag. (#3754)
 - Fixed noisy build warning on "unexpected number of arguments in call to strtold" (#3760)
 - Added new linker flag --separate-asm that splits the asm.js module and the
   handwritten JS functions to separate files.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.7...1.34.8
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.34.7: 9/5/2015
-----------------
 - Fixed uses of `i64*` in side modules.
 - Improved GL support when proxying, and fake WebAudio calls when proxying.
 - Added new main loop timing mode EM_TIMING_SETIMMEDIATE for rendering with
   vsync disabled (#3717)
 - Updated emrun to latest version, adds --safe_firefox_profile option to run
   emrun pages in clean isolated environment.
 - Implemented glGetStringi() method for WebGL2/GLES3. (#3472, #3725)
 - Automatically emit loading code for EMTERPRETIFY_FILE if emitting html.
 - Added new build option -s USE_PTHREADS=2 for running pthreads-enabled pages
   in browsers that do not support SharedArrayBuffer.
 - Added support for building SSE2 intrinsics based code (emmintrin.h), when
   -msse2 is passed to the build.
 - Added exports for getting FS objects by their name (#3690)
 - Updated LLVM to latest upstream PNaCl version (Clang 3.7, July 29th).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.6...1.34.7
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.34.6...1.34.7
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.34.6...1.34.7

v1.34.6: 8/20/2015
------------------
 - Added new build option -s EMULATED_FUNCTION_POINTERS=2.
 - Fixed a bug with calling functions pointers that take float as parameter
   across dynamic modules.
 - Improved dynamic linking support with -s LINKABLE=1.
 - Added new build option -s MAIN_MODULE=2.
 - Cleaned up a few redundant linker warnings (#3702, #3704)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.5...1.34.6
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.34.5...1.34.6
    - Emscripten-Clang: no changes.

v1.34.5: 8/18/2015
------------------
 - Added Bullet physics, ogg and vorbis to emscripten-ports.
 - Added FreeType 2.6 to emscripten-ports.
 - Fixed CMake handling when building OpenCV.
 - Fixed and issue with exceptions being thrown in empty glBegin()-glEnd()
   blocks (#3693)
 - Improved function pointer handling between dynamically linked modules
 - Fixed some OpenAL alGetSource get calls (#3669)
 - Fixed issues with building the optimizer on 32-bit Windows (#3673)
 - Increased optimizer stack size on Windows to 10MB (#3679)
 - Added support for passing multiple input files to opt, to speed up
   optimization and linking in opt.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.4...1.34.5
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.34.4...1.34.5
    - Emscripten-Clang: no changes.

v1.34.4: 8/4/2015
-----------------
 - Add special handling support for /dev/null as an input file (#3552)
 - Added basic printf support in NO_FILESYSTEM mode (#3627)
 - Update WebVR support to the latest specification, and add support for
   retrieving device names
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
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.3...1.34.4
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.34.3...1.34.4
    - Emscripten-Clang: no changes.

v1.34.3: 7/15/2015
------------------
 - Move libc to musl+syscalls
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.2...1.34.3
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
 - Fixed a LLVM backend problem which caused recursive stack behavior when
   linking large codebases, which was seen to cause a stack overflow crash on
   Windows.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.1...1.34.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.34.1...1.34.2
    - Emscripten-Clang: no changes.

v1.34.1: 6/18/2015
------------------
 - Fixed an issue with resize canvas not working with GLFW.
 - Fixed handling of empty else blocks.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.34.0...1.34.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.34.0: 6/16/2015
------------------
 - Fixed an issue when generating .a files from object files that reside on
   separate drives on Windows (#3525).
 - Added a missing dependency for GLFW (#3530).
 - Removed the Emterpreter YIELDLIST option.
 - Added support for enabling memory growth before the runtime is ready.
 - Added a new feature to store the memory initializer in a string literal
   inside the generated .js file.
 - Fixed a code miscompilation issue with a constexpr in fcmp.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.33.2...1.34.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.33.2...1.34.0
    - Emscripten-Clang: no changes.

v1.33.2: 6/9/2015
-----------------
 - Added support for OpenAL Extension AL_EXT_float32 (#3492).
 - Added support for handling command line flags -M and -MM (#3518).
 - Fixed a code miscompilation issue with missing ';' character (#3520).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.33.1...1.33.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.33.1...1.33.2
    - Emscripten-Clang: no changes.

v1.33.1: 6/3/2015
-----------------
 - Added support for multithreading with the POSIX threads API (pthreads), used
   when compiling and linking with the -s USE_PTHREADS=1 flag (#3266).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.33.0...1.33.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.33.0...1.33.1
    - Emscripten-Clang: no changes.

v1.33.0: 5/29/2015
------------------
 - Fix an issue with writing to /dev/null (#3454).
 - Added a hash to objects inside .a files to support to linking duplicate
   symbol names inside .a files (#2142).
 - Provide extensions ANGLE_instanced_arrays and EXT_draw_buffers as aliases to
   the WebGL ones.
 - Fixed LLVM/Clang to build again on Windows after previous LLVM upgrade.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.32.4...1.33.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.32.4...1.33.0
    - Emscripten-Clang: no changes.

v1.32.4: 5/16/2015
------------------
 - Update LLVM and Clang to PNaCl's current 3.7 merge point (April 17 2015)
 - Added libpng to Emscripten-ports.
 - Added intrinsic llvm_fabs_f32.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.32.3...1.32.4
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.32.3...1.32.4
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.32.3...1.32.4

v1.32.3: 5/15/2015
------------------
 - Improved dynamic linking support.
 - Added new option to file_packager.py to store metadata externally.
 - Improved CMake support with CMAKE_CROSSCOMPILING_EMULATOR (#3447).
 - Added support for `sysconf(_SC_PHYS_PAGES)` (#3405, 3442).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.32.2...1.32.3
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.32.2...1.32.3
    - Emscripten-Clang: no changes.

v1.32.2: 5/8/2015
-----------------
 - Removed a (name+num)+num -> name+newnum optimization, which caused heavy
   performance regressions in Firefox when the intermediate computation wraps
   around the address space (#3438).
 - Improved dynamic linking support.
 - Improved emterpreter when doing dynamic linking.
 - Fixed an issue with source maps debug info containing zeroes as line numbers.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.32.1...1.32.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.32.1...1.32.2
    - Emscripten-Clang: no changes.

v1.32.1: 5/2/2015
-----------------
 - Removed old deprecated options -s INIT_HEAP, MICRO_OPTS, CLOSURE_ANNOTATIONS,
   INLINE_LIBRARY_FUNCS, SHOW_LABELS, COMPILER_ASSERTIONS and
   COMPILER_FASTPATHS.
 - Added support for dynamic linking and dlopen().
 - Fixed a compilation issue that affected -O2 builds and higher (#3430).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.32.0...1.32.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.32.0...1.32.1
    - Emscripten-Clang: no changes.

v1.32.0: 4/28/2015
------------------
 - Compile .i files properly as C and not C++ (#3365).
 - Removed old deprecated options -s PRECISE_I32_MUL, CORRECT_ROUNDINGS,
   CORRECT_OVERFLOWS, CORRECT_SIGNS, CHECK_HEAP_ALIGN, SAFE_HEAP_LINES,
   SAFE_HEAP >= 2, ASM_HEAP_LOG, SAFE_DYNCALLS, LABEL_DEBUG, RUNTIME_TYPE_INFO
   and EXECUTION_TIMEOUT, since these don't apply to fastcomp, which is now the
   only enabled compilation mode.
 - Preliminary work towards supporting dynamic linking and dlopen().
 - Fixed an issue where emrun stripped some characters at output (#3394).
 - Fixed alignment issues with varargs.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.31.3...1.32.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.31.3...1.32.0
    - Emscripten-Clang: no changes.

v1.31.3: 4/22/2015
------------------
 - Improved support for -E command line option (#3365).
 - Removed the old optimizeShifts optimization pass that was not valid for
   asm.js code.
 - Fixed an issue when simultaneously using EMULATE_FUNCTION_POINTER_CASTS and
   EMULATED_FUNCTION_POINTERS.
 - Fixed an issue with -s PRECISE_I64_MATH=2 not working (#3374).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.31.2...1.31.3
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.31.2...1.31.3
    - Emscripten-Clang: no changes.

v1.31.2: 4/20/2015
------------------
 - Added support for file suffixes .i and .ii (#3365).
 - Fixed an issue with embind and wide strings (#3299).
 - Removed more traces of the old non-fastcomp compiler code.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.31.1...1.31.2
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.31.1: 4/17/2015
------------------
 - Added support for unicode characters in EM_ASM() blocks (#3348).
 - Removed the pointer masking feature as experimental and unsupported.
 - Fixed an issue where exit() did not terminate execution of Emterpreter (#3360).
 - Removed traces of the old non-fastcomp compiler code.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.31.0...1.31.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.31.0...1.31.1
    - Emscripten-Clang: no changes.

v1.31.0: 4/14/2015
------------------
 - Remove references to unsupported EMCC_FAST_COMPILER mode, fastcomp is always enabled (#3347).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.30.6...1.31.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.30.6...1.31.0
    - Emscripten-Clang: no changes.

v1.30.6: 4/14/2015
------------------
 - Removed support for the deprecated jcache functionality (#3313).
 - Added support to emscripten_GetProcAddress() to fetch symbols with the ANGLE
   suffix (#3304, #3315).
 - Added immintrin.h header file to include all SSE support.
 - Added an async option to ccall (#3307).
 - Stopped from using 0 as a valid source ID for OpenAL (#3303).
 - When project has disabled exception catching, build an exceptions-disabled
   version of libcxx.
 - Split libcxx into two parts to optimize code size for projects that only need
   small amount of libcxx (#2545, #3308).
 - Avoid fprintf usage in emscripten_GetProcAddress() to allow using it with -s
   NO_FILESYSTEM=1 (#3327).
 - Removed old deprecated functionalities USE_TYPED_ARRAYS, FHEAP, GC emulation
   and non-asmjs-emscripten ABI.
 - Don't refer to prefixed GL extensions when creating a GL context (#3324).
 - Removed support code for x86_fp80 type (#3341).
 - Optimize EM_ASM() calls even more (#2596).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.30.5...1.30.6
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.30.5...1.30.6
    - Emscripten-Clang: no changes.

v1.30.5: 4/7/2015
-----------------
 - Fixed WebIDL operation when closure is enabled after the previous EM_ASM()
   optimizations.
 - Optimized jsCall() to handle variadic cases of number of arguments faster
   (#3290, #3305).
 - Removed support for the getwd() function (#1115, #3309).
 - Fixed a problem with -s IGNORED_FUNCTIONS and -s DEAD_FUNCTIONS not working
   as expected (#3239).
 - Fixed an issue with -s EMTERPRETIFY_ASYNC=1 and emscripten_sleep() not
   working (#3307).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.30.4...1.30.5
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.30.4...1.30.5
    - Emscripten-Clang: no changes.

v1.30.4: 4/3/2015
-----------------
 - Optimized the performance and security of EM_ASM() blocks by avoiding the use
   of eval() (#2596).
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.30.3...1.30.4
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.30.3...1.30.4
    - Emscripten-Clang: no changes.

v1.30.3: 4/3/2015
-----------------
 - Improved error handling in library_idbstore.js.
 - Fixed an asm.js validation issue with EMULATE_FUNCTION_POINTER_CASTS=1 feature (#3300).
 - Fixed Clang build by adding missing nacltransforms project after latest
   LLVM/Clang upstream merge.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.30.2...1.30.3
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.30.2...1.30.3
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.30.2...1.30.3

v1.30.2: 4/1/2015
-----------------
 - Added support to writing to mmap()ed memory by implementing msync() (#3269).
 - Updated SDL2 port to version 7.
 - Exported new singleton function Module.createContext() for creating a GL
   context from SDL2.
 - Added support for asm.js/Emscripten arch in Clang.
 - Finished LLVM 3.6 upgrade merge.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.30.1...1.30.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.30.1...1.30.2
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.30.1...1.30.2

v1.30.1: 3/24/2015
------------------
 - Upgraded LLVM+Clang from version 3.5 to version 3.6.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.30.0...1.30.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.30.0...1.30.1
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.30.0...1.30.1

v1.30.0: 3/24/2015
------------------
 - Fixed a bug where html5.h API would not remove event handlers on request.
 - Fixed a regression issue that broke building on Windows when attempting to
   invoke tools/gen_struct_info.py.
 - Improved memory growth feature to better handle growing to large memory sizes
   between 1GB and 2GB (#3253).
 - Fixed issues with emrun with terminating target browser process, managing
   lingering sockets and command line quote handling.
 - Fixed a bug where unsigned integer return values in embind could be returned
   as signed (#3249).
 - Improved handling of lost GL contexts.
 - Changed malloc to be fallible (return null on failure) when memory growth is
   enabled (#3253).
 - Fixed a bug with WebIDL not being able to handle enums (#3258).
 - Updated POINTER_MASKING feature to behave as a boolean rather than a mask
   (#3240).
 - Improved "emcmake cmake" on Windows to automatically remove from path any
   entries that contain sh.exe in them, which is not supported by CMake.
 - Fixed an issue with symlink handling in readlink (#3277).
 - Updated SDL2 port to version 6.
 - Removed the obsolete FAST_MEMORY build option.
 - Added reciprocalApproximation and reciprocalSqrtApproximation SIMD intrinsics.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.12...1.30.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.29.12...1.30.0
    - Emscripten-Clang: no changes.

v1.29.12: 3/15/2015
-------------------
 - Fix a bug where SDL_malloc and SDL_free were not available. (#3247)
 - Fix various issues with emrun usage. (#3234)
 - Fixed an off-by-one memory access in native optimizer.
 - Improve emterpreter support.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.11...1.29.12
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.29.11: 3/11/2015
-------------------
 - Remove the requirement to pass -s PRECISE_F32=1 manually when building with
   SIMD support.
 - Fix a temp directory leak that could leave behind empty directories in the
   temp directory after build (#706)
 - Improve support for growable Emscripten heap in asm.js mode.
 - Added a warning message when generating huge asset bundles with file packager.
 - Fixed a bug where emscripten_get_gamepad_status might throw a JS exception if
   called after a gamepad was disconnected.
 - Improve emterpreter sleep support.
 - Optimize code generation when multiple consecutive bitshifts are present.
 - Optimize redundant stack save and restores, and memcpy/memsets.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.10...1.29.11
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.29.10...1.29.11
    - Emscripten-Clang: no changes.

v1.29.10: 2/19/2015
-------------------
 - Add a warning message when generating code that has a very large number of
   variables, which optimization flags could remove.
 - Improve support for SIMD casts and special loads.
 - Fix the process return code when using EMCONFIGURE_JS=1.
 - Improved the error message in abort().
 - Fix main loop handling during emterpreter sync save/load.
 - Handle emscripten_async_call and friends during sleep, by pausing all
   `safeSet*()` operations.
 - Add support for Google WTF when building with --tracing.
 - Improve emterpreter stability with fuzzing.
 - Add an option to load the memory initializer file from a typed array (#3187)
 - Remove linker warning message when linking to -lm, since Emscripten includes
   musl that implements the math libraries built-in.
 - Add support for SDL_WM_SetCaption(), which calls to Module['setWindowTitle'],
   or if not present, sets the web page title. (#3192)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.9...1.29.10
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.29.9...1.29.10
    - Emscripten-Clang: no changes.

v1.29.9: 2/9/2015
-------------------
 - Documented FORCE_ALIGNED_MEMORY to be no longer supported.
 - Fixes issues with native optimizer handling of "if () else {}" statements.
   (#3129)
 - Improved cross-browser support for EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST.
   (#3165)
 - Added new linker option --profiling-funcs, which generates output that is
   otherwise minified, except that function names are kept intact, for use in
   profilers and getting descriptive call stacks.
 - The Module object is no longer written in global scope. (#3167)
 - Added new `emscripten_idb_*` API. (#3169)
 - Added new function emscripten_wget_data().
 - Add support for GL_RED with GLES3/WebGL2. (#3176)
 - Added basic WebVR support. (#3177)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.8...1.29.9
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.29.8: 1/31/2015
-------------------
 - Fix a temp file leak with emterpreter. (#3156)
 - Fix a typo that broke glBlitFramebuffer. (#3159)
 - Added scandir() and alphasort() from musl. (#3161)
 - Add a warning if multiple .a files with same basename are being linked
   together. (#2619)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.7...1.29.8
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.29.7...1.29.8
    - Emscripten-Clang: no changes.

v1.29.7: 1/28/2015
-------------------
 - Fixed an issue with backwards compatibility in emscripten-ports. (#3144)
 - Warn on duplicate entries in archives. (#2619)
 - Removed the MAX_SETJMPS limitation to improve setjmp/longjpmp support.
   (#3151)
 - Improve the native optimizer to not emit empty if clauses in some cases.
   (#3154)
 - Optimize Math.clz32, Math.min, NaN, and inf handling in asm.js.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.6...1.29.7
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.29.6...1.29.7
    - Emscripten-Clang: no changes.

v1.29.6: 1/23/2015
-------------------
 - Fixed an issue where calling `glGen*()` when the GL context was lost might
   throw a JS exception, instead a GL_INVALID_OPERATION is now recorded.
 - Improve label handling in native optimizer.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.5...1.29.6
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.29.5: 1/23/2015
-------------------
 - Enable compiling source files with the extension ".c++".
 - Enable versioning of the emscripten ports so that older Emscripten versions
   can keep using older versions of the ports (#3144)
 - Added a whitelist option to emterpreter, a linker flag of form -s
   EMTERPRETIFY_WHITELIST=["symbol1","symbol2"]. (#3129)
 - Improved emscripten_get_pointerlock_status() to always fill the output
   structure even when pointer lock is not supported.
 - Added an environment variable EMCC_NO_OPT_SORT=0/1 option to configure
   whether the generated output should have the functions sorted by length,
   useful for debugging.
 - Added new tool tools/merge_pair.py which allows bisecting differences between
   two output files to find discrepancies.
 - Improved parsing in cashew.
 - Improved output message from emconfigure and emmake when inputs are unexpected.
 - Added built-in asm handler for LLVM fabs operation.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.4...1.29.5
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.29.4...1.29.5
    - Emscripten-Clang: no changes.

v1.29.4: 1/21/2015
-------------------
 - Added new C <-> JS string marshalling functions asciiToString(),
   stringToAscii(), UTF8ToString(), stringToUTF8() that can be used to copy
   strings across the JS and C boundaries. (#2363)
 - Added new functions lengthBytesUTF8(), lengthBytesUTF16() and
   lengthBytesUTF32() to allow computing the byte lengths of strings in
   different encodings. (#2363)
 - Upgraded SDL2 port to version 4.
 - Add support for saving the emterpreter stack when there are functions
   returning a value on the stack (#3129)
 - Notice async state in emterpreter trampolines (#3129)
 - Optimize SDL1 pixel copying to the screen.
 - Fixed an issue with emterpreter parsing. (#3141)
 - Fixed an issue with native optimizer and -s PPRECISE_F32=1.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.3...1.29.4
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.29.3...1.29.4
    - Emscripten-Clang: no changes.

v1.29.3: 1/16/2015
-------------------
 - Fixed a bug with OpenGL context initialization enableExtensionsByDefault. (#3135)
 - Fixed an issue with nested if parsing in native optimizer.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.2...1.29.3
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.29.2: 1/16/2015
-------------------
 - Fixed an issue with embind compilation in LLVM 3.5.
 - Fixed an issue with SDL audio queueing stability, which would queue audio too
   eagerly and cause stutter in some applications (#3122, #3124)
 - Enabled native JS optimizer to be built automatically on Windows, requires
   VS2012 or VS2013.
 - Improve error message to reflect the fact that DLOPEN_SUPPORT is currently
   not available (#2365)
 - Improve SIMD load and store support.
 - Upgraded SDL2 port to version 3.
 - Fix a bug with native JS optimizer and braces in nested ifs.
 - Improved emterpreter support.
 - Fixed LLVM 3.5 to build with Visual Studio on Windows (emscripten-fastcomp #61)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.1...1.29.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.29.1...1.29.2
    - Emscripten-Clang: no changes.

v1.29.1: 1/7/2015
-------------------
 - Migrated to upstream PNaCl LLVM+Clang 3.5 from the previous 3.4.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.29.0...1.29.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.29.0...1.29.1
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.29.0...1.29.1

v1.29.0: 1/7/2015
-------------------
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.28.3...1.29.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.28.3...1.29.0
    - Emscripten-Clang: no changes.

v1.28.3: 1/4/2015
-------------------
 - embuilder.py tool
 - Many fixes for native optimizer on Windows
 - Perform LLVM LTO in a separate invocation of opt, so that it does not mix
   with legalization and other stuff we do at link time
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.28.2...1.28.3
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.28.2...1.28.3
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.28.2...1.28.3

v1.28.2: 12/17/2014
-------------------
 - Enable native optimizer by default
 - Disable slow2asm legacy testing (asm.js mode in pre-fastcomp)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.28.1...1.28.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.28.1...1.28.2
    - Emscripten-Clang: no changes.

v1.28.1: 12/15/2014
-------------------
 - Use a lot more MUSL math functions
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.28.0...1.28.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.28.0...1.28.1
    - Emscripten-Clang: no changes.

v1.28.0: 12/12/2014
-------------------
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.27.2...1.28.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.27.2...1.28.0
    - Emscripten-Clang: no changes.

v1.27.2: 12/10/2014
-------------------
 - Added more complete support for SSE1 SIMD intrinsics API. (#2792)
 - Fixed an issue with glTexImage2D on GL_LUMINANCE + GL_FLOAT textures. (#3039)
 - Use the cashew asm.js parser in native optimizer.
 - Fixed issues with IE when running closure minified pages. (#3012)
 - Enabled asm.js validation for SIMD compilation.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.27.1...1.27.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.27.1...1.27.2
    - Emscripten-Clang: no changes.

v1.27.1: 11/20/2014
-------------------
 - Migrated to upstream PNaCl LLVM+Clang 3.4 from the previous 3.3.
 - Added a FindOpenGL.cmake to support find_package() for OpenGL in CMake scripts.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.27.0...1.27.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.27.0...1.27.1
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.27.0...1.27.1

v1.27.0: 11/20/2014
-------------------
 - Added new work in progress option -s NATIVE_OPTIMIZER=1 that migrates
   optimizer code from JS to C++ for better performance.
 - Fixed an embind issue when compiling with closure (#2974)
 - Fixed an embind issue with unique_ptr (#2979)
 - Fixed a bug with new GL context initialization in proxy to worker mode.
 - Fixed an issue where GL context event handlers would leak after a GL context
   has been freed.
 - Optimized embind operation in Chrome by avoiding using Function.prototype.bind().
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.26.1...1.27.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.26.1...1.27.0
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
 - Added new HTML5 C API for managing fullscreen mode transitions to resolve
   cross-browser issue #2556 (#2975)
 - Fixed an issue with using structs in va_args (#2923)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.26.0...1.26.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.26.0...1.26.1
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.26.0...1.26.1

v1.26.0: 10/29/2014
-------------------
 - Fixed an issue where emar would forward --em-config to llvm-ar (#2886)
 - Added a new "emterpreter" feature that allows running Emscripten compiled
   code in interpreted form until asm.js compilation is ready (-s
   EMTERPRETIFY=1).
    - For more information, see
      https://groups.google.com/d/msg/emscripten-discuss/vhaPL9kULxk/_eD2G06eucwJ
 - Added new "Emscripten Ports" architecture that enables building SDL2 with -s
   USE_SDL=2 command line flag.
 - Added support for SDL 1.2 SDL_CreateRGBSurfaceFrom() function.
 - Improved experimental SIMD support.
 - Use only minimum necessary digits to print floating point literals in
   generated JS code for smaller code output.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.25.2...1.26.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.25.2...1.26.0
    - Emscripten-Clang: no changes.

v1.25.2: 10/16/2014
-------------------
 - Fixed a bug in tmpfile() function not allocating the mode argument correctly.
 - Fixed a bug with handling empty files in IDBFS (#2845)
 - Added an implementation of the utimes() function (#2845)
 - Added experimental WebGL 2.0 support with the linker flag -s USE_WEBGL2=1.
   (#2873)
 - Fixed a UnboundTypeError occurring in embind (#2875)
 - Fixed an error "IndexSizeError: Index or size is negative or greater than the
   allowed amount" being thrown by Emscripten SDL 1.2 surface blit code. (#2879)
 - Fixed a JS minifier issue that generated "x--y from x - -y" (#2869)
 - Added a new emcc command line flag "--cache <dir>" to control the location of
   the Emscripten cache directory (#2816)
 - Implemented SDL_ConvertSurface() and added support for SDL_SRCALPHA in
   SDL_SetAlpha (#2871)
 - Fixed issues with the GL library handling of invalid input values.
 - Optimized SDL copyIndexedColorData function (#2890)
 - Implemented GLES3 emulation for glMapBufferRange() for upcoming WebGL 2
   support, using the -s FULL_ES3=1 linker option.
 - Fixed a bug where setting up and cancelling the main loop multiple times
   would stack up the main loop to be called too frequently (#2839)
 - Introduced a new API emscripten_set_main_loop_timing() for managing the
   Emscripten main loop calling frequency (#2839)
 - Added new optimization flags SDL.discardOnLock and SDL.opaqueFrontBuffer to
   Emscripten SDL 1.2 SDL_LockSurface() and SDL_UnlockSurface() (#2870)
 - Fixed a bug with glfwGetProcAddress().
 - Added option to customize GLOBAL_BASE (the starting address of global
   variables in the Emscripten HEAP).
 - Added the ability to register mouseover and mouseout events from the HTML5
   API.
 - Improved experimental SIMD support.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.25.1...1.25.2
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.25.1: 10/1/2014
------------------
 - Updated heap resize support code when -s ALLOW_MEMORY_GROWTH=1 is defined.
 - Updated libc++ to new version from upstream svn revision 218372, 2014-09-24.
 - Fixed a bug where building on Windows might generate output JS files with
   incorrect syntax (emscripten-fastcomp #52)
 - Improved experimental SIMD support.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.25.0...1.25.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.25.0...1.25.1
    - Emscripten-Clang: no changes.


v1.25.0: 9/30/2014
------------------
 - Fixed a warning message with -s EXPORTED_FUNCTIONS.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.24.1...1.25.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.24.1: 9/27/2014
------------------
 - Fixed issues with the tmpnam and tmpfile functions (#2797, 2798)
 - Fixed CMake package find code to not search any system directories, because
   Emscripten is a cross-compiler.
 - Improved support for the proposed solution for heap resizing.
 - Fixed an issue where one could not run a main loop without having first a GL
   context created when -s FULL_ES2 or -s LEGACY_GL_EMULATION were set.
 - For compatibility, Emscripten will no longer warn about missing library files
   for -lGL, -lGLU and -lglut libraries, since Emscripten provides the
   implementation for these without having to explicitly link to anything.
 - Added support for readonly (const) attributes and automatically call
   Pointer_stringify on DOMStrings in WebIDL.
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Added support for GLFW 3.0.
 - Added new Emscripten HTML 5 functions emscripten_set_mouseenter_callback()
   and emscripten_set_mouseleave_callback().
 - Emscripten now recognizes an environment variable
   EMCC_JSOPT_BLACKLIST=a,b,c,d which can be used to force-disable Emscripten to
   skip running specific JS optimization passes. This is intended as a debugging
   aid to help zoom in on JS optimizer bugs when compiling with -O1 and greater.
   (#2819)
 - Fixed a bug where Module['TOTAL_STACK'] was ignored (#2837).
 - Improved SIMD support for the experimental Ecmascript SIMD spec. Preliminary asm.js validation.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.24.0...1.24.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.24.0...1.24.1
    - Emscripten-Clang: no changes.

v1.24.0: 9/16/2014
------------------
 - Renamed the earlier Module.locateFilePackage() to Module.locateFile() added
   in v1.22.2 to better reflect its extended usage.
 - Improved exceptions support with exception_ptr.
 - Fixed a bug where restoring files from IDBFS would not preserve their file modes.
 - Fixed and issue where one could not pass a null pointer to strftime() function.
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.23.5...1.24.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.23.5...1.24.0
    - Emscripten-Clang: no changes.

v1.23.5: 9/12/2014
------------------
 - Added new functions emscripten_get_device_pixel_ratio(),
   emscripten_set_canvas_css_size() and emscripten_get_canvas_css_size() which
   allow handling High DPI options from C code.
 - Fixed bugs with timzone-related functions in the JS-implemented C standard
   library.
 - Implemented clock_gettime(CLOCK_MONOTONIC) and added a new function
   emscripten_get_now_is_monotonic() to query whether the JS-provided timer is
   monotonic or not.
 - Fixed an issue where the user could not pass --llvm-opts=xxx when also
   specifying --llvm-lto=2.
 - Renamed the linker option -profiling to --profiling for consistency. The old
   form is still supported.
 - Formalized the set of valid characters to be used in files passed to the
   file_packager.py (#2765).
 - Implemented SDL function SDL_BlitScaled.
 - Fixed a bug with right modifier keys in SDL.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.23.4...1.23.5
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.23.4: 9/7/2014
------------------
 - Implemented new targetX and targetY fields for native HTML5 mouse and touch
   events (#2751)
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.23.3...1.23.4
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.23.3...1.23.4
    - Emscripten-Clang: no changes.

v1.23.3: 9/7/2014
------------------
 - Removed the scons-tools SCons build system as unused.
 - Fixed an issue where applications could not handle WebGL context creation
   failures gracefully.
 - Fixed a bug where the stringToC function in ccall/cwrap might not allocate
   enough space to hold unicode strings.
 - Removed CMake from attempting to link to library -ldl when building projects,
   by unsetting CMAKE_DL_LIBS.
 - Fixed a bug where write_sockaddr might return undefined data in its output
   structure.
 - Added a new _experimental_ -s POINTER_MASKING=1 linker option that might help
   JS VMs to optimize asm.js code.
 - Added first version of a memory tracing API to profile memory usage in
   Emscripten applications.
 - Added functions glob and globfree from musl regex library.
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.23.2...1.23.3
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.23.2...1.23.3
    - Emscripten-Clang: no changes.

v1.23.2: 9/2/2014
------------------
 - Adjusted the process and group ids reported by the stub library functions to
   be closer to native unix values.
 - Set stack to be aligned to 16 bytes. (#2721)
 - Fixed a compiler error "unresolved symbol:
   __cxa_decrement_exception_refcount" (#2715)
 - Added a new warning message that instructs that building .so, .dll and .dylib
   files is not actually supported, and is faked for compatibility reasons for
   existing build chains. (#2562)
 - Fixed problems with SDL mouse scrolling (#2643)
 - Implemented OpenAL function alSourceRewind.
 - Removed several old header files from the Emscripten repository that had been
   included for emulation purposes (zlib.h, png.h, tiff.h, tiffio.h), but their
   implementation is not included.
 - Work around an issue in d8 with binary file reading that broke e.g. printf
   when running in d8. (#2731)
 - Rigidified the semantics of Module.preRun and Module.postRun: These must
   always be JS arrays, single functions are not allowed (#2729)
 - Improved compiler warning diagnostics when generating output that will not
   validate as asm.js (#2737)
 - Updated to latest emrun version to enable support for passing arguments with
   hyphens to the program. (#2742)
 - Added Bessel math functions of the first kind  (j0, j1, jn) from musl.
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.23.1...1.23.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.23.1...1.23.2
    - Emscripten-Clang: no changes.

v1.23.1: 8/26/2014
------------------
 - Add support for the Chrome variant of the Gamepad API.
 - Updates to SIMD.js support.
 - Implemented glutSetCursor function.
 - Added new link-time options -s NO_FILESYSTEM=1 and -s NO_BROWSER=1 to enable
   reducing output file sizes when those functionalities are not necessary.
 - Added a new option --closure 2 to allow running closure even on the asm.js output.
 - Fixed a regression bug that broke the use of
   emscripten_set_socket_error_callback() in emscripten.h
 - Removed the support for old discontinued Mozilla Audio Data API in src/library_sdl.js.
 - Removed the support for using Web Audio ScriptProcessorNode to stream audio.
 - Improved SDL audio streaming by using the main rAF() callback instead of a
   separate setTimeout() callback to schedule the audio data.
 - Deprecated compiling without typed arrays support.
 - Migrated to using musl PRNG functions. Fixes reported bugs about the quality of randomness (#2341)
 - Improved SIMD support for the experimental Ecmascript SIMD spec.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.23.0...1.23.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.23.0...1.23.1
    - Emscripten-Clang: no changes.

v1.23.0: 8/21/2014
------------------
 - Added support for array attributes in WebIDL bindings.
 - Allow cloning pointers that are scheduled for deletion in embind, and add
   support for null in embind_repr().
 - Fixed possible issues with rounding and flooring operations.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.22.2...1.23.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.22.2: 8/19/2014
------------------
 - Adds stack overflow checks when building with the link flag -s ASSERTIONS=1.
 - Fix an issue where EM_ASM was not usable with closure when closure removed
   the Module object (#2639)
 - The locale "POSIX" is now recognized (#2636)
 - Fixed a problem with embind on IE11.
 - Added OpenAL functions alSource3i, alListener3f, alGetEnumValue and
   alSpeedOfSound and also recognize ALC_MAX_AUXILIARY_SENDS.
 - Fixed an issue where emcc would create .o files in the current directory when
   compiling multiple code files simultaneously (#2644)
 - The -s PROXY_TO_WORKER1= option now looks for a GET option "?noProxy" in the
   page URL to select at startup time whether proxying should be on or off.
 - Added new functions emscripten_yield, emscripten_coroutine_create and
   emscripten_coroutine_next which implement coroutines when building with the
   -s ASYNCIFY=1 option.
 - Optimized the size of intermediate generated .o files by omitting LLVM debug
   info from them when not needed. (#2657)
 - Fixed WebSocket connection URLs to allow a port number in them, e.g.
   "server:port/addr" (2610)
 - Added support for void* to the WebIDL binder, via the identifier VoidPtr.
 - Optimize emcc to not copy bitcode files around redundantly.
 - Fix stat() to correctly return ENOTDIR when expected (#2669).
 - Fixed issues with nested exception catching (#1714).
 - Increased the minimum size of the Emscripten HEAP to 64k instead of a previous 4k.
 - The {{{ cDefine('name') }}} macros now raise a compile-time error if the
   define name is not found, instead of hiding the error message inside the
   compiled output (#2672)
 - Fixed an issue where --emrun parameter was not compatible with the -s
   PROXY_TO_WORKER=1 option.
 - Improved WebGL support when compiling with the PROXY_TO_WORKER=1 option.
 - Fixed a regression issue with the handling of running dtors of classes that
   use virtual inheritance. (#2682)
 - Added an option Module.locateFilePackage() as a means to customize where data
   files are found in relative to the running page (#2680). NOTE: This parameter
   was later renamed to Module.locateFile() instead in release 1.24.0.
 - Fixed a bug where OpenAL sources would not properly delete.
 - Fixed a bug with upstream libc++ on std::map, std::multimap and
   std::unordered_map self-assignment
   (http://llvm.org/bugs/show_bug.cgi?id=18735)
 - Allow using __asm__ __volatile__("": : :"memory") as a compile-time
   reordering barrier (#2647)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.22.1...1.22.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.22.1...1.22.2
    - Emscripten-Clang: no changes.

v1.22.1: 8/7/2014
------------------
 - Added support for prefixing functions with '$' in JS libraries, in order to
   cause them not be prefixed with '_' when compiling.
 - Improved WebIDL compiler to support enums.
 - Fixed a bug with emscripten_force_exit() that would throw an exception (#2629).
 - Fixed setlocale() when setting a bad locale. (#2630)
 - Fixed a compiler miscompilation bug when optimizing loops. (#2626)
 - Fixed an issue with rethrowing an exception (#2627)
 - Fixed a bug where malloc()ing from JS code would leak memory if the C/C++
   side does not use malloc() (#2621)
 - Removed an unnecessary assert() in glReadPixels, and improved it to support
   more texture pixel types.
 - Fixed a bug with std::locale accepting unknown locale names (#2636)
 - Added support for WebIDL binder to work with Closure (#2620)
 - Added no-op SDL IMG_Quit() and TTF_Quit() symbols.
 - Migrated to building libcxx and libcxxapi with -Oz optimization flags.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.22.0...1.22.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.22.0: 8/5/2014
------------------
 - Added support to emrun to dump files to the local filesystem for debugging
   purposes.
 - Implemented emscripten_wget in ASYNCIFY mode.
 - Improved extension catching support (#2616)
 - Fixed .a link groups to also work when linking to bitcode. (#2568)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.10...1.22.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.21.10...1.22.0
    - Emscripten-Clang: no changes.

v1.21.10: 7/29/2014
-------------------
 - Fixed a Windows-specific issue where the generated output files might contain
   line endings of form \r\r\n. This caused browser debuggers to get confused
   with line numbers. (#2133)
 - Improved the node.js workaround introduced in v1.21.8.
 - Implemented new HTML5 API for direct WebGL context creation, emscripten_webgl_*().
 - Fixed a bug when loading in node.js and loaded by another module (#2586)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.9...1.21.10
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.9: 7/28/2014
------------------
 - Fixed issues with exception catching. (#2531)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.8...1.21.9
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.8: 7/28/2014
------------------
 - Fixed an issue when using --embed-file to embed very large files.
 - Worked around a Windows node.js bug where the compiler output might get cut
   off when the compilation ends in an error.
   (https://github.com/joyent/node/issues/1669)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.7...1.21.8
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.21.7...1.21.8
    - Emscripten-Clang: no changes.

v1.21.7: 7/25/2014
------------------
 - Added new environment variable EMCC_ONLY_FORCED_STDLIBS which can be used to
   restrict to only linking to the chosen set of Emscripten-provided libraries.
   (See also EMCC_FORCE_STDLIBS)
 - Adjusted argv[0] and environment variables USER, HOME, LANG and _ to report a
   more convenient set of default values. (#2565)
 - Fixed an issue where the application could not use environ without also
   referring to getenv() (#2557)
 - Fixed an issue with IDBFS running in web workers.
 - Print out an error if IDBFS is used without IDB support.
 - Fixed calling Runtime.getFuncWrapper() when -s ALIASING_FUNCTION_POINTERS=1 (#2010)
 - Fixed an issue where deleting files during directory iteration would produce
   incorrect iteration results (#2528)
 - Fixed support for strftime with %z and %Z (#2570)
 - Fixed a bug with truncate() throwing an exception (#2572)
 - Improved the linker to generate warning messages if user specifies -s X=Y
   linker flags that do not exist (#2579)
 - Fixed an issue with creating read-only files (#2573)
 - Added first implementation for the ASYNCIFY option, which splits up
   synchronous blocking loops to asynchronous execution. For more information on
   this approach, see https://github.com/emscripten-core/emscripten/wiki/Asyncify
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.6...1.21.7
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.21.6...1.21.7
    - Emscripten-Clang: no changes.

v1.21.6: 7/22/2014
------------------
 - Separated OpenAL AL and ALC errors to properly separate fields.
 - When using EGL to initialize a GL context, initialize a stencil buffer to the
   context as well, since proper EGL context choosing is not yet implemented.
 - Added new linker flag -s DEMANGLE_SUPPORT to choose whether to compile the
   application with libcxxabi-provided demangling support ___cxa_demangle().
 - Fixed a problem where calling stat() on a nonexisting file in the runtime VFS
   would result in an exception being thrown. (#2552)
 - When using the -v flag, no longer retain intermediate compilation files. To
   preserve the intermediate files, set the EMCC_DEBUG=1 environment variable.
   (#2538)
 - Added a new HTML setting Module.memoryInitializerPrefixURL which specifies a
   prefix for where the memory initializer file .mem.js should be loaded from
   (#2542)
 - Implemented eglReleaseThread to work according to spec.
 - Implemented a new function emscripten_force_exit() which immediately shuts
   down the C runtime.
 - Fixed a bug with exception handling that resulted in an error unresolved
   symbol: _ZTISt13bad_exception (#2560)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.5...1.21.6
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.5: 7/21/2014
------------------
 - Added support for glDrawBuffers with the WEBGL_draw_buffers extension.
 - Added stub implementation for eglReleaseThread.
 - Fixed a bug where passing -E to emcc used the system include headers instead
   of the built-in ones. (#2534)
 - Fixed the stacktrace() function to work on MSIE as well.
 - Removed the zlib.h header file from system include directory, since
   Emscripten does not provide an implementation of zlib built-in.
 - Added support for __cxa_bad_typeid (#2547)
 - Fixed an internal compiler crash with a certain pattern involving optimized
   builds and int64_t (#2539)
 - Fixed an issue with -s EXCEPTION_CATCHING_WHITELIST handling where an
   extension that was a substring of another might get erroneously handled.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.4...1.21.5
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.21.4...1.21.5
    - Emscripten-Clang: no changes.

v1.21.4: 7/17/2014
------------------
 - Implemented the getsockopt() function.
 - Added new event callback functions emscripten_set_socket_xx_callback() that
   allow listening to WebSocket events in an asynchronous manner.
 - Greatly improved CMake support, now various forms of configure-time test
   builds are supported, and the default extension is set to ".js"
 - Prohibit the virtual filesystem from creating files with name '.' or '..' at
   runtime.
 - Have runtime mkdir() function call normalize the path to be created before
   creation.
 - Fixed an issue with omitting the third parameter in cwrap() call (#2511).
 - Fixed an issue where mouse event handling would throw an exception if the
   page did not contain a canvas object.
 - Fixed a GL initialization problem when user has extended Array with custom
   functions (#2514)
 - Added new compiler defines __EMSCRIPTEN_major__, __EMSCRIPTEN_minor__ and
   __EMSCRIPTEN_tiny__ which communicate the compiler version major.minor.tiny
   to compiled applications (#2343)
 - Fixed a bug where emrun did not properly capture the exit code when exit
   runtime via not calling exit().
 - Fixed an error message when symlinkin invalid filenams at runtime.
 - Fixed a bug in EGL context creation that parsed the input context creation
   parameters with wrong terminator.
 - Improved ffdb.py to be smarter when to attempt port forwarding to connect to
   a FFOS device DevTools port.
 - Implemented strsignal() function (#2532)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.3...1.21.4
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.3: 7/10/2014
------------------
 - Added implementations for SDL function SDL_AudioQuit and SDL_VideoQuit.
 - Fix an issue with the optimizeShifts optimization enabled in previous version.
 - Fixed the -s RELOOPER command line parameter to work.
 - Fixed a bug where building the system libc might result in a compiler deadlock
   on Windows.
 - Removed emcc from trying to link in .dll files as static libraries on
   Windows.
 - Added support for GL_HALF_FLOAT_OES.
 - Fixed a bug where emcmake did not work on Windows.
 - Use multithreaded compilation to build libc.
 - Fixed an issue where the GL interop library could throw an exception in an
   error condition, instead of raising a GL error.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.2...1.21.3
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.2: 7/5/2014
------------------
 - Improved the checks that detect that code is run only while the runtime is
   initialized.
 - The memory initializer file (.mem.js) is now emitted by default when
   compiling with at least -O2 optimization level.
 - Fixed a performance issue where built-in math functions (Math.sqrt, etc.)
   took a slightly slower path (#2484).
 - Added support for the ffs libc function.
 - Re-enabled optimizeShifts optimization when not compiling for asm.js (#2481)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.1...1.21.2
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.1: 7/3/2014
------------------
 - Fixed an issue where wrong python interpreter could get invoked on Windows
   when both native and cygwin python were installed.
 - Updated musl from version 0.9.13 to version 1.0.3.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.21.0...1.21.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.21.0: 7/2/2014
------------------
 - Enable memory init files (.mem) by default in optimized builds (-O2+), as if
   --memory-init-file 1  is specified. This makes the default behavior on
   optimized builds emit smaller and faster-to-load code, but does require that
   you ship both a .js and a .mem file (if you prefer not to, can use
   --memory-init-file 1  ).
 - Implemented new SDL 1.2 functions SDL_GetRGB, SDL_GetRGBA and SDL_putenv.
 - Added support for /dev/random, /dev/urandom and C++11 std::random_device,
   which will use cryptographically secure random api if available. (#2447)
 - Added support for CMake find_path() directive.
 - Added support for std::unique_ptr in embind.
 - Improved Windows support for ffdb.py.
 - Implemented the clip_rect structure for created SDL surfaces.
 - Fixed a regression with SDL touch events (#2466)
 - Added support for C++11 std::thread::hardware_concurrency which backs to
   navigator.hardwareConcurrency. See
   http://wiki.whatwg.org/wiki/Navigator_HW_Concurrency (#2456)
 - Optimized embind code generation with constexprs.
 - Enabled the use of Runtime.add&removeFunction when closure minification is
   active (#2446)
 - Implemented support for accessing WebGL when building via the proxy to worker
   architecture.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.20.0...1.21.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.20.0: 6/13/2014
------------------
 - Optimize in-memory virtual filesystem performance when serialized to an IndexedDB.
 - Fixed memcpy regression with ta0 and ta1 modes.
 - Fixed an issue with line numbers being messed up when generating source maps (#2410)
 - Fixed an ffdb logging bug that could cause it to drop messages if they were
   being received too fast. Added support getting memory and system descriptions
   with ffdb.
 - Added a new extension to SDL "emscripten_SDL_SetEventHandler()" which enabled
   application to perform SDL event handling inside a JS event handler to
   overcome browser security restrictions. (#2417)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.19.2...1.20.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.19.2: 6/9/2014
------------------
 - Updated CMake support for response file handling.
 - Fixed issues with glfwGetProcAddress and glfwSetWindowSizeCallback.
 - Fixed an issue with regexes that caused issues on IE11 runtime (#2400)
 - Added a new functions emscripten_get_preloaded_image_data() and
   emscripten_get_preloaded_image_data_from_FILE() to obtain pixel data of
   preloaded images.
 - Greatly improved ffdb capabilities to operate a FFOS device.
 - Fixed a Windows-specific bug where the user temp directory was littered with
   temporary .rsp files that did not get cleaned up.
 - Improved SIMD support.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.19.1...1.19.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.19.1...1.19.2
    - Emscripten-Clang: no changes.

v1.19.1: 6/3/2014
------------------
 - Migrate to using musl sscanf and sprintf and the family that writes to
   memory, and not directly to the filesystem.
 - Improve the error messages from -s SAFE_HEAP_ACCESS=1 runtime checks.
 - Added new linker flag -s NO_DYNAMIC_EXECUTION=1 which removes the use of
   eval() and new Function() in the generated output. For more information, see
   "Eval and related functions are disabled" in
   https://developer.chrome.com/extensions/contentSecurityPolicy .
 - Fixed a compiler issue when very large double constants are present. (#2392)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.19.0...1.19.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.19.0: 5/29/2014
------------------
 - Added an error message to signal that linkable modules are not supported in fastcomp.
 - Fixed a miscompilation issue that resulted in an error "SyntaxError: invalid
   increment operand" and a statement +(+0) being generated (#2314)
 - Make optimized compiler output smaller by running the shell code through
   uglify when not using closure.
 - Fixed a crash in SDL audio loading code introduced in v1.18.3
 - Fixed an issue where glTex(Sub)Image2D might throw an exception on error,
   instead of setting glGetError().
 - Added new typedefs emscripten_align1_short, emscripten_align{1/2}_int,
   emscripten_align{1/2}_float and emscripten_align{1/2/4}_double to ease
   signaling the compiler that unaligned data is present. (#2378)
 - Fixed an embind issue with refcount tracking on smart pointers.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.18.4...1.19.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.18.4...1.19.0
    - Emscripten-Clang: no changes.

v1.18.4: 5/27/2014
------------------
 - Fixed error message on unsupported linking options (#2365)
 - Updated embind to latest version from IMVU upstream.
 - Fixed an issue where source maps did not load properly in Firefox.
 - Added a more descriptive error message to fastcomp when MAX_SETJMPS limit is
   violated. (#2379)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.18.3...1.18.4
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.18.3...1.18.4
    - Emscripten-Clang: no changes.

v1.18.3: 5/21/2014
------------------
 - Added support to emcc command line for "archive groups": -Wl,--start-group
   and -Wl,--end-group
 - Greatly optimized ccall and cwrap implementations.
 - Added new support for SDL_Mix backend to use WebAudio to play back audio clips.
 - Fixed a registerizeHarder issue with elimination of conditional expressions.
 - Migrated single-character standard C functions (islower, tolower, and the
   family) to use musl implementations.
 - Updated relooper to not optimize out breaks if it causes excessive nesting.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.18.2...1.18.3
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.18.2...1.18.3
    - Emscripten-Clang: no changes.

v1.18.2: 5/19/2014
------------------
 - Fixed a problem which blocked user applications from handling WebGL context
   loss events themselves.
 - Added a new HTML5 api function emscripten_is_webgl_context_lost() which
   allows polling for context loss in addition to receiving events.
 - Improved async wget progress events to work better across browsers.
 - Improved WebIDL binder support.
 - Added new typeof() function to emscripten::val.
 - Added support for SDL window events SDL_WINDOWEVENT_FOCUS_GAINED,
   SDL_WINDOWEVENT_FOCUS_LOST, SDL_WINDOWEVENT_SHOWN, SDL_WINDOWEVENT_HIDDEN.
 - Fixed a compiler miscompilation on unsigned i1 bitcasts (#2350)
 - Fixed a compiler bug where doubles in varargs might not get 8-byte aligned (#2358)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.18.1...1.18.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.18.1...1.18.2
    - Emscripten-Clang: no changes.

v1.18.1: 5/12/2014
------------------
 - Fixed an issue where the mouse wheel scroll did not work with SDL.
 - Fixed an issue with emscripten_async_wget, which undesirably expected that
   the string pointer passed to it stayed alive for the duration of the
   operation (#2349)
 - Emscripten now issues a warning message when the EXPORTED_FUNCTIONS list
   contains invalid symbol names (#2338)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.18.0...1.18.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.18.0: 5/10/2014
------------------
 - Enable support for low-level C<->JS interop to marshall 64 bit integers from
   C to JS.
 - Fixed an issue that caused some programs to immediately run out of memory
   "(cannot enlarge memory arrays)" at startup. (#2334)
 - Fixed a crash issue with generated touch events that didn't correspond to a real touch.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.17.0...1.18.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.17.0...1.18.0
    - Emscripten-Clang: no changes.

v1.17.0: 5/6/2014
------------------
 - Enabled asm.js compilation and -s PRECISE_F32 support when using embind.
 - Improved relooper to emit switches in many-entried blocks.
 - Fixed a GLFW bug where mouse wheel direction was reversed.
 - Fixed glfwGetKey to work even when no callback is registered with
   glfwGetKeyCallback (#1320)
 - Added a new tool 'webidl_binder' that generates C <-> JS interop code from
   WebIDL descriptions.
 - Fix emscripten compilation to work on pages that don't contain a HTML canvas.
 - Added a new error message to default shell when an uncaught exception is thrown.
 - Improved error diagnostics reported by -s SAFE_HEAP=1.
 - Added support for registering callbacks hook to VFS file open, write, move,
   close and delete.
 - Added embind support to std::basic_string<unsigned char>
 - By default, the C runtime will no longer exit after returning from main()
   when safeSetTimeout() or safeSetInterval() is used.
 - Fixed an issue with sscanf formatting (#2322)
 - Fixed an issue where precompiled headers were given a wrong output filename (#2320)
 - Enabled registerizeHarder optimization pass to work when outlining is enabled.
 - Fixed an issue with strptime month handling (#2324)
 - Added an initial implementation of a new tool 'ffdb' which can be used to
   operate a Firefox OS phone from the command line.
 - Fixed a compiler crash on assertion failure '!contains(BranchesOut, Target)'
   (emscripten-fastcomp #32)
 - Added a new ABI to Clang that targets Emscripten specifically. Stop aligning
   member functions to save some space in the function table array.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.16.0...1.17.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.16.0...1.17.0
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.16.0...1.17.0

v1.16.0: 4/16/2014
------------------
 - Removed browser warnings message in VFS library about replacing __proto__ performance issue.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.15.1...1.16.0
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.15.1...1.16.0

v1.15.1: 4/15/2014
------------------
 - Added support for SDL2 touch api.
 - Added new user-controllable emdind-related define #define
   EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES, which allows optimizing embind for minimal
   size when std::type_info is not needed.
 - Fixed issues with CMake support where CMAKE_AR and CMAKE_RANLIB were not
   accessible from CMakeLists.txt files.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.15.0...1.15.1
    - Emscripten-LLVM: no changes.
    - Emscripten-Clang: no changes.

v1.15.0: 4/11/2014
------------------
 - Fix outlining feature for functions that return a double (#2278)
 - Added support for C++11 atomic constructs (#2273)
 - Adjusted stdout and stderr stream behavior in the default shell.html to
   always print out to both web page text log box, and the browser console.
 - Fixed an issue with loop variable optimization.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.14.1...1.15.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.14.1...1.15.0
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.14.1...1.15.0

v1.14.1: 4/8/2014
------------------
 - Added new command line utility 'emcmake', which can be used to call
   emconfigure for cmake.
 - Added a new emcc command line parameter '--valid-abspath', which allows
   selectively suppressing warning messages that occur when using absolute path
   names in include and link directories.
 - Added a new emcc linker command line parameter '--emit-symbol-map', which
   will save a map file between minified global names and the original function
   names.
 - Fixed an issue with --default-object-ext not always working properly.
 - Added optimizations to eliminate redundant loop variables and redundant
   self-assignments.
 - Migrated several libc functions to use compiled code from musl instead of
   handwritten JS implementations.
 - Improved embind support.
 - Renamed the EM_ASM_() macro to the form EM_ASM_ARGS().
 - Fixed mouse button ordering issue in glfw.
 - Fixed an issue when creating a path name that ends in a slash (#2258, #2263)
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.14.0...1.14.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.14.0...1.14.1
    - Emscripten-Clang: no changes.

v1.14.0: 3/25/2014
------------------
 - Added new emcc linker command line option '-profiling', which defaults JS
   code generation options suited for benchmarking and profiling purposes.
 - Implemented the EGL function eglWaitGL().
 - Fixed an issue with the HTML5 API that caused the HTML5 event listener unregistration to fail.
 - Fixed issues with numpad keys in SDL support library.
 - Added a new JS optimizer pass 'simplifyIfs', which is run when -s
   SIMPLIFY_IFS=1 link flag is set and -g is not specified. This pass merges
   multiple nested if()s together into single comparisons, where possible.
 - Removed false positive messages on missing internal "emscripten_xxx" symbols at link stage.
 - Updated to latest relooper version.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.13.2...1.14.0
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.13.2...1.14.0
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
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.13.1...1.13.2
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.13.1...1.13.2
    - Emscripten-Clang: no changes.

v1.13.1: 3/10/2014
------------------
 - Disallow C implicit function declarations by making it an error instead of a
   warning by default. These will not work with Emscripten, due to strict
   Emscripten signature requirements when calling function pointers (#2175).
 - Allow transitioning to full screen from SDL as a response to mouse press
   events.
 - Fixed a bug in previous 1.13.0 release that broke fullscreen transitioning
   from working.
 - Fixed emscripten/html5.h to be used in C source files.
 - Fix an issue where extraneous system libraries would get included in the
   generated output (#2191).
 - Added a new function emscripten_async_wget2_data() that allows reading from
   an XMLHTTPRequest directly into memory while supporting advanced features.
 - Fixed esc key code in GLFW.
 - Added new emscripten_debugger() intrinsic function, which calls into JS
   "debugger;" statement to break into a JS debugger.
 - Fixed varargs function call alignment of doubles to 8 bytes.
 - Switched to using default function local stack alignment to 16 bytes to be SIMD-friendly.
 - Improved error messages when user code has a syntax error in em_asm() statements.
 - Switched to using a new custom LLVM datalayout format for Emscripten. See
   https://github.com/emscripten-core/emscripten-fastcomp/commit/65405351ba0b32a8658c65940e0b65ceb2601ad4
 - Optimized function local stack space to use fewer temporary JS variables.
 - Full list of changes:
    - Emscripten: https://github.com/emscripten-core/emscripten/compare/1.13.0...1.13.1
    - Emscripten-LLVM: https://github.com/emscripten-core/emscripten-fastcomp/compare/1.13.0...1.13.1
    - Emscripten-Clang: https://github.com/emscripten-core/emscripten-fastcomp-clang/compare/1.13.0...1.13.1

v1.13.0: 3/3/2014
------------------
 - Fixed the deprecated source mapping syntax warning.
 - Fixed a buffer overflow issue in emscripten_get_callstack (#2171).
 - Added support for -Os (optimize for size) and -Oz (aggressively optimize for
   size) arguments to emcc.
 - Fixed a typo that broko the call signature of glCompressedTexSubImage2D()
   function (#2173).
 - Added new browser fullscreen resize logic that always retains aspect ratio
   and adds support for IE11.
 - Improve debug messaging with bad function pointer calls when -s ASSERTIONS=2
   is set.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.12.3...1.13.0

v1.12.3: 2/27/2014
------------------
 - Fixed alcOpenDevice on Safari.
 - Improved the warning message on missing symbols to not show false positives (#2154).
 - Improved EmscriptenFullscreenChangeEvent HTML5 API structure to return
   information about HTML element and screen sizes for convenience.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.12.2...1.12.3

v1.12.2: 2/25/2014
------------------
 - Added better warning message if Emscripten, LLVM and Clang versions don't match.
 - Introduced the asmjs-unknown-emscripten target triple that allows
   specializing LLVM codegen for Emscripten purposes.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.12.1...1.12.2

v1.12.1: 2/25/2014
------------------
 - TURNED ON FASTCOMP BY DEFAULT. This means that you will need to migrate to
   fastcomp-clang build. Either use an Emscripten SDK distribution, or to build
   manually, see
   http://kripken.github.io/emscripten-site/docs/building_from_source/LLVM-Backend.html
   for info.
 - Migrate to requiring Clang 3.3 instead of Clang 3.2. The fastcomp-clang
   repository by Emscripten is based on Clang 3.3.
 - Deprecated old Emscripten libgc implementation.
 - asm.js will now be always enabled, even in -O0 builds in fastcomp.
 - Remove support for -s RUNTIME_TYPE_INFO, which is unsupported in fastcomp.
 - Added a new "powered by Emscripten" logo.
 - Updated default shell.html graphical layout.
 - Added new macro EM_ASM_, which allows sending values to JS without returning anything.
 - Deprecated the jcache compiler option. It should not be needed anymore.
 - Added support for fetching callstack column information in Firefox 30 in emscripten_get_callstack.
 - Fix issues with missing exceptions-related symbols in fastcomp.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.12.0...1.12.1

v1.12.0: 2/22/2014
------------------
 - Improved the runtime abort error message when calling an invalid function
   pointer if compiled with -s ASSERTIONS=1 and 2. This allows the developer to
   better deduce errors with bad function pointers or function pointers casted
   and invoked via a wrong signature.
 - Added a new api function emscripten_set_main_loop_arg, which allows passing a
   userData pointer that will be carried via the function call, useful for
   object-oriented encapsulation purposes (#2114).
 - Fixed CMake MinSizeRel configuration type to actually optimize for minimal size with -Os.
 - Added support for GLES2 VAO extension OES_vertex_array_object for browsers that support it.
 - Fix issues with emscripten/html5.f when compiled with the SAFE_HEAP option.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.11.1...1.12.0

v1.11.1: 2/19/2014
------------------
 - Improved eglSwapBuffers to be spec-conformant.
 - Fixed an issue with asm.js validation and va_args (#2120).
 - Fixed asm.js validation issues found with fuzzing.
 - Added new link-time compiler flag -s RETAIN_COMPILER_SETTINGS=1, which
   enables a runtime API for querying which Emscripten settings were used to
   compile the file.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.11.0...1.11.1

v1.11.0: 2/14/2014
------------------
 - Implemented some new SDL library functions.
 - Renamed standard file descriptors to have handles 0, 1 and 2 rather than 1, 2
   and 3 to coincide with unix numbering.
 - Improved embind support with smart pointers and mixins.
 - Improved the registerization -O3 optimization pass around switch-case constructs.
 - Upper-case files with suffix .C are now also recognized (#2109).
 - Fixed an issue with glGetTexParameter (#2112).
 - Improved exceptions support in fastcomp.
 - Added new linker option -s NO_EXIT_RUNTIME=1, which can be used to set a
   default value for the Module["noExitRuntime"] parameter at compile-time.
 - Improved SDL audio buffer queueing when the sample rate matches the native
   web audio graph sample rate.
 - Added an optimization that removes redundant Math.frounds in -O3.
 - Improved the default shell.html file.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.10.4...1.11.0

v1.10.4: 2/10/2014
------------------
 - Added support for legacy GL emulation in fastcomp.
 - Deprecated the --split-js compiler option. This is not supported in fastcomp.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.10.3...1.10.4

v1.10.3: 2/9/2014
------------------
 - Work on supporting GL/EGL GetProcAddress.
 - Fixed issues with shared lib linking support.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.10.2...1.10.3

v1.10.2: 2/7/2014
------------------
 - Added basic FS unmount support.
 - Improved screen orientation lock API to return a success code.
 - Added PRECISE_F32 support to fastcomp.
 - Fixed issues in fastcomp related to special floating point literal
   serialization.
 - Improved SDL audio buffer queueing.
 - Added new link-time option -s WARN_UNALIGNED=1 to fastcomp to report compiler
   warnings about generated unaligned memory accesses, which can hurt
   performance.
 - Optimized libc strcmp and memcmp with the implementations from musl libc.
 - Optimized libc memcpy and memset to back to native code for large buffer sizes.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.10.1...1.10.2

v1.10.1: 1/31/2014
------------------
 - Improve srand() and rand() to be seedable and use a Linear Congruential
   Generator (LCG) for the rng generation for performance.
 - Improved OpenAL library support.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.10.0...1.10.1

v1.10.0: 1/29/2014
------------------
 - Improved C++ exception handling.
 - Improved OpenAL library support.
 - Fixed an issue where loading side modules could try to allocate from sealed
   heap (#2060).
 - Fixed safe heap issues (2068).
 - Added new EM_ASM variants that return a value but do not receive any inputs
   (#2070).
 - Add support for simultaneously using setjmp and C++ exceptions in fastcomp.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.9.5...1.10.0

v1.9.5: 1/25/2014
------------------
 - Added a spinner logo to default html shell.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.9.4...1.9.5

v1.9.4: 1/24/2014
------------------
 - Add support for Ninja and Eclipse+Ninja builds with Emscripten+CMake.
 - Fixed regressions with GL emulation.
 - Added support for #if !X in .js library preprocessor.
 - Make the syntax EM_ASM("code"); not silently fail. Note that the proper form
   is EM_ASM(code); without double-quotes.
 - Optimize generated code size by minifying loop labels as well.
 - Revised the -O3 optimization level to mean "safe, but very slow optimizations
   on top of -O2", instead of the old meaning "unsafe optimizations". Using -O3
   will now only do safe optimizations, but can be very slow compared to -O2.
 - Implemented a new registerization optimization pass that does extra variable
   elimination in -O3 and later to reduce the number of local variables in
   functions.
 - Implemented a new emscripten/html5.h interface that exposes common HTML5 APIs
   directly to C code without having to handwrite JS wrappers.
 - Improved error messages reported on user-written .js libraries containing
   syntax errors (#2033).
 - Fixed glBufferData() function call signature with null data pointer.
 - Added new option Module['filePackagePrefixURL'] that allows customizing the
   URL where the VFS package is loaded from.
 - Implemented glGetTexEnviv and glGetTexEnvfv in GL emulation mode.
 - Optimized the size of large memory initializer sections.
 - Fixed issues with the safe heap compilation option.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.9.3...1.9.4

v1.9.3: 1/17/2014
------------------
 - re-merge split blocks in multiples
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.9.2...1.9.3

v1.9.2: 1/16/2014
------------------
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.9.1...1.9.2

v1.9.1: 1/16/2014
------------------
 - Optimize desktop GL fixed function pipeline emulation texture load
   instruction counts when GL_COMBINE is used.
 - fix Math_floor coercion in unrecommended codegen modes
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.9.0...1.9.1

v1.9.0: 1/16/2014
------------------
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.14...1.9.0

v1.8.14: 1/15/2014
------------------
 - add musl fputws and fix vswprintf.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.13...1.8.14

v1.8.13: 1/15/2014
------------------
 - remove musl use of fwritex
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.12...1.8.13

v1.8.12: 1/15/2014
------------------
 - Added new GLEW 1.10.0 emulation support.
 - Fixed an issue where the runtime could start more than once when run in a
   browser (#1992)
 - Fix a regression in wprintf.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.11...1.8.12

v1.8.11: 1/15/2014
------------------
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.10...1.8.11

v1.8.10: 1/14/2014
------------------
 - Update libc implementation from musl libc.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.9...1.8.10

v1.8.9: 1/14/2014
------------------
 - add fputwc, which enables wprintf.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.8...1.8.9

v1.8.8: 1/14/2014
------------------
 - Update to latest libcxx and libcxxabi libraries.
 - Fix handling of floating point negative zero (#1898)
 - Fixed a memory leak in relooper in previous release.
 - Fixed an issue in previous release with VBO handling in GL optimizations.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.7...1.8.8

v1.8.7: 1/13/2014
------------------
 - Added support to numpad keycodes in glut support library.
 - Fix SIMD support with fastcomp.
 - Fixed a compiler error 'ran out of names' that could occur with too many
   minified symbol names.
 - Work around webkit imul bug https://bugs.webkit.org/show_bug.cgi?id=126345
   (#1991)
 - Optimized desktop GL fixed function pipeline emulation path for better
   performance.
 - Added support for exceptions when building with fastcomp.
 - Fix and issue where the run() function could be called multiple times at
   startup (#1992)
 - Removed a relooper limitation with fixed buffer size.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.6...1.8.7

v1.8.6: 1/8/2014
------------------
 - Added support for the libuuid library, see http://linux.die.net/man/3/libuuid.
 - Fixed .js file preprocessor to preprocess recursively (#1984).
 - Fixed a compiler codegen issue related to overflow arithmetic (#1975)
 - Added new link-time optimization flag -s AGGRESSIVE_VARIABLE_ELIMINATION=1
   that enables the aggressiveVariableElimination js optimizer pass, which tries
   to remove temporary variables in generated JS code at the expense of code
   size.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.5...1.8.6

v1.8.5: 1/7/2014
------------------
 - Fixed compiler issues when used with LLVM 3.4.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.4...1.8.5

v1.8.4: 1/6/2014
------------------
 - Added support to Return and Backspace keys to glut
 - Fixed compiler issues when used with LLVM 3.4.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.3...1.8.4

v1.8.3: 1/5/2014
------------------
 - Improved SDL and page scroll pos handling support for IE10 and IE11.
 - Optimized SDL_UnlockSurface performance.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.2...1.8.3

v1.8.2: 1/4/2014
------------------
 - Fixed glGetFramebufferAttachmentParameteriv and an issue with glGetXXX when
   the returned value was null.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.1...1.8.2

v1.8.1: 1/3/2014
------------------
 - Added support for WebGL hardware instancing extension.
 - Improved fastcomp native LLVM backend support.
 - Added support for #include filename.js to JS libraries.
 - Deprecated --compression emcc command line parameter that manually compressed
   output JS files, due to performance issues. Instead, it is best to rely on
   the web server to serve compressed JS files.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.8.0...1.8.1

v1.8.0: 12/28/2013
------------------
 - Fix two issues with function outliner and relooper.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.9...1.8.0

v1.7.9: 12/27/2013
------------------
 - Added new command line parameter --em-config that allows specifying a custom
   location for the .emscripten configuration file.
 - Reintroduced relaxed asm.js heap sizes, which no longer need to be power of
   2, but a multiple of 16MB is sufficient.
 - Added emrun command line tool that allows launching .html pages from command
   line on desktop and Android as if they were native applications. See
   https://groups.google.com/forum/#!topic/emscripten-discuss/t2juu3q1H8E . Adds
   --emrun compiler link flag.
 - Began initial work on the "fastcomp" compiler toolchain, a rewrite of the
   previous JS LLVM AST parsing and codegen via a native LLVM backend.
 - Added --exclude-file command line flag to emcc and a matching --exclude
   command line flag to file packager, which allows specifying files and
   directories that should be excluded while packaging a VFS data blob.
 - Improved GLES2 and EGL support libraries to be more spec-conformant.
 - Optimized legacy GL emulation code path. Added new GL_FFP_ONLY optimization
   path to fixed function pipeline emulation.
 - Added new core functions emscripten_log() and emscripten_get_callstack() that
   allow printing out log messages with demangled and source-mapped callstack
   information.
 - Improved BSD Sockets support. Implemented getprotobyname() for BSD Sockets library.
 - Fixed issues with simd support.
 - Various bugfixes: #1573, #1846, #1886, #1908, #1918, #1930, #1931, #1942, #1948, ..
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.8...1.7.9

v1.7.8: 11/19/2013
------------------
 - Fixed an issue with -MMD compilation parameter.
 - Added EM_ASM_INT() and EM_ASM_DOUBLE() macros. For more information, read
   https://groups.google.com/forum/#!topic/emscripten-discuss/BFGTJPCgO6Y .
 - Fixed --split parameter to also work on Windows.
 - Fixed issues with BSD sockets accept() call.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.7...1.7.8

v1.7.7: 11/16/2013
------------------
 - Improve SDL audio buffer queue timing support.
 - Improved default precision of clock_gettime even when not using CLOCK_REALTIME.
 - Optimize and fix issues with LLVM IR processing.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.6...1.7.7

v1.7.6: 11/15/2013
------------------
 - Added regex implementation from musl libc.
 - The command line parameter -s DEAD_FUNCTIONS=[] can now be used to explicitly
   kill functions coming from built-in library_xx.js.
 - Improved EGL support and GLES2 spec conformance.
 - Reverted -s TOTAL_MEMORY=x to require pow2 values, instead of the relaxed
   'multiples of 16MB'. This is because the relaxed rule is released only in
   Firefox 26 which is currently in Beta and ships on the week of December 10th
   (currently in Beta). As of writing, current stable Firefox 25 does not yet
   support these.
 - Adjusted the default linker behavior to warn about all missing symbols,
   instead of silently ignoring them. Use -s WARN_ON_UNDEFINED_SYMBOLS=0 to
   suppress these warnings if necessary.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.5...1.7.6

v1.7.5: 11/13/2013
------------------
 - Fix issues with the built-in C++ function name demangler.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.4...1.7.5

v1.7.4: 11/12/2013
------------------
 - Fixed issues with BSD sockets code and SDL joystick implementation.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.3...1.7.4

v1.7.3: 11/12/2013
------------------
 - Added support for generating single-precision floating point instructions.
    - For more information, read
      https://blog.mozilla.org/javascript/2013/11/07/efficient-float32-arithmetic-in-javascript/
 - Made GLES2 support library more spec-conformant by throwing fewer exceptions
   on errors. Be sure to build with -s GL_ASSERTIONS=1, remember to use
   glGetError() and check the browser console to best detect WebGL rendering
   errors.
 - Converted return value of emscripten_get_now() from float to double, to not
   lose precision in the function call.
 - Added support for joysticks in SDL via the Gamepad API
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.2...1.7.3

v1.7.2: 11/9/2013
------------------
 - The compiler now always generates a .js file that contains the generated
   source code even when compiling to a .html file.
    - Read https://groups.google.com/forum/#!topic/emscripten-discuss/EuHMwqdSsEs
 - Implemented depth+stencil buffer choosing behavior in GLUT, SDL and GLFW.
 - Fixed memory leaks generated by glGetString and eglGetString.
 - Greatly optimized startup times when virtual filesystems with a large amount
   of files in them.
 - Added some support for SIMD generated by LLVM.
 - Fixed some mappings with SDL keyboard codes.
 - Added a new command line parameter --no-heap-copy to compiler and file
   packager that can be used to optimize VFS memory usage at startup.
 - Updated libcxx to revision 194185, 2013-11-07.
 - Improvements to various library support.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.1...1.7.2

v1.7.1: 10/24/2013
------------------
 - Remove old call to Runtime.warn in file packager code
 - Fix bug with parsing of empty types.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.7.0...1.7.1

v1.7.0: 10/23/2013
------------------
 - Adds mouse wheel events support in GLUT library.
 - Adds support for a new link parameter -s CASE_INSENSITIVE_VFS=1 to enable
   Emscripten virtual filesystem to search files ignoring case.
 - *Numerous* optimizations in both compilation and runtime stages.
 - Remove unnecessary whitespace, compact postSets function, and other
   optimizations in compilation output to save on generated file size.
 - Fixes float parsing from negative zero.
 - Removes the -s EMIT_GENERATED_FUNCTIONS link parameter as unneeded.
 - Fixes an issue where updating subranges of GL uniform arrays was not
   possible.
 - asm.js heap size (-s TOTAL_MEMORY=x) no longer needs to be a power of 2. As a
   relaxed rule, choosing any multiple of 16MB is now possible.
 - O1 optimization no longer runs the 'simplifyExpressions' optimization pass.
   This is to improve build iteration times when using -O1. Use -O2 to run that
   pass.
 - EM_ASM() can now be used even when compiling to asm.js.
 - All currently specified non-debugging-related WebGL 1 extensions are now
   enabled by default on startup, no need to ctx.getExtension() manually to
   enable them.
 - Improve readability of uncaught JavaScript exceptions that are thrown all the
   way up to the web console by printing out the stack trace of where the throw
   occurred.
 - Fix an issue when renaming a directory to a subdirectory.
 - Several compiler stability fixes.
 - Adds a JavaScript implementation of cxa_demangle function for demangling call
   stack traces at runtime for easier debugging.
 - GL context MSAA antialiasing is now DISABLED by default, to make the GL
   behavior consistent with desktop usage.
 - Added support to SDL, GLUT and GLFW libraries to specify MSAA on/off at startup.
 - Implemented glColor4ubv in GL emulation mode.
 - Fix an issue with LLVM keyword __attribute__ ((__constructor__)) (#1155).
 - Fix an issue with va_args and -s UNALIGNED_MEMORY=1 (#1705).
 - Add initial support code for LLVM SIMD constructs and a JavaScript SIMD
   polyfill implementation from
   https://github.com/johnmccutchan/ecmascript_simd/ .
 - Fixed support for node.js native filesystem API NODEFS on Windows.
 - Optimize application startup times of Emscripten-compiled programs by
   enabling the virtual filesystem XHR and asm.js compilation to proceed in
   parallel when opening a page.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.6.4...1.7.0

v1.6.4: 9/30/2013
------------------
 - Implements a new preprocessor tool for preparsing C struct definitions
   (#1554), useful for Emscripten support library implementors.
 - Fix parsing issue with sscanf (#1668).
 - Improved the responsiveness of compiler print output on Windows.
 - Improved compilation times at link stage.
 - Added support for new "NODEFS" filesystem that directly accesses files on the
   native filesystem. Only usable with node.js when compiling to JS.
 - Added support for new IDBFS filesystem for accessing files in IndexedDB storage (#1601.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.6.3...1.6.4

v1.6.3: 9/26/2013
------------------
 - Emscripten CMake toolchain now generates archive files with .a suffix when
   project target type is static library, instead of generatic .bc files
   (#1648).
 - Adds iconv library from the musl project to implement wide functions in C
   library (#1670).
 - Full list of changes:
   https://github.com/emscripten-core/emscripten/compare/1.6.2...1.6.3

v1.6.2: 9/25/2013
------------------
 - Added support for dprintf() function (#1250).
 - Fixes several compiler stability issues (#1637, #1166, #1661, #1651 and more).
 - Enables support for WEBGL_depth_texture.
 - Adds support for new link flag -s GL_ASSERTIONS=1 which can be used to add
   extra validation layer to the Emscripten GL library to catch code issues.
 - Adds support to Web Audio API in SDL audio backend so that SDL audio now
   works in Chrome and new Opera as well.
 - Fixes an alpha blending issue with SDL_SetAlpha.
 - Implemented locale-related code in C library.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.6.1...1.6.2

v1.6.1: 9/22/2013
------------------
 - Several optimizations to compiler link stage.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.6.0...1.6.1

v1.6.0: 9/21/2013
------------------
 - Enable support for %[] pattern in scanf.
 - Added dependency tracking support to linked .js files in CMake toolchain.
 - The hex prefix 0x is now properly handled in sscanf (#1632).
 - Simplify internal compiler operations by removing the internal framework.js.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.9...1.6.0

v1.5.9: 9/15/2013
------------------
 - Add support for SDL_Delay in web workers.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.8...1.5.9

v1.5.8: 9/14/2013
------------------
 - Add support for the GCC -E compiler flag.
 - Update Emscripten libc headers to musl-0.9.13.
 - Added new utility function emscripten_async_load_script() to asynchronously
   load a new .js script URL.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.7...1.5.8

v1.5.7: 8/30/2013
------------------
 - The script tag in default shell.html is now marked 'async', which enables
   loading the JS script code asynchronously in Firefox without making the main
   thread unresponsive.
 - Implemented new utility function emscripten_get_canvas_size() which returns
   the current Module <canvas> element size in pixels.
 - Optimize code size in compiled side modules.
 - Optimize startup memory usage by avoiding unnecessary copying of VFS data at
   startup.
 - Add support for SDL_WM_ToggleFullScreen().
 - Add support for emscripten_get_now() when running in SpiderMonkey shell.
 - Added new environment variable EM_BUILD_VERBOSE=0,1,2,3 to set an extra
   compiler output verbosity level for debugging.
 - Added better support for dlopen() to simulate dynamic library loading in
   JavaScript.
 - Improved support for BSD sockets and networking.
 - Added new SOCKFS filesystem, which reads files via a network connection.
 - Avoid issues with long command line limitations in CMake toolchain by using
   response files.
 - Fix issues with client-side vertex data rendering in GL emulation mode.
 - Improved precision of clock_gettime().
 - Improve function outlining support.
 - Added support for using NMake generator with CMake toolchain.
 - Improved support for flexible arrays in structs (#1602).
 - Added ability to marshal UTF16 and UTF32 strings between C++ <-> JS code.
 - Added a new commandline tool validate_asms.py to help automating asm.js
   validation testing.
 - Improved stability with inline asm() syntax.
 - Updated libc headers to new version.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.6...1.5.7

v1.5.6: 8/17/2013
------------------
 - Improved BSD sockets support.
 - Added touch events support to GLUT library.
 - Added new --js-opts=0/1 command line option to control whether JS optimizer
   is run or not.
 - Improved OpenAL support.
 - Added new command line tool tools/find_bigvars.py which can be used on an
   output file to detect large functions and needs for outlining.
 - Merged link flags -s FORCE_GL_EMULATION and -s DISABLE_GL_EMULATION to a
   single opt-in flag -s LEGACY_GL_EMULATION=0/1 to control whether GL emulation
   is active.
 - Improved SDL input support.
 - Several stability-related compiler fixes.
 - Fixed source mapping generation support on Windows.
 - Added back the EMSCRIPTEN_KEEPALIVE attribute qualifier to help prevent
   inlining and to retain symbols in output without dead code elimination
   occurring.
 - Fix issues when marshalling UTF8 strings between C<->JS.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.5...1.5.6

v1.5.5: 8/9/2013
------------------
 - Update libcxx to revision 187959, 2013-08-08.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.4...1.5.5

v1.5.4: 8/9/2013
------------------
 - Fixed multiple issues with C stdlib support.
 - Fix audio buffer queueing issues with OpenAL.
 - Improved BSD sockets support.
 - Added a new compile+link time command line option -Wno-warn-absolute-paths to
   hide the emscripten compiler warning when absolute paths are passed into the
   compiler.
 - Added new link flag -s STB_IMAGE=0/1 and integrate it to SDL image loading to
   enable synchronous image loading support with SDL.
 - Several improvements on function outlining support.
 - Fix issues with GLES2 interop support.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.3...1.5.4

v1.5.3: 6/28/2013
------------------
 - Added new optimization level --llvm-lto 3 to run even more aggressive LTO
   optimizations.
 - Improve optimizations for libc and other libraries.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.2...1.5.3

v1.5.2: 6/27/2013
------------------
 - Added support for generating source maps along the built application when -g
   is specified. This lets the browser show original .cpp sources when
   debugging.
 - GLUT and SDL improvements.
 - Added new link option -g<level> where level=0-4, which allows controlling
   various levels of debuggability added to the output.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.1...1.5.2

v1.5.1: 6/22/2013
------------------
 - File packager now skips all directories and files starting with '.', and hidden files on Windows.
 - Fix issues with strnlen, memmove, LDBL_ constants, va_args, float.h, and others.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.5.0...1.5.1

v1.5.0: 6/17/2013
------------------
 - Several compiler optimizations.
 - Improve SDL key events support.
 - Increase debug logging when specifying emcc -v.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.4.9...1.5.0

v1.4.9: 6/8/2013
------------------
 - Several compiler optimizations.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.4.8...1.4.9

v1.4.8: 6/6/2013
------------------
 - Add support for webrtc-based sockets.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.4.7...1.4.8

v1.4.7: 6/2/2013
------------------
 - Remove more unneeded break and continue statements in relooper.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.4.6...1.4.7

v1.4.6: 6/2/2013
------------------
 - Improve relooper code.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.4.5...1.4.6

v1.4.5: 6/1/2013
------------------
 - Improve relooper code.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.4.4...1.4.5

v1.4.4: 6/1/2013
------------------
 - Add support for symlinks in source files.
 - Fix various issues with SDL.
 - Added -s FORCE_ALIGNED_MEMORY=0/1 link time flag to control whether all loads
   and stores are assumed to be aligned.
 - Fix file packager to work with closure.
 - Major improvements to embind support, and optimizations.
 - Improve GL emulation.
 - Optimize VFS usage.
 - Allow emscripten to compile .m and .mm files.
 - Added new syntax --preload-file src@dst to file packager command line to
   allow placing source files to custom destinations in the FS.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.4.3...1.4.4

v1.4.3: 5/8/2013
------------------
 - Fix issue with strcat.
 - Major embind improvements.
 - Switch to le32-unknown-nacl LLVM target triple as default build option
   instead of the old i386-pc-linux-gnu target triple.
 - Improve compiler logging behavior.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.4.2...1.4.3

v1.4.2: 5/3/2013
------------------
 - Fix issues with le32-unknown-nacl LLVM target triple.
 - Add some GLEW support.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.4.1...1.4.2

v1.4.1: 4/28/2013
------------------
 - Implement support for le32-unknown-nacl LLVM target triple.
 - Added new cmdline option -s ERROR_ON_UNDEFINED_SYMBOLS=0/1 to give
   compile-time error on undefined symbols at link time. Default off.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.3.8...1.4.1

v1.3.8: 4/29/2013
------------------
 - Improved 64-bit integer ops codegen.
 - Added Indexed DB support to vfs.
 - Improve warning message on dangerous function pointer casts when compiling in
   asm.js mode.
 - Added --use-preload-cache command line option to emcc, to be used with the
   file packager.
 - Fixes to libcextra.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.3.7...1.3.8

v1.3.7: 4/24/2013
------------------
 - Merge IMVU implementation of embind to emscripten trunk. Embind allows
   high-level C++ <-> JS types interop.
 - Enable asm.js compilation in -O1 and higher by default. Fix issues when
   compiling to asm.js.
 - Improve libc support with Emscripten with the musl libc headers.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.3.6...1.3.7

v1.3.6: 4/2/2013
------------------
 - Fix hang issue with strtof.
 - Update libcxx to upstream r178253 from March 29, 2013.
 - Fix issues with GL emulation.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.3.5...1.3.6

v1.3.5: 3/25/2013
------------------
 - Get exceptions working as they did before.
 - Remove symbol removing hack.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.3.4...1.3.5

v1.3.4: 3/24/2013
------------------
 - Update to new libcxx and libcxxabi versions from upstream.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.3.3...1.3.4

v1.3.3: 3/23/2013
------------------
 - Remove unneeded check from relooper.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.3.2...1.3.3

v1.3.2: 3/22/2013
------------------
 - Fix issues with fgets.
 - Add support for non-fullscreen pointer lock.
 - Improve OpenAL support.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.3.1...1.3.2

v1.3.1: 3/19/2013
------------------
 - Improve SDL audio and mixer support.
 - Add GLES2 emulation features when -s FULL_ES2=1 is specified.
 - Add support for OpenAL.
 - Add new -s OPENAL_DEBUG=0/1 link command line option.
 - Fixed an issue with mouse coordinate being offset with canvas.
 - Removed -s UTF_STRING_SUPPORT=0 parameter, this is now always on.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.3.0...1.3.1

v1.3.0: 3/11/2013
------------------
 - Improve GLES2 emulation with -s FULL_ES2=1.
 - Deprecated -s USE_TYPED_ARRAYS=1 and -s QUANTUM_SIZE=1.
 - Implement a minifier infrastructure when compiling for asm.js.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.9...1.3.0

v1.2.9: 3/7/2013
------------------
 - Improved canvas behavior when transitioning between fullscreen.
 - Added support for getopt().
 - Fixed several libc issues.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.8...1.2.9

v1.2.8: 3/6/2013
------------------
 - Remove unnecessary recursion in relooper RemoveUnneededFlows.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.7...1.2.8

v1.2.7: 3/6/2013
------------------
 - Added SDL_Mixer support.
 - Implemented stubs for several Unix and threading-related functions.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.6...1.2.7

v1.2.6: 3/5/2013
------------------
 - Relooper updates.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.5...1.2.6

v1.2.5: 3/5/2013
------------------
 - Greatly improve GL emulation support.
 - Handle %c in sscanf.
 - Improve compilation times by optimizing parallel execution in the linker.
 - Improve several compiler stability issues detected from fuzzing tests.
 - Implemented emscripten_jcache_printf.
 - Allow running emscripten.py outside emcc itself.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.4...1.2.5

v1.2.4: 2/2/2013
------------------
 - Work on adding support for asm.js compilation.
 - Improve EGL support.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.3...1.2.4

v1.2.3: 1/9/2013
------------------
 - Work on adding support for asm.js compilation.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.2...1.2.3

v1.2.2: 1/8/2013
------------------
 - Work on adding support for asm.js compilation.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.1...1.2.2

v1.2.1: 1/8/2013
------------------
 - Improvements to GLUT, SDL and BSD sockets headers.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.2.0...1.2.1

v1.2.0: 1/1/2013
------------------
 - Work on adding support for asm.js compilation.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.1.0...1.2.0

v1.1.0: 12/12/2012
------------------
 - Fix several issues with Windows support.
 - Added a standalone toolchain for CMake.
 - Added emscripten_run_script_string().
 - Optimize compilation times via threading.
 - Update to requiring Clang 3.2. Older versions may no longer work.
 - Several improvements to emscripten library support headers.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.0.1a...1.1.0

v1.0.1a: 11/11/2012
------------------
 - Add relooper code to repository.
 - Full list of changes: https://github.com/emscripten-core/emscripten/compare/1.0.1...1.0.1a

v1.0.1: 11/11/2012
------------------
 - First commit that introduced versioning to the Emscripten compiler.
