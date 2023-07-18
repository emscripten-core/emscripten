/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */


#if !AUTODEBUG
#error "Should only be incldued in AUTODEBUG mode"
#endif

mergeInto(LibraryManager.library, {
  $log_execution: function(loc) {
    out('log_execution ' + loc);
  },
  $get_i32: function(loc, index, value) {
    out('get_i32 ' + [loc, index, value]);
    return value;
  },
  $get_i64__deps: ['setTempRet0'],
  $get_i64: function(loc, index, low, high) {
    out('get_i64 ' + [loc, index, low, high]);
    setTempRet0(high);
    return low;
  },
  $get_f32: function(loc, index, value) {
    out('get_f32 ' + [loc, index, value]);
    return value;
  },
  $get_f64: function(loc, index, value) {
    out('get_f64 ' + [loc, index, value]);
    return value;
  },
  $get_anyref: function(loc, index, value) {
    out('get_anyref ' + [loc, index, value]);
    return value;
  },
  $get_exnref: function(loc, index, value) {
    out('get_exnref ' + [loc, index, value]);
    return value;
  },
  $set_i32: function(loc, index, value) {
    out('set_i32 ' + [loc, index, value]);
    return value;
  },
  $set_i64__deps: ['setTempRet0'],
  $set_i64: function(loc, index, low, high) {
    out('set_i64 ' + [loc, index, low, high]);
    setTempRet0(high);
    return low;
  },
  $set_f32: function(loc, index, value) {
    out('set_f32 ' + [loc, index, value]);
    return value;
  },
  $set_f64: function(loc, index, value) {
    out('set_f64 ' + [loc, index, value]);
    return value;
  },
  $set_anyref: function(loc, index, value) {
    out('set_anyref ' + [loc, index, value]);
    return value;
  },
  $set_exnref: function(loc, index, value) {
    out('set_exnref ' + [loc, index, value]);
    return value;
  },
  $load_ptr: function(loc, bytes, offset, ptr) {
    out('load_ptr ' + [loc, bytes, offset, ptr]);
    return ptr;
  },
  $load_val_i32: function(loc, value) {
    out('load_val_i32 ' + [loc, value]);
    return value;
  },
  $load_val_i64__deps: ['setTempRet0'],
  $load_val_i64: function(loc, low, high) {
    out('load_val_i64 ' + [loc, low, high]);
    setTempRet0(high);
    return low;
  },
  $load_val_f32: function(loc, value) {
    out('load_val_f32 ' + [loc, value]);
    return value;
  },
  $load_val_f64: function(loc, value) {
    out('load_val_f64 ' + [loc, value]);
    return value;
  },
  $store_ptr: function(loc, bytes, offset, ptr) {
    out('store_ptr ' + [loc, bytes, offset, ptr]);
    return ptr;
  },
  $store_val_i32: function(loc, value) {
    out('store_val_i32 ' + [loc, value]);
    return value;
  },
  $store_val_i64__deps: ['setTempRet0'],
  $store_val_i64: function(loc, low, high) {
    out('store_val_i64 ' + [loc, low, high]);
    setTempRet0(high);
    return low;
  },
  $store_val_f32: function(loc, value) {
    out('store_val_f32 ' + [loc, value]);
    return value;
  },
  $store_val_f64: function(loc, value) {
    out('store_val_f64 ' + [loc, value]);
    return value;
  },
});

DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push(
  '$log_execution',
  '$get_i32',
  '$get_i64',
  '$get_f32',
  '$get_f64',
  '$get_anyref',
  '$get_exnref',
  '$set_i32',
  '$set_i64',
  '$set_f32',
  '$set_f64',
  '$set_anyref',
  '$set_exnref',
  '$load_ptr',
  '$load_val_i32',
  '$load_val_i64',
  '$load_val_f32',
  '$load_val_f64',
  '$store_ptr',
  '$store_val_i32',
  '$store_val_i64',
  '$store_val_f32',
  '$store_val_f64',
);
