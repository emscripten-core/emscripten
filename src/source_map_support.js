/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

class WasmSourceMap {
  mapping = {};
  offsets = [];

  constructor(sourceMap) {
    this.version = sourceMap.version;
    this.sources = sourceMap.sources;
    this.names = sourceMap.names;

    const vlqMap = {};
    'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/='.split('').forEach((c, i) => vlqMap[c] = i);

    // based on https://github.com/Rich-Harris/vlq/blob/master/src/vlq.ts
    function decodeVLQ(string) {
      const result = [];
      let shift = 0;
      let value = 0;

      for (const ch of string) {
        const integer = vlqMap[ch];
        if (integer === undefined) {
          throw new Error(`Invalid character (${ch})`);
        }

        value += (integer & 31) << shift;

        if (integer & 32) {
          shift += 5;
        } else {
          const negate = value & 1;
          value >>= 1;
          result.push(negate ? -value : value);
          value = shift = 0;
        }
      }
      return result;
    }

    let offset = 0, src = 0, line = 1, col = 1, name = 0;
    for (const segment of sourceMap.mappings.split(',')) {
      if (!segment) continue;
      const data = decodeVLQ(segment);
      const info = {};

      offset += data[0];
      if (data.length >= 2) info.source = src += data[1];
      if (data.length >= 3) info.line = line += data[2];
      if (data.length >= 4) info.column = col += data[3];
      if (data.length >= 5) info.name = name += data[4];
      this.mapping[offset] = info;
      this.offsets.push(offset);
    }
    this.offsets.sort((a, b) => a - b);
  }

  lookup(offset) {
    const normalized = this.normalizeOffset(offset);
    const info = this.mapping[normalized];
    // A segment with only an offset marks code that has no source location.
    if (!info || info.source === undefined) {
      return null;
    }
    return {
      file: this.sources[info.source],
      line: info.line,
      column: info.column,
      name: this.names[info.name],
    };
  }

  normalizeOffset(offset) {
    let lo = 0;
    let hi = this.offsets.length;
    let mid;

    while (lo < hi) {
      mid = Math.floor((lo + hi) / 2);
      if (this.offsets[mid] > offset) {
        hi = mid;
      } else {
        lo = mid + 1;
      }
    }
    return this.offsets[lo - 1];
  }
}

let wasmSourceMap;
#if MINIMAL_RUNTIME
const wasmSourceMapFile = '{{{ WASM_BINARY_FILE }}}.map';
#else
const wasmSourceMapFile = locateFile('{{{ WASM_BINARY_FILE }}}.map');
#endif

function receiveSourceMapJSON(sourceMap) {
  wasmSourceMap = new WasmSourceMap(sourceMap);
}

function getSourceMap() {
  const buf = readBinary(wasmSourceMapFile);
  return JSON.parse(UTF8ArrayToString(buf));
}

async function getSourceMapAsync() {
  if (ENVIRONMENT_IS_WEB
#if ENVIRONMENT_MAY_BE_WORKER
   || ENVIRONMENT_IS_WORKER
#endif
   ) {
    try {
      const response = await fetch(wasmSourceMapFile, {{{ makeModuleReceiveExpr('fetchSettings', "{ credentials: 'same-origin' }") }}});
      return response.json();
    } catch {
      // Fall back to getSourceMap below
    }
  }
  return getSourceMap();
}


#if PTHREADS || WASM_WORKERS
// Source map is received via postMessage on worker threads.
if ({{{ ENVIRONMENT_IS_MAIN_THREAD() }}}) {
#endif

#if !MINIMAL_RUNTIME // MINIMAL_RUNTIME integrates source map loading into postamble_minimal.js
#if WASM_ASYNC_COMPILATION
addRunDependency('source-map');
getSourceMapAsync().then((json) => {
  receiveSourceMapJSON(json);
  removeRunDependency('source-map');
});
#else
receiveSourceMapJSON(getSourceMap());
#endif
#endif

#if PTHREADS || WASM_WORKERS
}
#endif
