/*
 * Copyright 2012 The Closure Compiler Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @fileoverview Definitions for node's crypto module. Depends on the buffer module.
 * @see http://nodejs.org/api/crypto.html
 * @see https://github.com/joyent/node/blob/master/lib/crypto.js
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var crypto = require('crypto');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var crypto = {};

/**
 * @type {string}
 */
crypto.DEFAULT_ENCODING;

/**
 * @typedef {{pfx: (string|buffer.Buffer), key: (string|buffer.Buffer), passphrase: string, cert: (string|buffer.Buffer), ca: Array.<string|buffer.Buffer>, crl: (string|Array.<string>), ciphers: string}}
 */
crypto.Credentials;

/**
 * @param {Object.<string,string>=} details
 * @return {crypto.Credentials}
 */
crypto.createCredentials = function(details) {};

/**
 * @param {string} algorithm
 * @return {crypto.Hash}
 */
crypto.createHash = function(algorithm) {};

/**
 * @param {string} algorithm
 * @param {Object=} options
 * @constructor
 * @extends stream.Transform
 */
crypto.Hash = function(algorithm, options) {};

/**
 * @param {string|buffer.Buffer} data
 * @param {string=} input_encoding
 */
crypto.Hash.prototype.update = function(data, input_encoding) {};

/**
 * @param {string=} encoding
 * @return {string}
 */
crypto.Hash.prototype.digest = function(encoding) {};

/**
 * @param {string} algorithm
 * @param {string|buffer.Buffer} key
 * @return {crypto.Hmac}
 */
crypto.createHmac = function(algorithm, key) {};

/**
 * @param {string} hmac
 * @param {string|buffer.Buffer} key
 * @param {Object=} options
 * @constructor
 * @extends stream.Transform
 */
crypto.Hmac = function(hmac, key, options) {};

/**
 * @param {string|buffer.Buffer} data
 */
crypto.Hmac.prototype.update = function(data) {};

/**
 * @param {string} encoding
 */
crypto.Hmac.prototype.digest = function(encoding) {};

/**
 * @param {string} algorithm
 * @param {string|buffer.Buffer} password
 * @return {crypto.Cipher}
 */
crypto.createCipher = function(algorithm, password) {};

/**
 * @param {string} algorithm
 * @param {string|buffer.Buffer} key
 * @param {string|buffer.Buffer} iv
 * @return {crypto.Cipheriv}
 */
crypto.createCipheriv = function(algorithm, key, iv) {};

/**
 * @param {string|buffer.Buffer} cipher
 * @param {string} password
 * @param {Object=} options
 * @constructor
 * @extends stream.Transform
 */
crypto.Cipher = function(cipher, password, options) {};

/**
 * @param {string|buffer.Buffer} data
 * @param {string=} input_encoding
 * @param {string=} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Cipher.prototype.update = function(data, input_encoding, output_encoding) {};

/**
 * @name crypto.Cipher.prototype.final
 * @param {string} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Cipher.prototype['final'] = function(output_encoding) {};

/**
 * @param {boolean=} auto_padding
 */
crypto.Cipher.prototype.setAutoPadding = function(auto_padding) {};

/**
 * Note:  Cipheriv mixes update, final, and setAutoPadding from Cipher but
 * doesn't inherit directly from Cipher.
 *
 * @param {string} cipher
 * @param {string|buffer.Buffer} key
 * @param {string|buffer.Buffer} iv
 * @constructor
 * @extends stream.Transform
 */
crypto.Cipheriv = function(cipher, key, iv) {};

/**
 * @param {string|buffer.Buffer} data
 * @param {string=} input_encoding
 * @param {string=} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Cipheriv.prototype.update = function(data, input_encoding, output_encoding) {};

/**
 * @name crypto.Cipheriv.prototype.final
 * @param {string} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Cipheriv.prototype['final'] = function(output_encoding) {};

/**
 * @param {boolean=} auto_padding
 */
crypto.Cipheriv.prototype.setAutoPadding = function(auto_padding) {};

