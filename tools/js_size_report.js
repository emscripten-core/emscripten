var acorn = require('../third_party/acorn');
var fs = require('fs');
var assert = require('assert');
var path = require('path');

function calculateLineStartPositions(filename) {
  var fileSizeInBytes = fs.statSync(filename)['size'];
  var f = fs.readFileSync(filename);
  console.log('File ' + filename + ', size ' + fileSizeInBytes);
  var lineStartPositions = [0];
  for(var i = 0; i < fileSizeInBytes; ++i) {
    var byte = f.readUInt8(i);
    console.log(byte);
    if (byte == 0x0A) {
      lineStartPositions.push(i+1);
    }
  }
  console.dir(lineStartPositions);
  return lineStartPositions;
}

function mapLineColTo1DPos(line, col, lineStartPositions) {
  console.log(line);
  assert(line >= 0);
  assert(line < lineStartPositions.length);
  return lineStartPositions[line] + col;
}

function readSourceMap(sourceMapFile, mangledFileLineStartPositions) {
  var sourceMapJson = JSON.parse(fs.readFileSync(sourceMapFile).toString());
  var lines = sourceMapJson['mappings'].split(';');

  var base64DecodeMap = {};
  var chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
  for(var i = 0; i < chars.length; ++i) {
    base64DecodeMap[chars[i]] = i
  }

  function decodeVLQ(vlq) {
    var outputList = [];
    var val = 0, shift = 0;

    for(var i = 0; i < vlq.length; ++i) {
      var bits = base64DecodeMap[vlq[i]];
      val |= (bits & 0x1F) << shift;
      if (bits & 0x20) {
        shift += 5;
      } else {
        var sign = (val & 1) ? -1 : 1;
        val >>= 1;
        outputList.push(val * sign);
        val = shift = 0;
      }
    }
    return outputList;
  }

  var sourceMap = [];

  var smSourceFile = 0;
  var smSourceLine = 0;
  var smSourceColumn = 0;
  var smNameIndex = 0;
  var outputLine = 0;
  for(var l in lines) {
    var line = lines[l];
    var segments = line.split(',');

    var smOutputColumn = 0;
    for(var s in segments) {
      var segment = segments[s];
      var segmentArray = decodeVLQ(segment);

//      console.dir(segmentArray);
      if (segmentArray.length > 0) {
        smOutputColumn += segmentArray[0];
        if (segmentArray.length > 1) {
          smSourceFile += segmentArray[1];
          smSourceLine += segmentArray[2];
          smSourceColumn += segmentArray[3];
          if (segmentArray.length > 4) {
            smNameIndex += segmentArray[4];

            sourceMap.push([mapLineColTo1DPos(outputLine, smOutputColumn, mangledFileLineStartPositions), sourceMapJson['sources'][smSourceFile], smSourceLine, smSourceColumn, sourceMapJson['names'][smNameIndex]]);
/*
            sourceVariableName = Building.extract_variable_name(input_text, sm_source_line, sm_source_column, input_line_positions)
            minified_output_variable_name = Building.extract_variable_name(output_text, output_line, sm_output_column, output_line_positions)
#             print('VLQ ' + s + ', output: ' + str(segmentArray)
#               + ', source: ' 
#               + str(sm_source_line) + ':' + str(sm_source_column)
#               + ' source name: ' + source_variable_name
#               + ', output: ' + str(output_line) + ':' + str(sm_output_column) + ', unminified name: "' + source_map['names'][sm_name_index] + '"'
#               + ' minified name: "' + minified_output_variable_name + '"')

            if Building.points_to_a_function(input_text, sm_source_line, sm_source_column, input_line_positions):
              functions_symbol_map += minified_output_variable_name + ':' + source_variable_name + '\n'
*/
          }
        }
      }
    }
    ++outputLine;
  }

  return sourceMap;
}

function dump(node) {
  console.log(JSON.stringify(node, null, ' '));
}

function nodeSize(node) {
  return node.end - node.start;
}

function getSizeExcludingChildren(node) {
  var totalSize = nodeSize(node);
  if (Array.isArray(node.body)) {
    for(var i in node.body) {
      totalSize -= nodeSize(node.body[i]);
    }
  }
  return totalSize;
}

