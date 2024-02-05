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
 * @fileoverview Definitions for node's global process object. Depends on the stream module.
 * @see http://nodejs.org/api/process.html
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 * @constructor
 * @extends events.EventEmitter
 */
var process = function() {};

/**
 * @type {stream.ReadableStream}
 */
process.stdin;

/**
 * @type {stream.WritableStream}
 */
process.stdout;

/**
 * @type {stream.WritableStream}
 */
process.stderr;

/**
 * @type {Array.<string>}
 */
process.argv;

/**
 * @type {string}
 */
process.execPath;

/**
 */
process.abort = function() {};

/**
 * @param {string} directory
 */
process.chdir = function(directory) {};

/**
 * @return {string}
 * @nosideeffects
 */
process.cwd = function() {};

/**
 * @type {Object.<string,string>}
 */
process.env;

/**
 * @param {number=} code
 */
process.exit = function(code) {};

/**
 * @return {number}
 * @nosideeffects
 */
process.getgid = function() {};

/**
 * @param {number} id
 */
process.setgid = function(id) {};

/**
 * @return {number}
 * @nosideeffects
 */
process.getuid = function() {};

/**
 * @param {number} id
 */
process.setuid = function(id) {};

/**
 * @type {!string}
 */
process.version;

/**
 * @type {Object.<string,string>}
 */
process.versions;

/**
 * @type {Object.<string,*>}
 */
process.config;

/**
 * @param {number} pid
 * @param {string=} signal
 */
process.kill = function(pid, signal) {};

/**
 * @type {number}
 */
process.pid;

/**
 * @type {string}
 */
process.title;

/**
 * @type {string}
 */
process.arch;

/**
 * @type {string}
 */
process.platform;

/**
 * @return {Object.<string,number>}
 * @nosideeffects
 */
process.memoryUsage = function() {};

/**
 * @param {!function()} callback
 */
process.nextTick = function(callback) {};

/**
 * @param {number=} mask
 */
process.umask = function(mask) {};

/**
 * @return {number}
 * @nosideeffects
 */
process.uptime = function() {};

/**
 * @return {number}
 * @nosideeffects
 */
process.hrtime = function() {};

/**
 */
process.binding = function(name) {};

/**
 * @type {number}
 */
process.exitCode;
