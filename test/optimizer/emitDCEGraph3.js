// dyncalls
var dynCall_v = Module["dynCall_v"] = function() {  return Module["asm"]["dynCall_v"].apply(null, arguments) };
var dynCall_vi = Module["dynCall_vi"] = function() {  return Module["asm"]["dynCall_vi"].apply(null, arguments) };
var dynCall_vii = Module["dynCall_vii"] = function() {  return Module["asm"]["dynCall_vii"].apply(null, arguments) };
var dynCall_viii = Module["dynCall_viii"] = function() {  return Module["asm"]["dynCall_viii"].apply(null, arguments) };

// a dynamic dynCall
function dynCall(sig) {
  eval('?');
}

dynCall('vi' + 'ii'); // a dynamic dyncall, causes uses of all the dynCall_*

var asmLibraryArg = {
};