function getNodeName(node) {
  if (node.type == 'ExpressionStatement' && node.expression.type == 'AssignmentExpression'  && node.expression.left.type == 'Identifier') {
    return node.expression.left.name;
  }
  if (node.type == 'Property') {
    return node.key.name || node.value.name;
  }
  if (node.type == 'FunctionExpression' || node.type == 'FunctionDeclaration') {
    return node.id.name;
  }
  if (node.type == 'VariableDeclaration') {
    return node.declarations[0].id.name;
  }
}

function countNodeSizes(nodeSizes) {
  var totalSize = 0;
  for(var i in nodeSizes) {
    totalSize += nodeSizes[i].size;
  }
  return totalSize;
}

function countNodeCodeSizes(nodeSizes) {
  var totalSize = 0;
  for(var i in nodeSizes) {
    var node = nodeSizes[i];
    if (node.type != 'var' && node.type != 'function') {
      totalSize += node.size;
    }
  }
  return totalSize;
}

function mergeKeyValues(dst, src) {
  for(var i in src) {
    dst[i] = src[i];
  }
}

function recordSourceFile(nodeSizes, filename) {
  for(var n in nodeSizes) {
    nodeSizes[n].file = path.basename(filename);
  }
}

var inputFile;
var sourceMap;
var sourceLineStartPositions;
var fileSizeInBytes;

function demangleName(node, functionName) {
  var s = 0;
  var e = sourceMap.length-1;
  console.log('a');
  while(s < e) {
    var mid = ((s+e)/2)|0;
    console.log('s: ' + s + ', e: ' + e + ', mid: ' + mid + ', node.start: ' + node.start + ', midpos: ' + sourceMap[mid][0]);
    if (sourceMap[mid][0] < node.start) {
      s = mid+1;
    } else if (sourceMap[mid][0] > node.start) {
      e = mid;
    } else if (sourceMap[mid][0] == node.start) {
      return sourceMap[mid][4];
    } else {
      console.dir(sourceMap[mid]);
      assert(false);
    }
  }
  console.log('b');
//  console.dir(sourceMap);
//  console.dir(sourceMap[s]);
  return sourceMap[s][4];
}

function isLargeEnoughNodeToExpand(node) {
  return nodeSize(node) >= 0.10*fileSizeInBytes;
}

