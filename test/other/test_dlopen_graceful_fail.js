#if !MAIN_MODULE
#if !SIDE_MODULE
// This code represents stable end-user facing API. Changing any part of this file
// will break end users!
mergeInto(LibraryManager.library, {
  _dlopen_js__deps: ['$warnOnce'],
  _dlopen_js: function(handle) {
#if ASSERTIONS
    warnOnce('_dlopen_js: Unable to open DLL! Dynamic linking is not supported in WebAssembly builds due to limitations to performance and code size. Please statically link in the needed libraries.');
#endif
  },

  _emscripten_dlopen_js__deps: ['$warnOnce'],
  _emscripten_dlopen_js: function(filename, flags, user_data, onsuccess, onerror) {
#if ASSERTIONS
    warnOnce('_emscripten_dlopen_js: Unable to open DLL ' + UTF8ToString(filename) + '! Dynamic linking is not supported in WebAssembly builds due to limitations to performance and code size. Please statically link in the needed libraries.');
#endif
  },

  _dlsym_js__deps: ['$warnOnce'],
  _dlsym_js: function(handle, symbol, symbolIndex) {
#if ASSERTIONS
    warnOnce('_dlsym_js: Unable to open DLL! Dynamic linking is not supported in WebAssembly builds due to limitations to performance and code size. Please statically link in the needed libraries.');
#endif
  },

  _dlsym_catchup_js__deps: ['$warnOnce'],
  _dlsym_catchup_js: function(handle, symbolIndex) {
#if ASSERTIONS
    warnOnce('_dlsym_catchup_js: Unable to open DLL! Dynamic linking is not supported in WebAssembly builds due to limitations to performance and code size. Please statically link in the needed libraries.');
#endif
  },

  dlopen__deps: ['$warnOnce'],
  dlopen: function(handle) {
#if ASSERTIONS
    warnOnce('dlopen: Unable to open DLL! Dynamic linking is not supported in WebAssembly builds due to limitations to performance and code size. Please statically link in the needed libraries.');
#endif
  },

  emscripten_dlopen__deps: ['$warnOnce'],
  emscripten_dlopen: function(handle, onsuccess, onerror, user_data) {
#if ASSERTIONS
    warnOnce('emscripten_dlopen: Unable to open DLL! Dynamic linking is not supported in WebAssembly builds due to limitations to performance and code size. Please statically link in the needed libraries.');
#endif
  },

  __dlsym__deps: ['$warnOnce'],
  __dlsym: function(handle, symbol) {
#if ASSERTIONS
    warnOnce('__dlsym: Unable to open DLL! Dynamic linking is not supported in WebAssembly builds due to limitations to performance and code size. Please statically link in the needed libraries.');
#endif
  },
});
#endif
#endif
