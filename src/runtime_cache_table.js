#if CACHE_WASM_TABLE
// Wrap the wasm table with a cache, to avoid repeatedly calling table.get().
// That operation is fairly heavyweight in VMs.
// N.B.: This cache assumes that the wasm does not modify the table, which is
//       not necessarily true in relocatable builds (a side module segment may
//       add items), or if we some day use wasm table operations in wasm.
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
    wasmTable.length = originalWasmTable.length;
    return ret;
  }
};
#endif