function collectNodeSizes(nodeArray, parentPrefix) {
  while (!Array.isArray(nodeArray)) {
    if (nodeArray.type == 'BlockStatement') nodeArray = nodeArray.body;
    if (nodeArray.type == 'FunctionExpression') {
      parentPrefix += nodeArray.id.name + '/';
      nodeArray = nodeArray.body;
    }
    else break;
  }
  if (!Array.isArray(nodeArray)) {
    console.dir(nodeArray);
    console.log(inputFile.substring(nodeArray.start, nodeArray.end));
    console.log(inputFile.substring(nodeArray.start - 50, nodeArray.end + 50));
  }
  assert(Array.isArray(nodeArray));
  var sizes = {};
  for(var i in nodeArray) {
    var node = nodeArray[i];
    var childSize = nodeSize(node);
    var childName = getNodeName(node);
    var childType = null;

    var childSizes = null;

    if (node.type == 'ExpressionStatement' && node.expression.type == 'AssignmentExpression'
      && (node.expression.right.type == 'FunctionExpression' || node.expression.right.type == 'CallExpression')) {
      var target = node.expression.right.type == 'FunctionExpression' ? node.expression.right.body : node.expression.right.callee;
        if (target.type == 'BlockStatement' && isLargeEnoughNodeToExpand(target)) {
          childSizes = collectNodeSizes(target, parentPrefix + childName + '/');
        }
        childType = 'function';
    } else if (node.type == 'Property') {
      if (node.value.type == 'FunctionExpression') {
        if (isLargeEnoughNodeToExpand(node.value)) {
          childSizes = collectNodeSizes(node.value.body, parentPrefix + childName + '/');
        }
        childType = 'function';
      }
      else if (['Literal', 'ArrayExpression', 'ObjectExpression', 'Identifier'].indexOf(node.value.type) != -1) {
        childType = 'var';
      }
    } else if (node.type == 'VariableDeclaration') {
      childType = 'var';
      if (node.declarations[0].init && node.declarations[0].init.type == 'FunctionExpression') {
        if (isLargeEnoughNodeToExpand(node.declarations[0].init)) {
          childSizes = collectNodeSizes(node.declarations[0].init.body, parentPrefix + childName + '/');
        }
        childType = 'function';
      } else if (node.declarations[0].init && node.declarations[0].init.type == 'CallExpression' && node.declarations[0].init.callee.type == 'FunctionExpression') {
        if (isLargeEnoughNodeToExpand(node.declarations[0].init.callee)) {
          childSizes = collectNodeSizes(node.declarations[0].init.callee, parentPrefix + childName + '/');
        }
        childType = 'function';
      } else if (node.declarations[0].init && node.declarations[0].init.type == 'ObjectExpression') {
        if (isLargeEnoughNodeToExpand(node)) {
          childSizes = collectNodeSizes(node.declarations[0].init.properties, parentPrefix + childName + '.');
        }
      }
    } else if (node.type == 'FunctionExpression' || node.type == 'FunctionDeclaration') {
        if (isLargeEnoughNodeToExpand(node)) {
          childSizes = collectNodeSizes(node.body, parentPrefix + childName + '/');
        }
        childType = 'function';
    } else if (parentPrefix == '') {
      childType = 'code';
      var childCode = inputFile.substring(node.start, node.end).trim();
      if (childCode.length > 32) {
        childName = '"' + childCode.substring(0, 29).replace(/\n/g, ' ') + '..."';
      } else {
        childName = '"' + childCode.replace(/\n/g, ' ') + '"';
      }
    }

    // Try to demangle the name with source maps.
    if (sourceMap && (childType == 'function' || childType == 'var')) {
      var demangledName = demangleName(node, childName);
      if (demangledName) {
        childName = childName + ' (' + demangledName + ')';
      }
    }

    sizes[parentPrefix + childName] = {
      'type': childType,
      'name': parentPrefix + childName,
      'prefix': parentPrefix,
      'selfName': childName,
      'demangledName': demangledName,
      'size': childSize - countNodeSizes(childSizes)
    }
    mergeKeyValues(sizes, childSizes);
  }
  return sizes;
}

function extractJavaScriptCodeSize(sourceFile) {
  fileSizeInBytes = fs.statSync(sourceFile)['size'];
  inputFile = fs.readFileSync(sourceFile).toString();
  var ast = acorn.parse(inputFile, { ecmaVersion: 6 });

  var nodeSizes = collectNodeSizes(ast.body, '');
  var totalAccountedFor = 0;
  for(var i in nodeSizes) {
    var node = nodeSizes[i];
    if (node.type) {
      totalAccountedFor += node.size;
    }
  }

  var whitespaceBytes = fileSizeInBytes - countNodeSizes(nodeSizes);
  /*
  var topLevelCodeBytes = countNodeCodeSizes(nodeSizes);
  if (topLevelCodeBytes > 0) {
    var name = path.basename(sourceFile) + '/topLevelCode';
    sizes[name] = {
      'type': 'code',
      'name': name,
      'size': topLevelCodeBytes
    };
    console.log('top-level code: ' + topLevelCodeBytes);
  }
  */
  if (whitespaceBytes > 0) {
    var name = 'whitespace';
    nodeSizes[name] = {
      'type': 'other',
      'prefix': '',
      'name': name,
      'size': whitespaceBytes
    };
  //  console.log('Other unaccounted (e.g. whitespace and comments): ' + whitespaceBytes);
  }

  recordSourceFile(nodeSizes, sourceFile);
  return nodeSizes;

//  console.log('---');
//  console.log('File ' + sourceFile + ': total size ' + fileSizeInBytes + ' bytes.');
//  console.log('Total accounted above: ' + totalAccountedFor);
}

function sectionIdToString(id) {
  return ['CUSTOM', 'TYPE', 'IMPORT', 'FUNCTION', 'TABLE', 'MEMORY', 'GLOBAL', 'EXPORT', 'START', 'ELEMENT', 'CODE', 'DATA'][id];
}

function readLEB128(file, cursor) {
  var n = 0;
  var shift = 0;
  while(1) {
    var byte = file.readUInt8(cursor++);
    n |= (byte & 0x7F) << shift;
    shift += 7;
    if (!(byte & 0x80))
      return [n, cursor];
  }
}

