var acorn = require('../node_modules/acorn');
var fs = require('fs');
var assert = require('assert');
var path = require('path');

// Global state reused by various functions during a run:
var currentInputJsFile;
var currentInputJsFileSizeInBytes;
var dumpJsTextContents = [];
var runningCodeBlockCounter = 0;
var generatingSymbolMapFromSourceMap = false;

// Configurable command line options:
var expandSymbolsLargerThanPercents = 0.35;
var expandSymbolsLargerThanBytes = 32*1024;

// Given a text file, outputs an array containing byte locations
// to the start of each line. I.e. line N (zero-based) in the text file will start
// at byte location specified by index N of the returned array.
function calculateLineStartPositions(filename) {
  var fileSizeInBytes = fs.statSync(filename)['size'];
  var f = fs.readFileSync(filename);
  var lineStartPositions = [0];
  for(var i = 0; i < fileSizeInBytes; ++i) {
    var byte = f.readUInt8(i);
    if (byte == 0x0A) {
      lineStartPositions.push(i+1);
    }
  }
  return lineStartPositions;
}

// Given a line:col pair, returns the 1D byte location
// to that position in an input file, as mapped by lineStartPositions.
function mapLineColTo1DPos(line, col, lineStartPositions) {
  assert(line >= 0);
  if (line >= lineStartPositions.length) {
    console.error('Input file does not have a line:col ' + (line+1) + ':' + (col+1) + ', but only ' + lineStartPositions.length + ' lines!');
    line = lineStartPositions.length - 1;
  }

  if (line < lineStartPositions.length - 1) {
    var numColumnsOnThisLine = lineStartPositions[line+1] - lineStartPositions[line];
    if (col >= numColumnsOnThisLine) {
      console.error('Input file does not have a line:col ' + (line+1) + ':' + (col+1) + ', but given line only has ' + numColumnsOnThisLine + ' columns!');
      col = numColumnsOnThisLine - 1;
    }
  }
  return lineStartPositions[line] + col;
}

