//===-- sanitizer_emscripten.cc -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This provides implementations of some functions in sanitizer_linux_libcdep.c
// on emscripten. We are not using sanitizer_linux_libcdep.c because it contains
// a lot of threading and other code that does not work with emscripten yet,
// so instead, some minimal implementations are provided here so that UBSan can
// work.
//===----------------------------------------------------------------------===//

#include "sanitizer_platform.h"
#include "sanitizer_common.h"

#include <signal.h>

#if SANITIZER_EMSCRIPTEN

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

extern "C" uptr emscripten_get_stack_top();
extern "C" uptr emscripten_get_stack_base();

void GetThreadStackTopAndBottom(bool at_initialization, uptr *stack_top,
                                uptr *stack_bottom) {
  *stack_top = emscripten_get_stack_top();
  *stack_bottom = emscripten_get_stack_base();
}

char *fake_argv[] = {0};
char *fake_envp[] = {0};

char **GetArgv() {
  return fake_argv;
}

char **GetEnviron() {
  return fake_envp;
}

} // namespace __sanitizer

#endif
