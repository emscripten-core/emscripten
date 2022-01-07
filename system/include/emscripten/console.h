/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifdef __cplusplus
extern "C" {
#endif

// Write directly JavaScript console.  This can be useful for debugging since it
// bypasses the stdio and filesystem sub-systems.
void emscripten_console_log(const char *utf8String);
void emscripten_console_warn(const char *utf8String);
void emscripten_console_error(const char *utf8String);

// Write to the out() and err() hooks directly (defined in shell.js).
// These have different behavior compared to console.log/err.
// Under node, they write to stdout and stderr which is a more direct way to
// write output especially in worker threads.
// The default behavior of these functions can be overridden by print and
// printErr, if provided on the Module object.
// These functions are mainly intended for internal use.
// See https://github.com/emscripten-core/emscripten/issues/14804
void _emscripten_out(const char *utf8String);
void _emscripten_err(const char *utf8String);

// Similar to the above functions but operate with printf-like semantics.
void emscripten_console_logf(const char *utf8String, ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_console_warnf(const char *utf8String, ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_console_errorf(const char *utf8String, ...)__attribute__((__format__(printf, 1, 2)));
void _emscripten_outf(const char *utf8String, ...) __attribute__((__format__(printf, 1, 2)));
void _emscripten_errf(const char *utf8String, ...) __attribute__((__format__(printf, 1, 2)));

#ifdef __cplusplus
}
#endif