/**
 * @param {string} algorithm
 * @param {string|buffer.Buffer} password
 * @return {crypto.Decipher}
 */
crypto.createDecipher = function(algorithm, password) {};

/**
 * @param {string} algorithm
 * @param {string|buffer.Buffer} key
 * @param {string|buffer.Buffer} iv
 * @return {crypto.Decipheriv}
 */
crypto.createDecipheriv = function(algorithm, key, iv) {};

/**
 * Note:  Decipher mixes update, final, and setAutoPadding from Cipher but
 * doesn't inherit directly from Cipher.
 *
 * @param {string|buffer.Buffer} cipher
 * @param {string|buffer.Buffer} password
 * @param {Object=} options
 * @constructor
 * @extends stream.Transform
 */
crypto.Decipher = function(cipher, password, options) {}

/**
 * @param {string|buffer.Buffer} data
 * @param {string=} input_encoding
 * @param {string=} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Decipher.prototype.update = function(data, input_encoding, output_encoding) {};

/**
 * @name crypto.Decipher.prototype.final
 * @param {string} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Decipher.prototype['final'] = function(output_encoding) {};

/**
 * @param {string} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Decipher.prototype.finaltol = function(output_encoding) {};

/**
 * @param {boolean=} auto_padding
 */
crypto.Decipher.prototype.setAutoPadding = function(auto_padding) {};

/**
 * Note:  Decipheriv mixes update, final, and setAutoPadding from Cipher but
 * doesn't inherit directly from Cipher.
 *
 * @param {string|buffer.Buffer|crypto.Decipheriv} cipher
 * @param {string|buffer.Buffer} key
 * @param {string|buffer.Buffer} iv
 * @param {Object=} options
 * @constructor
 * @extends stream.Transform
 */
crypto.Decipheriv = function(cipher, key, iv, options) {};

/**
 * @param {string|buffer.Buffer} data
 * @param {string=} input_encoding
 * @param {string=} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Decipheriv.prototype.update = function(data, input_encoding, output_encoding) {};

/**
 * @name crypto.Decipheriv.prototype.final
 * @param {string} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Decipheriv.prototype['final'] = function(output_encoding) {};

/**
 * @param {string} output_encoding
 * @return {string|buffer.Buffer}
 */
crypto.Decipheriv.prototype.finaltol = function(output_encoding) {};

/**
 * @param {boolean=} auto_padding
 */
crypto.Decipheriv.prototype.setAutoPadding = function(auto_padding) {};

/**
 * @param {string} algorithm
 * @return {crypto.Sign}
 */
crypto.createSign = function(algorithm) {};

/**
 * @param {string} algorithm
 * @param {Object=} options
 * @constructor
 * @extends stream.Writable
 */
crypto.Sign = function(algorithm, options) {};

/**
 * @param {string|buffer.Buffer} data
 */
crypto.Sign.prototype.update = function(data) {};

/**
 * @param {string} private_key
 * @param {string=} output_format
 * @return {string|buffer.Buffer}
 */
crypto.Sign.prototype.sign = function(private_key, output_format) {};

/**
 * @param {string} algorithm
 * @return crypto.Verify
 */
crypto.createVerify = function(algorithm) {};

/**
 * @param {string} algorithm
 * @param {Object=} options
 * @constructor
 * @extends stream.Writable
 */
crypto.Verify = function(algorithm, options) {};

/**
 * @param {string|buffer.Buffer} data
 */
crypto.Verify.prototype.update = function(data) {};

/**
 * @param {string} object
 * @param {string|buffer.Buffer} signature
 * @param {string=} signature_format
 * @return {boolean}
 */
crypto.Verify.prototype.verify = function(object, signature, signature_format) {};

/**
 * @param {number} prime
 * @param {string=} encoding
 * @return {crypto.DiffieHellman}
 */
crypto.createDiffieHellman = function(prime, encoding) {};

/**
 * @param {number} sizeOrKey
 * @param {string=} encoding
 * @constructor
 */
crypto.DiffieHellman = function(sizeOrKey, encoding) {};

