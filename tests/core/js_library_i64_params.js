TestLibrary = {
  jscall__deps: i53ConversionDeps,
  jscall__sig: 'ij',
  jscall: function({{{ defineI64Param('foo') }}}) {
    {{{ receiveI64ParamAsI53('foo', `(err('overflow'), ${makeReturn64('42')})`) }}}
    err('js:got:       ' + foo);
    if (foo < 0)
      var rtn = Math.ceil(foo / 2);
    else
      rtn = Math.floor(foo / 2);
    err('js:returning: ' + rtn);
    return {{{ makeReturn64('rtn') }}};
  },
}

mergeInto(LibraryManager.library, TestLibrary);
