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
 * @fileoverview Definitions for node's core.
 * @see http://nodejs.org/api/globals.html
 * @see http://nodejs.org/api/modules.html
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 * @param name
 * @return {*}
 */
var require = function(name) {}

/**
 * @return {string}
 */
require.resolve = function() {};

/**
 * @type {Object.<string,*>}
 */
require.cache;

/**
 * @type {Array}
 */
require.extensions;

/**
 * @type {Object}
 */
require.main;

/**
 * @type {string}
 */
var __filename;

/**
 * @type {string}
 */
var __dirname;

/**
 * @type {Object}
 */
var module = {};

/**
 * @type {*}
 */
var exports;

/**
 * @type {Object.<string,*>}
 */
module.exports;

/**
 * @type {function(string)}
 */
module.require;

/**
 * @type {string}
 */
module.filename;

/**
 * @type {boolean}
 */
module.loaded;

/**
 * @type {*}
 */
module.parent;

/**
 * @type {Array}
 */
module.children;

/**
 * @type {Object.<string,*>}
 */
var global = {};
