var acorn = require('acorn');
var acorn_walk = require('acorn-walk');
var astring = require('astring');
var fs = require('fs');
var path = require('path');

// Setup

var print = function(x) {
  process.stdout.write(x + '\n');
};

var printErr = function(x) {
  process.stderr.write(x + '\n');
};

var read = function(x) {
  return fs.readFileSync(x).toString();
}

// Main

var arguments = process['argv'].slice(2);;
var infile = arguments[0];
var passes = arguments.slice(1);

var input = read(infile);
var ast = acorn.parse(input, { ecmaVersion: 6 });
print(JSON.stringify(ast));
var output = astring.generate(ast);
print(output);