function extractWasmCodeSection(wasm, cursor) {
  var numFunctions;
  [numFunctions, cursor] = readLEB128(wasm, cursor);
  var sizes = {};
  for(var i = 0; i < numFunctions; ++i) {
    var cursorStart = cursor;
    var functionSize;
    [functionSize, cursor] = readLEB128(wasm, cursor);
    cursor += functionSize;
    sizes[i] = {
      'type': 'function',
      'prefix': '',
      'name': i,
      'size': cursor - cursorStart
    };
  }
  return sizes;
}

function extractString(wasm, cursor) {
  var name = '';
  var nameLength;
  [nameLength, cursor] = readLEB128(wasm, cursor);
  for(var i = 0; i < nameLength; ++i) {
    var ch = wasm.readUInt8(cursor++);
    name += String.fromCharCode(ch);
  }
  return [name, cursor];
}

function extractWasmCustomSectionName(wasm, cursor) {
  return extractString(wasm, cursor)[0];
}

function extractWasmFunctionNames(wasm, cursor, sectionEndCursor) {
  var customSectionName;
  [customSectionName, cursor] = extractString(wasm, cursor);
  if (customSectionName != 'name') return [];

  var functionNames = {};

  while(cursor < sectionEndCursor) {
    var subsectionId = wasm.readUInt8(cursor++);
    var subsectionSize;
    [subsectionSize, cursor] = readLEB128(wasm, cursor);
    // Skip over other subsections than the function names subsection
    if (subsectionId != 1 /*function names subsection*/) {
      cursor += subsectionSize;
      continue;
    }
    var numNames;
    [numNames, cursor] = readLEB128(wasm, cursor);
    for(var i = 0; i < numNames; ++i) {
      var index, name;
      [index, cursor] = readLEB128(wasm, cursor);
      [name, cursor] = extractString(wasm, cursor);
//      console.log('i: ' + i + ', index: ' + index + ', name: ' + name);
      functionNames[index] = name;
    }
  }
  assert(cursor == sectionEndCursor);
  return Object.keys(functionNames).length > 0 ? functionNames : null;
}

function labelWasmFunctionsWithNames(sizes, wasmFunctionNames, numImports) {
  var labeledSizes = {};
  for(var name in sizes) {
    var symbol = sizes[name];
    if (symbol.type == 'function') {
      var functionNameIndex = (name|0) + numImports;
      var wasmName = wasmFunctionNames[functionNameIndex] || name;
      labeledSizes[wasmName] = sizes[name];
      labeledSizes[wasmName].name = wasmName;
    } else {
      labeledSizes[name] = symbol;
    }
  }
  return labeledSizes;
}

function extractNumWasmFunctionImports(wasm, cursor) {
  var numFunctionImports = 0;
  var numImports;
  [numImports, cursor] = readLEB128(wasm, cursor);
  for(var i = 0; i < numImports; ++i) {
    var mod, name;
    [mod, cursor] = extractString(wasm, cursor);
    [name, cursor] = extractString(wasm, cursor);
    var importType = wasm.readUInt8(cursor++);
    switch(importType) {
      case 0x00: /*function import*/
        ++numFunctionImports;
        cursor = readLEB128(wasm, cursor)[1]; // skip typeidx
        break;
      case 0x01: /*table import*/
        assert(wasm.readUInt8(cursor++) == 0x70);
        // pass-through: table import ends with a limits value, which is what memory also contains
      case 0x02: /*memory import*/
        var limitType = wasm.readUInt8(cursor++);
        assert(limitType == 0x00 || limitType == 0x01);
        cursor = readLEB128(wasm, cursor)[1]; // limit min
        if (limitType == 1) cursor = readLEB128(wasm, cursor)[1]; // limit max
        break;
      case 0x03: /*global import*/
        cursor += 2; // type of the global, and mutability (const/var).. skip over them
        break;
    }
  }
  return numFunctionImports;
}

