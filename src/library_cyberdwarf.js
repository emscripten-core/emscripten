var LibraryCyberdwarf = {
  // These are empty, designed to be replaced when a debugger is invoked
  metadata_llvm_dbg_value_constant: function(a,b,c,d) {
  },
  metadata_llvm_dbg_value_local: function(a,b,c,d) {
  }
};

mergeInto(LibraryManager.library, LibraryCyberdwarf);
