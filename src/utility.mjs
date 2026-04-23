/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// General JS utilities - things that might be useful in any JS project.
// Nothing specific to Emscripten appears here.

import * as url from 'node:url';
import * as path from 'node:path';
import * as fs from 'node:fs';
import * as vm from 'node:vm';
import assert from 'node:assert';

export function safeQuote(x) {
  return x.replace(/"/g, '\\"').replace(/'/g, "\\'");
}

export function dump(item) {
  let funcData;
  try {
    if (typeof item == 'object' && item != null && item.funcData) {
      funcData = item.funcData;
      item.funcData = null;
    }
    return '// ' + JSON.stringify(item, null, '  ').replace(/\n/g, '\n// ');
  } catch {
    const ret = [];
    for (const [i, j] of Object.entries(item)) {
      if (typeof j == 'string' || typeof j == 'number') {
        ret.push(`${i}: ${j}`);
      } else {
        ret.push(`${i}: [?]`);
      }
    }
    return ret.join(',\n');
  } finally {
    if (funcData) item.funcData = funcData;
  }
}

let warnings = false;

export function warningOccured() {
  return warnings;
}

let currentFile = [];

export function pushCurrentFile(f) {
  currentFile.push(f);
}

export function popCurrentFile() {
  currentFile.pop();
}

function errorPrefix(lineNo) {
  if (!currentFile.length) return '';
  const filename = currentFile[currentFile.length - 1];
  if (lineNo) {
    return `${filename}:${lineNo}: `;
  } else {
    return `${filename}: `;
  }
}

export function warn(msg, lineNo) {
  warnings = true;
  console.error(`warning: ${errorPrefix(lineNo)}${msg}`);
}

const seenWarnings = new Set();

export function warnOnce(msg) {
  if (!seenWarnings.has(msg)) {
    seenWarnings.add(msg);
    warn(msg);
  }
}

let abortExecution = false;

export function errorOccured() {
  return abortExecution;
}

export function error(msg, lineNo) {
  abortExecution = true;
  process.exitCode = 1;
  console.error(`error: ${errorPrefix(lineNo)}${msg}`);
}

function range(size) {
  return Array.from(Array(size).keys());
}

// Fixed merge function with Prototype Pollution protection
export function merge(target, source) {
  for (var key in source) {
    if (Object.prototype.hasOwnProperty.call(source, key)) {
      if (key === '__proto__' || key === 'constructor' || key === 'prototype') continue;
      target[key] = source[key];
    }
  }
  return target;
}

// Fixed mergeInto function with Prototype Pollution protection
export function mergeInto(obj, other, options = null) {
  if (options) {
    if (options.noOverride) {
      for (const key of Object.keys(other)) {
        if (obj.hasOwnProperty(key)) {
          error(`Symbol re-definition in JavaScript library: ${key}. Do not use noOverride if this is intended`);
          return;
        }
      }
    }

    if (options.checkSig) {
      for (const [key, value] of Object.entries(other)) {
        if (typeof value === 'function' && !other.hasOwnProperty(key + '__sig')) {
          error(`__sig is missing for function: ${key}. Do not use checkSig if this is intended`);
          return;
        }
      }
    }
  }

  if (!options || !options.allowMissing) {
    for (const ident of Object.keys(other)) {
      if (isDecorator(ident)) {
        const index = ident.lastIndexOf('__');
        const basename = ident.slice(0, index);
        if (!(basename in obj) && !(basename in other)) {
          error(`Missing library element '${basename}' for library config '${ident}'`);
        }
      }
    }
  }

  // Sanitize 'other' object before Object.assign to prevent prototype pollution
  const keys = Object.keys(other);
  for (const key of keys) {
    if (key === '__proto__' || key === 'constructor' || key === 'prototype') {
      delete other[key];
      continue;
    }

    if (isDecorator(key)) {
      if (key.endsWith('__sig')) {
        if (obj.hasOwnProperty(key)) {
          const oldsig = obj[key];
          const newsig = other[key];
          if (oldsig == newsig) {
            warn(`signature redefinition for: ${key}`);
          } else {
            error(`signature redefinition for: ${key}. (old=${oldsig} vs new=${newsig})`);
          }
        }
      }
    }
  }

  return Object.assign(obj, other);
}

export function isJsOnlySymbol(symbol) {
  return symbol[0] == '$';
}

export const decoratorSuffixes = [
  '__sig', '__proxy', '__asm', '__deps', '__postset', '__docs',
  '__nothrow', '__noleakcheck', '__internal', '__user', '__async', '__i53abi',
];

export function isDecorator(ident) {
  return decoratorSuffixes.some((suffix) => ident.endsWith(suffix));
}

export function readFile(filename) {
  return fs.readFileSync(filename, 'utf8');
}

const __dirname = url.fileURLToPath(new URL('.', import.meta.url));
export const srcDir = __dirname;

export function localFile(filename) {
  assert(!path.isAbsolute(filename));
  return path.join(srcDir, filename);
}

function read(filename) {
  if (!path.isAbsolute(filename)) {
    filename = localFile(filename);
  }
  return readFile(filename);
}

export function printErr(...args) {
  console.error(...args);
}

class NullProfiler {
  start(_id) {}
  stop(_id) {}
  terminate() {}
}

export const timer = new NullProfiler();

export const compileTimeContext = vm.createContext({
  process,
  console,
});

export function addToCompileTimeContext(object) {
  Object.assign(compileTimeContext, object);
}

const setLikeSettings = [
  'EXPORTED_FUNCTIONS', 'WASM_EXPORTS', 'SIDE_MODULE_EXPORTS',
  'INCOMING_MODULE_JS_API', 'ALL_INCOMING_MODULE_JS_API',
  'EXPORTED_RUNTIME_METHODS', 'WEAK_IMPORTS'
];

export function applySettings(obj) {
  for (const key of setLikeSettings) {
    if (typeof obj[key] !== 'undefined') {
      obj[key] = new Set(obj[key]);
    }
  }
  Object.assign(globalThis, obj);
  addToCompileTimeContext(obj);
}

export function loadSettingsFile(f) {
  const settings = {};
  vm.runInNewContext(readFile(f), settings, {filename: f});
  applySettings(settings);
  return settings;
}

export function loadDefaultSettings() {
  const rtn = loadSettingsFile(localFile('settings.js'));
  Object.assign(rtn, loadSettingsFile(localFile('settings_internal.js')));
  return rtn;
}

export function runInMacroContext(code, options) {
  compileTimeContext['__filename'] = options.filename;
  compileTimeContext['__dirname'] = path.dirname(options.filename);
  return vm.runInContext(code, compileTimeContext, options);
}

addToCompileTimeContext({
  assert,
  decoratorSuffixes,
  error,
  isDecorator,
  isJsOnlySymbol,
  mergeInto,
  read,
  warn,
  warnOnce,
  printErr,
  range,
});
