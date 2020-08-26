#!/usr/bin/env node
// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
//
// Preprocessor tool.  This is a wrapper for the 'preprocess' function which
// allows it to be called as a standalone tool.
//
// Parameters:
//    setting file.  Can specify 'settings.js' here, alternatively create a temp
//                   file with modified settings and supply the filename here.
//    shell file     This is the file that will be processed by the preprocessor


var fs = require('fs');
var path = require('path');

var arguments_ = process['argv'].slice(2);
var debug = false;

print = function(x) {
  process['stdout'].write(x + '\n');
};
printErr = function(x) {
  process['stderr'].write(x + '\n');
};

function find(filename) {
  var prefixes = [process.cwd(), path.join(__dirname, '..', 'src')];
  for (var i = 0; i < prefixes.length; ++i) {
    var combined = path.join(prefixes[i], filename);
    if (fs.existsSync(combined)) {
      return combined;
    }
  }
  return filename;
}

read = function(filename) {
  var absolute = find(filename);
  return fs.readFileSync(absolute).toString();
};

load = function(f) {
  eval.call(null, read(f));
};

var settings_file = arguments_[0];
var shell_file = arguments_[1];
var process_macros = arguments_.indexOf('--expandMacros') >= 0;

load(settings_file)
load('utility.js');
load('modules.js');
load('parseTools.js');

var from_html = read(shell_file);
var to_html = process_macros ? processMacros(preprocess(from_html, shell_file)) : preprocess(from_html, shell_file);

print(to_html);
