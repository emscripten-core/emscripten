#! /usr/bin/env node

"use strict";

function countLines(s) {
  var count = 0;
  for (var i = 0, l = s.length; i < l; i ++) {
    if (s[i] === '\n') count++;
  }
  return count;
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
  var functionStartIdx = source.indexOf('// EMSCRIPTEN_START_FUNCS');
  var functionEndIdx = source.lastIndexOf('// EMSCRIPTEN_END_FUNCS');
  var lineCount = countLines(source.slice(0, functionStartIdx));

  for (var i = functionStartIdx; i < functionEndIdx; i++) {
    var c = source[i];
    var nextC = source[i+1];
    if (c === '\n') lineCount++;
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
  }
}

function generateMap(fileName, sourceRoot, mapFileBaseName, generatedLineOffset) {
  var fs = require('fs');
  var path = require('path');
  var SourceMapGenerator = require('source-map').SourceMapGenerator;

  var generator = new SourceMapGenerator({ file: mapFileBaseName });
  var generatedSource = fs.readFileSync(fileName, 'utf-8');
  var seenFiles = Object.create(null);

  extractComments(generatedSource, function(content, generatedLineNumber) {
    var matches = /@line (\d+) "([^"]*)"/.exec(content);
    if (matches === null) return;
    var originalLineNumber = parseInt(matches[1], 10);
    var originalFileName = matches[2];

    if (!(originalFileName in seenFiles)) {
      seenFiles[originalFileName] = true;
      var rootedPath = originalFileName[0] === path.sep ?
          originalFileName : path.join(sourceRoot, originalFileName);
      try {
        generator.setSourceContent(originalFileName, fs.readFileSync(rootedPath, 'utf-8'));
      } catch (e) {
        console.warn("Unable to find original file for " + originalFileName +
          " at " + rootedPath);
      }
    }

    generator.addMapping({
      generated: { line: generatedLineNumber + generatedLineOffset, column: 0 },
      original: { line: originalLineNumber, column: 0 },
      source: originalFileName
    });
  });

  var mapFileName = mapFileBaseName + '.map';
  fs.writeFileSync(mapFileName, generator.toString());

  var lastLine = generatedSource.slice(generatedSource.lastIndexOf('\n'));
  if (!/sourceMappingURL/.test(lastLine))
    fs.appendFileSync(fileName, '//@ sourceMappingURL=' + path.basename(mapFileName));
}

if (require.main === module) {
  if (process.argv.length < 3) {
    console.log('Usage: ./sourcemapper.js <filename> <source root (default: .)> ' +
                '<map file basename (default: filename)>' +
                '<generated line offset (default: 0)>');
    process.exit(1);
  } else {
    var sourceRoot = process.argv.length > 3 ? process.argv[3] : ".";
    var mapFileBaseName = process.argv.length > 4 ? process.argv[4] : process.argv[2];
    var generatedLineOffset = process.argv.length > 5 ?
        parseInt(process.argv[5], 10) : 0;
    generateMap(process.argv[2], sourceRoot, mapFileBaseName, generatedLineOffset);
  }
}
