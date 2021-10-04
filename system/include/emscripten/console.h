/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifdef __cplusplus
extern "C" {
#endif

void emscripten_console_log(const char *utf8String);
void emscripten_console_warn(const char *utf8String);
void emscripten_console_error(const char *utf8String);

#ifdef __cplusplus
}
#endif
