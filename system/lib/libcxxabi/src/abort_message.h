//===-------------------------- abort_message.h-----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef __ABORT_MESSAGE_H_
#define __ABORT_MESSAGE_H_

#include "cxxabi.h"

#ifdef __EMSCRIPTEN__

// Avoid pulling in ::abort_message and associated string formatting code
#include <emscripten.h>
#include <stdlib.h>
#define abort_message(...) do { \
  emscripten_log(EM_LOG_ERROR, ##__VA_ARGS__); \
  abort(); \
} while(0);

#else

#ifdef __cplusplus
extern "C" {
#endif

_LIBCXXABI_HIDDEN _LIBCXXABI_NORETURN void
abort_message(const char *format, ...) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif

#endif // __EMSCRIPTEN__
#endif

