mergeInto(LibraryManager.library, {
  $ASSERTIONS: Boolean({{{ ASSERTIONS }}}),
  $DYNAMIC_EXECUTION: Boolean({{{ DYNAMIC_EXECUTION }}}),
  $EMBIND_STD_STRING_IS_UTF8: Boolean({{{ EMBIND_STD_STRING_IS_UTF8 }}}),
});
