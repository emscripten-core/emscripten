/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// These functions are defined once here, then included in the library below
// under two different names.
function setValueImpl(ptr, value, type = 'i8') {
  if (type.endsWith('*')) type = '*';
  switch (type) {
    case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1') }}}; break;
    case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8') }}}; break;
    case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16') }}}; break;
    case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32') }}}; break;
#if WASM_BIGINT
    case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64') }}}; break;
#else
    case 'i64': abort('to do setValue(i64) use WASM_BIGINT');
#endif
    case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float') }}}; break;
    case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double') }}}; break;
    case '*': {{{ makeSetValue('ptr', '0', 'value', '*') }}}; break;
    default: abort(`invalid type for setValue: ${type}`);
  }
}

function getValueImpl(ptr, type = 'i8') {
  if (type.endsWith('*')) type = '*';
  switch (type) {
    case 'i1': return {{{ makeGetValue('ptr', '0', 'i1') }}};
    case 'i8': return {{{ makeGetValue('ptr', '0', 'i8') }}};
    case 'i16': return {{{ makeGetValue('ptr', '0', 'i16') }}};
    case 'i32': return {{{ makeGetValue('ptr', '0', 'i32') }}};
#if WASM_BIGINT
    case 'i64': return {{{ makeGetValue('ptr', '0', 'i64') }}};
#else
    case 'i64': abort('to do getValue(i64) use WASM_BIGINT');
#endif
    case 'float': return {{{ makeGetValue('ptr', '0', 'float') }}};
    case 'double': return {{{ makeGetValue('ptr', '0', 'double') }}};
    case '*': return {{{ makeGetValue('ptr', '0', '*') }}};
    default: abort(`invalid type for getValue: ${type}`);
  }
}

var LibraryMemOps = {
  $setValue__docs: `
  /**
   * @param {number} ptr
   * @param {number} value
   * @param {string} type
   */`,
  $setValue: setValueImpl,

  $getValue__docs: `
  /**
   * @param {number} ptr
   * @param {string} type
   */`,
  $getValue: getValueImpl,
};

addToLibrary(LibraryMemOps);
