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
 * @fileoverview Definitions for node's assert module
 * @see http://nodejs.org/api/assert.html
 * @see https://github.com/joyent/node/blob/master/lib/assert.js
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var assert = require('assert');
 END_NODE_INCLUDE
 */

/**
 * @param {*} value
 * @param {string} message
 * @throws {assert.AssertionError}
 */
var assert = function(value, message) {};

/**
 * @param {{message: string, actual: *, expected: *, operator: string}} options
 * @constructor
 * @extends Error
 */
assert.AssertionError = function(options) {};

/**
 * @return {string}
 */
assert.AssertionError.prototype.toString = function() {};

/**
 * @param {*} value
 * @param {string=} message
 * @throws {assert.AssertionError}
 */
assert.ok = function(value, message) {};

/**
 * @param {*} actual
 * @param {*} expected
 * @param {string} message
 * @param {string} operator
 * @throws {assert.AssertionError}
 */
assert.fail = function(actual, expected, message, operator) {};

/**
 * @param {*} actual
 * @param {*} expected
 * @param {string} message
 * @throws {assert.AssertionError}
 */
assert.equal = function(actual, expected, message) {};

/**
 * @param {*} actual
 * @param {*} expected
 * @param {string} message
 * @throws {assert.AssertionError}
 */
assert.notEqual = function(actual, expected, message) {};

/**
 * @param {*} actual
 * @param {*} expected
 * @param {string} message
 * @throws {assert.AssertionError}
 */
assert.deepEqual = function(actual, expected, message) {};

/**
 * @param {*} actual
 * @param {*} expected
 * @param {string} message
 * @throws {assert.AssertionError}
 */
assert.notDeepEqual = function(actual, expected, message) {};

/**
 * @param {*} actual
 * @param {*} expected
 * @param {string} message
 * @throws {assert.AssertionError}
 */
assert.strictEqual = function(actual, expected, message) {};

/**
 * @param {*} actual
 * @param {*} expected
 * @param {string} message
 * @throws {assert.AssertionError}
 */
assert.notStrictEqual = function(actual, expected, message) {};

/**
 * @name assert.throws
 * @function
 * @param {function()} block
 * @param {Function|RegExp|function(*)} error
 * @param {string=} message
 * @throws {assert.AssertionError}
 */
// Error: .\assert.js:120: ERROR - Parse error. missing name after . operator
// assert.throws = function(block, error, message) {};

/**
 * @param {function()} block
 * @param {Function|RegExp|function(*)} error
 * @param {string=} message
 * @throws {assert.AssertionError}
 */
assert.doesNotThrow = function(block, error, message) {};

/**
 * @param {*} value
 * @throws {assert.AssertionError}
 */
assert.ifError = function(value) {};
