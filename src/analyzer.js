// Analyze intertype data

VAR_NATIVE = 'native';
VAR_NATIVIZED = 'nativized';
VAR_EMULATED = 'emulated';

function cleanFunc(func) {
  func.lines = func.lines.filter(function(line) { return line.intertype !== null });
  func.labels.forEach(function(label) {
    label.lines = label.lines.filter(function(line) { return line.intertype !== null });
  });
}

function analyzer(data) {
  substrate = new Substrate('Analyzer');

  // Sorter
  substrate.addZyme('Sorter', {
    processItem: function(item) {
      item.items.sort(function (a, b) { return a.lineNum - b.lineNum });
      this.forwardItem(item, 'Gatherer');
    },
  });

  // Gatherer
  substrate.addZyme('Gatherer', {
    processItem: function(item) {
      // Single-liners
      ['globalVariable', 'functionStub', 'type'].forEach(function(intertype) {
        var temp = splitter(item.items, function(item) { return item.intertype == intertype });
        item[intertype + 's'] = temp.splitOut;
        item.items = temp.leftIn;
      });
      // Functions & labels
      item.functions = []
      for (var i = 0; i < item.items.length; i++) {
        var subItem = item.items[i];
        if (subItem.intertype == 'function') {
          item.functions.push(subItem);
          subItem.endLineNum = null;
          subItem.lines = [];
          subItem.labels = [];
        } else if (subItem.intertype == 'functionEnd') {
          item.functions.slice(-1)[0].endLineNum = subItem.lineNum;
        } else if (subItem.intertype == 'label') {
          item.functions.slice(-1)[0].labels.push(subItem);
          subItem.lines = [];
        } else if (item.functions.slice(-1)[0].endLineNum === null) {
          // Internal line
          item.functions.slice(-1)[0].lines.push(subItem);
          item.functions.slice(-1)[0].labels.slice(-1)[0].lines.push(subItem);
        } else {
          print("ERROR: what is this? " + JSON.stringify(subItem));
        }
      }
      delete item.items;
      this.forwardItem(item, 'Identinicer');
    },
  });

  // IdentiNicer
  substrate.addZyme('Identinicer', {
    processItem: function(output) {
      walkJSON(output, function(item) {
        ['', '2', '3', '4', '5'].forEach(function(ext) {
          if (item && item['ident' + ext])
          item['ident' + ext] = toNiceIdent(item['ident' + ext]);
        });
      });
      this.forwardItem(output, 'Typevestigator');
    }
  });

  function addType(type, data) {
    if (type.length == 1) return;
    if (data.types[type]) return;
    if (['internal', 'inbounds', 'void'].indexOf(type) != -1) return;
    dprint('types', '// addType: ' + type);
    var check = new RegExp(/^\[(\d+)\ x\ (.*)\]$/g).exec(type);
    // 'blocks': [14 x %struct.X] etc.
    if (check) {
      var num = parseInt(check[1]);
      var subType = check[2];
      data.types[type] = {
        name_: type,
        fields: range(num).map(function() { return subType }),
        lineNum: '?',
      };
      return;
    }
    if (['['].indexOf(type) != -1) return;
    if (isNumberType(type) || isPointerType(type)) return;
    data.types[type] = {
      name_: type,
      fields: [ 'i32' ], // XXX
      flatSize: 1,
      lineNum: '?',
    };
  }

  // Typevestigator
  substrate.addZyme('Typevestigator', {
    processItem: function(data) {
      // Convert types list to dict
      var old = data.types;
      data.types = {};
      old.forEach(function(type) { data.types[type.name_] = type });

      // Find additional types
      walkJSON(data, function(item) {
        if (!item) return;
        if (item.type) {
          addType(!item.type.text ? item.type : item.type.text, data);
        }
        if (item.type2) {
          addType(!item.type2.text ? item.type2 : item.type2.text, data);
        }
      });
      this.forwardItem(data, 'Typeanalyzer');
    }
  });

  // Type analyzer
  substrate.addZyme('Typeanalyzer', {
    processItem: function(item) {
      // 'fields' is the raw list of LLVM fields. However, we embed
      // child structures into parent structures, basically like C.
      // So { int, { int, int }, int } would be represented as
      // an Array of 4 ints. getelementptr on the parent would take
      // values 0, 1, 2, where 2 is the entire middle structure.
      // We also need to be careful with getelementptr to child
      // structures - we return a pointer to the same slab, just
      // a different offset. Likewise, need to be careful for
      // getelementptr of 2 (the last int) - it's real index is 4.
      // The benefit of this approach is inheritance -
      //    { { ancestor } , etc. } = descendant
      // In this case it is easy to bitcast ancestor to descendant
      // pointers - nothing needs to be done. If the ancestor were
      // a new slab, it would need some pointer to the outer one
      // for casting in that direction.
      // TODO: bitcasts of non-inheritance cases of embedding (not at start)
      var more = true;
      while (more) {
        more = false;
        values(item.types).forEach(function(type) {
          if (type.flatIndexes) return;
          var ready = true;
          type.fields.forEach(function(field) {
            //print('// zz getT: ' + type.name_ + ' : ' + field);
            if (isStructType(field)) {
              if (!item.types[field]) {
                addType(field, item);
                ready = false;
              } else {
                if (!item.types[field].flatIndexes) {
                  ready = false;
                }
              }
            }
          });
          if (!ready) {
            more = true;
            return;
          }
          type.flatSize = 0;
          var sizes = [];
          type.flatIndexes = type.fields.map(function(field) {
            var soFar = type.flatSize;
            var size;
            if (isNumberType(field) || isPointerType(field)) {
              size = getNativeFieldSize(field);
            } else if (isStructType(field)) {
              size = item.types[field].flatSize;
            } else {
              assert(0);
            }
            type.flatSize += size;
            sizes.push(size);
            return soFar;
          });
          if (dedup(sizes).length == 1) {
            type.flatFactor = sizes[0];
          }
          type.needsFlattening = (this.flatFactor != 1);
          dprint('types', 'type: ' + type.name_ + ' : ' + JSON.stringify(type.fields));
          dprint('types', '                        has final size of ' + type.flatSize + ', flatting: ' + type.needsFlattening + ' ? ' + (type.flatFactor ? type.flatFactor : JSON.stringify(type.flatIndexes)));
        });
      }

      this.forwardItem(item, 'VariableAnalyzer');
    },
  });
  
  // Variable analyzer
  substrate.addZyme('VariableAnalyzer', {
    processItem: function(item) {
      item.functions.forEach(function(func) {
        func.variables = {};

        // LLVM is SSA, so we always have a single assignment/write. We care about
        // the reads/other uses.
        walkJSON(func.lines, function(item) {
          if (item && item.intertype == 'assign' && ['alloca', 'load', 'call', 'bitcast', 'mathop', 'getelementptr', 'fastgetelementptrload'].indexOf(item.value.intertype) != -1) {
            func.variables[item.ident] = {
              ident: item.ident,
              type: item.value.type.text,
              origin: item.value.intertype,
              uses: parseInt(item.value.tokens.slice(-1)[0].item[0].tokens[0].text.split('=')[1]),
            };
          }
        });

        for (vname in func.variables) {
          var variable = func.variables[vname];

          // Whether the value itself is used. For an int, always yes. For a pointer,
          // we might never use the pointer's value - we might always just store to it /
          // read from it. If so, then we can optimize away the pointer.
          variable.hasValueTaken = false;
          // Whether our address was used. If not, then we do not need to bother with
          // implementing this variable in a way that other functions can access it.
          variable.hasAddrTaken = false;

          variable.pointingLevels = pointingLevels(variable.type);

          // Analysis!

          if (variable.pointingLevels > 0) {
            // Pointers
            variable.loads = 0;
            variable.stores = 0;

            func.lines.forEach(function(line) {
              //print(dump(line))
              if (line.intertype == 'store' && line.ident == vname) {
                variable.stores ++;
              } else if ((line.intertype == 'assign' && line.value.intertype == 'load' && line.value.ident == vname) ||
                         (line.intertype == 'fastgetelementptrload' && line.ident == vname)) {
                variable.loads ++;
              }
            });

            variable.otherUses = variable.uses - variable.loads - variable.stores;
            if (variable.otherUses > 0)
              variable.hasValueTaken = true;
          }
 
          // Decision time

          if (variable.origin == 'getelementptr') {
            // Use our implementation that emulates pointers etc.
            variable.impl = VAR_EMULATED;
          } else if ( variable.pointingLevels === 0 && !variable.hasAddrTaken ) {
            // A simple int value, can be implemented as a native variable
            variable.impl = VAR_NATIVE;
          } else if ( variable.pointingLevels === 1 && variable.origin === 'alloca' && !isStructPointerType(variable.type) && !variable.hasAddrTaken && !variable.hasValueTaken ) {
            // A pointer to a value which is only accessible through this pointer. Basically
            // a local value on the stack, which nothing fancy is done on. So we can
            // optimize away the pointing altogether, and just have a native variable
            variable.impl = VAR_NATIVIZED;
          } else {
            variable.impl = VAR_EMULATED;
          }
          dprint('vars', '// var ' + vname + ': ' + JSON.stringify(variable));
        }
      });
      this.forwardItem(item, 'LabelAnalyzer');
    },
  });

  // Label analyzer
  substrate.addZyme('LabelAnalyzer', {
    processItem: function(item) {
      item.functions.forEach(function(func) {
        func.hasPhi = false;
        func.remarkableLabels = [];
        func.labels.forEach(function(label) {
          label.lines.forEach(function(line) {
            if (line.value && line.value.intertype == 'phi') {
              func.remarkableLabels.push(toNiceIdent(line.value.label1));
              func.remarkableLabels.push(toNiceIdent(line.value.label2));
              func.hasPhi = true;
            }
          });
        });
      });
      this.forwardItem(item, 'Relooper');
    },
  });

  // ReLooper - reconstruct nice loops, as much as possible
  substrate.addZyme('Relooper', {
    processItem: function(item) {
      var that = this;
      function finish() {
        that.forwardItem(item, 'Optimizer');
      }

      // Tools

      function replaceLabels(line, labelId, toLabelId) {
        function process(item) {
          ['label', 'labelTrue', 'labelFalse', 'toLabel', 'unwindLabel', 'defaultLabel'].forEach(function(id) {
            if (item[id] && item[id] == labelId) {
              item[id] = toLabelId;
            }
          });
        }
        if (['branch', 'invoke'].indexOf(line.intertype) != -1) {
          process(line);
        } else if (line.intertype == 'switch') {
          process(line);
          line.switchLabels.forEach(process);
        }
      }

      function replaceLabelLabels(label, labelId, toLabelId) {
        label.lines.forEach(function(line) { replaceLabels(line, labelId, toLabelId) });
        return label;
      }

      function replaceInLabels(labels, toReplace, replaceWith) {
        assertEq(!replaceWith || toReplace.length == 1, true); // TODO: implement other case
        labels.forEach(function(label) {
          ['inLabels'].forEach(function(l) {
            label[l] = label[l].map(function(labelId) { return toReplace.indexOf(labelId) == -1 ? labelId : replaceWith})
                               .filter(function(labelId) { return !!labelId });
          });
        });
        return labels;
      }

      function calcLabelBranchingData(labels, labelsDict) {
        item.functions.forEach(function(func) {
          labels.forEach(function(label) {
            label.outLabels = [];
            label.inLabels = [];
            label.hasReturn = false;
            label.hasBreak = false;
          });
        });
        // Find direct branchings
        labels.forEach(function(label) {
          label.lines.forEach(function(line) {
            function process(item) {
              ['label', 'labelTrue', 'labelFalse', 'toLabel', 'unwindLabel'].forEach(function(id) {
                if (item[id]) {
                  if (item[id][0] == 'B') { // BREAK, BCONT, BNOPP
                    label.hasBreak = true;
                  } else {
                    label.outLabels.push(item[id]);
                    labelsDict[item[id]].inLabels.push(label.ident);
                  }
                }
              });
            }
            if (['branch', 'invoke'].indexOf(line.intertype) != -1) {
              process(line);
            } else if (line.intertype == 'switch') {
              process(line);
              line.switchLabels.forEach(process);
            }

            label.hasReturn |= line.intertype == 'return';
          });
        });
        // Find all incoming and all outgoing - recursively
        labels.forEach(function(label) {
          label.allInLabels = [];
          label.allOutLabels = [];
          //! MustGetTo ignores return - |if (x) return; else Y| must get to Y.
          label.mustGetHere = [];
        });

        var worked = true;
        while (worked) {
          worked = false;
          labels.forEach(function(label) {
            function inout(s, l) {
              var temp = label[s].slice(0);
              label[s].forEach(function(label2Id) {
                temp = temp.concat(labelsDict[label2Id][l]);
              });
              temp = dedup(temp);
              temp.sort();
              if (JSON.stringify(label[l]) != JSON.stringify(temp)) {
                label[l] = temp;
                worked = true;
              }
            }
            inout('inLabels', 'allInLabels');
            inout('outLabels', 'allOutLabels');
          });
        }

        // Find all mustGetTos
        labels.forEach(function(label) {
          function walk(path, label) {
            // If all we are is a break/return - then stop here. Otherwise, continue to other path
            if (label.hasReturn || (label.hasBreak && label.outLabels.length == 0)) {
              return [path.concat([label])]
            };
            if (path.indexOf(label) != -1) {
              return []; // loop - drop this path
            }
            path = path.concat([label]);
            return label.outLabels.map(function(outLabelId) { return walk(path, labelsDict[outLabelId]) })
                        .reduce(function(a, b) { return a.concat(b) }, [])
                        .filter(function(path) { return path.length > 0 });
          }
          var paths = walk([], label).map(function(path) { return getLabelIds(path) });
          var possibles = dedup(paths.reduce(function(a,b) { return a.concat(b) }, []));
          label.mustGetTo = possibles.filter(function(possible) {
            return paths.filter(function(path) { return path.indexOf(possible) == -1 }) == 0;
          }).filter(function(possible) { return possible != label.ident });
        });

        labels.forEach(function(label) {
          label.mustGetTo.forEach(function (mustGetTo) {
            labelsDict[mustGetTo].mustGetHere.push(label.ident);
          });
        });

        if (dcheck('relooping')) {
          labels.forEach(function(label) {
            print('// label: ' + label.ident + ' :out      : ' + JSON.stringify(label.outLabels));
            print('//        ' + label.ident + ' :in       : ' + JSON.stringify(label.inLabels));
            print('//        ' + label.ident + ' :ALL out  : ' + JSON.stringify(label.allOutLabels));
            print('//        ' + label.ident + ' :ALL in   : ' + JSON.stringify(label.allInLabels));
            print('// ZZZZZZ ' + label.ident + ' must get to all of ' + JSON.stringify(label.mustGetTo) + '\n');
          });
        }
      }

/* // Disabled merging as it seems it just removes a return now and then.
        function mergeLabels(label1Ident, label2Ident) {
          var label1 = func.labelsDict[label1Ident];
          var label2 = func.labelsDict[label2Ident];
          label1.lines.pop();
          label1.lines = label1.lines.concat(label2.lines);
          label1.outLabels = label2.outLabels;
          label2.lines = null;
          func.labels = func.labels.filter(function(label) { return !!label.lines });
          delete func.labelsDict[label2.ident];
          replaceInLabels(func.labels, [label2.ident], label1.ident);
        }

        // Merge trivial labels
        var worked = true;
        while (worked) {
          worked = false;
          func.labels.forEach(function(label) {
            if (label.lines === null) return; // We were deleted
            if (label.outLabels.length == 1 &&
                label.lines.slice(-1)[0].intertype == 'branch' &&
                label.lines.slice(-1)[0].label == label.outLabels[0] &&
                func.labelsDict[label.outLabels[0]].inLabels.length == 1) {
              mergeLabels(label.ident, label.outLabels[0]);
              worked = true;
            }
          });
        }
*/

      // 'block': A self-enclosed part of the program, for example a loop or an if
      function makeBlock(labels, entry, labelsDict) {
        var def = {
          type: 'emulated', // a block we cannot map to a nicer structure like a loop. We emulate it with a barbaric switch
          labels: labels,
          entry: entry,
        };
        if (!RELOOP) return def;

        if (!entry) {
          assertTrue(labels.length == 0);
          return null; // Empty block - not even an entry
        }

        function getLastLine(block) {
          if (!block) return null; // Completely empty block (probably had only a branch,
                                   // which was optimized out)
          dprint('relooping', 'get last line at: ' + block.labels[0].ident);
          if (block.next) return getLastLine(block.next);
          switch(block.type) {
            case 'loop':
              return getLastLine(block.rest);
            case 'if':
            case 'breakingif':
              return getLastLine(block.ifTrue);
            case 'emulated':
            case 'simple':
              if (block.labels.length == 1) {
                return block.labels[0].lines.slice(-1)[0];
              } else {
                return null;
              }
          }
        }
        function getAll(fromId, beforeIds) {
          beforeIds = beforeIds ? beforeIds : [];
          if (beforeIds && beforeIds.indexOf(fromId) != -1) return [];
          var from = labelsDict[fromId];
          return dedup([from].concat(
            from.outLabels.map(function(outLabel) { return getAll(outLabel, beforeIds.concat(fromId)) })
                          .reduce(function(a,b) { return a.concat(b) }, [])
          ), 'ident');
        }
        function isolate(label) {
          label.inLabels = [];
          label.outLabels = [];
          return label;
        }
        dprint('relooping', "\n\n// XXX MAKEBLOCK " + entry + ', num labels: ' + labels.length + ' and they are: ' + getLabelIds(labels));
        if (labels.length == 0 || !entry) {
          dprint('relooping', '//empty labels or entry');
          return;
        }
        function forLabelLines(labels, func) {
          labels.forEach(function(label) {
            label.lines.forEach(function(line) { func(line, label) });
          });
        }

        // Begin

        calcLabelBranchingData(labels, labelsDict);

        var split = splitter(labels, function(label) { return label.ident == entry });
        var first = split.splitOut[0];
        if (!first) {
          dprint('relooping', "//no first line");
          return;
        }
        var others = split.leftIn;
        var lastLine = first.lines.slice(-1)[0];
        dprint('relooping', "//     makeBlock " + entry + ' : ' + getLabelIds(labels) + ' IN: ' + first.inLabels + '   OUT: ' + first.outLabels);
        // If we have one outgoing, and none incoming - make this a block of 1,
        // and move on the others (forgetting ourself, so they are now also
        // totally self-enclosed, once we start them)
        if (first.inLabels.length == 0 && first.outLabels.length == 1) {
          dprint('relooping', '   Creating simple emulated');
          assertEq(lastLine.intertype, 'branch');
//          assertEq(!!lastLine.label, true);
          return {
            type: 'simple',
            labels: [replaceLabelLabels(first, first.outLabels[0], 'BNOPP')],
            entry: entry,
            next: makeBlock(replaceInLabels(others, entry), first.outLabels[0], labelsDict),
          };
        }
        // Looping structures - in some way, we can get back to here
        if (first.outLabels.length > 0 && first.allInLabels.indexOf(entry) != -1) {
          // Look for outsiders - labels no longer capable of getting here. Those must be
          // outside the loop. Insiders are those that can get back to the entry
          var split2 = splitter(others, function(label) { return label.allOutLabels.indexOf(entry) == -1 });
          var outsiders = split2.splitOut;
          var insiders = split2.leftIn;
          // Hopefully exactly one of the outsiders is a 'pivot' - a label to which all those leaving
          // the loop must go. Then even some |if (falala) { ... break; }| will get ...
          // as an outsider, but it will actually still be in the loop
          var pivots =
            outsiders.filter(function(outsider) {
              return insiders.filter(function(insider) {
                return insider.mustGetTo.indexOf(outsider.ident) == -1;
              }) == 0;
            });
          // Find the earliest pivot. They must be staggered, each leading to another,
          // as all insiders must go through *all* of these. So we seek a pivot that
          // is never reached by another pivot. That must be the one with fewest
          // mustGetTo
          var pivot = null;
          if (pivots.length >= 1) {
            pivots.sort(function(a, b) { return b.mustGetTo.length - a.mustGetTo.length });
            pivot = pivots[0];
            if (!(pivot.ident in searchable(first.outLabels))) {
              // pivot must be right outside - no intermediates
              // TODO: Handle this, so we can reloop a lot more stuff
              pivot = null;
            }
          }
          if (pivot) { // We have ourselves a loop
            dprint('relooping', '   Creating LOOP : ' + entry + ' insiders: ' + getLabelIds(insiders) + ' to pivot: ' + pivot.ident);
            var otherLoopLabels = insiders;
            var loopLabels = insiders.concat([first]);
            var nextLabels = outsiders;
            // Rework branches out of the loop into new 'break' labels
            forLabelLines(loopLabels, function(line) {
              replaceLabels(line, pivot.ident, 'BREAK' + entry);
            });
            // Rework branches to the inc part of the loop into |continues|
            forLabelLines(loopLabels, function(line, label) {
              if (0) {// XXX - make this work :label.outLabels.length == 1 && label.outLabels[0] == entry && !label.hasBreak && !label.hasReturn) {
                      //       it can remove unneeded continues (but does too much right now, as the continue might have been
                      //       placed into an emulated while(1) switch { }
                replaceLabels(line, entry, 'BNOPP');
              } else {
                replaceLabels(line, entry, 'BCONT' + entry);
              }
            });
            // Fix inc branch into rest
            var nextEntry = null;
            first.outLabels.forEach(function(outLabel) {
              if (outLabel != pivot.ident) {
                replaceLabels(lastLine, outLabel, 'BNOPP');
                nextEntry = outLabel;
              }
            });
            if (nextEntry == entry) { // We loop back to ourselves, the entire loop is just us
              nextEntry = null;
            }
            dprint('relooping', "Entry into next: " + nextEntry);
            var ret = {
              type: 'loop',
              labels: loopLabels,
              entry: entry,
              inc: makeBlock([isolate(first)], entry, labelsDict),
              rest: makeBlock(replaceInLabels(otherLoopLabels, entry), nextEntry, labelsDict),
            };
            dprint('relooping', '  getting last line for block starting with ' + entry + ' and nextEntry: ' + nextEntry);
            var lastLoopLine = getLastLine(nextEntry ? ret.rest : ret.inc);
            if (lastLoopLine) {
              replaceLabels(lastLoopLine, 'BCONT' + entry, 'BNOPP'); // Last line will feed back into the loop anyhow
            }
            ret.next = makeBlock(replaceInLabels(nextLabels, getLabelIds(loopLabels)), pivot.ident, labelsDict);
            return ret;
          }
        }

        // Try an 'if' structure
        if (first.outLabels.length == 2) {
          if (labelsDict[first.outLabels[1]].mustGetTo.indexOf(first.outLabels[0]) != -1) {
            first.outLabels.push(first.outLabels.shift()); // Reverse order - normalize. Very fast check anyhow
          }
          if (labelsDict[first.outLabels[0]].mustGetTo.indexOf(first.outLabels[1]) != -1) {
            var ifLabelId = first.outLabels[0];
            var outLabelId = first.outLabels[1];
            // 0 - the if area. 1 - where we all exit to later
            var ifTrueLabels = getAll(ifLabelId, [outLabelId]);
            var ifLabels = ifTrueLabels.concat([first]);
            var nextLabels = getAll(outLabelId);
            // If we can get to the outside in more than 2 ways (one from if, one from True clause) - have breaks
            var breaking = labelsDict[outLabelId].allInLabels.length > 2;
            dprint('relooping', '   Creating XXX IF: ' + getLabelIds(ifTrueLabels) + ' to ' + outLabelId + ' ==> ' + getLabelIds(nextLabels) + ' breaking: ' + breaking);
            if (breaking) {
              // Rework branches out of the if into new 'break' labels
              forLabelLines(ifTrueLabels, function(line) {
                replaceLabels(line, outLabelId, 'BREAK' + entry);
              });
            }
            // Remove branching op - we will do it manually
            replaceLabels(lastLine, ifLabelId, 'BNOPP');
            replaceLabels(lastLine, outLabelId, 'BNOPP');
            return {
              type: (breaking ? 'breaking' : '') + 'if',
              labels: ifLabels,
              entry: entry,
              ifVar: lastLine.ident,
              check: makeBlock([isolate(first)], entry, labelsDict),
              ifTrue: makeBlock(replaceInLabels(ifTrueLabels, entry), ifLabelId, labelsDict),
              next: makeBlock(replaceInLabels(nextLabels, getLabelIds(ifLabels)), outLabelId, labelsDict),
            };
          }
        }

        // Give up on this structure - emulate it
        dprint('relooping', '   Creating complex emulated');
        return def;
      }

      // TODO: each of these can be run in parallel
      item.functions.forEach(function(func) {
        dprint('relooping', "// relooping function: " + func.ident);
        func.labelsDict = {};
        func.labels.forEach(function(label) {
          func.labelsDict[label.ident] = label;
        });
        func.block = makeBlock(func.labels, toNiceIdent('%entry'), func.labelsDict);
      });

      return finish();
    },
  });

  // Optimizer
  // XXX: load, store and gep now have pointer/value/data from which we copy the ident into a toplevel ident.
  //      However, we later read the non-toplevel ident in some cases, so optimizer changes can lead to bugs.
  //      Need to remove the toplevel, work entirely with the non-toplevel. Single location.
  substrate.addZyme('Optimizer', {
    processItem: function(item) {
      var that = this;
      function finish() {
        item.__finalResult__ = true;
        return [item];
      }
      if (!OPTIMIZE) return finish();

      // Check if a line has side effects *aside* from an explicit assign if it has one
      function isLineSideEffecting(line) {
        if (line.intertype == 'assign' && line.value.intertype !== 'call') return false;
        if (['fastgetelementptrload'].indexOf(line.intertype) != -1) return false;
        return true;
      }

      function replaceVars(line, ident, replaceWith) {
        if (!replaceWith) {
          print('// Not replacing ' + dump(ident) + ' : ' + dump(replaceWith));
          return false;
        }
        var found = false;
        // assigns, loads, mathops
        var POSSIBLE_VARS = ['ident', 'ident2'];
        for (var i = 0; i < POSSIBLE_VARS.length; i++) {
          var possible = POSSIBLE_VARS[i];
          if (line[possible] == ident) {
            line[possible] = replaceWith;
            found = true;
          }
          if (line.value && line.value[possible] == ident) {
            line.value[possible] = replaceWith;
            found = true;
          }
        }
        // getelementptr, call params
        [line, line.value].forEach(function(element) {
          if (!element || !element.params) return;
          var params = element.params;
          for (var j = 0; j < params.length; j++) {
            var param = params[j];
            if (param.intertype == 'value' && param.ident == ident) {
              param.ident = replaceWith;
              found = true;
            }
          }
        });
        return found;
      }

      // Fast getelementptr loads
      item.functions.forEach(function(func) {
        for (var i = 0; i < func.lines.length-1; i++) {
          var a = func.lines[i];
          var b = func.lines[i+1];
          if (a.intertype == 'assign' && a.value.intertype == 'getelementptr' &&
              b.intertype == 'assign' && b.value.intertype == 'load' &&
              a.ident == b.value.ident && func.variables[a.ident].uses == 1) {
//            print("// LOADSUSPECT: " + i + ',' + (i+1) + ':' + a.ident + ':' + b.value.ident);
            a.intertype = 'fastgetelementptrload';
            a.ident = b.ident;
            b.intertype = null;
            i++;
          }
        }
        cleanFunc(func);
      });

      // Fast getelementptr stores
      item.functions.forEach(function(func) {
        for (var i = 0; i < func.lines.length-1; i++) {
          var a = func.lines[i];
          var b = func.lines[i+1];
          if (a.intertype == 'assign' && a.value.intertype == 'getelementptr' &&
              b.intertype == 'store' && b.value.text &&
              a.ident == b.ident && func.variables[a.ident].uses == 1) {
            //print("// STORESUSPECT: " + a.lineNum + ',' + b.lineNum);
            a.intertype = 'fastgetelementptrstore';
            a.ident = toNiceIdent(b.value.text);
            b.intertype = null;
            i++;
          }
        }
        cleanFunc(func);
      });

      // TODO: Use for all that can
      function optimizePairs(worker, minSlice, maxSlice) {
        minSlice = minSlice ? minSlice : 2;
        maxSlice = maxSlice ? maxSlice : 2;
        item.functions.forEach(function(func) {
          func.labels.forEach(function(label) {
            for (var i = 0; i < label.lines.length-1; i++) {
              for (var j = i+minSlice-1; j < Math.min(i+maxSlice+1, label.lines.length); j++) {
                if (worker(func, label.lines.slice(i, j+1))) {
                  i += j-i;
                  break; // stop working on this i
                }
              }
            }
          });
          cleanFunc(func);
        });
      }

      // Fast bitcast&something after them
      optimizePairs(function(func, lines) {
        var a = lines[0], b = lines[1];
        if (a.intertype == 'assign' && a.value.intertype == 'bitcast' &&
            func.variables[a.ident].uses == 1 && replaceVars(b, a.ident, a.value.ident)) {
          a.intertype = null;
          return true;
        }
      });

/*
      // Remove unnecessary branches
      item.functions.forEach(function(func) {
        for (var i = 0; i < func.labels.length-1; i++) {
          var a = func.labels[i].lines.slice(-1)[0];
          var b = func.labels[i+1];
          if (a.intertype == 'branch' && a.label == b.ident) {
            a.intertype = null;
          }
        }
        cleanFunc(func);
      });
*/

      // Remove temp variables around nativized
      item.functions.forEach(function(func) {
        // loads, mathops
        var worked = true;
        while (worked) {
          worked = false;
          for (var i = 0; i < func.lines.length-1; i++) {
            var a = func.lines[i];
            var b = func.lines[i+1];
            if (a.intertype == 'assign' && a.value.intertype == 'load' &&
                func.variables[a.value.ident] && // Not global
                func.variables[a.value.ident].impl === VAR_NATIVIZED) {
              //print('// ??zzzz ' + dump(a) + ',\n // ??zzbb' + dump(b));
              // If target is only used on next line - do not need it.
              if (func.variables[a.ident].uses == 1 &&
                  replaceVars(b, a.ident, a.value.ident)) {
                a.intertype = null;
                i ++;
                worked = true;
              }
            }
          }
          cleanFunc(func);
        }

        // stores
        for (var i = 0; i < func.lines.length-1; i++) {
          var a = func.lines[i];
          var b = func.lines[i+1];
          if (b.intertype == 'store' &&
              func.variables[b.ident] && // Not global
              func.variables[b.ident].impl === VAR_NATIVIZED) {
            // If target is only used on prev line - do not need it.
            if (func.variables[b.value.ident] && func.variables[b.value.ident].uses == 1 &&
                ['assign', 'fastgetelementptrload'].indexOf(a.intertype) != -1 && a.ident == b.value.ident) {
              a.ident = b.ident;
              a.overrideSSA = true;
              b.intertype = null;
              i ++;
            }
          }
        }
        cleanFunc(func);
      });

      // Remove redundant vars - SLOW! XXX
      optimizePairs(function(func, lines) {
        // a - a line defining a var
        // b - a line defining a var that is identical to a
        // c - the only line using b, hopefully
        var a = lines[0], b = lines[lines.length-2], c = lines[lines.length-1];
        if (a.intertype == 'assign' && b.intertype == 'assign' &&
            func.variables[b.ident] && func.variables[b.ident].uses == 1 &&
            compareTokens(a.value, b.value) &&
            lines.slice(0,-1).filter(isLineSideEffecting).length == 0 &&
            replaceVars(c, b.ident, a.ident)) {
          b.intertype = null;
          return true;
        }
      }, 3, 12);

      return finish();
    },
  });

  substrate.addItem({
    items: data,
  }, 'Sorter');

  return substrate.solve();
}