// Reads and decodes a source map file
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

  var sourceFileIndex = 0;
  var sourceLineNumber = 0;
  var sourceColumnNumber = 0;
  var sourceNameIndex = 0;
  var outputLine = 0;
  for(var l in lines) {
    var line = lines[l];
    var segments = line.split(',');

    var outputColumn = 0;
    for(var s in segments) {
      var segment = segments[s];
      var segmentArray = decodeVLQ(segment);

      if (segmentArray.length > 0) {
        outputColumn += segmentArray[0];
        if (segmentArray.length > 1) {
          sourceFileIndex += segmentArray[1];
          sourceLineNumber += segmentArray[2];
          sourceColumnNumber += segmentArray[3];
          if (segmentArray.length > 4) {
            sourceNameIndex += segmentArray[4];
            var sourceMapEntry = [
              mapLineColTo1DPos(outputLine, outputColumn, mangledFileLineStartPositions),
              sourceMapJson['sources'][sourceFileIndex],
              sourceLineNumber,
              sourceColumnNumber,
              sourceMapJson['names'][sourceNameIndex]
            ];
            sourceMap.push(sourceMapEntry);
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

function getSizeIncludingChildren(node, parentNode) {
  // Fix up acorn size computation to exactly take into account the size of variable initializers,
  // i.e. in "var a, b, c;", account size of "var " to "a", and size of ";" to "c".
  if (node.type == 'VariableDeclarator' && parentNode && parentNode.type == 'VariableDeclaration') {
    if (parentNode.declarations.length > 1) {
      var i = parentNode.declarations.indexOf(node);
      // Account "var " to "a".
      if (i == 0) {
        return node.end - parentNode.start;
      }
      // Account ";" to last declaration.
      if (i == parentNode.declarations.length-1) {
        return node.end + 1 - (node.start - 1);
      }
      // Account "," to In-between declarations.
      return node.end - (node.start - 1);
    }
  }
  // Other nodes use the size returned by acorn.
  return node.end - node.start;
}

function getSizeExcludingChildren(node) {
  var totalSize = getSizeIncludingChildren(node);
  var childNodes = getChildNodesWithCode(node);
  for(var i in childNodes) {
    totalSize -= getSizeIncludingChildren(childNodes[i]);
  }
  return totalSize;
}

// Gets the name of given node, taking into account a few different node types that
// uniquely identify a name for the node.
function getNodeNameExact(node) {
  assert(node);
  assert(!Array.isArray(node));

  if (node.type == 'ExpressionStatement' && node.expression.type == 'AssignmentExpression'
    && node.expression.left.type == 'Identifier') {
    return node.expression.left.name;
  }
  if (node.type == 'Property') {
    return node.key && node.key.name;
  }
  if (node.type == 'FunctionExpression' || node.type == 'FunctionDeclaration') {
    return node.id && node.id.name;
  }
  if (node.type == 'VariableDeclarator') {
    return node.id && node.id.name;
  }
}

// Look into child nodes for name of this node.
function peekAheadGetNodeName(node) {
  if (node.type == 'ObjectExpression') return;

  if (node.type == 'ExpressionStatement' && node.expression.type == 'AssignmentExpression'
    && node.expression.left.type == 'Identifier') {
    return node.expression.left;
  }
  if (node.type == 'Property') {
    return node.key.name ? node.key : node.value;
  }
  if (node.type == 'FunctionExpression' || node.type == 'FunctionDeclaration') {
    return node.id;
  }
  if (node.type == 'VariableDeclarator') {
    return node.id;
  }
  if (node.type == 'VariableDeclaration') {
    if (node.declarations.length == 1) {
      return node.declarations[0].id;
    }
  }
}

// Given an array of nodes, tally up their total contribution to size, including their children.
function countNodeSizes(nodeSizes) {
  var totalSize = 0;
  for(var i in nodeSizes) {
    totalSize += nodeSizes[i].size;
  }
  return totalSize;
}

// Copies all fields from src to dst.
function mergeKeyValues(dst, src) {
  for(var i in src) {
    dst[i] = src[i];
  }
}

// Marks all nodes in array 'nodeSizes' to have been sourced from file 'filename'.
function recordSourceFile(nodeSizes, filename) {
  filename = path.basename(filename);
  for(var n in nodeSizes) {
    nodeSizes[n].file = filename;
  }
}

// Apply Wasm symbol name demangling (essentially UTF-8 decoding)
function demangleWasmSymbol(symbol) {
  var out = '';
  for(var i = 0; i < symbol.length; ++i) {
    if (symbol[i] != '\\') {
      out += symbol[i];
    } else {
      if (symbol[i + 1] == '\\') {
        out += '\\';
        ++i;
      } else {
        out += String.fromCharCode(parseInt(symbol.substring(i+1, i+3), 16));
        i += 2;
      }
    }
  }
  return out;
}

// Binary search to unminify the name of given node using a source map.
function unminifyNameWithSourceMap(node, sourceMap) {
  var nodeStart = node.start;
  var s = 0;
  var e = sourceMap.length-1;

  while(s < e) {
    var mid = ((s+e+1)/2)|0;
    if (sourceMap[mid][0] < nodeStart) {
      s = mid;
    } else if (sourceMap[mid][0] > nodeStart) {
      e = mid-1;
    } else if (sourceMap[mid][0] == nodeStart) {
      return sourceMap[mid][4];
    } else {
      assert(false);
    }
  }
  // Source map locations from Closure do not precisely match up with node locations from acorn,
  // so allow a bit of slack.
  var slack = 1;
  if (node.type == 'FunctionDeclaration') {
    slack = 'function '.length;
  }
  if (sourceMap[s][0] <= nodeStart && sourceMap[s][0] >= nodeStart - slack) {
    return sourceMap[mid][4];
  }
}

// Unminifies the given name using a symbol map
function unminifyNameWithSymbolMap(name, symbolMap) {
  var unminifiedName = symbolMap[name];
  return unminifiedName || name;
}

// Heuristic choice to decide when a node should be expanded/split to its children components
// for size reporting.
function isLargeEnoughNodeToExpand(node) {
  var size = getSizeIncludingChildren(node) - getSizeExcludingChildren(node);
  return size > (expandSymbolsLargerThanPercents*currentInputJsFileSizeInBytes)|0 || size > expandSymbolsLargerThanBytes;
}

// Given a node, returns an array of its child nodes.
// Or if passed an array of nodes, returns that directly.
function findListOfChildNodes(nodeOrArrayOfNodes) {
  while (!Array.isArray(nodeOrArrayOfNodes)) {
    if (nodeOrArrayOfNodes.type == 'BlockStatement') nodeOrArrayOfNodes = nodeOrArrayOfNodes.body;
    if (nodeOrArrayOfNodes.type == 'FunctionExpression') {
      parentPrefix += nodeOrArrayOfNodes.id.name + '/';
      nodeOrArrayOfNodes = nodeOrArrayOfNodes.body;
    }
    else break;
  }
  if (!Array.isArray(nodeOrArrayOfNodes)) {
    assert(false);
  }
  return nodeOrArrayOfNodes;
}

// Given a node and a list of its childNodes, attempts to assign
// a unique name & type for that node by performing a lookahed into the descendants
// of the node. If there multiple ways to name the node, returns false.
// If the node cannot be named, returns null. Otherwise returns
// the name as a string.
function findUniqueNameAndTypeFromChildren(node, childNodes) {
  var oneChildName = null;
  var oneChildType = null;
  for(var i in childNodes) {
    var child = childNodes[i];
    var childNameNode = peekAheadGetNodeName(child);
    var childName = childNameNode && childNameNode.name;
    var childType = null;
    if (childName) {
      childType = getNodeType(child);
    } else if (child.type == 'FunctionExpression' || child.type == 'CallExpression'){
      return [null, null];
    } else {
      [childName, childType] = findUniqueNameAndTypeFromChildren(child, getChildNodesWithCode(child));
    }
    if (childName === false) {
      return [false, null];
    }
    if (childName) {
      if (oneChildName) {
        return [false, null];
      }
      oneChildName = childName;
      oneChildType = childType;
    }
  }
  return [oneChildName, oneChildType];
}

function hasAnyDescendantThatCanBeNamed(nodeArray, exceptNode) {
  for(var i in nodeArray) {
    var node = nodeArray[i];
    var nodeName = peekAheadGetNodeName(node);
    if (nodeName && nodeName != exceptNode) {
      return true;
    }
    var childNodes = getChildNodesWithCode(node);

    var [uniqueName, uniqueType] = findUniqueNameAndTypeFromChildren(node, childNodes);
    if (uniqueName || uniqueName === false) {
      return true;
    }

    if (hasAnyDescendantThatCanBeNamed(childNodes, exceptNode)) {
      return true;
    }
  }
}

function printNodeContents(node) {
  console.log(currentInputJsFile.substring(node.start, node.end));  
}

function collectNodeSizes2(nodeArray, parentNode, parentPrefix, symbolMap) {
  var nodeSizes = {};
  for(var i in nodeArray) {
    var node = nodeArray[i];
    var nodeType = getNodeType(node);
    var childNodes = getChildNodesWithCode(node);

    var nodeSize = getSizeIncludingChildren(node, parentNode);
    var childNameNode = peekAheadGetNodeName(node);
    var nodeName = childNameNode && childNameNode.name;
    var shouldBreakDown = isLargeEnoughNodeToExpand(node) && hasAnyDescendantThatCanBeNamed(childNodes, childNameNode);

    if (!nodeName) {
      var [uniqueName, uniqueType] = findUniqueNameAndTypeFromChildren(node, childNodes);
      if (uniqueName !== false) {
        shouldBreakDown = false;
      }
      if (uniqueName) {
        nodeName = uniqueName;
        nodeType = uniqueType;
      }
    }
    if (['ObjectExpression', 'FunctionExpression', 'CallExpression', 'BlockStatement', 'MemberExpression', 'ExpressionStatement'].indexOf(node.type) != -1) {
      shouldBreakDown = true;
      nodeName = null;
    }
    var minifiedName = nodeName;
    nodeName = unminifyNameWithSymbolMap(nodeName, symbolMap);

    var childSizes = null;

    if (shouldBreakDown) {
      var delimiter = (node.type == 'ObjectExpression' || (childNodes && childNodes.length == 1 && childNodes[0].type == 'ObjectExpression')) ? '.' : '/';
      var childPrefix = nodeName ? parentPrefix + nodeName + delimiter : parentPrefix;
      childSizes = collectNodeSizes2(childNodes, node, childPrefix, symbolMap);
      mergeKeyValues(nodeSizes, childSizes);
      nodeSize -= countNodeSizes(childSizes);
    }

    var nodeDesc = null;
    if (!nodeName && nodeType == 'code') {
      var code = currentInputJsFile.substring(node.start, node.end).trim();
      nodeName = 'code#' + runningCodeBlockCounter++;
      if (code.length > 32) {
        nodeDesc = '"' + code.substring(0, 29).replace(/\n/g, ' ') + '..."';
      } else {
        nodeDesc = '"' + code.replace(/\n/g, ' ') + '"';
      }      
    }

    if (node.type != 'ObjectExpression' && node.type != 'FunctionExpression') {
      if (nodeName && (nodeType != 'code' || !parentPrefix)) {
        var fullName = parentPrefix + nodeName;
        nodeSizes[fullName] = {
          'type': nodeType,
          'name': fullName,
          'desc': nodeDesc,
          'prefix': parentPrefix,
          'selfName': nodeName,
          'minifiedName': minifiedName,
          'size': nodeSize,
          'node': node
        };

        if (dumpJsTextContents.indexOf(fullName) != -1) {
          console.log('Contents of symbol ' + fullName + ':');
          printNodeContents(node);
          console.dir(node);
        }
      }
    }
  }
  return nodeSizes;
}

function getNodeType(node) {
  if (node.type == 'Property') node = node.value;

  if (['FunctionDeclaration', 'FunctionExpression'].indexOf(node.type) != -1) {
    return 'function';
  } else if (['VariableDeclaration', 'VariableDeclarator'].indexOf(node.type) != -1) {
    return 'var';
  } else {
    return 'code';
  }
}

// Finds the list of child AST nodes of the given node, that can contain other code (and are relevant for size computation) 
function getChildNodesWithCode(node) {
  assert(node);
  assert(!Array.isArray(node));

  var children = [];
  function addChild(child) {
    assert(child);
    assert(!Array.isArray(child));
    children.push(child);
  }
  function maybeChild(child) {
    if (child) {
      assert(!Array.isArray(child));
      children.push(child);
    }
  }
  function addChildArray(childArray) {
    assert(Array.isArray(childArray));
    children = children.concat(childArray);
  }

  if (['BlockStatement', 'Program'].indexOf(node.type) != -1) {
    addChildArray(node.body);
  }
  else if (['IfStatement'].indexOf(node.type) != -1) {
    addChild(node.test);
    addChild(node.consequent);
    maybeChild(node.alternate);
  }
  else if (['BinaryStatement', 'BinaryExpression', 'LogicalExpression', 'AssignmentExpression'].indexOf(node.type) != -1) {
    addChild(node.left);
    addChild(node.right);
  }
  else if (['MemberExpression'].indexOf(node.type) != -1) {
    addChild(node.object);
    addChild(node.property);
  }
  else if (['Property'].indexOf(node.type) != -1) {
    addChild(node.key);
    addChild(node.value);
  }
  else if (['TryStatement'].indexOf(node.type) != -1) {
    addChild(node.block);
    addChild(node.handler);
  }
  else if (['CatchClause'].indexOf(node.type) != -1) {
    // Ignored node: addChild(node.param);
    addChild(node.body);
  }
  else if (['FunctionDeclaration'].indexOf(node.type) != -1) {
    // Ignored node: addChild(node.id);
    addChild(node.body);
    // Ignored node: addChildArray(node.params);
  }
  else if (['FunctionExpression'].indexOf(node.type) != -1) {
    // Ignored node: maybeChild(node.id);
    addChild(node.body);
    // Ignored node: addChildArray(node.params);
  }
  else if (['ThrowStatement', 'ReturnStatement'].indexOf(node.type) != -1) {
    maybeChild(node.argument);
  }
  else if (['UnaryExpression', 'UpdateExpression'].indexOf(node.type) != -1) {
    addChild(node.argument);
  }
  else if (['CallExpression', 'NewExpression'].indexOf(node.type) != -1) {
    addChild(node.callee);
    addChildArray(node.arguments);
  }
  else if (['VariableDeclaration'].indexOf(node.type) != -1) {
    // When we are creating a symbol map from a source map, process the whole
    // tree pedantically. When actually parsing code size, shortcut the AST
    // to account size in a nicer way
    if (node.declarations.length == 1 && !generatingSymbolMapFromSourceMap) {
      maybeChild(node.declarations[0].init);
    } else {
      addChildArray(node.declarations);
    }
  }
  else if (['ArrayExpression'].indexOf(node.type) != -1) {
    addChildArray(node.elements);
  }
  else if (['VariableDeclarator'].indexOf(node.type) != -1) {
    // Ignored node: addChild(node.id);
    maybeChild(node.init);
  }
  else if (['ObjectExpression'].indexOf(node.type) != -1) {
    addChildArray(node.properties);
  }
  else if (['ExpressionStatement'].indexOf(node.type) != -1) {
    addChild(node.expression);
  }
  else if (['BreakStatement'].indexOf(node.type) != -1) {
    // Ignored node: maybeChild(node.label);
  }
  else if (['LabeledStatement'].indexOf(node.type) != -1) {
    addChild(node.body);
    // Ignored node: addChild(node.label);
  }
  else if (['SwitchStatement'].indexOf(node.type) != -1) {
    addChild(node.discriminant);
    addChildArray(node.cases);
  }
  else if (['SwitchCase'].indexOf(node.type) != -1) {
    addChildArray(node.consequent);
    maybeChild(node.test);
  }
  else if (['SequenceExpression'].indexOf(node.type) != -1) {
    addChildArray(node.expressions);
  }
  else if (['ConditionalExpression'].indexOf(node.type) != -1) {
    addChild(node.test);
    addChild(node.consequent);
    addChild(node.alternate);
  }
  else if (['ForStatement'].indexOf(node.type) != -1) {
    maybeChild(node.init);
    maybeChild(node.test);
    maybeChild(node.update);
    addChild(node.body);
  }
  else if (['WhileStatement', 'DoWhileStatement'].indexOf(node.type) != -1) {
    addChild(node.test);
    addChild(node.body);
  }
  else if (['ForInStatement'].indexOf(node.type) != -1) {
    addChild(node.left);
    addChild(node.right);
    addChild(node.body);
  }
  else if (['Identifier', 'Literal', 'ThisExpression', 'EmptyStatement', 'DebuggerStatement', 'ContinueStatement'].indexOf(node.type) != -1) {
    // no children
  } else {
    console.error('----UNKNOWN NODE TYPE!----');
    console.error(node);
    console.error('----UNKNOWN NODE TYPE!----');
    assert(false);
  }
  return children;
}

function walkNodesForSymbolMap(nodeArray, minifiedParentPrefix, unminifiedParentPrefix, sourceMap) {
  var symbolMap = {};
  assert(Array.isArray(nodeArray));
  for(var i in nodeArray) {
    var node = nodeArray[i];
    var childNodes = getChildNodesWithCode(node);

    var minifiedName = getNodeNameExact(node);

    // Try to demangle the name with source map.
    var unminifiedName = unminifyNameWithSourceMap(node, sourceMap);

    if (['FunctionDeclaration', 'VariableDeclaration', 'VariableDeclarator'].indexOf(node.type) != -1) {
      if (minifiedName && unminifiedName && unminifiedName != minifiedName) {
        symbolMap[minifiedParentPrefix + minifiedName] = unminifiedParentPrefix + unminifiedName;
      }
    }

    if (childNodes) {
      var delimiter = (node.type == 'ObjectExpression' || (childNodes && childNodes.length == 1 && childNodes[0].type == 'ObjectExpression')) ? '.' : '/';
      var minifiedChildPrefix = minifiedName ? minifiedParentPrefix + minifiedName + delimiter : minifiedParentPrefix;
      var unminifiedChildPrefix = minifiedName && unminifiedName ? unminifiedParentPrefix + unminifiedName + delimiter : unminifiedParentPrefix;
      mergeKeyValues(symbolMap, walkNodesForSymbolMap(childNodes, minifiedChildPrefix, unminifiedChildPrefix, sourceMap));
    }
  }
  return symbolMap;
}

function createSymbolMapFromSourceMap(sourceMap, sourceFile) {
  generatingSymbolMapFromSourceMap = true;
  var lineStartPositions = calculateLineStartPositions(sourceFile);
  sourceFile = fs.readFileSync(sourceFile).toString();
  currentInputJsFile = sourceFile;
  var sourceMap = readSourceMap(sourceMap, lineStartPositions);
  var ast = acorn.parse(sourceFile, { ecmaVersion: 6 });
  var symbolMap = walkNodesForSymbolMap(ast.body, '', '', sourceMap);

  return symbolMap;
}

function extractJavaScriptCodeSize(sourceFile, symbolMap) {
  currentInputJsFileSizeInBytes = fs.statSync(sourceFile)['size'];
  currentInputJsFile = fs.readFileSync(sourceFile).toString();
  var ast = acorn.parse(currentInputJsFile, { ecmaVersion: 6 });

  var nodeSizes = collectNodeSizes2(ast.body, null, '', symbolMap);
  var totalAccountedFor = 0;
  for(var i in nodeSizes) {
    var node = nodeSizes[i];
    if (node.type) {
      totalAccountedFor += node.size;
    }
  }

  var whitespaceBytes = currentInputJsFileSizeInBytes - countNodeSizes(nodeSizes);

  if (whitespaceBytes > 0) {
    var name = 'unclassified';
    nodeSizes[name] = {
      'type': 'other',
      'prefix': '',
      'name': name,
      'size': whitespaceBytes
    };
  }

  recordSourceFile(nodeSizes, sourceFile);
  return nodeSizes;
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
      functionNames[index] = name;
    }
  }
  assert(cursor == sectionEndCursor);
  return Object.keys(functionNames).length > 0 ? functionNames : null;
}

function renumberFunctionNames(sizes, numImports) {
  var renumberedSymbols = {};
  for(var name in sizes) {
    var symbol = sizes[name];
    if (symbol.type == 'function') {
      var renumberedName = (name|0) + numImports;
      renumberedSymbols[renumberedName] = sizes[name];
      renumberedSymbols[renumberedName].name = renumberedName;
    } else {
      renumberedSymbols[name] = symbol;
    }
  }
  return renumberedSymbols;
}

function labelWasmFunctionsWithNames(sizes, wasmFunctionNames) {
  var labeledSymbols = {};
  for(var name in sizes) {
    var symbol = sizes[name];
    if (symbol.type == 'function') {
      var functionNameIndex = (name|0);
      var wasmName = wasmFunctionNames[functionNameIndex] || name;
      labeledSymbols[wasmName] = sizes[name];
      labeledSymbols[wasmName].name = wasmName;
    } else {
      labeledSymbols[name] = symbol;
    }
  }
  return labeledSymbols;
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

  if (numImports > 0) {
    // Reorder wasm function names list to account for imports
    sizes = renumberFunctionNames(sizes, numImports);
  }
  if (wasmFunctionNames) {
    sizes = labelWasmFunctionsWithNames(sizes, wasmFunctionNames);
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

  function splitInTwo(s, delim) {
    var idx = s.indexOf(delim);
    return [s.substr(0, idx), s.substr(idx+1)];
  }
  var symbolFile = fs.readFileSync(filename).toString();
  var symbols = symbolFile.split('\n');
  for(var i in symbols) {
    var [minified, unminified] = splitInTwo(symbols[i], ':');
    symbolMap[minified] = demangleWasmSymbol(unminified);
  }
  return symbolMap;
}

function extractBoolCmdLineInput(args, param, defaultValue) {
  var idx = args.indexOf(param);
  if (idx != -1) {
    args.splice(idx, 1);
    return true;
  }
  return defaultValue;
}

function extractStringCmdLineInput(args, param, defaultValue) {
  var idx = args.indexOf(param);
  if (idx != -1) {
    var value = args[idx+1];
    args.splice(idx, 2);
    return value;
  }
  return defaultValue;
}

function extractNumberCmdLineInput(args, param, defaultValue) {
  var idx = args.indexOf(param);
  if (idx != -1) {
    var value = args[idx+1];
    args.splice(idx, 2);
    return parseFloat(value);
  }
  return defaultValue;
}

function run(args, printOutput) {
  var outputJson = extractBoolCmdLineInput(args, '--json');
  var symbolMap = extractStringCmdLineInput(args, '--symbols');
  var sourceMap = extractStringCmdLineInput(args, '--createSymbolMapFromSourceMap');
  var dumpSymbol = extractStringCmdLineInput(args, '--dump');
  expandSymbolsLargerThanPercents = extractNumberCmdLineInput(args, '--expandLargerThanPercents', expandSymbolsLargerThanPercents);
  if (expandSymbolsLargerThanPercents > 1) expandSymbolsLargerThanPercents /= 100.0;
  expandSymbolsLargerThanBytes = extractNumberCmdLineInput(args, '--expandLargerThanBytes', expandSymbolsLargerThanBytes);
  while (dumpSymbol) {
    dumpJsTextContents.push(dumpSymbol);
    dumpSymbol = extractStringCmdLineInput(args, '--dump');
  }
  var sources = args;

  symbolMap = symbolMap ? readSymbolMap(symbolMap) : {};

  // Doing Source Map -> Symbol Map translation?
  if (sourceMap) {
    assert(sources.length == 1); // Must present exactly one input JS file when doing source map -> symbol map translation
    var symbolMap = createSymbolMapFromSourceMap(sourceMap, sources[0]);

    if (printOutput) {
      if (outputJson) {
        console.log(JSON.stringify(symbolMap));
      } else {
        for(var i in symbolMap) {
          console.log(i + ':' + symbolMap[i]);
        }
      }
    }
    return symbolMap;
  }
  // ... if not, then proceed with a regular JS size analysis run:

  var codeSizes = {};
  for(var s in sources) {
    var src = sources[s];
    if (src.toLowerCase().endsWith('.js')) {
      mergeKeyValues(codeSizes, extractJavaScriptCodeSize(src, symbolMap));
    } else if (src.endsWith('.wasm')) {
      mergeKeyValues(codeSizes, extractWasmCodeSize(src));
    }
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

  if (printOutput) {
    if (outputJson) {
      console.log(JSON.stringify(codeSizes));
    } else {
      console.log('--- Code sizes:');
      for(var i in codeSizes) {
        var node = codeSizes[i];
        console.log(node.file + '/' + node.type + ' ' + demangleSymbol(node, symbolMap) + (node.desc ? ('=' + node.desc) : '') + ': ' + node.size);
      }
    }
  }
  return codeSizes;
}

function jsonContains(text, actual, expected) {
  console.log(text);
  console.log('---- actual result: ----');
  console.dir(actual);

  function findObjectByName(arr, obj) {
    for(var i in arr) {
      if (arr[i].name == obj.name) {
        return arr[i];
      }
    }
  }

  for(var i in expected) {
    var e = expected[i];
    var a = findObjectByName(actual, e);
    if (!a) {
      throw new Error('Could not find a key with name ' + e.name + '!');
    }
    for(var j in e) {
      if (!a[j]) {
        throw new Error('Actual result did not have a key by name ' + j + '! (searching for ' + e + ')');
      }
      if (a[j] != e[j]) {
        console.log('ACTUAL: ');
        console.dir(a);
        if (a.node) {
          console.dir(a.node);
        }
        console.log('EXPECTED: ');
        console.dir(e);
        throw new Error('Actual result did not agree on value of key "' + j + '"! (actual: ' + a[j] + ', expected: ' + e[j] + ')');
      }
    }
  }
  return true;
}

function runTests() {
  console.log('Running tests:');
  var testCases = [
    ['var foo;', [{ name: 'foo', type: 'var', size: 8}]],
    ['var foo=3;', [{ name: 'foo', type: 'var', size: 10}]],
    ['var foo=4,bar=2,baz=3;', [{ name: 'foo', type: 'var', size: 9}, { name: 'bar', type: 'var', size: 6}, { name: 'baz', type: 'var', size: 7}]],
    ['var foo = "var foo";', [{ name: 'foo', type: 'var', size: 20}]],
    ['function foo(){}', [{ name: 'foo', type: 'function', size: 16}]],
    ['function longFunctionName(){var x=4;}', [{ name: 'longFunctionName', type: 'function', size: 37}]],
    ['function foo(){var longVariableName=4;}', [{ name: 'foo', type: 'function', size: 16}, {name: 'foo/longVariableName', type: 'var', size: 23}]],
    ['var foo = {};', [{ name: 'foo', type: 'var', size: 13}]],
    ['var foo = {a:1};', [{ name: 'foo', type: 'var', size: 16}]],
    ['var foo = { bar: function(){"thisIsAFunctionAssignedToAMember";} };', [{ name: 'foo.bar', type: 'function', size: 52}]],
    ['var WebAssembly = { Instance: function(module, info) { var exports = (function() { function asmFunc() { function memcpy() {}} return asmFunc() })(asmLibraryArg, wasmMemory, wasmTable); } }',
      [{ name: 'WebAssembly.Instance/exports/asmFunc', type: 'function', size: 42}],
      [{ name: 'WebAssembly.Instance/exports', type: 'var', size: 87}],
      [{ name: 'WebAssembly.Instance', type: 'function', size: 37}],
      [{ name: 'WebAssembly', type: 'var', size: 22}]],
  ];
  for(var i in testCases) {
    [test, expected] = testCases[i];
    var testFile = 'generatedTestCase.js';
    fs.writeFileSync(testFile, test);
    assert(jsonContains(test, run([testFile]), expected));
  }
}

var args = process['argv'].slice(2);

if (extractBoolCmdLineInput(args, '--runTests')) {
  runTests();
} else if (extractBoolCmdLineInput(args, '--help')) {
  console.log(`${process['argv'][1]}: Break down size report of used code. Usage:`);
  console.log(`\n  node ${process['argv'][1]} [--options] file1.[js|wasm] [file2.[js|wasm]] ... [fileN.[js|wasm]]`);
  console.log(`\nwhere supported --options:\n`);
  console.log(`  --json: Print JSON instead of human-readable output`);
  console.log(`  --symbols <a.symbols>: Use the symbol map file a.symbols to unminify the symbol names`);
  console.log(`  --dump <symbolName>: Print the contents of the given symbol`);
  console.log(`  --expandLargerThanPercents <0.35>: Expand nested JavaScript blocks larger than given percentage of whole file size`);
  console.log(`  --expandLargerThanBytes <1000>: Expand nested JavaScript blocks larger than given number of bytes`);
  console.log('');
} else {
  run(args, /*printOutput=*/true);
}
