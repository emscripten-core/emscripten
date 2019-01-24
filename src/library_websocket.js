var LibraryWebSocket = {
  $WS: {
    sockets: [null],
    socketEvent: null
  },

  emscripten_websocket_get_ready_state__proxy: 'sync',
  emscripten_websocket_get_ready_state__sig: 'iii',
  emscripten_websocket_get_ready_state: function(socketId, readyState) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_get_ready_state(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

    HEAPU16[readyState>>1] = socket.readyState;
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_get_buffered_amount__proxy: 'sync',
  emscripten_websocket_get_buffered_amount__sig: 'iii',
  emscripten_websocket_get_buffered_amount: function(socketId, bufferedAmount) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_get_buffered_amount(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

    {{{ makeSetValue('bufferedAmount', '0', 'socket.bufferedAmount', 'i64') }}};
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_get_extensions__proxy: 'sync',
  emscripten_websocket_get_extensions__sig: 'iiii',
  emscripten_websocket_get_extensions: function(socketId, extensions, extensionsLength) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_get_extensions(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_get_extensions_length__proxy: 'sync',
  emscripten_websocket_get_extensions_length__sig: 'iii',
  emscripten_websocket_get_extensions_length: function(socketId, extensionsLength) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_get_extensions_length(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_get_protocol__proxy: 'sync',
  emscripten_websocket_get_protocol__sig: 'iiii',
  emscripten_websocket_get_protocol: function(socketId, protocol, protocolLength) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_get_protocol(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_get_protocol_length__proxy: 'sync',
  emscripten_websocket_get_protocol_length__sig: 'iii',
  emscripten_websocket_get_protocol_length: function(socketId, protocolLength) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_get_protocol_length(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_get_url__proxy: 'sync',
  emscripten_websocket_get_url__sig: 'iiii',
  emscripten_websocket_get_url: function(socketId, url, urlLength) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_get_url(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_get_url_length__proxy: 'sync',
  emscripten_websocket_get_url_length__sig: 'iii',
  emscripten_websocket_get_url_length: function(socketId, urlLength) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_get_url_length(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_set_onopen_callback_on_thread__proxy: 'sync',
  emscripten_websocket_set_onopen_callback_on_thread__sig: 'iiiii',
  emscripten_websocket_set_onopen_callback_on_thread: function(socketId, userData, callbackFunc, thread) {
// TODO:
//    if (thread == {{{ cDefine('EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD') }}} ||
//      (thread == _pthread_self()) return emscripten_websocket_set_onopen_callback_on_calling_thread(socketId, userData, callbackFunc);

    if (!WS.socketEvent) WS.socketEvent = _malloc(1024); // TODO: sizeof(EmscriptenWebSocketCloseEvent), which is the largest event struct

    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_set_onopen_callback(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

#if WEBSOCKET_DEBUG
    console.error('emscripten_websocket_set_onopen_callback(socketId='+socketId+',userData='+userData+',callbackFunc='+callbackFunc+')');
#endif
    socket.onopen = function(e) {
#if WEBSOCKET_DEBUG
      console.error('websocket event "open": socketId='+socketId+',userData='+userData+',callbackFunc='+callbackFunc+')');
#endif
      HEAPU32[WS.socketEvent>>2] = socketId;
      {{{ makeDynCall('iiii') }}}(callbackFunc, 0/*TODO*/, WS.socketEvent, userData);
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_set_onerror_callback_on_thread__proxy: 'sync',
  emscripten_websocket_set_onerror_callback_on_thread__sig: 'iiiii',
  emscripten_websocket_set_onerror_callback_on_thread: function(socketId, userData, callbackFunc, thread) {
    if (!WS.socketEvent) WS.socketEvent = _malloc(1024); // TODO: sizeof(EmscriptenWebSocketCloseEvent), which is the largest event struct

    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_set_onerror_callback(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

#if WEBSOCKET_DEBUG
    console.error('emscripten_websocket_set_onerror_callback(socketId='+socketId+',userData='+userData+',callbackFunc='+callbackFunc+')');
#endif
    socket.onerror = function(e) {
#if WEBSOCKET_DEBUG
      console.error('websocket event "error": socketId='+socketId+',userData='+userData+',callbackFunc='+callbackFunc+')');
#endif
      HEAPU32[WS.socketEvent>>2] = socketId;
      {{{ makeDynCall('iiii') }}}(callbackFunc, 0/*TODO*/, WS.socketEvent, userData);
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_set_onclose_callback_on_thread__proxy: 'sync',
  emscripten_websocket_set_onclose_callback_on_thread__sig: 'iiiii',
  emscripten_websocket_set_onclose_callback_on_thread: function(socketId, userData, callbackFunc, thread) {
    if (!WS.socketEvent) WS.socketEvent = _malloc(1024); // TODO: sizeof(EmscriptenWebSocketCloseEvent), which is the largest event struct

    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_set_onclose_callback(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

#if WEBSOCKET_DEBUG
    console.error('emscripten_websocket_set_onclose_callback(socketId='+socketId+',userData='+userData+',callbackFunc='+callbackFunc+')');
#endif
    socket.onclose = function(e) {
#if WEBSOCKET_DEBUG
      console.error('websocket event "close": socketId='+socketId+',userData='+userData+',callbackFunc='+callbackFunc+')');
#endif
      HEAPU32[WS.socketEvent>>2] = socketId;
      HEAPU32[(WS.socketEvent+4)>>2] = e.wasClean;
      HEAPU32[(WS.socketEvent+8)>>2] = e.code;
      stringToUTF8(e.reason, HEAPU32[(WS.socketEvent+10)>>2], 512);
      {{{ makeDynCall('iiii') }}}(callbackFunc, 0/*TODO*/, WS.socketEvent, userData);
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_set_onmessage_callback_on_thread__proxy: 'sync',
  emscripten_websocket_set_onmessage_callback_on_thread__sig: 'iiiii',
  emscripten_websocket_set_onmessage_callback_on_thread: function(socketId, userData, callbackFunc, thread) {
    if (!WS.socketEvent) WS.socketEvent = _malloc(1024); // TODO: sizeof(EmscriptenWebSocketCloseEvent), which is the largest event struct

    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_set_onmessage_callback(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

#if WEBSOCKET_DEBUG
    console.error('emscripten_websocket_set_onmessage_callback(socketId='+socketId+',userData='+userData+',callbackFunc='+callbackFunc+')');
#endif
    socket.onmessage = function(e) {
#if WEBSOCKET_DEBUG == 2
      console.error('websocket event "message": socketId='+socketId+',userData='+userData+',callbackFunc='+callbackFunc+')');
#endif
      HEAPU32[WS.socketEvent>>2] = socketId;
      if (typeof e.data === 'string') {
        var len = lengthBytesUTF8(e.data)+1;
        var buf = _malloc(len);
        stringToUTF8(e.data, buf, len);
#if WEBSOCKET_DEBUG
        var s = (e.data.length < 256) ? e.data : (e.data.substr(0, 256) + ' (' + (e.data.length-256) + ' more characters)');
        console.error('WebSocket onmessage, received data: "' + e.data + '", ' + e.data.length + ' chars, ' + len + ' bytes encoded as UTF-8: "' + s + '"');
#endif
        HEAPU32[(WS.socketEvent+12)>>2] = 1; // text data
      } else {
        var len = e.data.byteLength;
        var buf = _malloc(len);
        HEAP8.set(new Uint8Array(e.data), buf);
#if WEBSOCKET_DEBUG
        var s = 'WebSocket onmessage, received data: ' + len + ' bytes of binary:';
        for(var i = 0; i < Math.min(len, 256); ++i) s += ' ' + HEAPU8[buf+i].toString(16);
        s += ', "';
        for(var i = 0; i < Math.min(len, 256); ++i) s += (HEAPU8[buf+i] >= 32 && HEAPU8[buf+i] <= 127) ? String.fromCharCode(HEAPU8[buf+i]) : '\uFFFD';
        s += '"';
        if (len > 256) s + ' ... (' + (len - 256) + ' more bytes)';

        console.error(s);
#endif
        HEAPU32[(WS.socketEvent+12)>>2] = 0; // binary data
      }
      HEAPU32[(WS.socketEvent+4)>>2] = buf;
      HEAPU32[(WS.socketEvent+8)>>2] = len;
      {{{ makeDynCall('iiii') }}}(callbackFunc, 0/*TODO*/, WS.socketEvent, userData);
      _free(buf);
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_new__deps: ['$WS'],
  emscripten_websocket_new__proxy: 'sync',
  emscripten_websocket_new__sig: 'ii',
  emscripten_websocket_new: function(createAttributes) {
    if (typeof WebSocket === 'undefined') {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_new(): WebSocket API is not supported by current browser)');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    if (!createAttributes) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_new(): Missing required "createAttributes" function parameter!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
    }

    var url = UTF8ToString(HEAP32[createAttributes>>2]);

    // TODO: protocols
    // TODO: createOnMainThread
    var socket = new WebSocket(url);
    socket.binaryType = 'arraybuffer';
    // TODO: While strictly not necessary, this ID would be good to be unique across all threads to avoid confusion.
    var socketId = WS.sockets.length;
    WS.sockets[socketId] = socket;

#if WEBSOCKET_DEBUG
    console.error('emscripten_websocket_new(url='+url+'): created socket ID ' + socketId + ')');
#endif
    return socketId;
  },

  emscripten_websocket_send_utf8_text__proxy: 'sync',
  emscripten_websocket_send_utf8_text__sig: 'iii',
  emscripten_websocket_send_utf8_text: function(socketId, textData) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_send_utf8_text(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

    var str = UTF8ToString(textData);
#if WEBSOCKET_DEBUG == 2
    console.error('emscripten_websocket_send_utf8_text(socketId='+socketId+',textData='+ str.length + ' chars, "' + str +'")');
#else
#if WEBSOCKET_DEBUG
    console.error('emscripten_websocket_send_utf8_text(socketId='+socketId+',textData='+ str.length + ' chars, "' + ((str.length > 8) ? (str.substring(0,8) + '...') : str) + '")');
#endif
#endif
    socket.send(str);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_send_binary__proxy: 'sync',
  emscripten_websocket_send_binary__sig: 'iiii',
  emscripten_websocket_send_binary: function(socketId, binaryData, dataLength) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_send_binary(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

#if WEBSOCKET_DEBUG
    var s = 'data: ' + dataLength + ' bytes of binary:';
    for(var i = 0; i < Math.min(dataLength, 256); ++i) s += ' '+ HEAPU8[binaryData+i].toString(16);
    s += ', "';
    for(var i = 0; i < Math.min(dataLength, 256); ++i) s += (HEAPU8[binaryData+i] >= 32 && HEAPU8[binaryData+i] <= 127) ? String.fromCharCode(HEAPU8[binaryData+i]) : '\uFFFD';
    s += '"';
    if (dataLength > 256) s + ' ... (' + (dataLength - 256) + ' more bytes)';

    console.error('emscripten_websocket_send_binary(socketId='+socketId+',binaryData='+binaryData+ ',dataLength='+dataLength+'), ' + s);
#endif
#if USE_PTHREADS
    // TODO: This is temporary to cast a shared Uint8Array to a non-shared Uint8Array. This could be removed if WebSocket API is improved
    // to allow passing in views to SharedArrayBuffers
    socket.send(new Uint8Array({{{ makeHEAPView('U8', 'binaryData', 'binaryData+dataLength') }}}));
#else
    socket.send({{{ makeHEAPView('U8', 'binaryData', 'binaryData+dataLength') }}});
#endif
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_close__proxy: 'sync',
  emscripten_websocket_close__sig: 'iiii',
  emscripten_websocket_close: function(socketId, code, reason) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_close(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

    var reasonStr = reason ? UTF8ToString(reason) : undefined;
#if WEBSOCKET_DEBUG
    console.error('emscripten_websocket_close(socketId='+socketId+',code='+code+',reason='+reasonStr+')');
#endif
    if (!code) code = undefined;
    socket.close(code, reasonStr);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_delete__proxy: 'sync',
  emscripten_websocket_delete__sig: 'ii',
  emscripten_websocket_delete: function(socketId) {
    var socket = WS.sockets[socketId];
    if (!socket) {
#if WEBSOCKET_DEBUG
      console.error('emscripten_websocket_delete(): Invalid socket ID ' + socketId + ' specified!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

#if WEBSOCKET_DEBUG
    console.error('emscripten_websocket_delete(socketId='+socketId+')');
#endif
    socket.onopen = socket.onerror = socket.onclose = socket.onmessage = null;
    socket = null;
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_websocket_is_supported__proxy: 'sync',
  emscripten_websocket_is_supported__sig: 'i',
  emscripten_websocket_is_supported: function() {
    return typeof WebSocket !== 'undefined';
  },

  emscripten_websocket_deinitialize__proxy: 'sync',
  emscripten_websocket_deinitialize__sig: 'v',
  emscripten_websocket_deinitialize__deps: ['emscripten_websocket_delete'],
  emscripten_websocket_deinitialize: function() {
#if WEBSOCKET_DEBUG
    console.error('emscripten_websocket_deinitialize()');
#endif
    for(var i in WS.sockets) {
      var socket = WS.sockets[i];
      if (socket) {
        socket.close();
        _emscripten_websocket_delete(i);
      }
    }
  }
}

mergeInto(LibraryManager.library, LibraryWebSocket);
