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

export {assert};

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

let currentFile = null;

export function setCurrentFile(f) {
  let rtn = currentFile;
  currentFile = f;
  return rtn;
}

function errorPrefix() {
  if (currentFile) {
    return currentFile + ': ';
  } else {
    return '';
  }
}

export function warn(a, msg) {
  warnings = true;
  if (!msg) {
    msg = a;
    a = false;
  }
  if (!a) {
    printErr(`warning: ${errorPrefix()}${msg}`);
  }
}

export function warnOnce(a, msg) {
  if (!msg) {
    msg = a;
    a = false;
  }
  if (!a) {
    warnOnce.msgs ||= {};
    if (msg in warnOnce.msgs) return;
    warnOnce.msgs[msg] = true;
    warn(msg);
  }
}

let abortExecution = false;

export function errorOccured() {
  return abortExecution;
}

export function error(msg) {
  abortExecution = true;
  printErr(`error: ${errorPrefix()}${msg}`);
}

function range(size) {
  return Array.from(Array(size).keys());
}

export function mergeInto(obj, other, options = null) {
  if (options) {
    // check for unintended symbol redefinition
    if (options.noOverride) {
      for (const key of Object.keys(other)) {
        if (obj.hasOwnProperty(key)) {
          error(
            `Symbol re-definition in JavaScript library: ${key}. Do not use noOverride if this is intended`,
          );
          return;
        }
      }
    }

    // check if sig is missing for added functions
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

  for (const key of Object.keys(other)) {
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

      const index = key.lastIndexOf('__');
      const decoratorName = key.slice(index);
      const type = typeof other[key];

      // Specific type checking for `__deps` which is expected to be an array
      // (not just any old `object`)
      if (decoratorName === '__deps') {
        const deps = other[key];
        if (!Array.isArray(deps)) {
          error(
            `JS library directive ${key}=${deps.toString()} is of type '${type}', but it should be an array`,
          );
        }
        for (let dep of deps) {
          if (dep && typeof dep !== 'string' && typeof dep !== 'function') {
            error(
              `__deps entries must be of type 'string' or 'function' not '${typeof dep}': ${key}`,
            );
          }
        }
      } else {
        // General type checking for all other decorators
        const decoratorTypes = {
          __sig: 'string',
          __proxy: 'string',
          __asm: 'boolean',
          __inline: 'boolean',
          __postset: ['string', 'function'],
          __docs: 'string',
          __nothrow: 'boolean',
          __noleakcheck: 'boolean',
          __internal: 'boolean',
          __user: 'boolean',
          __async: 'boolean',
          __i53abi: 'boolean',
        };
        const expected = decoratorTypes[decoratorName];
        if (type !== expected && !expected.includes(type)) {
          error(`Decorator (${key}} has wrong type. Expected '${expected}' not '${type}'`);
        }
      }
    }
  }

  return Object.assign(obj, other);
}

export function isNumber(x) {
  // XXX this does not handle 0xabc123 etc. We should likely also do x == parseInt(x) (which handles that), and remove hack |// handle 0x... as well|
  return x == parseFloat(x) || (typeof x == 'string' && x.match(/^-?\d+$/)) || x == 'NaN';
}

// Symbols that start with '$' are not exported to the wasm module.
// They are intended to be called exclusively by JS code.
export function isJsOnlySymbol(symbol) {
  return symbol[0] == '$';
}

export function isDecorator(ident) {
  const suffixes = [
    '__sig',
    '__proxy',
    '__asm',
    '__inline',
    '__deps',
    '__postset',
    '__docs',
    '__nothrow',
    '__noleakcheck',
    '__internal',
    '__user',
    '__async',
    '__i53abi',
  ];
  return suffixes.some((suffix) => ident.endsWith(suffix));
}

export function isPowerOfTwo(x) {
  return x > 0 && (x & (x - 1)) == 0;
}

export function read(filename) {
  const absolute = find(filename);
  return fs.readFileSync(absolute).toString();
}

export function find(filename) {
  const dirname = url.fileURLToPath(new URL('.', import.meta.url));
  const prefixes = [process.cwd(), path.join(dirname, '..', 'src')];
  for (let i = 0; i < prefixes.length; ++i) {
    const combined = path.join(prefixes[i], filename);
    if (fs.existsSync(combined)) {
      return combined;
    }
  }
  return filename;
}

// Anything needed by the script that we load below must be added to the
// global object.  These, for example, are all needed by parseTools.js.
export function print(x) {
  process.stdout.write(x + '\n');
}

export function printErr(x) {
  process.stderr.write(x + '\n');
}

export class Benchmarker {
  totals = {};
  ids = [];
  lastTime = 0;

  start(id) {
    const now = Date.now();
    if (this.ids.length > 0) {
      this.totals[this.ids[this.ids.length - 1]] += now - this.lastTime;
    }
    this.lastTime = now;
    this.ids.push(id);
    this.totals[id] ||= 0;
  }

  stop(id) {
    const now = Date.now();
    assert(id === this.ids[this.ids.length - 1]);
    this.totals[id] += now - this.lastTime;
    this.lastTime = now;
    this.ids.pop();
  }

  print(text) {
    const ids = Object.keys(this.totals);
    if (ids.length > 0) {
      ids.sort((a, b) => this.totals[b] - this.totals[a]);
      printErr(
        text + ' times: \n' + ids.map((id) => id + ' : ' + this.totals[id] + ' ms').join('\n'),
      );
    }
  }
}

/**
 * Context in which JS library code is evaluated.  This is distinct from the
 * global scope of the compiler itself which avoids exposing all of the compiler
 * internals to user JS library code.
 */
export const compileTimeContext = vm.createContext({
  process,
  console,
});

/**
 * A symbols to the macro context.
 * This will makes the symbols available to JS library code at build time.
 */
export function addToCompileTimeContext(object) {
  Object.assign(compileTimeContext, object);
}

export function applySettings(obj) {
  // Make settings available both in the current / global context
  // and also in the macro execution contexted.
  Object.assign(globalThis, obj);
  addToCompileTimeContext(obj);
}

export function loadSettingsFile(f) {
  var settings = {};
  vm.runInNewContext(read(f), settings, {filename: find(f)});
  applySettings(settings);
}

export function runInMacroContext(code, options) {
  compileTimeContext['__filename'] = options.filename;
  compileTimeContext['__dirname'] = path.dirname(options.filename);
  return vm.runInContext(code, compileTimeContext, options);
}

addToCompileTimeContext({
  assert,
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
