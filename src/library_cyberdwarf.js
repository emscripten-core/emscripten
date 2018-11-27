// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var LibraryCyberdwarf = {
  // These are empty, designed to be replaced when a debugger is invoked
  metadata_llvm_dbg_value_constant: function(a,b,c,d) {
  },
  metadata_llvm_dbg_value_local: function(a,b,c,d) {
  }
};

mergeInto(LibraryManager.library, LibraryCyberdwarf);
