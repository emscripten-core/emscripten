/**
 * @license
 * Copyright 2011 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryWget = {
  $wget: {
    wgetRequests: {},
    nextWgetRequestHandle: 0,

    getNextWgetRequestHandle() {
      var handle = wget.nextWgetRequestHandle;
      wget.nextWgetRequestHandle++;
      return handle;
    },
  },

  emscripten_async_wget__deps: [
    '$PATH_FS', '$wget', '$callUserCallback', '$Browser',
    '$stackRestore', '$stringToUTF8OnStack',
    '$FS_mkdirTree',
    '$FS_createPreloadedFile',
    '$FS_unlink',
  ],
  emscripten_async_wget__proxy: 'sync',
  emscripten_async_wget: (url, file, onload, onerror) => {
    {{{ runtimeKeepalivePush() }}}

    var _url = UTF8ToString(url);
    var _file = UTF8ToString(file);
    _file = PATH_FS.resolve(_file);
    function doCallback(callback) {
      if (callback) {
        {{{ runtimeKeepalivePop() }}}
        callUserCallback(() => {
          var sp = stackSave();
          {{{ makeDynCall('vp', 'callback') }}}(stringToUTF8OnStack(_file));
          stackRestore(sp);
        });
      }
    }
    var destinationDirectory = PATH.dirname(_file);
    FS_createPreloadedFile(
      destinationDirectory,
      PATH.basename(_file),
      _url, true, true,
      () => doCallback(onload),
      () => doCallback(onerror),
      false, // dontCreateFile
      false, // canOwn
      () => { // preFinish
        // if a file exists there, we overwrite it
        try {
          FS_unlink(_file);
        } catch (e) {}
        // if the destination directory does not yet exist, create it
        FS_mkdirTree(destinationDirectory);
      }
    );
  },

  emscripten_async_wget_data__deps: ['$asyncLoad', 'malloc', 'free', '$callUserCallback'],
  emscripten_async_wget_data__proxy: 'sync',
  emscripten_async_wget_data: (url, userdata, onload, onerror) => {
    {{{ runtimeKeepalivePush() }}}
    asyncLoad(UTF8ToString(url), (byteArray) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        var buffer = _malloc(byteArray.length);
        HEAPU8.set(byteArray, buffer);
        {{{ makeDynCall('vppi', 'onload') }}}(userdata, buffer, byteArray.length);
        _free(buffer);
      });
    }, () => {
      if (onerror) {
        {{{ runtimeKeepalivePop() }}}
        callUserCallback(() => {
          {{{ makeDynCall('vp', 'onerror') }}}(userdata);
        });
      }
    }, true /* no need for run dependency, this is async but will not do any prepare etc. step */ );
  },

  emscripten_async_wget2__deps: ['$PATH_FS', '$wget', '$stackRestore', '$stringToUTF8OnStack'],
  emscripten_async_wget2__proxy: 'sync',
  emscripten_async_wget2: (url, file, request, param, userdata, onload, onerror, onprogress) => {
    {{{ runtimeKeepalivePush() }}}

    var _url = UTF8ToString(url);
    var _file = UTF8ToString(file);
    _file = PATH_FS.resolve(_file);
    var _request = UTF8ToString(request);
    var _param = UTF8ToString(param);
    var index = _file.lastIndexOf('/');

    var http = new XMLHttpRequest();
    http.open(_request, _url, true);
    http.responseType = 'arraybuffer';

    var handle = wget.getNextWgetRequestHandle();

    var destinationDirectory = PATH.dirname(_file);

    // LOAD
    http.onload = (e) => {
      {{{ runtimeKeepalivePop() }}}
      if (http.status >= 200 && http.status < 300) {
        // if a file exists there, we overwrite it
        try {
          FS.unlink(_file);
        } catch (e) {}
        // if the destination directory does not yet exist, create it
        FS.mkdirTree(destinationDirectory);

        FS.createDataFile( _file.substr(0, index), _file.substr(index + 1), new Uint8Array(/** @type{ArrayBuffer}*/(http.response)), true, true, false);
        if (onload) {
          var sp = stackSave();
          {{{ makeDynCall('vipp', 'onload') }}}(handle, userdata, stringToUTF8OnStack(_file));
          stackRestore(sp);
        }
      } else {
        if (onerror) {{{ makeDynCall('vipi', 'onerror') }}}(handle, userdata, http.status);
      }

      delete wget.wgetRequests[handle];
    };

    // ERROR
    http.onerror = (e) => {
      {{{ runtimeKeepalivePop() }}}
      if (onerror) {{{ makeDynCall('vipi', 'onerror') }}}(handle, userdata, http.status);
      delete wget.wgetRequests[handle];
    };

    // PROGRESS
    http.onprogress = (e) => {
      if (e.lengthComputable || (e.lengthComputable === undefined && e.total != 0)) {
        var percentComplete = (e.loaded / e.total)*100;
        if (onprogress) {{{ makeDynCall('vipi', 'onprogress') }}}(handle, userdata, percentComplete);
      }
    };

    // ABORT
    http.onabort = (e) => {
      {{{ runtimeKeepalivePop() }}}
      delete wget.wgetRequests[handle];
    };

    if (_request == "POST") {
      //Send the proper header information along with the request
      http.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      http.send(_param);
    } else {
      http.send(null);
    }

    wget.wgetRequests[handle] = http;

    return handle;
  },

  emscripten_async_wget2_data__deps: ['$wget', 'malloc', 'free'],
  emscripten_async_wget2_data__proxy: 'sync',
  emscripten_async_wget2_data: (url, request, param, userdata, free, onload, onerror, onprogress) => {
    var _url = UTF8ToString(url);
    var _request = UTF8ToString(request);
    var _param = UTF8ToString(param);

    var http = new XMLHttpRequest();
    http.open(_request, _url, true);
    http.responseType = 'arraybuffer';

    var handle = wget.getNextWgetRequestHandle();

    function onerrorjs() {
      if (onerror) {
        var sp = stackSave();
        var statusText = 0;
        if (http.statusText) {
          statusText = stringToUTF8OnStack(http.statusText);
        }
        {{{ makeDynCall('vipip', 'onerror') }}}(handle, userdata, http.status, statusText);
        stackRestore(sp);
      }
    }

    // LOAD
    http.onload = (e) => {
      if (http.status >= 200 && http.status < 300 || (http.status === 0 && _url.substr(0,4).toLowerCase() != "http")) {
        var byteArray = new Uint8Array(/** @type{ArrayBuffer} */(http.response));
        var buffer = _malloc(byteArray.length);
        HEAPU8.set(byteArray, buffer);
        if (onload) {{{ makeDynCall('vippi', 'onload') }}}(handle, userdata, buffer, byteArray.length);
        if (free) _free(buffer);
      } else {
        onerrorjs();
      }
      delete wget.wgetRequests[handle];
    };

    // ERROR
    http.onerror = (e) => {
      onerrorjs();
      delete wget.wgetRequests[handle];
    };

    // PROGRESS
    http.onprogress = (e) => {
      if (onprogress) {{{ makeDynCall('viiii', 'onprogress') }}}(handle, userdata, e.loaded, e.lengthComputable || e.lengthComputable === undefined ? e.total : 0);
    };

    // ABORT
    http.onabort = (e) => {
      delete wget.wgetRequests[handle];
    };

    if (_request == "POST") {
      //Send the proper header information along with the request
      http.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      http.send(_param);
    } else {
      http.send(null);
    }

    wget.wgetRequests[handle] = http;

    return handle;
  },

  emscripten_async_wget2_abort__deps: ['$wget'],
  emscripten_async_wget2_abort__proxy: 'sync',
  emscripten_async_wget2_abort: (handle) => {
    var http = wget.wgetRequests[handle];
    http?.abort();
  },
};

addToLibrary(LibraryWget);
