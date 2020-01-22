// === Auto-generated preamble library stuff ===

//========================================
// Runtime essentials
//========================================

{{{
(function() {
  // add in RUNTIME_LINKED_LIBS, if provided
  //
  // for side module we only set Module.dynamicLibraries - and loading them
  // will be handled by dynamic linker runtime in the main module.
  if (RUNTIME_LINKED_LIBS.length > 0) {
    return "if (!Module['dynamicLibraries']) Module['dynamicLibraries'] = [];\n" +
           "Module['dynamicLibraries'] = " + JSON.stringify(RUNTIME_LINKED_LIBS) + ".concat(Module['dynamicLibraries']);\n";
  }
  return '';
})()
}}}

// === Body ===

