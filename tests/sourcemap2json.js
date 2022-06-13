#!/usr/bin/env node
// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Quick utility script USED ONLY FOR TESTING.
// Could be replaced if a good python source map library is found.

'use strict';

var SourceMapConsumer = require('source-map').SourceMapConsumer;
var fs = require('fs');

new SourceMapConsumer(fs.readFileSync(process.argv[2], 'utf-8')).then((consumer) => {
  var mappings = [];

  consumer.eachMapping(function(mapping) {
    mappings.push(mapping);
  });

  console.log(JSON.stringify(mappings, null, 2));
});
