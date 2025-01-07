// dyncalls
var dynCall_v = Module["dynCall_v"] = () => wasmExports["dynCall_v"]();
var dynCall_vi = Module["dynCall_vi"] = () => wasmExports["dynCall_vi"]();
var dynCall_vii = Module["dynCall_vii"] = () => wasmExports["dynCall_vii"]();
var dynCall_viii = Module["dynCall_viii"] = () => wasmExports["dynCall_viii"]();

// a dynamic dynCall
function dynCall(sig) {
  Module['dynCall_' + sig]();
}
// not exported, not used, so it should not succeed in keeping dynCall_* alive

var wasmImports = {
};

