#!/usr/bin/env node
// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

"use strict";

var fs = require('fs');
var path = require('path');

var START_MARKER = '// EMSCRIPTEN_START_FUNCS\n';
var END_MARKER = '// EMSCRIPTEN_END_FUNCS\n';

function countLines(s) {
  var count = 0;
  for (var i = 0, l = s.length; i < l; i ++) {
    if (s[i] === '\n') count++;
  }
  return count;
}

// For a minor optimization, only do win32->unix normalization if we are actually on Windows,
// which avoids redundantly scanning files if not needed.
var isWindows = (process.platform === 'win32');

var unixPathRe = new RegExp('\\\\', 'g');
// Returns the given (possibly Windows) path p normalized to unix path separators '/'.
function toUnixPath(p) {
  if (isWindows) {
    return p.replace(unixPathRe, '/');
  } else {
    return p;
  }
}

var unixLineEndRe = new RegExp('\r\n', 'g');
// Returns the given (possibly Windows) text data t normalized to unix line endings '\n'.
function toUnixLineEnding(t) {
  if (isWindows) {
    return t.replace(unixLineEndRe, '\n');
  } else {
    return t;
  }
}

// If path "p2" is a relative path, joins paths p1 and p2 to form "p1/p2". If p2 is an absolute path, "p2" is returned.
function joinPath(p1, p2) {
  if (p2[0] == '/' || (p2.length >= 3 && p2[1] == ':' && (p2[2] == '/' || p2[2] == '\\'))) // Is p2 an absolute path?
    return p2;
  else
    return toUnixPath(path.join(p1, p2));
}

/*
 * Extracts the line (not block) comments from the generated function code and
 * invokes commentHandler with (comment content, line number of comment). This
 * function implements a simplistic lexer with the following assumptions:
 *     1. "// EMSCRIPTEN_START_FUNCS" and "// EMSCRIPTEN_END_FUNCS" are unique
 *     markers for separating library code from generated function code. Things
 *     will break if they appear as part of a string in library code (but OK if
 *     they occur in generated code).
 *     2. Between these two markers, no regexes are used.
 */
function extractComments(source, commentHandler) {
  var state = 'code';
  var commentContent = '';
  var functionStartIdx = source.indexOf(START_MARKER);
  var functionEndIdx = source.lastIndexOf(END_MARKER);
  var lineCount = countLines(source.slice(0, functionStartIdx)) + 2;

  for (var i = functionStartIdx + START_MARKER.length; i < functionEndIdx; i++) {
    var c = source[i];
    var nextC = source[i+1];
    switch (state) {
      case 'code':
          if (c === '/') {
            if (nextC === '/') { state = 'lineComment'; i++; }
            else if (nextC === '*') { state = 'blockComment'; i++; }
          }
          else if (c === '"') state = 'doubleQuotedString';
          else if (c === '\'') state = 'singleQuotedString';
          break;
      case 'lineComment':
          if (c === '\n') {
            state = 'code';
            commentHandler(commentContent, lineCount);
            commentContent = "";
          } else {
            commentContent += c;
          }
          break;
      case 'blockComment':
          if (c === '*' && nextC === '/') state = 'code';
      case 'singleQuotedString':
          if (c === '\\') i++;
          else if (c === '\'') state = 'code';
          break;
      case 'doubleQuotedString':
          if (c === '\\') i++;
          else if (c === '"') state = 'code';
          break;
    }

    if (c === '\n') lineCount++;
  }
}

function getMappings(source) {
  // generatedLineNumber -> { originalLineNumber, originalFileName }
  var mappings = {};
  extractComments(source, function(content, generatedLineNumber) {
    var matches = /@line (\d+)(?: "([^"]*)")?/.exec(content);
    if (matches === null) return;
    var originalFileName = matches[2];
    mappings[generatedLineNumber] = {
      originalLineNumber: parseInt(matches[1], 10),
      originalFileName: originalFileName
    }
  });
  return mappings;
}

