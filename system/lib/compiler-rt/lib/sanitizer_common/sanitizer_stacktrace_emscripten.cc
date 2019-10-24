//===-- sanitizer_stacktrace_emscripten.cc --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is shared between AddressSanitizer and ThreadSanitizer
// run-time libraries.
//
// Implementation of fast stack unwinding for Emscripten.
//===----------------------------------------------------------------------===//

#ifdef __EMSCRIPTEN__

#include "sanitizer_common.h"
#include "sanitizer_stacktrace.h"

namespace __sanitizer {

extern "C" {
  uptr emscripten_stack_snapshot();
  uptr emscripten_return_address(int level);
  u32 emscripten_stack_unwind_buffer(uptr pc, uptr *buffer, u32 depth);
}

bool StackTrace::snapshot_stack = true;

uptr StackTrace::GetCurrentPc() {
  return snapshot_stack ? emscripten_stack_snapshot() : 0;
}

void BufferedStackTrace::FastUnwindStack(uptr pc, uptr bp, uptr stack_top,
                                         uptr stack_bottom, u32 max_depth) {
  max_depth = Min(max_depth, kStackTraceMax);
  size = emscripten_stack_unwind_buffer(pc, trace_buffer, max_depth);
  trace_buffer[0] = pc;
  size = Max(size, 1U);
}

}  // namespace __sanitizer

#endif  // __EMSCRIPTEN__
