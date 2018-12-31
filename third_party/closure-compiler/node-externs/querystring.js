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
 * @fileoverview Definitions for node's querystring module.
 * @see http://nodejs.org/api/querystring.html
 * @see https://github.com/joyent/node/blob/master/lib/querystring.js
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var querystring = require('querystring');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var querystring = {};

/**
 * @param {Object.<string,*>} obj
 * @param {string=} sep
 * @param {string=} eq
 * @return {string}
 * @nosideeffects
 */
querystring.stringify = function(obj, sep, eq) {};

/**
 * @param {string} str
 * @param {string=} sep
 * @param {string=} eq
 * @param {*=} options
 * @nosideeffects
 */
querystring.parse = function(str, sep, eq, options) {};

/**
 * @param {string} str
 * @return {string}
 */
querystring.escape = function(str) {};

/**
 * @param {string} str
 * @return {string}
 */
querystring.unescape = function(str) {};

/**
 * @param {buffer.Buffer} s
 * @param {boolean} decodeSpaces
 */
querystring.unescapeBuffer = function(s, decodeSpaces) {};
