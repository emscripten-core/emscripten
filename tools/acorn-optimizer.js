var acorn = require('acorn');
var acorn_walk = require('acorn-walk');
var astring = require('astring');
var fs = require('fs');
var path = require('path');

// Setup

var print = function(x) {
  process['stdout'].write(x + '\n');
};

var printErr = function(x) {
  process['stderr'].write(x + '\n');
};

// Main

var arguments = process['argv'].slice(2);;

var ast = acorn.parse(code, { ecmaVersion: 6 });
var code = astring.generate(ast);
print(code);

