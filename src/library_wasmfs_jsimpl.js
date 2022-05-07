/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  // Backend support. wasmFS$backends will contain a mapping of backend IDs to
  // the JS code that implements them. This is the JS side of the JSImpl* class
  // in C++, together with the js_impl calls defined right after it.
  $wasmFS$backends: {},

  _wasmfs_jsimpl_alloc_file: function(backend, file) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].allocFile(file);
  },

  _wasmfs_jsimpl_free_file: function(backend, file) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].freeFile(file);
  },

  _wasmfs_jsimpl_write: function(backend, file, buffer, length, {{{ defineI64Param('offset') }}}) {
    {{{ receiveI64ParamAsDouble('offset') }}}
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].write(file, buffer, length, offset);
  },

  _wasmfs_jsimpl_read: function(backend, file, buffer, length, {{{ defineI64Param('offset') }}}) {
    {{{ receiveI64ParamAsDouble('offset') }}}
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].read(file, buffer, length, offset);
  },

  _wasmfs_jsimpl_get_size: function(backend, file) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].getSize(file);
  },

  // ProxiedAsyncJSImpl. Each function receives a function pointer and a
  // parameter. We convert those into a convenient Promise API for the
  // implementors of backends: the hooks we call should return Promises, which
  // we then connect to the calling C++.

  // TODO: arg is void*, which for MEMORY64 will be 64-bit. we need a way to
  //       declare arg in the function signature here (like defineI64Param,
  //       but that varies for wasm32/wasm64), and a way to do makeDynCall that
  //       adds a 'p' signature type for pointer, or something like that
  //       (however, dyncalls might also just work, given in MEMORY64 we assume
  //       WASM_BIGINT so the pointer is just a single argument, just like in
  //       wasm32).
  _wasmfs_jsimpl_async_alloc_file__deps: ['$runtimeKeepalivePush', '$runtimeKeepalivePop'],
  _wasmfs_jsimpl_async_alloc_file: async function(backend, file, fptr, arg) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    await wasmFS$backends[backend].allocFile(file);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },

  _wasmfs_jsimpl_async_free_file__deps:  ['$runtimeKeepalivePush', '$runtimeKeepalivePop'],
  _wasmfs_jsimpl_async_free_file: async function(backend, file, fptr, arg) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    await wasmFS$backends[backend].freeFile(file);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },

  _wasmfs_jsimpl_async_write__deps: ['$runtimeKeepalivePush', '$runtimeKeepalivePop'],
  _wasmfs_jsimpl_async_write: async function(backend, file, buffer, length, {{{ defineI64Param('offset') }}}, fptr, arg) {
    {{{ receiveI64ParamAsDouble('offset') }}}
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    var size = await wasmFS$backends[backend].write(file, buffer, length, offset);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.result, '0', 'i32') }}};
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.offset, 'size', 'i64') }}};
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },

  _wasmfs_jsimpl_async_read__deps: ['$runtimeKeepalivePush', '$runtimeKeepalivePop'],
  _wasmfs_jsimpl_async_read: async function(backend, file, buffer, length, {{{ defineI64Param('offset') }}}, fptr, arg) {
    {{{ receiveI64ParamAsDouble('offset') }}}
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    var size = await wasmFS$backends[backend].read(file, buffer, length, offset);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.result, '0', 'i32') }}};
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.offset, 'size', 'i64') }}};
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },

  _wasmfs_jsimpl_async_get_size__deps:  ['$runtimeKeepalivePush', '$runtimeKeepalivePop'],
  _wasmfs_jsimpl_async_get_size: async function(backend, file, fptr, arg) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    var size = await wasmFS$backends[backend].getSize(file);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.result, '0', 'i32') }}};
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.offset, 'size', 'i64') }}};
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },
});
