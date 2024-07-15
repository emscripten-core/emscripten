// V8 externs for Closure to know about

/**
 * @param {string} filename
 * @param {string=} type
 * @return {string}
 * @suppress {duplicate}
 */
var read = function(filename, type) {};
/**
 * @param {string} expression
 * @suppress {duplicate}
 */
var print = function(expression) {};
/**
 * @param {string} expression
 * @suppress {duplicate}
 */
var printErr = function(expression) {};
/**
 * @param {string} filename
 * @return {ArrayBuffer}
 */
var readbuffer = function(filename) {};
/**
 * @const
 * @suppress {duplicate}
 */
var scriptArgs = [];
/**
 * @param {number=} status
 * @suppress {duplicate}
 */
var quit = function(status) {};
