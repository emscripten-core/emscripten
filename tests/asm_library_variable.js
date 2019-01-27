var LibraryAsmVar = {
  // By annotating a var with __asm: true, this generates a variable
  // that lives only inside the asm.js/wasm module scope.
  $asmVariable__asm: true,
  $asmVariable: 42,

  foo__deps: ['$asmVariable'],
  foo__asm: true,
  foo__sig: 'i',
  foo: function() {
    return asmVariable|0;
  },
};

mergeInto(LibraryManager.library, LibraryAsmVar);
