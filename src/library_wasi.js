/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

mergeInto(LibraryManager.library, {
  proc_exit__deps: ['exit'],
  proc_exit: function(code) {
    return _exit(code);
  },

  emscripten_get_environ: function() {
    if (!_emscripten_get_environ.ENV) {
      var ENV = {};
      ENV['USER'] = ENV['LOGNAME'] = 'web_user';
      ENV['PATH'] = '/';
      ENV['PWD'] = '/';
      ENV['HOME'] = '/home/web_user';
      // Browser language detection #8751
      ENV['LANG'] = ((typeof navigator === 'object' &&navigator.languages && navigator.languages[0]) || 'C').replace('-', '_') + '.UTF-8';
      ENV['_'] = thisProgram;
      _emscripten_get_environ.ENV = ENV;
    }
    return _emscripten_get_environ.ENV;
  },

  environ_sizes_get__deps: ['emscripten_get_environ'],
  environ_sizes_get: function(environ_count, environ_buf_size) {
    var ENV = _emscripten_get_environ();
    var count = 0;
    var bufSize = 0;
    for (var key in ENV) {
      count++;
      bufSize += key.length + 1 + ENV[key].length + 1;
    }
    {{{ makeSetValue('environ_count', 0, 'count', 'i32') }}};
    {{{ makeSetValue('environ_buf_size', 0, 'bufSize', 'i32') }}};
    return 0;
  },

  environ_get__deps: ['emscripten_get_environ'],
  environ_get: function(__environ, environ_buf) {
    var ENV = _emscripten_get_environ();
    var str = '';
    var i = 0;
    for (var key in ENV) {
      {{{ makeSetValue('__environ', 'i * 4', 'environ_buf + str.length', 'i32') }}};
      str += key + '=' + ENV[key] + String.fromCharCode(0);
    }
    writeAsciiToMemory(str, environ_buf, true /* don'tAddNull */);
    return 0;
  },
});
