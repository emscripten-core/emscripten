/**
 * @license
 * Copyright 2018 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryWebSocket = {
  $webSockets__deps: ['$HandleAllocator'],
  $webSockets: "new HandleAllocator();",

  $WS__deps: ['$webSockets', 'malloc'],
  $WS: {
    socketEvent: null,
    getSocket(socketId) {
      if (!webSockets.has(socketId)) {
        return 0;
      }
      return webSockets.get(socketId);
    },
    getSocketEvent(socketId) {
      // Singleton event pointer.  Use EmscriptenWebSocketCloseEvent, which is
      // the largest event struct
      this.socketEvent ||= _malloc({{{ C_STRUCTS.EmscriptenWebSocketCloseEvent.__size__ }}});
      {{{ makeSetValue('this.socketEvent', 0, 'socketId', 'u32') }}};
      return this.socketEvent;
    },
  },

  emscripten_websocket_get_ready_state__deps: ['$WS'],
  emscripten_websocket_get_ready_state__proxy: 'sync',
  emscripten_websocket_get_ready_state: (socketId, readyState) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_get_ready_state(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

    {{{ makeSetValue('readyState', '0', 'socket.readyState', 'i16') }}};
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_get_buffered_amount__deps: ['$WS'],
  emscripten_websocket_get_buffered_amount__proxy: 'sync',
  emscripten_websocket_get_buffered_amount: (socketId, bufferedAmount) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_get_buffered_amount(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

    {{{ makeSetValue('bufferedAmount', '0', 'socket.bufferedAmount', '*') }}};
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_get_extensions__deps: ['$WS', '$stringToUTF8'],
  emscripten_websocket_get_extensions__proxy: 'sync',
  emscripten_websocket_get_extensions: (socketId, extensions, extensionsLength) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_get_extensions(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }
    if (!extensions) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    stringToUTF8(socket.extensions, extensions, extensionsLength);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_get_extensions_length__deps: ['$WS'],
  emscripten_websocket_get_extensions_length__proxy: 'sync',
  emscripten_websocket_get_extensions_length: (socketId, extensionsLength) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_get_extensions_length(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }
    if (!extensionsLength) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    {{{ makeSetValue('extensionsLength', '0', 'lengthBytesUTF8(socket.extensions)+1', 'i32') }}};
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_get_protocol__deps: ['$WS', '$stringToUTF8'],
  emscripten_websocket_get_protocol__proxy: 'sync',
  emscripten_websocket_get_protocol: (socketId, protocol, protocolLength) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_get_protocol(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }
    if (!protocol) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    stringToUTF8(socket.protocol, protocol, protocolLength);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_get_protocol_length__deps: ['$WS'],
  emscripten_websocket_get_protocol_length__proxy: 'sync',
  emscripten_websocket_get_protocol_length: (socketId, protocolLength) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_get_protocol_length(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }
    if (!protocolLength) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    {{{ makeSetValue('protocolLength', '0', 'lengthBytesUTF8(socket.protocol)+1', 'i32') }}};
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_get_url__deps: ['$WS', '$stringToUTF8'],
  emscripten_websocket_get_url__proxy: 'sync',
  emscripten_websocket_get_url: (socketId, url, urlLength) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_get_url(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }
    if (!url) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    stringToUTF8(socket.url, url, urlLength);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_get_url_length__deps: ['$WS'],
  emscripten_websocket_get_url_length__proxy: 'sync',
  emscripten_websocket_get_url_length: (socketId, urlLength) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_get_url_length(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }
    if (!urlLength) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    {{{ makeSetValue('urlLength', '0', 'lengthBytesUTF8(socket.url)+1', 'i32') }}};
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_set_onopen_callback_on_thread__deps: ['$WS'],
  emscripten_websocket_set_onopen_callback_on_thread__proxy: 'sync',
  emscripten_websocket_set_onopen_callback_on_thread: (socketId, userData, callbackFunc, thread) => {
// TODO:
//    if (thread == {{{ cDefs.EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD }}} ||
//      (thread == _pthread_self()) return emscripten_websocket_set_onopen_callback_on_calling_thread(socketId, userData, callbackFunc);
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_set_onopen_callback(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

#if WEBSOCKET_DEBUG
    dbg(`emscripten_websocket_set_onopen_callback(socketId=${socketId},userData=${userData},callbackFunc='+callbackFunc})`);
#endif
    socket.onopen = function(e) {
#if WEBSOCKET_DEBUG
      dbg(`websocket event "open": socketId=${socketId},userData=${userData},callbackFunc=${callbackFunc})`);
#endif
      var eventPtr = WS.getSocketEvent(socketId);
      {{{ makeDynCall('iipp', 'callbackFunc') }}}(0/*TODO*/, eventPtr, userData);
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_set_onerror_callback_on_thread__deps: ['$WS'],
  emscripten_websocket_set_onerror_callback_on_thread__proxy: 'sync',
  emscripten_websocket_set_onerror_callback_on_thread: (socketId, userData, callbackFunc, thread) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_set_onerror_callback(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

#if WEBSOCKET_DEBUG
    dbg(`emscripten_websocket_set_onerror_callback(socketId=${socketId},userData=${userData},callbackFunc=${callbackFunc})`);
#endif
    socket.onerror = function(e) {
#if WEBSOCKET_DEBUG
      dbg(`websocket event "error": socketId=${socketId},userData=${userData},callbackFunc=${callbackFunc})`);
#endif
      var eventPtr = WS.getSocketEvent(socketId);
      {{{ makeDynCall('iipp', 'callbackFunc') }}}(0/*TODO*/, eventPtr, userData);
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_set_onclose_callback_on_thread__deps: ['$WS', '$stringToUTF8'],
  emscripten_websocket_set_onclose_callback_on_thread__proxy: 'sync',
  emscripten_websocket_set_onclose_callback_on_thread: (socketId, userData, callbackFunc, thread) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_set_onclose_callback(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

#if WEBSOCKET_DEBUG
    dbg(`emscripten_websocket_set_onclose_callback(socketId=${socketId},userData=${userData},callbackFunc=${callbackFunc})`);
#endif
    socket.onclose = function(e) {
#if WEBSOCKET_DEBUG
      dbg(`websocket event "close": socketId=${socketId},userData=${userData},callbackFunc=${callbackFunc})`);
#endif
      var eventPtr = WS.getSocketEvent(socketId);
      {{{ makeSetValue('eventPtr', C_STRUCTS.EmscriptenWebSocketCloseEvent.wasClean, 'e.wasClean', 'i8') }}},
      {{{ makeSetValue('eventPtr', C_STRUCTS.EmscriptenWebSocketCloseEvent.code, 'e.code', 'i16') }}},
      stringToUTF8(e.reason, eventPtr + {{{ C_STRUCTS.EmscriptenWebSocketCloseEvent.reason }}}, 512);
      {{{ makeDynCall('iipp', 'callbackFunc') }}}(0/*TODO*/, eventPtr, userData);
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_set_onmessage_callback_on_thread__deps: ['$WS', '$stringToNewUTF8', 'malloc', 'free'],
  emscripten_websocket_set_onmessage_callback_on_thread__proxy: 'sync',
  emscripten_websocket_set_onmessage_callback_on_thread: (socketId, userData, callbackFunc, thread) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_set_onmessage_callback(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

#if WEBSOCKET_DEBUG
    dbg(`emscripten_websocket_set_onmessage_callback(socketId=${socketId},userData=${userData},callbackFunc=${callbackFunc})`);
#endif
    socket.onmessage = function(e) {
#if WEBSOCKET_DEBUG == 2
      dbg(`websocket event "message": socketId=${socketId},userData=${userData},callbackFunc=${callbackFunc})`);
#endif
      var isText = typeof e.data == 'string';
      if (isText) {
        var buf = stringToNewUTF8(e.data);
        var len = lengthBytesUTF8(e.data)+1;
#if WEBSOCKET_DEBUG
        var s = (e.data.length < 256) ? e.data : (e.data.substr(0, 256) + ` (${e.data.length-256} more characters)`);
        dbg(`WebSocket onmessage, received data: "${e.data}", ${e.data.length} chars, ${len} bytes encoded as UTF-8: "${s}"`);
#endif
      } else {
        var len = e.data.byteLength;
        var buf = _malloc(len);
        HEAP8.set(new Uint8Array(e.data), buf);
#if WEBSOCKET_DEBUG
        var s = `WebSocket onmessage, received data: ${len} bytes of binary:`;
        for (var i = 0; i < Math.min(len, 256); ++i) s += ' ' + HEAPU8[buf+i].toString(16);
        s += ', "';
        for (var i = 0; i < Math.min(len, 256); ++i) s += (HEAPU8[buf+i] >= 32 && HEAPU8[buf+i] <= 127) ? String.fromCharCode(HEAPU8[buf+i]) : '\uFFFD';
        s += '"';
        if (len > 256) s + ` ... (${len - 256} more bytes)`;

        dbg(s);
#endif
      }
      var eventPtr = WS.getSocketEvent(socketId);
      {{{ makeSetValue('eventPtr', C_STRUCTS.EmscriptenWebSocketMessageEvent.data, 'buf', '*') }}},
      {{{ makeSetValue('eventPtr', C_STRUCTS.EmscriptenWebSocketMessageEvent.numBytes, 'len', 'i32') }}},
      {{{ makeSetValue('eventPtr', C_STRUCTS.EmscriptenWebSocketMessageEvent.isText, 'isText', 'i8') }}},
      {{{ makeDynCall('iipp', 'callbackFunc') }}}(0/*TODO*/, eventPtr, userData);
      _free(buf);
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_new__deps: ['$WS'],
  emscripten_websocket_new__proxy: 'sync',
  emscripten_websocket_new: (createAttributes) => {
    if (typeof WebSocket == 'undefined') {
#if WEBSOCKET_DEBUG
      dbg('emscripten_websocket_new(): WebSocket API is not supported by current browser)');
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }
    if (!createAttributes) {
#if WEBSOCKET_DEBUG
      dbg('emscripten_websocket_new(): Missing required "createAttributes" function parameter!');
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    }

    var url = UTF8ToString({{{ makeGetValue('createAttributes', 0, '*') }}});
    var protocols = {{{ makeGetValue('createAttributes', C_STRUCTS.EmscriptenWebSocketCreateAttributes.protocols, '*') }}}
    // TODO: Add support for createOnMainThread==false; currently all WebSocket connections are created on the main thread.
    // var createOnMainThread = HEAP8[createAttributes+2];

    var socket = protocols ? new WebSocket(url, UTF8ToString(protocols).split(',')) : new WebSocket(url);
    // We always marshal received WebSocket data back to Wasm, so enable receiving the data as arraybuffers for easy marshalling.
    socket.binaryType = 'arraybuffer';
    // TODO: While strictly not necessary, this ID would be good to be unique across all threads to avoid confusion.
    var socketId = webSockets.allocate(socket);

#if WEBSOCKET_DEBUG
    dbg(`emscripten_websocket_new(url=${url}, protocols=${protocols ? UTF8ToString(protocols).split(',') : 'null'}): created socket ID ${socketId})`);
#endif
    return socketId;
  },

  emscripten_websocket_send_utf8_text__deps: ['$WS'],
  emscripten_websocket_send_utf8_text__proxy: 'sync',
  emscripten_websocket_send_utf8_text: (socketId, textData) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_send_utf8_text(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

    var str = UTF8ToString(textData);
#if WEBSOCKET_DEBUG == 2
    dbg(`emscripten_websocket_send_utf8_text(socketId=${socketId},textData=${str.length} chars, "${str}")`);
#else
#if WEBSOCKET_DEBUG
    dbg(`emscripten_websocket_send_utf8_text(socketId=${socketId},textData=${str.length} ' chars, "${(str.length > 8) ? (str.substring(0,8) + '...') : str}")`);
#endif
#endif
    socket.send(str);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_send_binary__deps: ['$WS'],
  emscripten_websocket_send_binary__proxy: 'sync',
  emscripten_websocket_send_binary: (socketId, binaryData, dataLength) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_send_binary(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

#if WEBSOCKET_DEBUG
    var s = `data: ${dataLength} bytes of binary:`;
    for (var i = 0; i < Math.min(dataLength, 256); ++i) s += ' '+ HEAPU8[binaryData+i].toString(16);
    s += ', "';
    for (var i = 0; i < Math.min(dataLength, 256); ++i) s += (HEAPU8[binaryData+i] >= 32 && HEAPU8[binaryData+i] <= 127) ? String.fromCharCode(HEAPU8[binaryData+i]) : '\uFFFD';
    s += '"';
    if (dataLength > 256) s + ` ... (${dataLength - 256} more bytes)`;

    dbg(`emscripten_websocket_send_binary(socketId=${socketId},binaryData=${binaryData},dataLength=${dataLength}), ${s}`);
#endif
#if SHARED_MEMORY
    // TODO: This is temporary to cast a shared Uint8Array to a non-shared Uint8Array. This could be removed if WebSocket API is improved
    // to allow passing in views to SharedArrayBuffers
    socket.send(new Uint8Array({{{ makeHEAPView('U8', 'binaryData', 'binaryData+dataLength') }}}));
#else
    socket.send({{{ makeHEAPView('U8', 'binaryData', 'binaryData+dataLength') }}});
#endif
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_close__deps: ['$WS'],
  emscripten_websocket_close__proxy: 'sync',
  emscripten_websocket_close: (socketId, code, reason) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_close(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

    var reasonStr = reason ? UTF8ToString(reason) : undefined;
#if WEBSOCKET_DEBUG
    dbg(`emscripten_websocket_close(socketId=${socketId},code=${code},reason=${reasonStr})`);
#endif
    // According to WebSocket specification, only close codes that are recognized have integer values
    // 1000-4999, with 3000-3999 and 4000-4999 denoting user-specified close codes:
    // https://developer.mozilla.org/en-US/docs/Web/API/CloseEvent#Status_codes
    // Therefore be careful to call the .close() function with exact number and types of parameters.
    // Coerce code==0 to undefined, since Wasm->JS call can only marshal integers, and 0 is not allowed.
    if (reason) socket.close(code || undefined, UTF8ToString(reason));
    else if (code) socket.close(code);
    else socket.close();
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_delete__deps: ['$WS'],
  emscripten_websocket_delete__proxy: 'sync',
  emscripten_websocket_delete: (socketId) => {
    var socket = WS.getSocket(socketId);
    if (!socket) {
#if WEBSOCKET_DEBUG
      dbg(`emscripten_websocket_delete(): Invalid socket ID ${socketId} specified!`);
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

#if WEBSOCKET_DEBUG
    dbg(`emscripten_websocket_delete(socketId=${socketId})`);
#endif
    socket.onopen = socket.onerror = socket.onclose = socket.onmessage = null;
    webSockets.free(socketId);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_websocket_is_supported__proxy: 'sync',
  emscripten_websocket_is_supported: () => typeof WebSocket != 'undefined',

  emscripten_websocket_deinitialize__deps: ['$WS'],
  emscripten_websocket_deinitialize__proxy: 'sync',
  emscripten_websocket_deinitialize__deps: ['emscripten_websocket_delete'],
  emscripten_websocket_deinitialize: () => {
#if WEBSOCKET_DEBUG
    dbg('emscripten_websocket_deinitialize()');
#endif
    for (var i in WS.sockets) {
      var socket = WS.sockets[i];
      if (socket) {
        socket.close();
        _emscripten_websocket_delete(i);
      }
    }
    WS.sockets = [];
  }
}

addToLibrary(LibraryWebSocket);
