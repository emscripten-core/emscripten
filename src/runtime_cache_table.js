#if CACHE_WASM_TABLE
var originalWasmTable = wasmTable;
wasmTable = {
  _cache: [],
  length: wasmTable.length,
  get: function(i) {
    if (i >= wasmTable._cache.length) {
      wasmTable._cache.length = i;
    }
    var existing = wasmTable._cache[i];
    if (existing) {
      return existing;
    }
    return wasmTable._cache[i] = originalWasmTable.get(i);
  },
  set: function(i, func) {
    if (i >= wasmTable._cache.length) {
      wasmTable._cache.length = i;
    }
    wasmTable._cache[i] = func;
    return originalWasmTable.set(i, func);
  },
  grow: function(by) {
    var ret = originalWasmTable.grow(by);
    length = wasmTable.length;
    return ret;
  }
};
#endif
