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
 * @suppress {duplicate}
 */
var process;

/**
 * @suppress {duplicate}
 */
var assert;

/**
 * @suppress {duplicate}
 */
var fs;

/**
 * @param {...*} var_args
 * @constructor
 * @nosideeffects
 */
var Buffer = function(var_args) {};

/**
 * @param {ArrayBuffer|SharedArrayBuffer} arrayBuffer
 * @param {number=} byteOffset
 * @param {number=} length
 * @return {nodeBuffer.Buffer}
 * @nosideeffects
 */
Buffer.from = function(arrayBuffer, byteOffset, length) {};
