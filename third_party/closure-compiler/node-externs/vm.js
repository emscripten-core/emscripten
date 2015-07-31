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
 * @fileoverview Definitions for node's vm module.
 * @see http://nodejs.org/api/vm.html
 * @see https://github.com/joyent/node/blob/master/lib/vm.js
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var vm = require('vm');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var vm = {};

/**
 * @constructor
 */
vm.Context = function() {}; // Does not really exist

/**
 * @param {string} code
 * @param {string=} filename
 */
vm.runInThisContext = function(code, filename) {};

/**
 * @param {string} code
 * @param {Object.<string,*>=} sandbox
 * @param {string=} filename
 */
vm.runInNewContext = function(code, sandbox, filename) {};

/**
 * @param {string} code
 * @param {vm.Context} context
 * @param {string=} filename
 */
vm.runInContext = function(code, context, filename) {};

/**
 * @param {Object.<string,*>=} initSandbox
 * @return {vm.Context}
 * @nosideeffects
 */
vm.createContext = function(initSandbox) {};

/**
 * @constructor
 */
vm.Script = function() {};

/**
 * @param {string} code
 * @param {string=} filename
 * @return {vm.Script}
 * @nosideeffects
 */
vm.createScript = function(code, filename) {};

/**
 */
vm.Script.prototype.runInThisContext = function() {};

/**
 * @param {Object.<string,*>=} sandbox
 */
vm.Script.prototype.runInNewContext = function(sandbox) {};
