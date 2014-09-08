//"use strict";

// Utilities for crypto
mergeInto(LibraryManager.library, {
  $CRYPTO__deps: ['__setErrNo', '$ERRNO_CODES', 'emscripten_async_resume'],
  $CRYPTO__postset: 'if (ENVIRONMENT_IS_NODE) CRYPTO.nodeInit();'+
                    'else if (ENVIRONMENT_IS_WEB) CRYPTO.browserInit();',
  $CRYPTO: {
    _objects: [],
    nodeInit: function() {
      var self = CRYPTO;
      self.getRandom = function(length) {
        try {
          var fn = require('crypto').randomBytes;
          return fn(length);
        } catch(e) { ___setErrNo(ERRNO_CODES.ENOSYS); }
        return null;
      };
      self.open = function(algo) {
        try {
          if (algo >= {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA1') }}} &&
              algo <= {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA512') }}}) {
            var fn = require('crypto').createHash;
            var hashObj = {
              nodeObj: null,
              hashFinal = self._hashFinal, hashUpdate = self._hashUpdate
            };
            switch (algo) {
            case {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA1') }}}:
              hashObj.nodeObj = fn('SHA1'); return hashObj;
            case {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA224') }}}:
              hashObj.nodeObj = fn('SHA224'); return hashObj;
            case {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA256') }}}:
              hashObj.nodeObj = fn('SHA256'); return hashObj;
            case {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA384') }}}:
              hashObj.nodeObj = fn('SHA384'); return hashObj;
            case {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA512') }}}:
              hashObj.nodeObj = fn('SHA512'); return hashObj;
            }
          }
          ___setErrNo(ERRNO_CODES.EINVAL);
        } catch(e) { ___setErrNo(ERRNO_CODES.ENOSYS); }
        return null;
      };
      self._hashUpdate = function(data, addedLen) {
        var buffer = new Buffer(addedLen);
        for (var i = 0; i < addedLen; ++i) {
          buffer[i] = {{{ makeGetValue('data', 'i', 'i8') }}};
        }
        this.update(buffer);
      };
      self._hashFinal = function(data, len) {
        var buffer = this.digest();
        var copyLen = Math.min(buffer.length, len);
        for (var i = 0; i < copyLen; ++i) {
          {{{ makeSetValue('data', 'i', 'buffer[i]', 'i8') }}};
        }
        return copyLen;
      };
    },
    browserInit: function() {
      var self = CRYPTO;
      self.getRandom = function(length) {
        if (typeof(window.crypto) !== 'undefined' &&
            typeof(window.crypto.getRandomValues) !== 'undefined') {
          try {
            bytes = new Uint8Array(length);
            window.crypto.getRandomValues(bytes);
            return bytes;
          } catch(e) { ___setErrNo(ERRNO_CODES.EACCES); }
        } else ___setErrNo(ERRNO_CODES.ENOSYS);
        return null;
      };
      self.open = function(algo) {
        if (typeof(window.crypto) === 'undefined' &&
            typeof(window.crypto.subtle) === 'undefined') {
          ___setErrNo(ERRNO_CODES.ENOSYS);
          return null;
        }
        var hashObj = {
          algorithm: { name: '' },
          data: { buf: null, view: null, length: 0 },
          hashUpdate: self._hashUpdate, hashFinal: self._hashFinal
        };
        switch (algo) {
        case {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA1') }}}:
          hashObj.algorithm.name = 'SHA-1'; return hashObj;
        case {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA256') }}}:
          hashObj.algorithm.name = 'SHA-256'; return hashObj;
        case {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA384') }}}:
          hashObj.algorithm.name = 'SHA-384'; return hashObj;
        case {{{ cDefine('EMSCRIPTEN_ALGORITHM_SHA512') }}}:
          hashObj.algorithm.name = 'SHA-512'; return hashObj;
        }
        ___setErrNo(ERRNO_CODES.EINVAL);
        return null;
      };
      self._hashUpdate = function(data, addedLen) {
        // Rubbishy WebCrypto API doesn't have a way to progressively add data
        // by updating the hash object; it doesn't even have a way to pass the
        // buffer in chunks.  Instead you have to concatenate everything into
        // one giant buffer before hashing it, an epic nuisance.
        if (!this.data.buf || this.data.length + addedLen > this.data.length) {
          // If we're resizing an array, let's at least double the allocation to
          // avoid a completely punitive number of copies.
          var newLen = Math.max(this.data.length + addedLen, this.data.length * 2);
          var newBuf = new ArrayBuffer(newLen);
          var newView = new Uint8Array(newBuf);
          if (this.data.buf)
            newView.set(this.data.view.subarray(0, this.data.length));
          this.data.buf = newBuf;
          this.data.view = newView;
        }
        for (var i = 0; i < addedLen; i++) {
          this.data.view[i+this.data.length] = {{{ makeGetValue('data', 'i', 'i8') }}};
        }
        this.data.length += addedLen;
        return 0;
      };
      self._hashFinal = function(data, len) {
        try {
#if ASYNCIFY
          var hashObject = this;
          var promise = window.crypto.subtle.digest(this.algorithm, this.data.view.subarray(0, this.data.length));
          promise.then(function fulfill(result) {
            var resultView = new Uint8Array(result);
            var copyLen = Math.min(resultView.length, len);
            for (var i = 0; i < copyLen; ++i) {
              {{{ makeSetValue('data', 'i', 'resultView[i]', 'i8') }}};
            }
            var addr = asm.getAsyncRetValAddr();
            {{{ makeSetValue('addr', 0, 'copyLen', 'i32') }}};
            _emscripten_async_resume();
          }, function error(err) {
            Module.printErr('Error finalising hash of type ' + hashObject.algorithm.name + ": " + err);
            ___setErrNo(ERRNO_CODES.EACCES);
            var addr = asm.getAsyncRetValAddr();
            {{{ makeSetValue('addr', 0, -1, 'i32') }}};
            _emscripten_async_resume();
          });
          asm.setAsync(); // tell the scheduler that we have a callback on hold
#else
          throw 'Please compile your program with -s ASYNCIFY=1 in order to use asynchronous operations like emscripten_wget';
#endif
        } catch (err) {
          Module.printErr('Error creating WebCrypto hash of type ' + hashObject.algorithm.name + ": " + err);
          ___setErrNo(ERRNO_CODES.ENOSYS);
          return -1;
        }
        return 0;
      };
    }
  },

  emscripten_crypto_random__deps: ['$CRYPTO'],
  emscripten_crypto_random: function(buffer, buffer_len) {
    // int emscripten_crypto_random(unsigned char* buffer, size_t buffer_len)
    var bytes = null;
    if (CRYPTO.getRandom)
      bytes = CRYPTO.getRandom(buffer_len);
    else
      ___setErrNo(ERRNO_CODES.ENOSYS);

    if (bytes === null) return -1;

    writeArrayToMemory(bytes, buffer);
    return 0;
  },

  emscripten_crypto_open__deps: ['$CRYPTO'],
  emscripten_crypto_open: function(algorithm) {
    // int emscripten_crypto_open(int algorithm)
    var obj = null;
    if (CRYPTO.open)
      obj = CRYPTO.open(algorithm);
    else
      ___setErrNo(ERRNO_CODES.ENOSYS);

    if (obj === null) return -1;

    for (var i = 0; ; ++i) {
      if (!(i in CRYPTO._objects)) break;
    }
    CRYPTO._objects[i] = obj;
    return i;
  },

  emscripten_crypto_close__deps: ['$CRYPTO'],
  emscripten_crypto_close: function(descriptor) {
    // int emscripten_crypto_close(int descriptor)
    if (!(descriptor in CRYPTO._objects)) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
    delete CRYPTO._objects[descriptor];
    return 0;
  },

  emscripten_crypto_hash_update: function(descriptor, data, len) {
    // int emscripten_crypto_hash_update(int d, const unsigned char* data, size_t len)
    if (!(descriptor in CRYPTO._objects)) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
    if (!(hashUpdate in CRYPTO._objects[descriptor])) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    return CRYPTO._objects[descriptor].hashUpdate(data, len);
  },

  emscripten_crypto_hash_final: function(descriptor, buffer, buffer_len) {
    // int emscripten_crypto_hash_final(int d, unsigned char* buffer, size_t buffer_len)
    if (!(descriptor in CRYPTO._objects)) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
    if (!(hashFinal in CRYPTO._objects[descriptor])) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    return CRYPTO._objects[descriptor].hashFinal(buffer, buffer_len);
  }

});
