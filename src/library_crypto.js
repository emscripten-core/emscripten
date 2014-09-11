//"use strict";

// Utilities for crypto
mergeInto(LibraryManager.library, {
  $CRYPTO__deps: ['__setErrNo', '$ERRNO_CODES'
#if ASYNCIFY
                  , 'emscripten_async_resume'
#endif
                 ],
  $CRYPTO__postset: 'if (ENVIRONMENT_IS_NODE) CRYPTO.nodeInit();'+
                    'else if (ENVIRONMENT_IS_WEB) CRYPTO.browserInit();',
  $CRYPTO: {
    _objects: [],
    callOn: function(descriptor, action, args) {
      if (!(descriptor in this._objects)) {
        ___setErrNo(ERRNO_CODES.EBADF);
        return -1;
      }
      var fn = this._objects[descriptor][action];
      if (typeof fn !== 'function') {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      }
      return fn.call(args);
    },
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
              hashFinal: self._hashFinal, hashUpdate: self._hashUpdate
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
        this.nodeObj.update(buffer);
      };
      self._hashFinal = function(data, len) {
        var buffer = this.nodeObj.digest();
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
        if (typeof(crypto) !== 'undefined' &&
            typeof(crypto.getRandomValues) !== 'undefined') {
          try {
            bytes = new Uint8Array(length);
            crypto.getRandomValues(bytes);
            return bytes;
          } catch(e) { ___setErrNo(ERRNO_CODES.EACCES); }
        } else ___setErrNo(ERRNO_CODES.ENOSYS);
        return null;
      };
      self.open = function(algo) {
        if (typeof(crypto) === 'undefined' &&
            typeof(crypto.subtle) === 'undefined') {
          ___setErrNo(ERRNO_CODES.ENOSYS);
          return null;
        }
        var hashObj = {
          algorithm: { name: '' },
          data: { buf: null, view: null, length: 0 },
          hashUpdate: self._hashUpdate, hashFinal: self._hashFinal
        };
        var rsaObj = {
          jwk: null, privateKey: null, publicKey: null,
          hash: { name: 'SHA-256' } /* default hash, it gets changed later if needed */,
          rsaGenerate: self._rsaGenerate, rsaImport: self._rsaImport,
          rsaExport: self._rsaExport, rsaGetSize: self._rsaGetSize,
          rsaCrypt: self._rsaCrypt
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
        case {{{ cDefine('EMSCRIPTEN_ALGORITHM_RSA_PKCS21') }}}:
          return rsaObj;
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
#if ASYNCIFY
        try {
          var hashObject = this;
          var promise = crypto.subtle.digest(this.algorithm, this.data.view.subarray(0, this.data.length));
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
            Module.printErr('Error finalising hash of type ' + hashObject.algorithm.name + ': ' + err);
            ___setErrNo(ERRNO_CODES.EACCES);
            var addr = asm.getAsyncRetValAddr();
            {{{ makeSetValue('addr', 0, -1, 'i32') }}};
            _emscripten_async_resume();
          });
          asm.setAsync(); // tell the scheduler that we have a callback on hold
        } catch (err) {
          Module.printErr('Error creating WebCrypto hash of type ' + hashObject.algorithm.name + ': ' + err);
          ___setErrNo(ERRNO_CODES.ENOSYS);
          return -1;
        }
        return 0;
#else
        throw 'Please compile your program with -s ASYNCIFY=1 in order to use emscripten_crypto_hash_final';
#endif
      };
      self._rsaGenerate = function(size, exponent) {
#if ASYNCIFY
        try {
          var rsaObj = this;
          var generatePromise = crypto.subtle.generateKey({
                { hash: rsaObj.hash },
                modulusLength:size,
                publicExponent: new Uint8Array([(exponent>>24)&0xff,(exponent>>16)&0xff,(exponent>>8)&0xff,exponent&0xff]),
                name:'RSA-OAEP'
          }, true, ['encrypt','decrypt']);
          generatePromise.then(function fulfill(keypair) {
            rsaObj.privateKey = keypair.privateKey;
            rsaObj.publicKey = keypair.publicKey;
            return crypto.subtle.exportKey('jwk', keypair.privateKey);
          }).then(function fulfill(jwk) {
            rsaObj.jwk = (typeof jwk === 'string') ? JSON.parse(jwk) : jwk;
            var addr = asm.getAsyncRetValAddr();
            {{{ makeSetValue('addr', 0, 'copyLen', 'i32') }}};
            _emscripten_async_resume();
          }, function error(err) {
            Module.printErr('Error during WebCrypto RSA key generation/export: ' + err);
            ___setErrNo(ERRNO_CODES.EACCES);
            var addr = asm.getAsyncRetValAddr();
            {{{ makeSetValue('addr', 0, -1, 'i32') }}};
            _emscripten_async_resume();
          });
          asm.setAsync(); // tell the scheduler that we have a callback on hold
        } catch (err) {
          Module.printErr('Error creating WebCrypto RSA key: ' + err);
          ___setErrNo(ERRNO_CODES.ENOSYS);
          return -1;
        }
        return 0;
#else
        throw 'Please compile your program with -s ASYNCIFY=1 in order to use emscripten_crypto_rsa_generate';
#endif
      };
      self._rsaImport = function(jwk) {
#if ASYNCIFY
        try {
          var rsaObj = this;
          rsaObj.privateKey = null;
          rsaObj.publicKey = null;
          var algorithm = { name: 'RSA-OAEP', hash: rsaObj.hash };
          jwk = JSON.parse(jwk);
          publicJwk = jwk;
          delete publicJwk['d'];
          var publicPromise = crypto.subtle.importKey('jwk', publicJwk, algorithm, true, ['encrypt', 'decrypt']);
          publicPromise.then(function fulfill(key) {
            rsaObj.publicKey = key;
            if ('d' in jwk)
              return crypto.subtle.importKey('jwk', jwk, algorithm, true, ['encrypt', 'decrypt']);
            return key;
          }).then(function fulfill(key) {
            if ('d' in jwk)
              rsaObj.privateKey = key;
            var addr = asm.getAsyncRetValAddr();
            {{{ makeSetValue('addr', 0, 'copyLen', 'i32') }}};
            _emscripten_async_resume();
          }, function error(err) {
            Module.printErr('Error during WebCrypto RSA import: ' + err);
            ___setErrNo(ERRNO_CODES.EACCES);
            var addr = asm.getAsyncRetValAddr();
            {{{ makeSetValue('addr', 0, -1, 'i32') }}};
            _emscripten_async_resume();
          });
          asm.setAsync(); // tell the scheduler that we have a callback on hold
        } catch (err) {
          Module.printErr('Error creating WebCrypto RSA key: ' + err);
          ___setErrNo(ERRNO_CODES.ENOSYS);
          return -1;
        }
        return 0;
#else
        throw 'Please compile your program with -s ASYNCIFY=1 in order to use emscripten_crypto_rsa_import';
#endif
      };
      self._rsaExport = function(value, buffer, buffer_len) {
        if (!(value in jwk)) {
          ___setErrNo(ERRNO_CODES.EINVAL);
          return -1;
        }

        // (Turn JWK's base64url string into the more normal Base64 syntax)
        var jwkString = jwk[value].replace(/-/g, '+').replace(/_/g, '/');
        while (jwkString.length % 4) jwkString = jwkString + '=';

        var ascii = window.atob(jwkString);
        if (buffer_len < ascii.length) {
          ___setErrNo(ERRNO_CODES.ENOMEM);
          return -1;
        }
        for (var i = 0; i < ascii.length; ++i) {
          {{{ makeSetValue('data', 'i', 'ascii.charCodeAt(i)', 'i8') }}};
        }
        return ascii.length;
      };
      self._rsaGetSize = function() {
        return this.publicKey ? this.publicKey.modulusLength : 0;
      };
      self._rsaCrypt = function(encrypt, hashAlgorithm, data, buffer, buffer_len) {
        if (parse(hashAlgorithm) != this.hash.name) {
          // really annoying, have to export and re-import everything, not my fault...
          // XXX
        }
        if ((encrypt ? this.publicKey : this.privateKey) === null) {
          ___setErrNo(ERRNO_CODES.EINVAL);
          return -1;
        }
#if ASYNCIFY
        try {
          var algo = { name: 'RSA-OAEP' };
          var promise = encrypt ? crypto.subtle.encrypt(algo, this.publicKey, data)
                                : crypto.subtle.decrypt(algo, this.privateKey, data);
          promise.then(function fulfill(result) {
            var addr = asm.getAsyncRetValAddr();
            if (result.length > buffer_len) {
              {{{ makeSetValue('addr', 0, '-1', 'i32') }}};
            } else {
              var view = new Uint8Array(result);
              for (var i = 0; i < result.length; ++i)
                {{{ makeSetValue('buffer', 'i', 'view[i]', 'i8') }}};
              {{{ makeSetValue('addr', 0, 'result.length', 'i32') }}};
            }
            _emscripten_async_resume();
          }, function error(err) {
            Module.printErr('Error during WebCrypto RSA encrypt/decrypt: ' + err);
            ___setErrNo(ERRNO_CODES.EACCES);
            var addr = asm.getAsyncRetValAddr();
            {{{ makeSetValue('addr', 0, -1, 'i32') }}};
            _emscripten_async_resume();
          });
          asm.setAsync(); // tell the scheduler that we have a callback on hold
        } catch (err) {
          Module.printErr('Error beginning WebCrypto RSA encrypt/decrypt: ' + err);
          ___setErrNo(ERRNO_CODES.ENOSYS);
          return -1;
        }
        return 0;
#else
        throw 'Please compile your program with -s ASYNCIFY=1 in order to use emscripten_crypto_rsa_crypt';
#endif
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
    // int emscripten_crypto_hash_update(int d, const unsigned char* data,
    //                                   size_t len)
    return CRYPTO.callOn(descriptor, 'hashUpdate', [data, len]);
  },

  emscripten_crypto_hash_final: function(descriptor, buffer, buffer_len) {
    // int emscripten_crypto_hash_final(int d, unsigned char* buffer,
    //                                  size_t buffer_len)
    return CRYPTO.callOn(descriptor, 'hashFinal', [buffer, buffer_len]);
  },

  emscripten_crypto_rsa_generate: function(descriptor, size, exponent) {
    // int emscripten_crypto_rsa_generate(int d, int size, int exponent)
    return CRYPTO.callOn(descriptor, 'rsaGenerate', [size, exponent]);
  },

  emscripten_crypto_rsa_import: function(descriptor, jwk) {
    // int emscripten_crypto_rsa_import(int d, const char* jwk)
    return CRYPTO.callOn(descriptor, 'rsaImport', [Pointer_stringify(jwk)]);
  },

  emscripten_crypto_rsa_export: function(descriptor, value, buffer, buffer_len) {
    // int emscripten_crypto_rsa_export(int d, const char* value,
    //                                  unsigned char* buffer, size_t buffer_len)
    return CRYPTO.callOn(descriptor, 'rsaExport',
                         [Pointer_stringify(value), buffer, buffer_len]);
  },

  emscripten_crypto_rsa_get_size: function(descriptor) {
    // int emscripten_crypto_rsa_get_size(int d)
    return CRYPTO.callOn(descriptor, 'rsaGetSize', []);
  },

  emscripten_crypto_rsa_crypt: function(descriptor, encrypt, hashAlgorithm, data,
                                        data_len, buffer, buffer_len) {
    // int emscripten_crypto_rsa_crypt(int d, int encrypt, int hashAlgorithm,
    //                                 const unsigned char* data, size_t data_len,
    //                                 unsigned char* buffer, size_t buffer_len)
    var dataArray = new Uint8Array(data_len);
    for (var i = 0; i < data.length; ++i)
      dataArray[i] = {{{ makeGetValue('data', 'i', 'i8') }}};
    return CRYPTO.callOn(descriptor, 'rsaCrypt', [encrypt, hashAlgorithm,
                                                  dataArray, buffer, buffer_len]);
  }

});
