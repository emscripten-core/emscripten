/**
 * @license
 * Copyright 2011 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryWget = {
  $wget: {
    wgetRequests: {},
    nextWgetRequestHandle: 0,

    getNextWgetRequestHandle: function() {
      var handle = wget.nextWgetRequestHandle;
      wget.nextWgetRequestHandle++;
      return handle;
    },
  },

  emscripten_async_wget__deps: ['$PATH_FS', '$wget', '$callUserCallback', '$Browser', '$withStackSave', '$stringToUTF8OnStack'],
  emscripten_async_wget__proxy: 'sync',
  emscripten_async_wget: function(url, file, onload, onerror) {
    {{{ runtimeKeepalivePush() }}}

    var _url = UTF8ToString(url);
    var _file = UTF8ToString(file);
    _file = PATH_FS.resolve(_file);
    function doCallback(callback) {
      if (callback) {
        {{{ runtimeKeepalivePop() }}}
        callUserCallback(function() {
          withStackSave(function() {
            {{{ makeDynCall('vi', 'callback') }}}(stringToUTF8OnStack(_file));
          });
        });
      }
    }
    var destinationDirectory = PATH.dirname(_file);
    FS.createPreloadedFile(
      destinationDirectory,
      PATH.basename(_file),
      _url, true, true,
      function() {
        doCallback(onload);
      },
      function() {
        doCallback(onerror);
      },
      false, // dontCreateFile
      false, // canOwn
      function() { // preFinish
        // if a file exists there, we overwrite it
        try {
          FS.unlink(_file);
        } catch (e) {}
        // if the destination directory does not yet exist, create it
        FS.mkdirTree(destinationDirectory);
      }
    );
  },

  emscripten_async_wget_data__deps: ['$asyncLoad', 'malloc', 'free', '$callUserCallback'],
  emscripten_async_wget_data__proxy: 'sync',
  emscripten_async_wget_data: function(url, arg, onload, onerror) {
    {{{ runtimeKeepalivePush() }}}
    asyncLoad(UTF8ToString(url), function(byteArray) {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        var buffer = _malloc(byteArray.length);
        HEAPU8.set(byteArray, buffer);
        {{{ makeDynCall('viii', 'onload') }}}(arg, buffer, byteArray.length);
        _free(buffer);
      });
    }, function() {
      if (onerror) {
        {{{ runtimeKeepalivePop() }}}
        callUserCallback(function() {
          {{{ makeDynCall('vi', 'onerror') }}}(arg);
        });
      }
    }, true /* no need for run dependency, this is async but will not do any prepare etc. step */ );
  },

  emscripten_async_wget2__deps: ['$PATH_FS', '$wget', '$withStackSave', '$stringToUTF8OnStack'],
  emscripten_async_wget2__proxy: 'sync',
  emscripten_async_wget2: function(url, file, request, param, arg, onload, onerror, onprogress) {
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
    http.onload = function http_onload(e) {
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
          withStackSave(function() {
            {{{ makeDynCall('viii', 'onload') }}}(handle, arg, stringToUTF8OnStack(_file));
          });
        }
      } else {
        if (onerror) {{{ makeDynCall('viii', 'onerror') }}}(handle, arg, http.status);
      }

      delete wget.wgetRequests[handle];
    };

    // ERROR
    http.onerror = function http_onerror(e) {
      {{{ runtimeKeepalivePop() }}}
      if (onerror) {{{ makeDynCall('viii', 'onerror') }}}(handle, arg, http.status);
      delete wget.wgetRequests[handle];
    };

    // PROGRESS
    http.onprogress = function http_onprogress(e) {
      if (e.lengthComputable || (e.lengthComputable === undefined && e.total != 0)) {
        var percentComplete = (e.loaded / e.total)*100;
        if (onprogress) {{{ makeDynCall('viii', 'onprogress') }}}(handle, arg, percentComplete);
      }
    };

    // ABORT
    http.onabort = function http_onabort(e) {
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
  emscripten_async_wget2_data: function(url, request, param, arg, free, onload, onerror, onprogress) {
    var _url = UTF8ToString(url);
    var _request = UTF8ToString(request);
    var _param = UTF8ToString(param);

    var http = new XMLHttpRequest();
    http.open(_request, _url, true);
    http.responseType = 'arraybuffer';

    var handle = wget.getNextWgetRequestHandle();

    function onerrorjs() {
      if (onerror) {
        withStackSave(() => {
          var statusText = 0;
          if (http.statusText) {
            statusText = stringToUTF8OnStack(http.statusText);
          }
          {{{ makeDynCall('viiii', 'onerror') }}}(handle, arg, http.status, statusText);
        });
      }
    }

    // LOAD
    http.onload = function http_onload(e) {
      if (http.status >= 200 && http.status < 300 || (http.status === 0 && _url.substr(0,4).toLowerCase() != "http")) {
        var byteArray = new Uint8Array(/** @type{ArrayBuffer} */(http.response));
        var buffer = _malloc(byteArray.length);
        HEAPU8.set(byteArray, buffer);
        if (onload) {{{ makeDynCall('viiii', 'onload') }}}(handle, arg, buffer, byteArray.length);
        if (free) _free(buffer);
      } else {
        onerrorjs();
      }
      delete wget.wgetRequests[handle];
    };

    // ERROR
    http.onerror = function http_onerror(e) {
      onerrorjs();
      delete wget.wgetRequests[handle];
    };

    // PROGRESS
    http.onprogress = function http_onprogress(e) {
      if (onprogress) {{{ makeDynCall('viiii', 'onprogress') }}}(handle, arg, e.loaded, e.lengthComputable || e.lengthComputable === undefined ? e.total : 0);
    };

    // ABORT
    http.onabort = function http_onabort(e) {
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
  emscripten_async_wget2_abort: function(handle) {
    var http = wget.wgetRequests[handle];
    if (http) {
      http.abort();
    }
  },
};

mergeInto(LibraryManager.library, LibraryWget);
