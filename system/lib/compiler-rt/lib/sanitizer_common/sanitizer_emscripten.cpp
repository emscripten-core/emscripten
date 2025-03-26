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
#include "sanitizer_stoptheworld.h"

#include <signal.h>
#include <time.h>

#if SANITIZER_EMSCRIPTEN

#include <emscripten.h>
#include <emscripten/stack.h>
#include <sys/types.h>

#include "emscripten_internal.h"

namespace __sanitizer {

void ListOfModules::init() {
  modules_.Initialize(2);

  char name[256];
  _emscripten_get_progname(name, 256);

  LoadedModule main_module;
  main_module.set(name, 0);

  // Emscripten represents program counters as offsets into WebAssembly
  // modules. For JavaScript code, the "program counter" is the line number
  // of the JavaScript code with the high bit set.
  // Therefore, PC values 0x80000000 and beyond represents JavaScript code.
  // As a result, 0x00000000 to 0x7FFFFFFF represents PC values for WASM code.
  // We consider WASM code as main_module.
  main_module.addAddressRange(0, 0x7FFFFFFF, /*executable*/ true,
                              /*writable*/ false);
  modules_.push_back(main_module);

  // The remaining PC values, 0x80000000 to 0xFFFFFFFF, are JavaScript,
  // and we consider it a separate module, js_module.
  LoadedModule js_module;
  js_module.set("JavaScript", 0x80000000);
  js_module.addAddressRange(0x80000000, 0xFFFFFFFF, /*executable*/ true,
                            /*writable*/ false);
  modules_.push_back(js_module);
}

void ListOfModules::fallbackInit() { clear(); }

int internal_sigaction(int signum, const void *act, void *oldact) {
  return sigaction(signum, (const struct sigaction *)act,
                   (struct sigaction *)oldact);
}

uptr internal_mmap(void *addr, uptr length, int prot, int flags, int fd,
                   u64 offset) {
  CHECK(IsAligned(offset, 4096));
  return (uptr)emscripten_builtin_mmap(addr, length, prot, flags, fd, offset / 4096);
}

uptr internal_munmap(void *addr, uptr length) {
  return emscripten_builtin_munmap(addr, length);
}

void GetThreadStackTopAndBottom(bool at_initialization, uptr *stack_top,
                                uptr *stack_bottom) {
  *stack_top = emscripten_stack_get_base();
  *stack_bottom = emscripten_stack_get_end();
}

char *fake_argv[] = {0};
char *fake_envp[] = {0};

char **GetArgv() {
  return fake_argv;
}

char **GetEnviron() {
  return fake_envp;
}

uptr GetTlsSize() {
  return 0;
}

void InitTlsSize() {}

void GetThreadStackAndTls(bool main, uptr *stk_addr, uptr *stk_size,
                          uptr *tls_addr, uptr *tls_size) {
  uptr stk_top;
  GetThreadStackTopAndBottom(true, &stk_top, stk_addr);
  *stk_size = stk_top - *stk_addr;
#ifdef __EMSCRIPTEN_PTHREADS__
  *tls_addr = (uptr) __builtin_wasm_tls_base();
  *tls_size = __builtin_wasm_tls_size();
#else
  *tls_addr = *tls_size = 0;
#endif
}

class SuspendedThreadsListEmscripten final : public SuspendedThreadsList {};

void StopTheWorld(StopTheWorldCallback callback, void *argument) {
  // TODO: have some workable alternative, since we can't just fork and suspend
  // the parent process. This does not matter when single thread.
  callback(SuspendedThreadsListEmscripten(), argument);
}

void InitializePlatformCommonFlags(CommonFlags *cf) {}

u64 MonotonicNanoTime() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (u64)ts.tv_sec * (1000ULL * 1000 * 1000) + ts.tv_nsec;
}

void GetMemoryProfile(fill_profile_f cb, uptr *stats) {}

} // namespace __sanitizer

#endif
