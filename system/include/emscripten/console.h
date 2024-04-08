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

// Write directly JavaScript console.  This can be useful for debugging since it
// bypasses the stdio and filesystem sub-systems.
void emscripten_console_log(const char *utf8String __attribute__((nonnull)));
void emscripten_console_warn(const char *utf8String __attribute__((nonnull)));
void emscripten_console_error(const char *utf8String __attribute__((nonnull)));

// Write to the out(), err() and dbg() JS functions directly.
// These are defined an defined in shell.js and have different behavior compared
// to console.log/err.  Under node, they write to stdout and stderr which is a
// more direct way to write output especially from worker threads.  The default
// behavior of these functions can be overridden by print and printErr, if
// provided on the Module object.  These functions are mainly intended for
// internal use.
void emscripten_out(const char *utf8String __attribute__((nonnull)));
void emscripten_err(const char *utf8String __attribute__((nonnull)));
void emscripten_dbg(const char *utf8String __attribute__((nonnull)));

// Same as above but only with the length of string specified by the second
// argument.  This allows for non-NULL-terminated strings to be passed.
void emscripten_outn(const char *utf8String __attribute__((nonnull)), size_t len);
void emscripten_errn(const char *utf8String __attribute__((nonnull)), size_t len);
void emscripten_dbgn(const char *utf8String __attribute__((nonnull)), size_t len);

// Legacy/internal names for the above
#define _emscripten_out(x) emscripten_out(x)
#define _emscripten_err(x) emscripten_err(x)
#define _emscripten_dbg(x) emscripten_dbg(x)

// Similar to the above functions but operate with printf-like semantics.
void emscripten_console_logf(const char *format __attribute__((nonnull)), ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_console_warnf(const char *format __attribute__((nonnull)), ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_console_errorf(const char *format __attribute__((nonnull)), ...)__attribute__((__format__(printf, 1, 2)));
void emscripten_outf(const char *format __attribute__((nonnull)), ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_errf(const char *format __attribute__((nonnull)), ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_dbgf(const char *format __attribute__((nonnull)), ...) __attribute__((__format__(printf, 1, 2)));

// Legacy/internal names for the above
#define _emscripten_outf(format, ...) emscripten_outf(format, ##__VA_ARGS__)
#define _emscripten_errf(format, ...) emscripten_errf(format, ##__VA_ARGS__)
#define _emscripten_dbgf(format, ...) emscripten_dbgf(format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
