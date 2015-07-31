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
 * @fileoverview Definitions for node's path module.
 * @see http://nodejs.org/api/path.html
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var path = require('path');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var path = {};

/**
 * @param {string} p
 * @return {string}
 * @nosideeffects
 */
path.normalize = function(p) {};

/**
 * @param {...string} var_args
 * @return {string}
 * @nosideeffects
 */
path.join = function(var_args) {};

/**
 * @param {string} from
 * @param {string=} to
 * @return {string}
 * @nosideeffects
 */
path.resolve = function(from, to) {};

/**
 * @param {string} from
 * @param {string} to
 * @return {string}
 * @nosideeffects
 */
path.relative = function(from, to) {};

/**
 * @param {string} p
 * @return {string}
 * @nosideeffects
 */
path.dirname = function(p) {};

/**
 * @param {string} p
 * @param {string=} ext
 * @return {string}
 * @nosideeffects
 */
path.basename = function(p, ext) {};

/**
 * @param {string} p
 * @return {string}
 * @nosideeffects
 */
path.extname = function(p) {};

/**
 * @type {string}
 */
path.sep;
