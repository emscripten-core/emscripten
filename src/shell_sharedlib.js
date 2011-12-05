// Capture the output of this into a variable, if you want
(function(FUNCTION_TABLE_OFFSET, globalScope) {
  var Module = {};
  var args = [];
  Module.arguments = [];

  {{BODY}}

  // {{MODULE_ADDITIONS}}

  return Module;
});
