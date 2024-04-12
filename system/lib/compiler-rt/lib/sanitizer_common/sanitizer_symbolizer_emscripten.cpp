//===-- sanitizer_symbolizer_emscripten.cc --------------------------------===//
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
// Emscripten-specific implementation of symbolizer parts.
//===----------------------------------------------------------------------===//

#include "sanitizer_platform.h"

#if SANITIZER_EMSCRIPTEN

#include "sanitizer_symbolizer_internal.h"
#include "emscripten_internal.h"

namespace __sanitizer {

class EmscriptenSymbolizerTool : public SymbolizerTool {
 public:
  bool SymbolizePC(uptr addr, SymbolizedStack *stack) override;
  bool SymbolizeData(uptr addr, DataInfo *info) override {
    return false;
  }
  const char *Demangle(const char *name) override {
    return name;
  }
};

bool EmscriptenSymbolizerTool::SymbolizePC(uptr addr, SymbolizedStack *frame) {
  const char *func_name = emscripten_pc_get_function(addr);
  if (func_name) {
    frame->info.function = internal_strdup(func_name);
    frame->info.function_offset = addr;
  }

  const char *file_name = emscripten_pc_get_file(addr);
  if (file_name) {
    frame->info.file = internal_strdup(file_name);
    frame->info.line = emscripten_pc_get_line(addr);
    frame->info.column = emscripten_pc_get_column(addr);
  }

  return !!func_name;
}

static void ChooseSymbolizerTools(IntrusiveList<SymbolizerTool> *list,
                                  LowLevelAllocator *allocator) {
  if (!common_flags()->symbolize) {
    VReport(2, "Symbolizer is disabled.\n");
    return;
  }

  list->push_back(new(*allocator) EmscriptenSymbolizerTool());
}

const char *Symbolizer::PlatformDemangle(const char *name) {
  return name;
}

Symbolizer *Symbolizer::PlatformInit() {
  IntrusiveList<SymbolizerTool> list;
  list.clear();
  ChooseSymbolizerTools(&list, &symbolizer_allocator_);

  return new(symbolizer_allocator_) Symbolizer(list);
}

void Symbolizer::LateInitialize() {
  Symbolizer::GetOrInit();
}

} // namespace __sanitizer

#endif  // SANITIZER_EMSCRIPTEN
