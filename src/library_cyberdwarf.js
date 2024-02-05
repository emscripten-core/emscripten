/**
 * @license
 * Copyright 2016 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryCyberdwarf = {
  // These are empty, designed to be replaced when a debugger is invoked
  metadata_llvm_dbg_value_constant: (a,b,c,d) => {},
  metadata_llvm_dbg_value_local: (a,b,c,d) => {}
};

addToLibrary(LibraryCyberdwarf);
