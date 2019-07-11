/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * C++ exception handling support stubs. This is included when exception
 * throwing is disabled - so no exceptions should exist at all. If the code still
 * uses them, these stubs will throw at runtime.
 */

var LibraryExceptions = {};

[
  '__cxa_allocate_exception',
  '__cxa_free_exception',
  '__cxa_increment_exception_refcount',
  '__cxa_decrement_exception_refcount',
  '__cxa_throw',
  '__cxa_rethrow',
  'llvm_eh_exception',
  'llvm_eh_selector',
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
].forEach(function(name) {
  LibraryExceptions[name] = function() { abort(); };
  LibraryExceptions[name + '__deps'] = [function() {
    error('DISABLE_EXCEPTION_THROWING was set (likely due to -fno-exceptions), which means no C++ exception throwing support code is linked in, but such support is required by symbol ' + name + '. Either do not set DISABLE_EXCEPTION_THROWING (if you do want exception throwing) or compile all source files with -fno-except (so that no exceptions support code is required); also make sure DISABLE_EXCEPTION_CATCHING is set to the right value - if you want exceptions, it should be off, and vice versa.');
  }];
});

mergeInto(LibraryManager.library, LibraryExceptions);
