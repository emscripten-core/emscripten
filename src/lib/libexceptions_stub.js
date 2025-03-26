/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryExceptions = {};

[
  '__cxa_throw',
  '__cxa_rethrow',
  'llvm_eh_typeid_for',
  '__cxa_begin_catch',
  '__cxa_end_catch',
  '__cxa_get_exception_ptr',
  '_ZSt18uncaught_exceptionv',
  '__cxa_call_unexpected',
  '__cxa_current_primary_exception',
  '__cxa_rethrow_primary_exception',
  '__cxa_find_matching_catch',
  '__resumeException',
].forEach((name) => {
  LibraryExceptions[name] = function() { abort(); };
#if !INCLUDE_FULL_LIBRARY
  // This method of link-time error generation is not compatible with INCLUDE_FULL_LIBRARY
  LibraryExceptions[name + '__deps'] = [function() {
    error(`DISABLE_EXCEPTION_THROWING was set (likely due to -fno-exceptions), which means no C++ exception throwing support code is linked in, but such support is required by symbol '${name}'. Either do not set DISABLE_EXCEPTION_THROWING (if you do want exception throwing) or compile all source files with -fno-except (so that no exceptions support code is required); also make sure DISABLE_EXCEPTION_CATCHING is set to the right value - if you want exceptions, it should be off, and vice versa.`);
  }];
#endif
});

addToLibrary(LibraryExceptions);
