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
 * @fileoverview Definitions for node's readline module. Depends on the events module.
 * @see http://nodejs.org/api/readline.html
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var readline = require('readline');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var readline = {};

/**
 * @param {{input: stream.ReadableStream, output: stream.WritableStream, completer: function(string, function(*, Array)=), terminal: boolean}} options
 * @return {readline.Interface}
 */
readline.createInterface = function(options) {};

/**
 * @constructor
 * @extends events.EventEmitter
 */
readline.Interface = function() {};

/**
 * @param {string} prompt
 * @param {number} length
 */
readline.Interface.prototype.setPrompt = function(prompt, length) {};

/**
 * @param {boolean=} preserveCursor
 */
readline.Interface.prototype.prompt = function(preserveCursor) {};

/**
 * @param {string} query
 * @param {function(string)} callback
 */
readline.Interface.prototype.question = function(query, callback) {};

/**
 */
readline.Interface.prototype.pause = function() {};

/**
 */
readline.Interface.prototype.resume = function() {};

/**
 */
readline.Interface.prototype.close = function() {};

/**
 * @param {string} data
 * @param {Object.<string,*>=} key
 */
readline.Interface.prototype.write = function(data, key) {};
