#! /usr/bin/env node

global.sys = require(/^v0\.[012]/.test(process.version) ? "sys" : "util");
var fs = require("fs");
var uglify = require("uglify-js"), // symlink ~/.node_libraries/uglify-js.js to ../uglify-js.js
    jsp = uglify.parser,
    pro = uglify.uglify;

var code = fs.readFileSync("hoist.js", "utf8");
var ast = jsp.parse(code);

ast = pro.ast_lift_variables(ast);

console.log(pro.gen_code(ast, {
        beautify: true
}));
