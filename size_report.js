var acorn = require('../third_party/acorn');
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

function calculateLineStartPositions(filename) {
  var fileSizeInBytes = fs.statSync(filename)['size'];
  var f = fs.readFileSync(filename);
  //console.log('File ' + filename + ', size ' + fileSizeInBytes);
  var lineStartPositions = [0];
  for(var i = 0; i < fileSizeInBytes; ++i) {
    var byte = f.readUInt8(i);
    if (byte == 0x0A) {
      lineStartPositions.push(i+1);
    }
  }
//  console.dir(lineStartPositions);
  return lineStartPositions;
}

function mapLineColTo1DPos(line, col, lineStartPositions) {
//  console.log(line);
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

            var sourceMapEntry = [mapLineColTo1DPos(outputLine, smOutputColumn, mangledFileLineStartPositions), sourceMapJson['sources'][smSourceFile], smSourceLine, smSourceColumn, sourceMapJson['names'][smNameIndex]];
//            console.dir(sourceMapEntry);
//            console.log(currentInputJsFile.substring(sourceMapEntry[0]-30, sourceMapEntry[0]) + '     |     ' + currentInputJsFile.substring(sourceMapEntry[0], sourceMapEntry[0] + 75) + '\n');
            sourceMap.push(sourceMapEntry);
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

function getSizeIncludingChildren(node, parentNode) {
  // Fix up acorn size computation to exactly take into account the size of variable initializers.
  if (node.type == 'VariableDeclarator' && parentNode && parentNode.type == 'VariableDeclaration') {
    if (parentNode.declarations.length > 1) {
      var i = parentNode.declarations.indexOf(node);
      if (i == 0) {
        return node.end - parentNode.start;
      }
      if (i == parentNode.declarations.length-1) {
        return node.end + 1 - (node.start - 1);
      }
      return node.end - (node.start - 1);
    }
  }
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

function getNodeNameExact(node) {
  assert(node);
  assert(!Array.isArray(node));

/*
  if (['Identifier'].indexOf(node.type) != -1) {
    assert(node.name && typeof node.name === 'string');
    return node.name;
  }
*/
  if (node.type == 'ExpressionStatement' && node.expression.type == 'AssignmentExpression'
    && node.expression.left.type == 'Identifier') {
    return node.expression.left.name;
  }
  if (node.type == 'Property') {
    return node.key && node.key.name;// || (node.value ) ? node.key : node.value;
  }
  if (node.type == 'FunctionExpression' || node.type == 'FunctionDeclaration') {
    return node.id && node.id.name;
  }
  if (node.type == 'VariableDeclarator') {
    return node.id && node.id.name;
  }
}

function peekAheadGetNodeName(node) {
  if (node.type == 'ObjectExpression') return;

  if (node.type == 'ExpressionStatement' && node.expression.type == 'AssignmentExpression'
    && node.expression.left.type == 'Identifier') {
    return node.expression.left;//.name;
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

function unminifyNameWithSourceMap(node, functionName, sourceMap) {
  var nodeStart = node.start;
//  if (node.type == 'VariableDeclaration') {
//    nodeStart += 'var '.length;
//  }
  var s = 0;
  var e = sourceMap.length-1;
  /*
  console.log('Unminifying ');
  console.dir(node);
  console.log('At ' + currentInputJsFile.substring(node.start, node.end));
  console.log('Context ' + currentInputJsFile.substring(node.start-50, node.end+50));
  */
  while(s < e) {
    var mid = ((s+e+1)/2)|0;
//    console.log('s: ' + s + ', e: ' + e + ', mid: ' + mid + ', nodeStart: ' + nodeStart + ', midpos: ' + sourceMap[mid][0]);
    if (sourceMap[mid][0] < nodeStart) {
      s = mid;
    } else if (sourceMap[mid][0] > nodeStart) {
      e = mid-1;
    } else if (sourceMap[mid][0] == nodeStart) {
 //     console.log('Found ');
  //    console.dir(sourceMap[mid]);
      return sourceMap[mid][4];
    } else {
 //     console.dir(sourceMap[mid]);
 //     console.dir(node);
  //    console.dir(typeof node);
      assert(false);
    }
  }
//  if (sourceMap[mid][0] < node.start
//  console.log('b');
//  console.dir(sourceMap);
//  console.dir(sourceMap[s]);
//  return null;
  // Source maps generated by Closure do not match up properly on all symbols, but sometimes they are off by one at least,
  // pointing to a preceding semicolon, colon or comma. Allow some slack.
  var slack = 1;
  if (node.type == 'FunctionDeclaration') {
    slack = 'function '.length;
  }
  if (sourceMap[s][0] <= nodeStart && sourceMap[s][0] >= nodeStart - slack) {
  //  console.log('2Found ');
  //  console.dir(sourceMap[mid]);
    return sourceMap[mid][4];
  }
  /*
  console.log('Found nothing');
  console.dir(sourceMap[s-4]);
  console.dir(sourceMap[s-3]);
  console.dir(sourceMap[s-2]);
  console.dir(sourceMap[s-1]);
  console.dir(sourceMap[s]);
  console.dir(sourceMap[mid]);
  console.dir(sourceMap[e]);
  console.dir(sourceMap[e+1]);
  console.dir(sourceMap[e+2]);
  console.dir(sourceMap[e+3]);
  */
}

function unminifyNameWithSymbolMap(name, symbolMap) {
  var unminifiedName = symbolMap[name];
  return unminifiedName || name;
}

function isLargeEnoughNodeToExpand(node) {
  var size = getSizeIncludingChildren(node) - getSizeExcludingChildren(node);
  return size > (expandSymbolsLargerThanPercents*currentInputJsFileSizeInBytes)|0 || size > expandSymbolsLargerThanBytes;
}

function findListOfChildNodes(nodeArray) {
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
    console.log(currentInputJsFile.substring(nodeArray.start, nodeArray.end));
    console.log(currentInputJsFile.substring(nodeArray.start - 50, nodeArray.end + 50));
    assert(false);
  }
  return nodeArray;
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
    //var childName = getNodeNameExact(child);
    var childNameNode = peekAheadGetNodeName(child);
    var childName = childNameNode && childNameNode.name;
    var childType = null;
//    console.log('childName: ' + childName);
    if (childName) {
      childType = getNodeType(child);
    } else if (child.type == 'FunctionExpression' || child.type == 'CallExpression'){
      return [null, null];
    } else {
      [childName, childType] = findUniqueNameAndTypeFromChildren(child, getChildNodesWithCode(child));
//      console.dir(node);
//      console.log('deep childName of node: ' + childName);
    }
    if (childName === false) {
  //    console.log('Child has multiple names - return false!');
      return [false, null];
    }
    if (childName) {
      if (oneChildName) {
  //    console.log('Found second child name ' + childName + ' (first was ' + oneChildName + ') - return false');
        return [false, null];
      }
      oneChildName = childName;
      oneChildType = childType;
  //    console.log('Found first child name ' + oneChildName);
    }
  }
  return [oneChildName, oneChildType];
}

function hasAnyDescendantThatCanBeNamed(nodeArray, exceptNode) {
//  console.log('hasAnyDescendantThatCanBeNamed: ');
//  console.dir(nodeArray);
  for(var i in nodeArray) {
    var node = nodeArray[i];
    var nodeName = peekAheadGetNodeName(node);
//    console.dir(nodeName);
    if (nodeName && nodeName != exceptNode) {
      return true;
    }
    var childNodes = getChildNodesWithCode(node);
//    console.dir(node);
//    console.dir(childNodes);

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
  //  console.log('PROCESSING:');
 //   console.dir(node);
    var childNodes = getChildNodesWithCode(node);

    var nodeSize = getSizeIncludingChildren(node, parentNode);
    var childNameNode = peekAheadGetNodeName(node);
    var nodeName = childNameNode && childNameNode.name;
//    console.log(nodeName);
    var shouldBreakDown = isLargeEnoughNodeToExpand(node) && hasAnyDescendantThatCanBeNamed(childNodes, childNameNode);
    /*
    console.log('nodeName: ' + nodeName + ', node.type: ' + node.type + ', shouldBreakDown: ' + shouldBreakDown + ', isLargeEnoughNodeToExpand(node):' + isLargeEnoughNodeToExpand(node) + ', hasAnyDescendantThatCanBeNamed(childNodes):' + hasAnyDescendantThatCanBeNamed(childNodes));
    console.log('Code: ' + currentInputJsFile.substring(node.start, Math.min(node.end, node.start + 100)).trim());
    console.log('Children:');
    console.dir(childNodes);
    */

    if (!nodeName) {
      var [uniqueName, uniqueType] = findUniqueNameAndTypeFromChildren(node, childNodes);
//      console.log('uniqueName: ' + uniqueName + ', shouldBreakDown: ' + shouldBreakDown);
      if (uniqueName !== false) {
  //      console.log('Node ' + nodeName + ', type: ' + node.type + ' does not have a unique nameable child: shouldBreakDown <- false');
        shouldBreakDown = false;
      }
      if (uniqueName) {
        nodeName = uniqueName;
  //      console.log('nodeName <- ' + uniqueName);
        nodeType = uniqueType;
      }
    }
    if (['ObjectExpression', 'FunctionExpression', 'CallExpression', 'BlockStatement', 'MemberExpression', 'ExpressionStatement'].indexOf(node.type) != -1) {
 //     console.log('Node ' + nodeName + ' is of special unnamed type: shouldBreakDown <- true');
      shouldBreakDown = true;
      nodeName = null;
    }
    var minifiedName = nodeName;
    nodeName = unminifyNameWithSymbolMap(nodeName, symbolMap);
//    console.log('Unminified name of ' + minifiedName + ' is ' + nodeName);
//    console.log('nodeName is ' + nodeName + ', shouldBreakDown=' + shouldBreakDown);

    var childSizes = null;

    if (shouldBreakDown) {
 //     console.log('breaking down node name ' + nodeName);
 //     console.dir(node);
      var delimiter = (node.type == 'ObjectExpression' || (childNodes && childNodes.length == 1 && childNodes[0].type == 'ObjectExpression')) ? '.' : '/';
      var childPrefix = nodeName ? parentPrefix + nodeName + delimiter : parentPrefix;
      childSizes = collectNodeSizes2(childNodes, node, childPrefix, symbolMap);
      mergeKeyValues(nodeSizes, childSizes);
      nodeSize -= countNodeSizes(childSizes);
    } else {
//      console.log('not breaking down node name ' + nodeName);
//      console.dir(node);      
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
//    console.log('asdf ' + nodeName + ' type ' + nodeType + ' parentPrefix ' + parentPrefix);
    if (node.type != 'ObjectExpression' && node.type != 'FunctionExpression') {
      if (nodeName && (nodeType != 'code' || !parentPrefix)) {
//        console.log('nodeName: ' + nodeName);
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
 //       console.log(fullName + ' = ');
 //       console.dir(nodeSizes[fullName]);

        if (dumpJsTextContents.indexOf(fullName) != -1) {
          console.log('Contents of symbol ' + fullName + ':');
          printNodeContents(node);
          console.dir(node);
        }
      }
    }
//    mergeKeyValues(sizes, childSizes);
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
//    console.log('NODE TYPE:');
//    console.dir(node);
    return 'code';
  }
}

// Finds the list of child AST nodes of the given node, that can contain other code (and are relevant for size computation) 
function getChildNodesWithCode(node) {
  assert(node);
  assert(!Array.isArray(node));
  //if (Array.isArray(node)) return node;
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
//    addChild(node.param);
    addChild(node.body);
  }
  else if (['FunctionDeclaration'].indexOf(node.type) != -1) {
//    addChild(node.id);
    // Skip directly into processing the statements in a function body
    // so that the curly braces in function(){var foo;} are counted
    // towards function() rather than var foo;
//    if (node.body.type == 'BlockStatement') {
  //    addChildArray(node.body.body);
   //} else {
      addChild(node.body);
  //  }
//    addChildArray(node.params);
  }
  else if (['FunctionExpression'].indexOf(node.type) != -1) {
//    maybeChild(node.id);
    addChild(node.body);
//    addChildArray(node.params);
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
//    addChild(node.id);
    maybeChild(node.init);
  }
  else if (['ObjectExpression'].indexOf(node.type) != -1) {
    addChildArray(node.properties);
  }
  else if (['ExpressionStatement'].indexOf(node.type) != -1) {
    addChild(node.expression);
  }
  else if (['BreakStatement'].indexOf(node.type) != -1) {
//    maybeChild(node.label);
  }
  else if (['LabeledStatement'].indexOf(node.type) != -1) {
    addChild(node.body);
//    addChild(node.label);
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
    console.error('----NODE----');
    console.error(node);
    console.error('----ENDNODE----');
    assert(false);
  }
  return children;
}

function walkNodesForSymbolMap(nodeArray, minifiedParentPrefix, unminifiedParentPrefix, sourceMap) {
  var symbolMap = {};
  //var childNodes = findListOfChildNodes(nodeOrNodeArray);
  //var childNodes = getChildNodes(nodeOrNodeArray);
  assert(Array.isArray(nodeArray));
  for(var i in nodeArray) {
    var node = nodeArray[i];
    var childNodes = getChildNodesWithCode(node);


    /*
    var childNameNode = peekAheadGetNodeName(node);
    var nodeName = childNameNode && childNameNode.name;
    if (!nodeName) {
      var [uniqueName, uniqueType] = findUniqueNameAndTypeFromChildren(node, childNodes);
      if (uniqueName) {
        nodeName = uniqueName;
      }
    }
    if (['ObjectExpression', 'FunctionExpression', 'CallExpression', 'BlockStatement'].indexOf(node.type) != -1) {
      nodeName = null;
    }
    var minifiedName = nodeName;

*/








//////    console.log('\n');
    var minifiedName = getNodeNameExact(node);
//    var minifiedNameNode = peekAheadGetNodeName(node); // TODO: something like this
//    var minifiedName = minifiedNameNode && minifiedNameNode.name;

    // Try to demangle the name with source map.
    var unminifiedName = unminifyNameWithSourceMap(node, minifiedName, sourceMap);
//    console.log(minifiedParentPrefix + minifiedName + ':' + unminifiedParentPrefix + unminifiedName);

    if (['FunctionDeclaration', 'VariableDeclaration', 'VariableDeclarator'].indexOf(node.type) != -1) {
      if (minifiedName && unminifiedName && unminifiedName != minifiedName) {
  //      console.log(minifiedParentPrefix + minifiedName + ':' + unminifiedParentPrefix + unminifiedName);
        symbolMap[minifiedParentPrefix + minifiedName] = unminifiedParentPrefix + unminifiedName;
      }
    }

//    var childNodeArray = getChildNodesWithCode(node);
    if (childNodes) {
/////      console.log('minifiedName: ' + minifiedName);
/////      console.log('unminifiedName: ' + unminifiedName);
      var delimiter = (node.type == 'ObjectExpression' || (childNodes && childNodes.length == 1 && childNodes[0].type == 'ObjectExpression')) ? '.' : '/';
      var minifiedChildPrefix = minifiedName ? minifiedParentPrefix + minifiedName + delimiter : minifiedParentPrefix;
      var unminifiedChildPrefix = minifiedName && unminifiedName ? unminifiedParentPrefix + unminifiedName + delimiter : unminifiedParentPrefix;
/////      console.log('minifiedChildPrefix: ' + minifiedChildPrefix);
/////      console.log('unminifiedChildPrefix: ' + unminifiedChildPrefix);
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
//  console.dir(lineStartPositions);
  var sourceMap = readSourceMap(sourceMap, lineStartPositions);
//  console.dir(sourceMap);
  var ast = acorn.parse(sourceFile, { ecmaVersion: 6 });
  var symbolMap = walkNodesForSymbolMap(ast.body, '', '', sourceMap);
  //console.dir(symbolMap);

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
    var name = 'unclassified';
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
//  console.log('File ' + sourceFile + ': total size ' + currentInputJsFileSizeInBytes + ' bytes.');
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
      //  if (node.type) {
          console.log(node.file + '/' + node.type + ' ' + demangleSymbol(node, symbolMap) + (node.desc ? ('=' + node.desc) : '') + ': ' + node.size);
      //  }
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