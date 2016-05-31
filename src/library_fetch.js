var LibraryFetch = {
  $Fetch: {
    xhrs: [],
  },

  emscripten_fetch__deps: ['$Fetch'],
  emscripten_fetch: function(fetch_attr, url) {
    var xhr = new XMLHttpRequest();
    var requestType = Pointer_stringify(fetch_attr);
    var url_ = Pointer_stringify(url);
    xhr.responseType = 'arraybuffer';
    console.log('xhr.open(requestType=' + requestType + ', url: ' + url_ +')');
    xhr.open(requestType, url_, true);
    Fetch.xhrs.push(xhr);
    var id = Fetch.xhrs.length;
    var data = null; // TODO: Support user to pass data to request.
    // TODO: Support specifying custom headers to the request.

    var onsuccess = {{{ makeGetValue('fetch_attr', 36/*TODO:structs_info.jsonify this*/, 'i32') }}};
    var onerror = {{{ makeGetValue('fetch_attr', 40/*TODO:structs_info.jsonify this*/, 'i32') }}};
    var onprogress = {{{ makeGetValue('fetch_attr', 44/*TODO:structs_info.jsonify this*/, 'i32') }}};

    xhr.onload = function(e) {
      if (onsuccess) Runtime.dynCall('vi', onsuccess, [0]);
    }
    xhr.onerror = function(e) {
      if (onerror) Runtime.dynCall('vi', onerror, [0]); // TODO: pass error reason
    }
    xhr.onprogress = function(e) {
      if (onprogress) Runtime.dynCall('vi', onprogress, [0]);
    }
    console.log('xhr.send(data=' + data + ')');
    try {
      xhr.send(data);
    } catch(e) {
      if (onerror) Runtime.dynCall('vi', onerror, [0]); // TODO: pass error reason
    }
  }
};

mergeInto(LibraryManager.library, LibraryFetch);
