// Capture the output of this into a variable, if you want
(function(FUNCTION_TABLE_OFFSET, parentModule) {
  var Module = {};
  var args = [];
  Module.arguments = [];

  {{BODY}}

  // {{MODULE_ADDITIONS}}

  return Module;
});
