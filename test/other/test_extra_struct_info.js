console.error("(before) AF_INET=" + {{{ cDefs.AF_INET }}});

/* Loading this struct info file will override the default AF_INET value */
{{{ loadStructInfo('generated.json'), null }}}

console.error("(after) AF_INET=" + {{{ cDefs.AF_INET }}});
