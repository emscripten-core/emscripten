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
var fs;

/**
 * @param {...*} var_args
 * @constructor
 * @nosideeffects
 */
var Buffer = function(var_args) {};

/**
 * @param {ArrayBuffer|SharedArrayBuffer|string} arrayBufferOrString
 * @param {number|string=} byteOffsetOrEncoding
 * @param {number=} length
 * @return {nodeBuffer.Buffer}
 * @nosideeffects
 */
Buffer.from = function(arrayBufferOrString, byteOffsetOrEncoding, length) {};

/**
 * @param {number} size
 * @param {(string|!Buffer|number)=} fill
 * @param {string=} encoding
 * @return {!Buffer}
 */
Buffer.alloc = function(size, fill, encoding) {};

/**
 * @param {number=} start
 * @param {number=} end
 * @return {Buffer}
 * @nosideeffects
 */
Buffer.prototype.slice = function(start, end) {};

/**
 * @param {string=} encoding
 * @param {number=} start
 * @param {number=} end
 * @nosideeffects
 */
Buffer.prototype.toString = function(encoding, start, end) {};

Worker.prototype.ref = function() {};
Worker.prototype.unref = function() {};

/**
 * @type {number}
 */
fs.Stats.prototype.atimeMs;

/**
 * @type {number}
 */
fs.Stats.prototype.mtimeMs;

/**
 * @type {number}
 */
fs.Stats.prototype.ctimeMs;
