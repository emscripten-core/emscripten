"use strict";

/*
// Capture the output of this into a variable, if you want
(function(Module, args) {
  Module = Module || {};
  Module.arguments = args || [];
*/

///*
// Runs much faster, for some reason
if (!this['Module']) {
  this['Module'] = {};
}
try {
  Module.arguments = scriptArgs;
} catch(e) {
  try {
    Module.arguments = arguments;
  } catch(e) {
    Module.arguments = [];
  }
}
//*/

  {{BODY}}

  // {{MODULE_ADDITIONS}}

/*
  return Module;
}).call(this, {}, arguments); // Replace parameters as needed
*/

