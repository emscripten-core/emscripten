/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryMemOps = {
  $setValue__docs: `
  /** @param {number} ptr
      @param {number} value
      @param {string} type
      @param {number|boolean=} noSafe */`,
  $setValue: function(ptr, value, type = 'i8', noSafe) {
    if (type.endsWith('*')) type = '{{{ POINTER_WASM_TYPE }}}';
#if SAFE_HEAP
    if (noSafe) {
      switch (type) {
        case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1', undefined, undefined, undefined, /*noSafe=*/true) }}}; break;
        case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8', undefined, undefined, undefined, /*noSafe=*/true) }}}; break;
        case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16', undefined, undefined, undefined, /*noSafe=*/true) }}}; break;
        case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32', undefined, undefined, undefined, /*noSafe=*/true) }}}; break;
        case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64', undefined, undefined, undefined, /*noSafe=*/true) }}}; break;
        case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float', undefined, undefined, undefined, /*noSafe=*/true) }}}; break;
        case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double', undefined, undefined, undefined, /*noSafe=*/true) }}}; break;
        default: abort('invalid type for setValue: ' + type);
      }
      return;
    }
#endif
    switch (type) {
      case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1') }}}; break;
      case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8') }}}; break;
      case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16') }}}; break;
      case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32') }}}; break;
      case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64') }}}; break;
      case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float') }}}; break;
      case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double') }}}; break;
      default: abort('invalid type for setValue: ' + type);
    }
  },

  $getValue__docs: `
  /** @param {number} ptr
      @param {string} type
      @param {number|boolean=} noSafe */`,
  $getValue: function(ptr, type = 'i8', noSafe) {
    if (type.endsWith('*')) type = '{{{ POINTER_WASM_TYPE }}}';
#if SAFE_HEAP
    if (noSafe) {
      switch (type) {
        case 'i1': return {{{ makeGetValue('ptr', '0', 'i1', undefined, undefined, undefined, undefined, /*noSafe=*/true) }}};
        case 'i8': return {{{ makeGetValue('ptr', '0', 'i8', undefined, undefined, undefined, undefined, /*noSafe=*/true) }}};
        case 'i16': return {{{ makeGetValue('ptr', '0', 'i16', undefined, undefined, undefined, undefined, /*noSafe=*/true) }}};
        case 'i32': return {{{ makeGetValue('ptr', '0', 'i32', undefined, undefined, undefined, undefined, /*noSafe=*/true) }}};
        case 'i64': return {{{ makeGetValue('ptr', '0', 'i64', undefined, undefined, undefined, undefined, /*noSafe=*/true) }}};
        case 'float': return {{{ makeGetValue('ptr', '0', 'float', undefined, undefined, undefined, undefined, /*noSafe=*/true) }}};
        case 'double': return {{{ makeGetValue('ptr', '0', 'double', undefined, undefined, undefined, undefined, /*noSafe=*/true) }}};
        default: abort('invalid type for getValue: ' + type);
      }
      return;
    }
#endif
    switch (type) {
      case 'i1': return {{{ makeGetValue('ptr', '0', 'i1') }}};
      case 'i8': return {{{ makeGetValue('ptr', '0', 'i8') }}};
      case 'i16': return {{{ makeGetValue('ptr', '0', 'i16') }}};
      case 'i32': return {{{ makeGetValue('ptr', '0', 'i32') }}};
      case 'i64': return {{{ makeGetValue('ptr', '0', 'i64') }}};
      case 'float': return {{{ makeGetValue('ptr', '0', 'float') }}};
      case 'double': return {{{ makeGetValue('ptr', '0', 'double') }}};
      default: abort('invalid type for getValue: ' + type);
    }
    return null;
  },
};

mergeInto(LibraryManager.library, LibraryMemOps);
