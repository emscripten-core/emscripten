/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

  read_ = function shell_read(url) {
#if SUPPORT_BASE64_EMBEDDING
    try {
#endif
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      xhr.send(null);
      return xhr.responseText;
#if SUPPORT_BASE64_EMBEDDING
    } catch (err) {
      var data = tryParseAsDataURI(url);
      if (data) {
        return intArrayToString(data);
      }
      throw err;
    }
#endif
  };

  if (ENVIRONMENT_IS_WORKER) {
    readBinary = function readBinary(url) {
#if SUPPORT_BASE64_EMBEDDING
      try {
#endif
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, false);
        xhr.responseType = 'arraybuffer';
        xhr.send(null);
        return new Uint8Array(/** @type{!ArrayBuffer} */(xhr.response));
#if SUPPORT_BASE64_EMBEDDING
      } catch (err) {
        var data = tryParseAsDataURI(url);
        if (data) {
          return data;
        }
        throw err;
      }
#endif
    };
  }

  readAsync = function readAsync(url, onload, onerror) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'arraybuffer';
    xhr.onload = function xhr_onload() {
      if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
        onload(xhr.response);
        return;
      }
#if SUPPORT_BASE64_EMBEDDING
      var data = tryParseAsDataURI(url);
      if (data) {
        onload(data.buffer);
        return;
      }
#endif
      onerror();
    };
    xhr.onerror = onerror;
    xhr.send(null);
  };

