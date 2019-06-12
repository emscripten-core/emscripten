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
  void emscripten_stack_snapshot();
  uptr emscripten_stack_unwind(uptr pc, int level);
  uptr emscripten_return_address(int level);
}

uptr StackTrace::GetCurrentPc() {
  emscripten_stack_snapshot();
  return GET_CALLER_PC();
}

void BufferedStackTrace::FastUnwindStack(uptr pc, uptr bp, uptr stack_top,
                                         uptr stack_bottom, u32 max_depth) {
  bool saw_pc = false;
  trace_buffer[0] = pc;
  size = 1;

  uptr pc1;
  if (emscripten_stack_unwind(pc, 0)) {
    // If the last stack snapshot was taken with this pc, we use that snapshot.
    for (int level = 0; (pc1 = emscripten_stack_unwind(pc, level)); ++level) {
      if (saw_pc) {
        trace_buffer[size++] = pc1;
      }
      saw_pc |= pc == pc1;
    }
  } else {
    emscripten_stack_snapshot();

    for (int level = 0; (pc1 = emscripten_return_address(level)); ++level) {
      if (saw_pc) {
        trace_buffer[size++] = pc1;
      }
      saw_pc |= pc == pc1;
    }
  }
}

}  // namespace __sanitizer

#endif  // __EMSCRIPTEN__
