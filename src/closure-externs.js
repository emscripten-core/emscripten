/**
 * This file contains definitions for things that we'd really rather the closure compiler *didn't* minify.
 * See http://code.google.com/p/closure-compiler/wiki/FAQ#How_do_I_write_an_externs_file
 * See also the discussion here: https://github.com/kripken/emscripten/issues/1979
 *
 * The closure_compiler() method in tools/shared.py refers to this file when calling closure.
 */

// Closure externs used by library_uuid.js

/**
 * @param {Array} typedArray
 */
crypto.getRandomValues = function(typedArray) {};

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
 * @param {number} size
 * @param {function(Error, buffer.Buffer)} callback
 */
crypto.randomBytes = function(size, callback) {};


// Closure externs used by library_sockfs.js

/**
 BEGIN_NODE_INCLUDE
 var ws = require('ws');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var ws = {};

/**
 * @param {string} event
 * @param {function()} callback
 */
ws.on = function(event, callback) {};

/**
 * @param {Object} data
 * @param {Object} flags
 * @param {function()=} callback
 */
ws.send = function(data, flags, callback) {};

/**
* @type {boolean}
*/
ws.binaryType;

/**
 * @type {Object.<string,*>}
 */
var wss = ws.Server;

/**
 * @param {string} event
 * @param {function()} callback
 */
wss.on = function(event, callback) {};

/**
 * @param {function()} callback
 */
wss.broadcast = function(callback) {};

/**
* @type {Object.<string,*>}
*/
wss._socket;

/**
* @type {string}
*/
wss.url;

/**
* @type {string}
*/
wss._socket.remoteAddress;

/**
* @type {number}
*/
wss._socket.remotePort;

/**
* @type {Object.<string,*>}
*/
var flags = {};
/**
* @type {boolean}
*/
flags.binary;


