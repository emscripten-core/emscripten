TestLibrary = {
#if !WASM_BIGINT
  jscall__deps: ['$convertI32PairToI53' ],
#endif
  jscall__sig: 'ij',
  jscall: function({{{ defineI64Param('foo') }}}) {
    {{{ receiveI64ParamAsDouble('foo') }}}
    return foo * 2;
  },
}

mergeInto(LibraryManager.library, TestLibrary);
