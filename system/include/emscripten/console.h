/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stddef.h>  // for size_t

#ifdef __cplusplus
extern "C" {
#endif

// Write directly to the JavaScript console.  This can be useful for debugging since it
// bypasses the stdio and filesystem sub-systems.
void emscripten_console_log(const char * _Nonnull utf8String);
void emscripten_console_warn(const char * _Nonnull utf8String);
void emscripten_console_error(const char * _Nonnull utf8String);
void emscripten_console_trace(const char * _Nonnull utf8String);

// Write to the out(), err() and dbg() JS functions directly.
// These are defined in shell.js and have different behavior compared
// to console.log/err.  Under node, they write to stdout and stderr which is a
// more direct way to write output especially from worker threads.  The default
// behavior of these functions can be overridden by print and printErr, if
// provided on the Module object.  These functions are mainly intended for
// internal use.
void emscripten_out(const char * _Nonnull utf8String);
void emscripten_err(const char * _Nonnull utf8String);
void emscripten_dbg(const char * _Nonnull utf8String);
void emscripten_dbg_backtrace(const char * _Nonnull utf8String);

// Same as above but only with the length of string specified by the second
// argument.  This allows for non-NULL-terminated strings to be passed.
void emscripten_outn(const char * _Nonnull utf8String, size_t len);
void emscripten_errn(const char * _Nonnull utf8String, size_t len);
void emscripten_dbgn(const char * _Nonnull utf8String, size_t len);

// Legacy/internal names for the above
#define _emscripten_out(x) emscripten_out(x)
#define _emscripten_err(x) emscripten_err(x)
#define _emscripten_dbg(x) emscripten_dbg(x)

// Similar to the above functions but operate with printf-like semantics.
void emscripten_console_logf(const char * _Nonnull format, ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_console_warnf(const char * _Nonnull format, ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_console_errorf(const char * _Nonnull format, ...)__attribute__((__format__(printf, 1, 2)));
void emscripten_console_tracef(const char * _Nonnull format, ...)__attribute__((__format__(printf, 1, 2)));
void emscripten_outf(const char * _Nonnull format, ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_errf(const char * _Nonnull format, ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_dbgf(const char * _Nonnull format, ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_dbg_backtracef(const char * _Nonnull format, ...) __attribute__((__format__(printf, 1, 2)));

// Legacy/internal names for the above
#define _emscripten_outf(format, ...) emscripten_outf(format, ##__VA_ARGS__)
#define _emscripten_errf(format, ...) emscripten_errf(format, ##__VA_ARGS__)
#define _emscripten_dbgf(format, ...) emscripten_dbgf(format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
