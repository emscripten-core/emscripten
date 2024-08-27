/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */


#if !AUTODEBUG
#error "Should only be included in AUTODEBUG mode"
#endif

addToLibrary({
  $log_execution: (loc) => dbg('log_execution ' + loc),
  $get_i32: (loc, index, value) => {
    dbg('get_i32 ' + [loc, index, value]);
    return value;
  },
  $get_i64__deps: ['setTempRet0'],
  $get_i64: (loc, index, low, high) => {
    dbg('get_i64 ' + [loc, index, low, high]);
    setTempRet0(high);
    return low;
  },
  $get_f32: (loc, index, value) => {
    dbg('get_f32 ' + [loc, index, value]);
    return value;
  },
  $get_f64: (loc, index, value) => {
    dbg('get_f64 ' + [loc, index, value]);
    return value;
  },
  $get_funcref: (loc, index, value) => {
    dbg('get_funcref ' + [loc, index, value]);
    return value;
  },
  $get_externref: (loc, index, value) => {
    dbg('get_externref ' + [loc, index, value]);
    return value;
  },
  $get_anyref: (loc, index, value) => {
    dbg('get_anyref ' + [loc, index, value]);
    return value;
  },
  $get_exnref: (loc, index, value) => {
    dbg('get_exnref ' + [loc, index, value]);
    return value;
  },
  $set_i32: (loc, index, value) => {
    dbg('set_i32 ' + [loc, index, value]);
    return value;
  },
  $set_i64__deps: ['setTempRet0'],
  $set_i64: (loc, index, low, high) => {
    dbg('set_i64 ' + [loc, index, low, high]);
    setTempRet0(high);
    return low;
  },
  $set_f32: (loc, index, value) => {
    dbg('set_f32 ' + [loc, index, value]);
    return value;
  },
  $set_f64: (loc, index, value) => {
    dbg('set_f64 ' + [loc, index, value]);
    return value;
  },
  $set_funcref: (loc, index, value) => {
    dbg('set_afuncef ' + [loc, index, value]);
    return value;
  },
  $set_externref: (loc, index, value) => {
    dbg('set_externref ' + [loc, index, value]);
    return value;
  },
  $set_anyref: (loc, index, value) => {
    dbg('set_anyref ' + [loc, index, value]);
    return value;
  },
  $set_exnref: (loc, index, value) => {
    dbg('set_exnref ' + [loc, index, value]);
    return value;
  },
  $load_ptr: (loc, bytes, offset, ptr) => {
    dbg('load_ptr ' + [loc, bytes, offset, ptr]);
    return ptr;
  },
  $load_val_i32: (loc, value) => {
    dbg('load_val_i32 ' + [loc, value]);
    return value;
  },
  $load_val_i64__deps: ['setTempRet0'],
  $load_val_i64: (loc, low, high) => {
    dbg('load_val_i64 ' + [loc, low, high]);
    setTempRet0(high);
    return low;
  },
  $load_val_f32: (loc, value) => {
    dbg('load_val_f32 ' + [loc, value]);
    return value;
  },
  $load_val_f64: (loc, value) => {
    dbg('load_val_f64 ' + [loc, value]);
    return value;
  },
  $store_ptr: (loc, bytes, offset, ptr) => {
    dbg('store_ptr ' + [loc, bytes, offset, ptr]);
    return ptr;
  },
  $store_val_i32: (loc, value) => {
    dbg('store_val_i32 ' + [loc, value]);
    return value;
  },
  $store_val_i64__deps: ['setTempRet0'],
  $store_val_i64: (loc, low, high) => {
    dbg('store_val_i64 ' + [loc, low, high]);
    setTempRet0(high);
    return low;
  },
  $store_val_f32: (loc, value) => {
    dbg('store_val_f32 ' + [loc, value]);
    return value;
  },
  $store_val_f64: (loc, value) => {
    dbg('store_val_f64 ' + [loc, value]);
    return value;
  },
});

extraLibraryFuncs.push(
  '$log_execution',
  '$get_i32',
  '$get_i64',
  '$get_f32',
  '$get_f64',
  '$get_funcref',
  '$get_externref',
  '$get_anyref',
  '$get_exnref',
  '$set_i32',
  '$set_i64',
  '$set_f32',
  '$set_f64',
  '$set_funcref',
  '$set_externref',
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