function extractWasmCodeSize(sourceFile) {
  var fileSizeInBytes = fs.statSync(sourceFile)['size'];
  var wasm = fs.readFileSync(sourceFile);
  var magic = wasm.readUInt32LE(0);
  assert(magic == 0x6D736100);
  var version = wasm.readUInt32LE(4);
  assert(version == 1);
  var cursor = 8;

  var sizes = {};

  var numImports;
  var wasmFunctionNames;
  var unaccountedBytes = fileSizeInBytes;
  while(cursor < fileSizeInBytes) {
    var sectionStartCursor = cursor;
    var id = wasm.readInt8(cursor++);
    if (id === undefined) throw 'Failed to parse section ID in wasm file!';
    var name = sectionIdToString(id);
    [size, cursor] = readLEB128(wasm, cursor);
    var sectionEndCursor = cursor + size;
    var sectionSize = sectionEndCursor - sectionStartCursor;

    var functionSizes = null;
    if (id == 0 /*custom*/) {
      name = 'CUSTOM/"' + extractWasmCustomSectionName(wasm, cursor) + '"';
      wasmFunctionNames = extractWasmFunctionNames(wasm, cursor, sectionEndCursor) || wasmFunctionNames;
    } else if (id == 2 /*import*/) {
      numImports = extractNumWasmFunctionImports(wasm, cursor);
    } else if (id == 10 /*code*/) {
      functionSizes = extractWasmCodeSection(wasm, cursor);
    }
    cursor = sectionEndCursor;
    if (functionSizes) {
      var funcSizes = countNodeSizes(functionSizes);
      sectionSize -= funcSizes;
      unaccountedBytes -= funcSizes;
      mergeKeyValues(sizes, functionSizes);
    }
    sizes[name] = {
      'type': 'section',
      'prefix': '',
      'name': name,
      'size': sectionSize
    };
    unaccountedBytes -= sectionSize;
  }
  if (cursor > fileSizeInBytes) {
    throw 'Failed to parse sections in Wasm file!';
  }

  if (wasmFunctionNames) {
    sizes = labelWasmFunctionsWithNames(sizes, wasmFunctionNames, numImports);
  }

  if (unaccountedBytes > 0) {
    var name = 'file header';
    sizes[name] = {
      'type': 'wasmHeader',
      'prefix': '',
      'name': name,
      'size': unaccountedBytes
    };
  }
  recordSourceFile(sizes, sourceFile);
  return sizes;
}

function readSymbolMap(filename) {
  var symbolMap = {};

  var symbolFile = fs.readFileSync(filename).toString();
  var symbols = symbolFile.split('\n');
  for(var i in symbols) {
    var mangledUnmangled = symbols[i].split(':');
    symbolMap[mangledUnmangled[0]] = mangledUnmangled[1];
  }
  return symbolMap;
}

var sources = process['argv'].slice(2);

function readParam(param) {
  var idx = sources.indexOf(param);
  if (idx != -1) {
    var value = sources[idx+1];
    sources.splice(idx, 2);
    return value;
  }
}

var symbolMap = readParam('--symbols');
var sourceMap = readParam('--sourceMap');

if (symbolMap) {
  symbolMap = readSymbolMap(symbolMap);
}

if (sourceMap) {
  var mangledFileLineStartPositions = calculateLineStartPositions(sources[0]);
  console.dir(mangledFileLineStartPositions);
  sourceMap = readSourceMap(sourceMap, mangledFileLineStartPositions);
}

var codeSizes = {};

for(var s in sources) {
  var src = sources[s];
  var sizes;
  if (src.toLowerCase().endsWith('.js')) {
	  sizes = extractJavaScriptCodeSize(src);
    sourceMap = null; // Assume --sourceMap applies to the first .js file input
  } else if (src.endsWith('.wasm')) {
    sizes = extractWasmCodeSize(src);
  }
  mergeKeyValues(codeSizes, sizes);
}

codeSizes = Object.values(codeSizes).sort((a, b) => {
  return b.size - a.size;
})

function printedNodeType(nodeType) {
  if (nodeType == 'var' || nodeType == 'function' || nodeType == 'section') return nodeType + ' ';
  else return '';
}

function demangleSymbol(node, symbolMap) {
  if (symbolMap) {
    var demangledName = symbolMap[node.name] || symbolMap[node.selfName];
    if (demangledName) return node.prefix + demangledName;
  }
  return node.name;
}

for(var i in codeSizes) {
  var node = codeSizes[i];
  if (node.type) {
    console.log(node.file + '/' + printedNodeType(node.type) + demangleSymbol(node, symbolMap) + ': ' + node.size);
  }
}
