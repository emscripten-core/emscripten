/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

function log_execution(loc) {
  console.log('log_execution ' + loc);
}
function get_i32(loc, index, value) {
  console.log('get_i32 ' + [loc, index, value]);
  return value;
}
function get_i64(loc, index, low, high) {
  console.log('get_i64 ' + [loc, index, low, high]);
  setTempRet0(high);
  return low;
}
function get_f32(loc, index, value) {
  console.log('get_f32 ' + [loc, index, value]);
  return value;
}
function get_f64(loc, index, value) {
  console.log('get_f64 ' + [loc, index, value]);
  return value;
}
function get_anyref(loc, index, value) {
  console.log('get_anyref ' + [loc, index, value]);
  return value;
}
function get_exnref(loc, index, value) {
  console.log('get_exnref ' + [loc, index, value]);
  return value;
}
function set_i32(loc, index, value) {
  console.log('set_i32 ' + [loc, index, value]);
  return value;
}
function set_i64(loc, index, low, high) {
  console.log('set_i64 ' + [loc, index, low, high]);
  setTempRet0(high);
  return low;
}
function set_f32(loc, index, value) {
  console.log('set_f32 ' + [loc, index, value]);
  return value;
}
function set_f64(loc, index, value) {
  console.log('set_f64 ' + [loc, index, value]);
  return value;
}
function set_anyref(loc, index, value) {
  console.log('set_anyref ' + [loc, index, value]);
  return value;
}
function set_exnref(loc, index, value) {
  console.log('set_exnref ' + [loc, index, value]);
  return value;
}
function load_ptr(loc, bytes, offset, ptr) {
  console.log('load_ptr ' + [loc, bytes, offset, ptr]);
  return ptr;
}
function load_val_i32(loc, value) {
  console.log('load_val_i32 ' + [loc, value]);
  return value;
}
function load_val_i64(loc, low, high) {
  console.log('load_val_i64 ' + [loc, low, high]);
  setTempRet0(high);
  return low;
}
function load_val_f32(loc, value) {
  console.log('load_val_f32 ' + [loc, value]);
  return value;
}
function load_val_f64(loc, value) {
  console.log('load_val_f64 ' + [loc, value]);
  return value;
}
function store_ptr(loc, bytes, offset, ptr) {
  console.log('store_ptr ' + [loc, bytes, offset, ptr]);
  return ptr;
}
function store_val_i32(loc, value) {
  console.log('store_val_i32 ' + [loc, value]);
  return value;
}
function store_val_i64(loc, low, high) {
  console.log('store_val_i64 ' + [loc, low, high]);
  setTempRet0(high);
  return low;
}
function store_val_f32(loc, value) {
  console.log('store_val_f32 ' + [loc, value]);
  return value;
}
function store_val_f64(loc, value) {
  console.log('store_val_f64 ' + [loc, value]);
  return value;
}

{{{
EXTRA_WASM_IMPORTS.push([
  'log_execution',
  'get_i32',
  'get_i64',
  'get_f32',
  'get_f64',
  'get_anyref',
  'get_exnref',
  'set_i32',
  'set_i64',
  'set_f32',
  'set_f64',
  'set_anyref',
  'set_exnref',
  'load_ptr',
  'load_val_i32',
  'load_val_i64',
  'load_val_f32',
  'load_val_f64',
  'store_ptr',
  'store_val_i32',
  'store_val_i64',
  'store_val_f32',
  'store_val_f64'
]);
}}}
