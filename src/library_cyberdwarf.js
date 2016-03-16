var LibraryCyberdwarf = {
  metadata_llvm_dbg_value_constant_d: function(a,b) {
    if (Module["cyberdwarf_debug_constant"]) {
      var mdd = Module['cyberdwarf'].get_intrinsic_data(b);
      Module["cyberdwarf_debug_constant"](a,mdd[0],mdd[1],mdd[2],mdd[3]);
    }
  },
  metadata_llvm_dbg_value_constant_i: function(a,b) {
    if (Module["cyberdwarf_debug_constant"]) {
      var mdd = Module['cyberdwarf'].get_intrinsic_data(b);
      Module["cyberdwarf_debug_constant"](a,mdd[0],mdd[1],mdd[2],mdd[3]);
    }
  },
  metadata_llvm_dbg_value_local_d: function(a,b) {
    if (Module["cyberdwarf_debug_value"]) {
      var mdd = Module['cyberdwarf'].get_intrinsic_data(b);
      Module["cyberdwarf_debug_value"](a,mdd[0],mdd[1],mdd[2],mdd[3]);
    }
  },
  metadata_llvm_dbg_value_local_i: function(a,b) {
    if (Module["cyberdwarf_debug_value"]) {
      var mdd = Module['cyberdwarf'].get_intrinsic_data(b);
      Module["cyberdwarf_debug_value"](a,mdd[0],mdd[1],mdd[2],mdd[3]);
    }
  }
};

mergeInto(LibraryManager.library, LibraryCyberdwarf);
