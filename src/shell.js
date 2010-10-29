
var that = this;
// Capture the output of this into a variable, if you want
(function(Module, args) {
  Module = Module || {};
  arguments = arguments || [];

  {{BODY}}

  return Module;
}).apply(this, this.arguments); // Replace parameters as needed

