/**
 * @license
 * Copyright 2016 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var Fetch = {
  // HandleAllocator for the fetch request object
  // requestHandles: undefined,

  init() {
    Fetch.requestHandles = new HandleAllocator();
  }
};

function startFetch(fetchHandle) {
  // Avoid shutting down the runtime since we want to wait for the async
  // response.
  {{{ runtimeKeepalivePush() }}}

  var url = {{{ makeGetValue('fetchHandle', C_STRUCTS.emscripten_fetch_t.url, '*') }}};
  if (!url) {
    onerror(fetchHandle, undefined, 'no url specified!');
    {{{ runtimeKeepalivePop() }}}
    return;
  }
  var url_ = UTF8ToString(url);

  var fetch_attr = fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var requestMethod = UTF8ToString(fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.requestMethod }}});
  requestMethod ||= 'GET';
  var timeoutMsecs = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.timeoutMSecs, 'u32') }}};
  var userName = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.userName, '*') }}};
  var password = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.password, '*') }}};
  var requestHeaders = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.requestHeaders, '*') }}};
  var overriddenMimeType = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.overriddenMimeType, '*') }}};
  var dataPtr = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.requestData, '*') }}};
  var dataLength = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.requestDataSize, '*') }}};

  var fetchAttributes = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.attributes, 'u32') }}};
  var fetchAttrLoadToMemory = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_LOAD_TO_MEMORY }}});
  var fetchAttrStreamData = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_STREAM_DATA }}});
  var fetchAttrPersistFile = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_PERSIST_FILE }}});
  var fetchAttrReplaceFile = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_REPLACE  }}});
  var fetchAttrNoDownloadFile = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_NO_DOWNLOAD  }}});
  
  var fetchAttrHighPriority = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_HIGH_PRIORITY  }}});
  var fetchAttrLowPriority = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_LOW_PRIORITY  }}});

  var headers = new Headers();
  if (requestHeaders) {
    for (;;) {
      var key = {{{ makeGetValue('requestHeaders', 0, '*') }}};
      if (!key) break;
      var value = {{{ makeGetValue('requestHeaders', POINTER_SIZE, '*') }}};
      if (!value) break;
      requestHeaders += {{{ 2 * POINTER_SIZE }}};
      var keyStr = UTF8ToString(key);
      var valueStr = UTF8ToString(value);
      
      headers.append(keyStr, valueStr);
    }
  }

  if (overriddenMimeType) {
    var overriddenMimeTypeStr = UTF8ToString(overriddenMimeType);
    headers.append("Content-Type", overriddenMimeTypeStr);
  }

  if(userName && password) {
    var userNameStr = userName ? UTF8ToString(userName) : undefined;
    var passwordStr = password ? UTF8ToString(password) : undefined;

    headers.set('Authorization', 'Basic ' + btoa(userNameStr + ":" + passwordStr));
  }

  var body = (dataPtr && dataLength) ? HEAPU8.slice(dataPtr, dataPtr + dataLength) : null;

  const controller = new AbortController();
  const timeoutId = timeoutMsecs > 0 ? setTimeout(() => controller.abort(), timeoutMsecs) : -1;

  HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 0; // XMLHttpRequest UNSENT, kept for compatibility

  var onsuccess = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.onsuccess }}} >> 2];
  var onerror = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.onerror }}} >> 2];
  var onprogress = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.onprogress }}} >> 2];
  var onreadystatechange = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.onreadystatechange }}} >> 2];

  var fetchData = {};
  fetchData.controller = controller;
  fetchData.offset = 0;
  var id = Fetch.requestHandles.allocate(fetchData);
  {{{ makeSetValue('fetchHandle', C_STRUCTS.emscripten_fetch_t.id, 'id', 'u32') }}};

  fetchData.promise = fetch(url_, {
    method: requestMethod,
    headers: headers,
    body: body,
    cache: fetchAttrPersistFile ? "default": (fetchAttrReplaceFile ? "reload" : (fetchAttrNoDownloadFile ? "only-if-cached" : "no-store")),
    redirect: "follow",
    priority: fetchAttrHighPriority ? "high": (fetchAttrLowPriority ? "low" : "auto"), 
    signal: controller.signal
  })
    .then((response) => {
      // check if the request was aborted by user and don't try to call back
      if (!Fetch.requestHandles.has(id)) {
        {{{ runtimeKeepalivePop() }}}
        return;
      }

      HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 2; // XMLHttpRequest HEADERS_RECEIVED, kept for compatibility
      HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = response.status;
      if (response.statusText) {
        stringToUTF8(response.statusText, fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      }
      fetchData.headers = "";
      response.headers.forEach(function(value, name) {
        if(fetchData.headers.length > 0)
        {
          fetchData.headers += "\r\n";
        }
        fetchData.headers += name;
        fetchData.headers += ": ";
        fetchData.headers += value;
      });

      const totalLen = Number(response.headers.get("content-length"));
      writeI53ToI64(fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, totalLen);

      if (onreadystatechange) {{{ makeDynCall('vp', 'onreadystatechange') }}}(fetchHandle);

      if(fetchAttrStreamData) {
        const reader = response.body.getReader();
        reader.read().then(function pump({ done, value }) {
          // check if the request was aborted by user and don't try to call back
          if (!Fetch.requestHandles.has(id)) {
            {{{ runtimeKeepalivePop() }}}
            return;
          }

          if (done) {
            // Finished receiving chunks, notify listener

            HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // XMLHttpRequest DONE, kept for compatibility
            if (onreadystatechange) {{{ makeDynCall('vp', 'onreadystatechange') }}}(fetchHandle);

            {{{ runtimeKeepalivePop() }}}
            Fetch.requestHandles.free(id);
            if (onsuccess) {{{ makeDynCall('vp', 'onsuccess') }}}(fetchHandle);

            return;
          }
          // Received a streamed chunk

          // Make sure we can't timeout now that we are receiving responses
          if(timeoutId != -1) clearTimeout(timeoutId);

          const newReadyState = 3; // XMLHttpRequest LOADING, kept for compatibility
          if(HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] != newReadyState) {
            HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = newReadyState;
            if (onreadystatechange) {{{ makeDynCall('vp', 'onreadystatechange') }}}(fetchHandle);
          }

          var ptrLen = 0;
          var ptr = 0;
          if (value) {
            ptrLen = value.byteLength;

            // Allocate byte data in Emscripten heap for the streamed memory block (freed immediately after onprogress call)
            ptr = _malloc(ptrLen);
            HEAPU8.set(value, ptr);
          }
          HEAPU32[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] = ptr;
          writeI53ToI64(fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, ptrLen);
          writeI53ToI64(fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, fetchData.offset);
          if (onprogress) {{{ makeDynCall('vp', 'onprogress') }}}(fetchHandle);
          if (ptr) {
            _free(ptr);
            HEAPU32[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] = 0;
          }

          fetchData.offset += ptrLen;

          return reader.read().then(pump);
        });
      } else { // Not streamed path
        // Make sure we can't timeout now that we are receiving responses
        if(timeoutId != -1) clearTimeout(timeoutId);

        response.blob().then(function(responseBlob) {
          
          // Store response data
          if (responseBlob && fetchAttrLoadToMemory && HEAPU32[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] === 0) {
            responseBlob.arrayBuffer().then(function(responseBuffer) {
              HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // XMLHttpRequest DONE, kept for compatibility
              if (onreadystatechange) {{{ makeDynCall('vp', 'onreadystatechange') }}}(fetchHandle);

              var ptrLen = responseBuffer.byteLength;

              // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
              // freed when emscripten_fetch_close() is called.
              var ptr = _malloc(ptrLen);
              HEAPU8.set(new Uint8Array(responseBuffer), ptr);

              HEAPU32[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] = ptr;
              writeI53ToI64(fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, ptrLen);
              writeI53ToI64(fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, 0);
              writeI53ToI64(fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, ptrLen);

              {{{ runtimeKeepalivePop() }}}
              Fetch.requestHandles.free(id);
              if (onsuccess) {{{ makeDynCall('vp', 'onsuccess') }}}(fetchHandle);
            });
          }
          else {
            HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // XMLHttpRequest DONE, kept for compatibility
            if (onreadystatechange) {{{ makeDynCall('vp', 'onreadystatechange') }}}(fetchHandle);

            HEAPU32[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] = 0;
            writeI53ToI64(fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, 0);
            writeI53ToI64(fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, 0);
            writeI53ToI64(fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, 0);

            {{{ runtimeKeepalivePop() }}}
            Fetch.requestHandles.free(id);
            if (onsuccess) {{{ makeDynCall('vp', 'onsuccess') }}}(fetchHandle);
          }
        });
      }
    })
    .catch((err) => {
      HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // XMLHttpRequest DONE, kept for compatibility
      if (onreadystatechange) {{{ makeDynCall('vp', 'onreadystatechange') }}}(fetchHandle);

      Fetch.requestHandles.free(id);
      {{{ runtimeKeepalivePop() }}}
      if (onerror) {{{ makeDynCall('vp', 'onerror') }}}(fetchHandle);
    });

    HEAPU16[fetchHandle + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 1; // XMLHttpRequest OPENED, kept for compatibility

  // TODO:
  // Check whether supporting fetchAttrSynchronous still makes sense
}

function fetchGetResponseHeadersLength(id) {
  if (Fetch.requestHandles.has(id)) {
    var fetchObject = Fetch.requestHandles.get(id);
    return lengthBytesUTF8(fetchObject.headers) + 1;
  }
}

function fetchGetResponseHeaders(id, dst, dstSizeBytes) {
  if (Fetch.requestHandles.has(id)) {
    var fetchObject = Fetch.requestHandles.get(id);
    var responseHeaders = fetchObject.headers;
    var lengthBytes = lengthBytesUTF8(responseHeaders) + 1;
    stringToUTF8(responseHeaders, dst, dstSizeBytes);
    return Math.min(lengthBytes, dstSizeBytes);
  }
}

// Abort the fetch request and delete its JS object, allowing it to be garbage collected.
function fetchFree(id) {
  if (Fetch.requestHandles.has(id)) {
    var fetchObject = Fetch.requestHandles.get(id);
    Fetch.requestHandles.free(id);
    // check if fetch is still in progress and should be aborted
    if (fetchObject.readyState > 0 && fetchObject.readyState < 4) {
      fetchObject.controller.abort();
    }
  }
}
