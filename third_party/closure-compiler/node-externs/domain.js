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
 * @fileoverview Definitions for node's domain module. Depends on the events module.
 * @see http://nodejs.org/api/domain.html
 * @see https://github.com/joyent/node/blob/master/lib/domain.js
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 * @type {Object.<string,*>}
 */
var domain = {};

/**
 * @type {domain.Domain}
 */
domain.active;

/**
 * @return {domain.Domain}
 */
domain.create = function() {};

/**
 * @constructor
 * @extends events.EventEmitter
 */
domain.Domain = function() {};

/**
 * @param {function()} fn
 */
domain.Domain.prototype.run = function(fn) {};

/**
 * @type {Array}
 */
domain.Domain.prototype.members;

/**
 * @param {events.EventEmitter} emitter
 */
domain.Domain.prototype.add = function(emitter) {};

/**
 * @param {events.EventEmitter} emitter
 */
domain.Domain.prototype.remove = function(emitter) {};

/**
 * @param {function(...[*])} callback
 * @return {function(...[*])}
 */
domain.Domain.prototype.bind = function(callback) {};

/**
 * @param {function(...[*])} callback
 * @return {function(...[*])}
 */
domain.Domain.prototype.intercept = function(callback) {};

/**
 */
domain.Domain.prototype.dispose = function() {};

// Undocumented

/**
 */
domain.Domain.prototype.enter = function() {};

/**
 */
domain.Domain.prototype.exit = function() {};
