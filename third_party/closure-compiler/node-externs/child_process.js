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
 * @fileoverview Definitions for node's child_process module. Depends on the events module.
 * @see http://nodejs.org/api/child_process.html
 * @see https://github.com/joyent/node/blob/master/lib/child_process.js
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var child_process = require('child_process');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var child_process = {};

/**
 * @constructor
 * @param {...*} var_args
 * @extends events.EventEmitter
 */
child_process.ChildProcess = function(var_args) {}; // Private?

/**
 * @type {stream.ReadableStream}
 */
child_process.ChildProcess.prototype.stdin;

/**
 * @type {stream.WritableStream}
 */
child_process.ChildProcess.prototype.stdout;

/**
 * @type {stream.WritableStream}
 */
child_process.ChildProcess.prototype.stderr;

/**
 * @type {number}
 */
child_process.ChildProcess.prototype.pid;

/**
 * @param {string=} signal
 */
child_process.ChildProcess.prototype.kill = function(signal) {};

/**
 * @param {Object.<string,*>} message
 * @param {*} sendHandle
 */
child_process.ChildProcess.prototype.send = function(message, sendHandle) {};

/**
 */
child_process.ChildProcess.prototype.disconnect = function() {};

/**
 * @typedef {{cwd: string, stdio: (Array|string), customFds: Array, env: Object.<string,*>, detached: boolean, uid: number, gid: number, encoding: string, timeout: number, maxBuffer: number, killSignal: string}}
 */
child_process.Options;

/**
 * @param {string} command
 * @param {Array.<string>=} args
 * @param {child_process.Options=} options
 * @return {child_process.ChildProcess}
 */
child_process.ChildProcess.spawn = function(command, args, options) {};

/**
 * @param {string} command
 * @param {child_process.Options|function(Error, buffer.Buffer, buffer.Buffer)=} options
 * @param {function(Error, buffer.Buffer, buffer.Buffer)=} callback
 * @return {child_process.ChildProcess}
 */
child_process.exec = function(command, options, callback) {};

/**
 * @param {string} file
 * @param {Array.<string>} args
 * @param {child_process.Options} options
 * @param {function(Error, buffer.Buffer, buffer.Buffer)} callback
 * @return {child_process.ChildProcess}
 */
child_process.execFile = function(file, args, options, callback) {};

/**
 * @param {string} modulePath
 * @param {Array.<string>=} args
 * @param {child_process.Options=} options
 * @return {child_process.ChildProcess}
 */
child_process.fork = function(modulePath, args, options) {};
