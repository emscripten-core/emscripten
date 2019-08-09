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
#include <emscripten.h>

#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_getauxval.h"
#include "sanitizer_common/sanitizer_linux.h"
#include "sanitizer_common/sanitizer_stackdepot.h"
#include "sanitizer_common/sanitizer_thread_registry.h"

#define LOG_THREADS(...)                           \
  do {                                             \
    if (flags()->log_threads) Report(__VA_ARGS__); \
  } while (0)

namespace __lsan {

extern "C" uptr emscripten_get_heap_size();

static const char kLinkerName[] = "ld";

static char linker_placeholder[sizeof(LoadedModule)] ALIGNED(64);
static LoadedModule *linker = nullptr;

static bool IsLinker(const LoadedModule& module) {
  return false;
}

thread_local int disable_counter;
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
  extern char __global_base;
  extern char __data_end;
}

// Scans global variables for heap pointers.
void ProcessGlobalRegions(Frontier *frontier) {
  if (!flags()->use_globals) return;
  ScanGlobalRange((uptr) &__global_base, (uptr) &__data_end, frontier);
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

u32 GetCurrentThread();

// This is based on ProcessThreads in lsan_common.cc.
// We changed this to be a callback that gets called per thread by
// ThreadRegistry::RunCallbackForEachThreadLocked.
// We do not scan registers or DTLS since we do not have those.
// Finally, we can only obtain the stack pointer for the current thread,
// so we scan the full stack for other threads.
static void ProcessThreadsCallback(ThreadContextBase *tctx, void *arg) {
  if (tctx->status != ThreadStatusRunning)
    return;

  Frontier *frontier = reinterpret_cast<Frontier *>(arg);
  tid_t os_id = tctx->os_id;

  uptr stack_begin, stack_end, tls_begin, tls_end, cache_begin, cache_end;
  DTLS *dtls;
  bool thread_found = GetThreadRangesLocked(os_id, &stack_begin, &stack_end,
                                            &tls_begin, &tls_end,
                                            &cache_begin, &cache_end, &dtls);
  if (!thread_found) {
    LOG_THREADS("Thread %d not found in registry.\n", os_id);
    return;
  }

  if (flags()->use_stacks) {
    LOG_THREADS("Stack at %p-%p.\n", stack_begin, stack_end);

    // We can't get the SP for other threads to narrow down the range, but we
    // we can for the current thread.
    if (tctx->tid == GetCurrentThread()) {
      uptr sp = (uptr) __builtin_frame_address(0);
      CHECK(stack_begin <= sp && sp < stack_end);
      stack_begin = sp;
    }

    ScanRangeForPointers(stack_begin, stack_end, frontier, "STACK", kReachable);
  }

  if (flags()->use_tls && tls_begin) {
    LOG_THREADS("TLS at %p-%p.\n", tls_begin, tls_end);
    // If the tls and cache ranges don't overlap, scan full tls range,
    // otherwise, only scan the non-overlapping portions
    if (cache_begin == cache_end || tls_end < cache_begin ||
        tls_begin > cache_end) {
      ScanRangeForPointers(tls_begin, tls_end, frontier, "TLS", kReachable);
    } else {
      if (tls_begin < cache_begin)
        ScanRangeForPointers(tls_begin, cache_begin, frontier, "TLS",
                             kReachable);
      if (tls_end > cache_end)
        ScanRangeForPointers(cache_end, tls_end, frontier, "TLS", kReachable);
    }
  }
}

void ProcessThreads(SuspendedThreadsList const &suspended_threads,
                    Frontier *frontier) {
  GetThreadRegistryLocked()->RunCallbackForEachThreadLocked(
    ProcessThreadsCallback, frontier);
}

} // namespace __lsan

extern "C" void __lsan_disable_in_this_thread() {
  __lsan::DisableInThisThread();
}

extern "C" void __lsan_enable_in_this_thread() {
  __lsan::EnableInThisThread();
}
#endif // CAN_SANITIZE_LEAKS && SANITIZER_EMSCRIPTEN
