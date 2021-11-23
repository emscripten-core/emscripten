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

// Similar to the above functions but operate with printf-like semantics.
void emscripten_console_logf(const char *utf8String, ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_console_warnf(const char *utf8String, ...) __attribute__((__format__(printf, 1, 2)));
void emscripten_console_errorf(const char *utf8String, ...)__attribute__((__format__(printf, 1, 2)));

#ifdef __cplusplus
}
#endif