function generateMap(mappings, sourceRoot, mapFileBaseName, generatedLineOffset) {
  var SourceMapGenerator = require('source-map').SourceMapGenerator;

  var generator = new SourceMapGenerator({ file: mapFileBaseName });
  var seenFiles = Object.create(null);

  for (var generatedLineNumber in mappings) {
    var generatedLineNumber = parseInt(generatedLineNumber, 10);
    var mapping = mappings[generatedLineNumber];
    var originalFileName = mapping.originalFileName;
    generator.addMapping({
      generated: { line: generatedLineNumber + generatedLineOffset, column: 0 },
      original: { line: mapping.originalLineNumber, column: 0 },
      source: originalFileName
    });

    // we could call setSourceContent repeatedly, but readFileSync is slow, so
    // avoid doing it unnecessarily
    if (!(originalFileName in seenFiles)) {
      seenFiles[originalFileName] = true;
      var rootedPath = joinPath(sourceRoot, originalFileName);
      try {
        generator.setSourceContent(originalFileName, fs.readFileSync(rootedPath, 'utf-8'));
      } catch (e) {
        console.warn("sourcemapper: Unable to find original file for " + originalFileName +
          " at " + rootedPath);
      }
    }
  }

  fs.writeFileSync(mapFileBaseName + '.map', generator.toString());
}

function appendMappingURL(fileName, source, mapFileName) {
  var lastLine = source.slice(source.lastIndexOf('\n'));
  if (!/sourceMappingURL/.test(lastLine))
    fs.appendFileSync(fileName, '//# sourceMappingURL=' + path.basename(mapFileName));
}

function parseArgs(args) {
  var rv = { _: [] }; // unflagged args go into `_`; similar to the optimist library
  for (var i = 0; i < args.length; i++) {
    if (/^--/.test(args[i])) rv[args[i].slice(2)] = args[++i];
    else rv._.push(args[i]);
  }
  return rv;
}

if (require.main === module) {
  if (process.argv.length < 3) {
    console.log('Usage: ./sourcemapper.js <original js> <optimized js file ...> \\\n' +
                '\t--sourceRoot <default "."> \\\n' +
                '\t--mapFileBaseName <default `filename`> \\\n' +
                '\t--offset <default 0>');
    process.exit(1);
  } else {
    var opts = parseArgs(process.argv.slice(2));
    var fileName = opts._[0];
    var sourceRoot = opts.sourceRoot ? toUnixPath(opts.sourceRoot) : ".";
    var mapFileBaseName = toUnixPath(opts.mapFileBaseName ? opts.mapFileBaseName : fileName);
    var generatedLineOffset = opts.offset ? parseInt(opts.offset, 10) : 0;

    var generatedSource = toUnixLineEnding(fs.readFileSync(fileName, 'utf-8'));
    var source = generatedSource;
    var mappings = getMappings(generatedSource);
    for (var i = 1, l = opts._.length; i < l; i ++) {
      var optimizedSource = toUnixLineEnding(fs.readFileSync(opts._[i], 'utf-8'))
      var optimizedMappings = getMappings(optimizedSource);
      var newMappings = {};
      // uglify processes the code between EMSCRIPTEN_START_FUNCS and
      // EMSCRIPTEN_END_FUNCS, so its line number maps are relative to those
      // markers. we correct for that here. +2 = 1 for the newline in the marker
      // and 1 to make it a 1-based index.
      var startFuncsLineNumber = countLines(source.slice(0, source.indexOf(START_MARKER))) + 2;
      for (var line in optimizedMappings) {
        var originalLineNumber = optimizedMappings[line].originalLineNumber + startFuncsLineNumber;
        if (originalLineNumber in mappings) {
          newMappings[line] = mappings[originalLineNumber];
        }
      }
      mappings = newMappings;
      source = optimizedSource;
    }

    generateMap(mappings, sourceRoot, mapFileBaseName, generatedLineOffset);
    appendMappingURL(opts._[opts._.length - 1], generatedSource,
                     opts.mapFileBaseName + '.map');
  }
}
