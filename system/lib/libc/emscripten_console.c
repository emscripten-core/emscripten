/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <alloca.h>
#include <stdarg.h>
#include <stdio.h>

#include <emscripten/console.h>

static void vlogf(const char* fmt, va_list ap, void (*callback)(const char*)) {
  va_list ap2;
  va_copy(ap2, ap);
  size_t len = vsnprintf(0, 0, fmt, ap2);
  va_end(ap2);
  char* buf = alloca(len + 1);
  vsnprintf(buf, len + 1, fmt, ap);
  callback(buf);
}

void emscripten_console_logf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vlogf(fmt, ap, &emscripten_console_log);
  va_end(ap);
}

void emscripten_console_errorf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vlogf(fmt, ap, &emscripten_console_error);
  va_end(ap);
}

void emscripten_console_warnf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vlogf(fmt, ap, &emscripten_console_warn);
  va_end(ap);
}

void emscripten_console_tracef(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vlogf(fmt, ap, &emscripten_console_trace);
  va_end(ap);
}

void emscripten_outf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vlogf(fmt, ap, &emscripten_out);
  va_end(ap);
}

void emscripten_errf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vlogf(fmt, ap, &emscripten_err);
  va_end(ap);
}

#ifndef NDEBUG
void emscripten_dbgf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vlogf(fmt, ap, &emscripten_dbg);
  va_end(ap);
}

void emscripten_dbg_backtracef(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vlogf(fmt, ap, &emscripten_dbg_backtrace);
  va_end(ap);
}
#endif
