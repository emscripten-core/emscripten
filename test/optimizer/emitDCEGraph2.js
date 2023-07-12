// dyncalls
var dynCall_v = Module["dynCall_v"] = () => Module["asm"]["dynCall_v"]();
var dynCall_vi = Module["dynCall_vi"] = () => Module["asm"]["dynCall_vi"]();
var dynCall_vii = Module["dynCall_vii"] = () => Module["asm"]["dynCall_vii"]();
var dynCall_viii = Module["dynCall_viii"] = () => Module["asm"]["dynCall_viii"]();

// a dynamic dynCall
function dynCall(sig) {
  Module['dynCall_' + sig]();
}
Module['dynCall'] = dynCall; // exported, so all dyncalls must be rooted!

var wasmImports = {
};

