//===-- sanitizer_common_nolibc.cc ----------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains stubs for libc function to facilitate optional use of
// libc in no-libcdep sources.
//===----------------------------------------------------------------------===//

#include "sanitizer_platform.h"
#include "sanitizer_common.h"
#include "sanitizer_libc.h"

namespace __sanitizer {

// The Windows implementations of these functions use the win32 API directly,
// bypassing libc.
#if !SANITIZER_WINDOWS
#if SANITIZER_LINUX
void LogMessageOnPrintf(const char *str) {}
#endif
void WriteToSyslog(const char *buffer) {}

#if !SANITIZER_EMSCRIPTEN
void Abort() { internal__exit(1); }
void SleepForSeconds(int seconds) { internal_sleep(seconds); }
#endif // !SANITIZER_EMSCRIPTEN
#endif // !SANITIZER_WINDOWS

#if !SANITIZER_WINDOWS && !SANITIZER_MAC
void ListOfModules::init() {}
#endif

#if SANITIZER_EMSCRIPTEN
void ListOfModules::fallbackInit() { clear(); }

#include <signal.h>

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
#endif // SANITIZER_EMSCRIPTEN

}  // namespace __sanitizer
