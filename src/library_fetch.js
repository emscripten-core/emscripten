var LibraryFetch = {
  $Fetch: {
    xhrs: [],
  },

  emscripten_fetch__deps: ['$Fetch'],
  emscripten_fetch: function(fetch_attr, url) {
    Module['noExitRuntime'] = true;

    var xhr = new XMLHttpRequest();
    var requestType = Pointer_stringify(fetch_attr);
    var url_ = Pointer_stringify(url);
    xhr.responseType = 'arraybuffer';
    console.log('xhr.open(requestType=' + requestType + ', url: ' + url_ +')');
    xhr.open(requestType, url_, true);
    Fetch.xhrs.push(xhr);
    var fetch = _malloc(1000 /*TODO:structs_info.sizeof(fetch)*/);
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
      xhrRequest = xhr;
      /*
      console.log('xhr.response.byteLength: ' + xhr.response.byteLength);
      var ta32 = new Uint32Array(xhr.response, 0, xhr.response.byteLength >> 2);
      console.log('ta32.length: ' + ta32.length);
      for (var i = 0; i < ta32.length; ++i) {
        {{{ makeSetValue('ptr', 'i', 'ta32[i]', 'i32')}}};
      }
      if ((xhr.response.byteLength & 3) != 0) {

      }
      */
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
    xhr.onprogress = function(e) {
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
  }
};

mergeInto(LibraryManager.library, LibraryFetch);
