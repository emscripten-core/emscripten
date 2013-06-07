/*
 * Quick utility script for the Python test script to call. Could be replaced if
 * a good Python source map library is found.
 */
var SourceMapConsumer = require('source-map').SourceMapConsumer;
var fs = require('fs');

var consumer = new SourceMapConsumer(fs.readFileSync(process.argv[2], 'utf-8'));
var mappings = [];

consumer.eachMapping(function(mapping) {
  mappings.push(mapping);
});

console.log(JSON.stringify(mappings));
