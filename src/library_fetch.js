var LibraryFetch = {
  $Fetch: {
    xhrs: [],
  },

  _emscripten_fetch_xhr: function(fetch_attr, url) {
    var xhr = new XMLHttpRequest();
    var requestMethod = Pointer_stringify(fetch_attr);
    var url_ = Pointer_stringify(url);
    xhr.responseType = 'arraybuffer';
    xhr.timeout = {{{ makeGetValue('fetch_attr', 52/*TODO*/, 'i32') }}};
    xhr.withCredentials = !!({{{ makeGetValue('fetch_attr', 56/*TODO*/, 'i32') }}});
    var synchronousRequest = !!({{{ makeGetValue('fetch_attr', 60/*TODO*/, 'i32') }}});
    var destinationPath = {{{ makeGetValue('fetch_attr', 64/*TODO*/, 'i32') }}};
    var userName = {{{ makeGetValue('fetch_attr', 68/*TODO*/, 'i32') }}};
    var userNameStr = userName ? Pointer_stringify(userName) : undefined;
    var password = {{{ makeGetValue('fetch_attr', 72/*TODO*/, 'i32') }}};
    var passwordStr = password ? Pointer_stringify(password) : undefined;
    var requestHeaders = {{{ makeGetValue('fetch_attr', 76/*TODO*/, 'i32') }}};
    var overriddenMimeType = {{{ makeGetValue('fetch_attr', 80/*TODO*/, 'i32') }}};
    var overriddenMimeTypeStr = overriddenMimeType ? Pointer_stringify(overriddenMimeType) : undefined;
#if FETCH_DEBUG
    console.log('fetch: xhr.timeout: ' + xhr.timeout + ', xhr.withCredentials: ' + xhr.withCredentials);
    console.log('fetch: xhr.open(requestMethod="' + requestMethod + '", url: "' + url_ +'", userName: ' + userNameStr + ', password: ' + passwordStr + ');');
#endif
    xhr.open(requestMethod, url_, !synchronousRequest, userNameStr, passwordStr);

    if (overriddenMimeType) {
#if FETCH_DEBUG
      console.log('fetch: xhr.overrideMimeType("' + overriddenMimeTypeStr + '");');
#endif
      xhr.overrideMimeType(overriddenMimeTypeStr);
    }
    if (requestHeaders) {
      for(;;) {
        var key = {{{ makeGetValue('requestHeaders', 0, 'i32') }}};
        if (!key) break;
        var value = {{{ makeGetValue('requestHeaders', 4, 'i32') }}};
        if (!value) break;
        requestHeaders += 8;
        var keyStr = Pointer_stringify(key);
        var valueStr = Pointer_stringify(value);
#if FETCH_DEBUG
        console.log('fetch: xhr.setRequestHeader("' + keyStr + '", "' + valueStr + '");');
#endif
        xhr.setRequestHeader(keyStr, valueStr);
      }
    }
    Fetch.xhrs.push(xhr);
    var fetch = _malloc(1000 /*TODO:structs_info.sizeof(fetch)*/);
    _memset(fetch, 0, 1000/*TODO:structs_info.sizeof(fetch)*/);
    var urlString = _malloc(url_.length /*TODO*/);
    {{{ makeSetValue('fetch', 8/*TODO:jsonify*/, 'urlString', 'i32')}}};
    writeStringToMemory(url_, urlString);
    var id = Fetch.xhrs.length;
    {{{ makeSetValue('fetch', 0/*TODO:jsonify*/, 'id', 'i32')}}};
    var userData = {{{ makeGetValue('fetch_attr', 32/*TODO:structs_info.jsonify this*/, 'i32') }}};
    {{{ makeSetValue('fetch', 4/*TODO:jsonify*/, 'userData', 'i32')}}};
    var data = null; // TODO: Support user to pass data to request.
    // TODO: Support specifying custom headers to the request.

    var onsuccess = {{{ makeGetValue('fetch_attr', 36/*TODO:structs_info.jsonify this*/, 'i32') }}};
    var onerror = {{{ makeGetValue('fetch_attr', 40/*TODO:structs_info.jsonify this*/, 'i32') }}};
    var onprogress = {{{ makeGetValue('fetch_attr', 44/*TODO:structs_info.jsonify this*/, 'i32') }}};

    xhr.onload = function(e) {
      var len = xhr.response.byteLength;
      var ptr = _malloc(len); // TODO: make emscripten_fetch_close() free() this data.
      HEAPU8.set(new Uint8Array(xhr.response), ptr);
      {{{ makeSetValue('fetch', 12/*TODO:jsonify*/, 'ptr', 'i64')}}};
      {{{ makeSetValue('fetch', 16/*TODO:jsonify*/, 'len', 'i64')}}};
      {{{ makeSetValue('fetch', 24/*TODO:jsonify*/, '0', 'i64')}}};
      {{{ makeSetValue('fetch', 32/*TODO:jsonify*/, 'len', 'i64')}}};
      if (xhr.status == 200 || xhr.status == 0) {
        if (onsuccess) Runtime.dynCall('vi', onsuccess, [fetch]);
      } else {
        if (onerror) Runtime.dynCall('vi', onerror, [fetch]); // TODO: pass error reason
      }
    }
    xhr.onerror = function(e) {
      if (onerror) Runtime.dynCall('vi', onerror, [fetch]); // TODO: pass error reason
    }
    xhr.ontimeout = function(e) {
      if (onerror) Runtime.dynCall('vi', onerror, [fetch]); // TODO: pass error reason      
    }
    xhr.onprogress = function(e) {
      Module.print(xhr.status);
      Module.print(xhr.readyState);
      Module.print(xhr.statusText);
      {{{ makeSetValue('fetch', 16/*TODO:jsonify*/, '0', 'i64')}}};
      {{{ makeSetValue('fetch', 24/*TODO:jsonify*/, 'e.loaded', 'i64')}}};
      {{{ makeSetValue('fetch', 32/*TODO:jsonify*/, 'e.total', 'i64')}}};
      if (onprogress) Runtime.dynCall('vi', onprogress, [fetch]);
    }
    console.log('xhr.send(data=' + data + ')');
    try {
      xhr.send(data);
    } catch(e) {
      if (onerror) Runtime.dynCall('vi', onerror, [fetch]); // TODO: pass error reason
    }
  },

  emscripten_fetch__deps: ['$Fetch', '_emscripten_fetch_xhr'],
  emscripten_fetch: function(fetch_attr, url) {
    Module['noExitRuntime'] = true;

    var fetchAttributes = {{{ makeGetValue('fetch_attr', 48/*TODO*/, 'i32') }}};
    var fetchAttrLoadToMemory = !!(fetchAttributes & 1/*EMSCRIPTEN_FETCH_LOAD_TO_MEMORY*/);
    var fetchAttrStreamData = !!(fetchAttributes & 2/*EMSCRIPTEN_FETCH_STREAM_DATA*/);
    var fetchAttrPersistFile = !!(fetchAttributes & 4/*EMSCRIPTEN_FETCH_PERSIST_FILE*/);
    var fetchAttrAppend = !!(fetchAttributes & 8/*EMSCRIPTEN_FETCH_APPEND*/);
    var fetchAttrReplace = !!(fetchAttributes & 16/*EMSCRIPTEN_FETCH_REPLACE*/);
    var fetchAttrNoDownload = !!(fetchAttributes & 32/*EMSCRIPTEN_FETCH_NO_DOWNLOAD*/);

    if (!fetchAttrNoDownload) {
      __emscripten_fetch_xhr(fetch_attr, url);
    } else {
      throw 'not implemented!';
    }
  }
};

mergeInto(LibraryManager.library, LibraryFetch);
