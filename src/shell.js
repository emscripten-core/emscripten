
// Capture the output of this into a variable, if you want
//(function(Module, args) {
//  Module = Module || {};
//  args = args || [];

// Runs much faster, for some reason
this['Module'] = {};
var args = arguments;
  {{BODY}}

  // {{MODULE_ADDITIONS}}

//  return Module;
//})({}, this.arguments); // Replace parameters as needed

