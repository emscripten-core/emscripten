//===-- sanitizer_emscripten.cc -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This provides implementations of some functions on emscripten.
//===----------------------------------------------------------------------===//

#include "sanitizer_platform.h"
#include "sanitizer_common.h"

#include <signal.h>

namespace __sanitizer {

void ListOfModules::init() {}
void ListOfModules::fallbackInit() { clear(); }

SANITIZER_WEAK_ATTRIBUTE int
real_sigaction(int signum, const void *act, void *oldact);

int internal_sigaction(int signum, const void *act, void *oldact) {
#if !SANITIZER_GO
  if (&real_sigaction)
    return real_sigaction(signum, act, oldact);
#endif
  return sigaction(signum, (const struct sigaction *)act,
                   (struct sigaction *)oldact);
}

void GetThreadStackTopAndBottom(bool at_initialization, uptr *stack_top,
                                uptr *stack_bottom) {
  Report("WARNING: Emscripten has no thread stack");
  *stack_top = 0;
  *stack_bottom = 0;
}

} // namespace __sanitizer
