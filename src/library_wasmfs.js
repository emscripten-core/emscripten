var WASMFS = {
    $WASMFS : {}

}

mergeInto(LibraryManager.library, WASMFS);

DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$WASMFS');