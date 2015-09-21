
// src/load-wasm-worker.js: this file is concatenated at the end of the
// Emscripten-compiled unpack.cpp into jslib/load-wasm-worker.js.

// This file implements a worker that responds to a single initial message
// containing a url to fetch and unpack and the name of the global callback
// function to pass the resulting asm.js module when the decoded script is
// executed. The worker responds by sending a Blob containing the decoded utf8
// chars.

function mallocArrayCopy(src) {
  var L = src.byteLength;
  var dst = _malloc(L);
  HEAP8.set(new Uint8Array(src), dst);
  return dst;
}

function mallocStringCopy(src) {
  var L = src.length;
  var dst = _malloc(L + 1);
  for (var i = 0; i < L; i++)
    HEAP8[dst + i] = src.charCodeAt(i);
  HEAP8[dst + L] = 0;
  return dst;
}

function unpack(inBytes, callbackName) {
  var packedPtr = mallocArrayCopy(inBytes);
  if (!_asmjs_has_magic_number(packedPtr))
    throw "File does not appear to be packed asm.js";
  var callbackNamePtr = mallocStringCopy(callbackName);
  var unpackedSize = _asmjs_unpacked_size(packedPtr, callbackNamePtr);
  var unpackedPtr = _malloc(unpackedSize);
  _asmjs_unpack(packedPtr, callbackNamePtr, unpackedSize, unpackedPtr);
  return HEAP8.subarray(unpackedPtr, unpackedPtr + unpackedSize);
}

onmessage = function(e) {
  var url = e.data.url;
  var callbackName = e.data.callbackName;
  function doUnpack(data) {
    try {
      var bef = Date.now();
      var utf8 = unpack(data, callbackName);
      var aft = Date.now();
      if (ENVIRONMENT_IS_WEB) console.log("unpack of " + url + " took " + (aft - bef) + "ms");
      postMessage({ callbackName: callbackName, data: new Blob([utf8]) });
    } catch (e) {
      postMessage("failed to unpack " + url + ": " + e);
    }
  }
  if (typeof XMLHttpRequest !== 'undefined') {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, true);
    xhr.responseType = 'arraybuffer';
    xhr.onerror = function (e) {
      postMessage('Loading ' + url + ' failed');
    }
    xhr.onload = function (e) {
      if (xhr.status !== 200) {
        postMessage("failed to download " + url + " with status: " + xhr.statusText);
      } else {
        doUnpack(xhr.response);
      }
    }
    xhr.send(null);
  } else {
    doUnpack(Module['readBinary'](url));
  }
}