/**
 * @param {string=} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellman.prototype.generateKeys = function(encoding) {};

/**
 * @param {string|buffer.Buffer} key
 * @param {string=} inEnc
 * @param {string=} outEnc
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellman.prototype.computeSecret = function(key, inEnc, outEnc) {};

/**
 * @param {string=} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellman.prototype.getPrime = function(encoding) {};

/**
 * @param {string=} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellman.prototype.getGenerator = function(encoding) {};

/**
 * @param {string=} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellman.prototype.getPublicKey = function(encoding) {};

/**
 * @param {string} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellman.prototype.getPrivateKey = function(encoding) {}

/**
 * @param {string|buffer.Buffer} key
 * @param {string=} encoding
 * @return {crypto.DiffieHellman}
 */
crypto.DiffieHellman.prototype.setPublicKey = function(key, encoding) {};

/**
 * @param {string|buffer.Buffer} key
 * @param {string=} encoding
 * @return {crypto.DiffieHellman}
 */
crypto.DiffieHellman.prototype.setPrivateKey = function(key, encoding) {};

/**
 * Note:  DiffieHellmanGroup mixes DiffieHellman but doesn't inherit directly.
 *
 * @param {string} name
 * @constructor
 */
crypto.DiffieHellmanGroup = function(name) {};

/**
 * @param {string=} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellmanGroup.prototype.generateKeys = function(encoding) {};

/**
 * @param {string|buffer.Buffer} key
 * @param {string=} inEnc
 * @param {string=} outEnc
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellmanGroup.prototype.computeSecret = function(key, inEnc, outEnc) {};

/**
 * @param {string=} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellmanGroup.prototype.getPrime = function(encoding) {};

/**
 * @param {string=} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellmanGroup.prototype.getGenerator = function(encoding) {};

/**
 * @param {string=} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellmanGroup.prototype.getPublicKey = function(encoding) {};

/**
 * @param {string} encoding
 * @return {string|buffer.Buffer}
 */
crypto.DiffieHellmanGroup.prototype.getPrivateKey = function(encoding) {}

/**
 * @param {string|buffer.Buffer} key
 * @param {string=} encoding
 * @return {crypto.DiffieHellmanGroup}
 */
crypto.DiffieHellmanGroup.prototype.setPublicKey = function(key, encoding) {};

/**
 * @param {string|buffer.Buffer} key
 * @param {string=} encoding
 * @return {crypto.DiffieHellmanGroup}
 */
crypto.DiffieHellmanGroup.prototype.setPrivateKey = function(key, encoding) {};

/**
 * @param {string} group_name
 * @return {crypto.DiffieHellmanGroup}
 */
crypto.getDiffieHellman = function(group_name) {};

/**
 * @param {string|buffer.Buffer} password
 * @param {string|buffer.Buffer} salt
 * @param {number} iterations
 * @param {number} keylen
 * @param {function(*, string)} callback
 */
crypto.pbkdf2 = function(password, salt, iterations, keylen, callback) {};

/**
 * @param {string|buffer.Buffer} password
 * @param {string|buffer.Buffer} salt
 * @param {number} iterations
 * @param {number} keylen
 */
crypto.pbkdf2Sync = function(password, salt, iterations, keylen) {};

/**
 * @param {number} size
 * @param {function(Error, buffer.Buffer)=} callback
 */
crypto.randomBytes = function(size, callback) {};

/**
 * @param {number} size
 * @param {function(Error, buffer.Buffer)=} callback
 */
crypto.pseudoRandomBytes = function(size, callback) {};

/**
 * @param {number} size
 * @param {function(Error, buffer.Buffer)=} callback
 */
crypto.rng = function(size, callback) {};

/**
 * @param {number} size
 * @param {function(Error, buffer.Buffer)=} callback
 */
crypto.prng = function(size, callback) {};

/**
 * @return {Array.<string>}
 */
crypto.getCiphers = function() {};

/**
 * @return {Array.<string>}
 */
crypto.getHashes = function() {};
