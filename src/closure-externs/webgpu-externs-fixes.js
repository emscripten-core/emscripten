/**
 * WebGPU extern fixes for Closure.
 */

/**
 * @suppress {duplicate}
 * @interface
 * @extends {Iterable<string>}
 */
function GPUSupportedFeatures() {}

/**
 * @return {!Iterator<string>}
 */
GPUSupportedFeatures.prototype[Symbol.iterator] = function() {};

/**
 * @suppress {duplicate}
 * @interface
 * @extends {Iterable<string>}
 */
function WGSLLanguageFeatures() {}

/**
 * @return {!Iterator<string>}
 */
WGSLLanguageFeatures.prototype[Symbol.iterator] = function() {};
