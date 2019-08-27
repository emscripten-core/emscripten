//=-- lsan.cc -------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of LeakSanitizer.
// Standalone LSan RTL.
//
//===----------------------------------------------------------------------===//

#include "lsan.h"

#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_flag_parser.h"
#include "sanitizer_common/sanitizer_stacktrace.h"
#include "lsan_allocator.h"
#include "lsan_common.h"
#include "lsan_thread.h"

#if SANITIZER_EMSCRIPTEN
extern "C" void emscripten_builtin_free(void *);
#include <emscripten/em_asm.h>
#endif

bool lsan_inited;
bool lsan_init_is_running;

namespace __lsan {

///// Interface to the common LSan module. /////
bool WordIsPoisoned(uptr addr) {
  return false;
}

}  // namespace __lsan

using namespace __lsan;  // NOLINT

static void InitializeFlags() {
  // Set all the default values.
  SetCommonFlagsDefaults();
  {
    CommonFlags cf;
    cf.CopyFrom(*common_flags());
#if !SANITIZER_EMSCRIPTEN
    // getenv on emscripten uses malloc, which we can't when using LSan.
    // You can't run external symbolizers anyway.
    cf.external_symbolizer_path = GetEnv("LSAN_SYMBOLIZER_PATH");
#endif
    cf.malloc_context_size = 30;
    cf.intercept_tls_get_addr = true;
    cf.detect_leaks = true;
    cf.exitcode = 23;
    OverrideCommonFlags(cf);
  }

  Flags *f = flags();
  f->SetDefaults();

  FlagParser parser;
  RegisterLsanFlags(&parser, f);
  RegisterCommonFlags(&parser);

  // Override from user-specified string.
  const char *lsan_default_options = MaybeCallLsanDefaultOptions();
  parser.ParseString(lsan_default_options);
#if SANITIZER_EMSCRIPTEN
  char *options = (char*) EM_ASM_INT({
    return _emscripten_with_builtin_malloc(function () {
      return allocateUTF8(Module['LSAN_OPTIONS'] || 0);
    });
  });
  parser.ParseString(options);
  emscripten_builtin_free(options);
#else
  parser.ParseString(GetEnv("LSAN_OPTIONS"));
#endif // SANITIZER_EMSCRIPTEN

#if SANITIZER_EMSCRIPTEN
  if (common_flags()->malloc_context_size <= 1)
    StackTrace::snapshot_stack = false;
#endif // SANITIZER_EMSCRIPTEN

  SetVerbosity(common_flags()->verbosity);

  if (Verbosity()) ReportUnrecognizedFlags();

  if (common_flags()->help) parser.PrintFlagDescriptions();

  __sanitizer_set_report_path(common_flags()->log_path);
}

static void OnStackUnwind(const SignalContext &sig, const void *,
                          BufferedStackTrace *stack) {
  GetStackTrace(stack, kStackTraceMax, sig.pc, sig.bp, sig.context,
                common_flags()->fast_unwind_on_fatal);
}

static void LsanOnDeadlySignal(int signo, void *siginfo, void *context) {
  HandleDeadlySignal(siginfo, context, GetCurrentThread(), &OnStackUnwind,
                     nullptr);
}

extern "C" void __lsan_init() {
  CHECK(!lsan_init_is_running);
  if (lsan_inited)
    return;
  lsan_init_is_running = true;
  SanitizerToolName = "LeakSanitizer";
  CacheBinaryName();
  AvoidCVE_2016_2143();
  InitializeFlags();
  InitCommonLsan();
  InitializeAllocator();
  ReplaceSystemMalloc();
  InitTlsSize();
  InitializeInterceptors();
  InitializeThreadRegistry();
#if !SANITIZER_EMSCRIPTEN
  // Emscripten does not have signals
  InstallDeadlySignalHandlers(LsanOnDeadlySignal);
#endif
  u32 tid = ThreadCreate(0, 0, true);
  CHECK_EQ(tid, 0);
  ThreadStart(tid, GetTid());
  SetCurrentThread(tid);

  if (common_flags()->detect_leaks && common_flags()->leak_check_at_exit)
    Atexit(DoLeakCheck);

  InitializeCoverage(common_flags()->coverage, common_flags()->coverage_dir);

  lsan_inited = true;
  lsan_init_is_running = false;
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __sanitizer_print_stack_trace() {
  GET_STACK_TRACE_FATAL;
  stack.Print();
}
