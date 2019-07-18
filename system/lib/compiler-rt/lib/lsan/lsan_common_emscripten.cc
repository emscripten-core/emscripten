//=-- lsan_common_emscripten.cc--------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of LeakSanitizer.
// Implementation of common leak checking functionality.
// Emscripten-specific code.
//
//===----------------------------------------------------------------------===//

#include "sanitizer_common/sanitizer_platform.h"
#include "lsan_common.h"

#if CAN_SANITIZE_LEAKS && SANITIZER_EMSCRIPTEN
#include <link.h>

#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_getauxval.h"
#include "sanitizer_common/sanitizer_linux.h"
#include "sanitizer_common/sanitizer_stackdepot.h"


namespace __lsan {

extern "C" uptr emscripten_get_heap_size();

static const char kLinkerName[] = "ld";

static char linker_placeholder[sizeof(LoadedModule)] ALIGNED(64);
static LoadedModule *linker = nullptr;

static bool IsLinker(const LoadedModule& module) {
  return false;
}

// TODO: make thread local storage actually work
/*__attribute__((tls_model("initial-exec")))
THREADLOCAL*/ int disable_counter;
bool DisabledInThisThread() { return disable_counter > 0; }
void DisableInThisThread() { disable_counter++; }
void EnableInThisThread() {
  if (disable_counter == 0) {
    DisableCounterUnderflow();
  }
  disable_counter--;
}

void InitializePlatformSpecificModules() {
  ListOfModules modules;
  modules.init();
  for (LoadedModule &module : modules) {
    if (!IsLinker(module))
      continue;
    if (linker == nullptr) {
      linker = reinterpret_cast<LoadedModule *>(linker_placeholder);
      *linker = module;
      module = LoadedModule();
    } else {
      VReport(1, "LeakSanitizer: Multiple modules match \"%s\". "
              "TLS and other allocations originating from linker might be "
              "falsely reported as leaks.\n", kLinkerName);
      linker->clear();
      linker = nullptr;
      return;
    }
  }
  if (linker == nullptr) {
    VReport(1, "LeakSanitizer: Dynamic linker not found. TLS and other "
               "allocations originating from linker might be falsely reported "
                "as leaks.\n");
  }
}

extern "C" {
  extern int __data_end;
  extern int __heap_base;
}

// Scans global variables for heap pointers.
void ProcessGlobalRegions(Frontier *frontier) {
  if (!flags()->use_globals) return;
  ScanGlobalRange(0, (uptr) &__data_end, frontier);
}

LoadedModule *GetLinker() { return linker; }

void ProcessPlatformSpecificAllocations(Frontier *frontier) {}

// While calling Die() here is undefined behavior and can potentially
// cause race conditions, it isn't possible to intercept exit on Emscripten,
// so we have no choice but to call Die() from the atexit handler.
void HandleLeaks() {
  if (common_flags()->exitcode) Die();
}

void DoStopTheWorld(StopTheWorldCallback callback, void *argument) {
  // Currently, on Emscripten this does nothing and just calls the callback.
  // This works fine on a single-threaded environment.
  StopTheWorld(callback, argument);
}

void ProcessThreads(SuspendedThreadsList const &suspended_threads,
                    Frontier *frontier) {
  uptr sp = (uptr) __builtin_frame_address(0);
  ScanRangeForPointers(sp, (uptr) &__heap_base, frontier, "STACK", kReachable);
}

} // namespace __lsan

#endif // CAN_SANITIZE_LEAKS && SANITIZER_EMSCRIPTEN
