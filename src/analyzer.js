// Analyze intertype data. Calculates things that are necessary in order
// to do the final conversion into JavaScript later, for example,
// properties of variables, loop structures of functions, etc.

VAR_NATIVE = 'native';
VAR_NATIVIZED = 'nativized';
VAR_EMULATED = 'emulated';

function cleanFunc(func) {
  func.lines = func.lines.filter(function(line) { return line.intertype !== null });
  func.labels.forEach(function(label) {
    label.lines = label.lines.filter(function(line) { return line.intertype !== null });
  });
}

function analyzer(data, givenTypes) {
  substrate = new Substrate('Analyzer');

  // Sorter
  substrate.addActor('Sorter', {
    processItem: function(item) {
      item.items.sort(function (a, b) { return a.lineNum - b.lineNum });
      this.forwardItem(item, 'Gatherer');
    }
  });

  // Gatherer
  substrate.addActor('Gatherer', {
    processItem: function(item) {
      // Single-liners
      ['globalVariable', 'functionStub', 'type', 'unparsedFunction'].forEach(function(intertype) {
        if (intertype === 'type' && givenTypes) {
          item.types = givenTypes;
          return;
        }
        var temp = splitter(item.items, function(item) { return item.intertype == intertype });
        item[intertype + 's'] = temp.splitOut;
        item.items = temp.leftIn;
      });
      // Functions & labels
      item.functions = [];
      var currLabelFinished; // Sometimes LLVM puts a branch in the middle of a label. We need to ignore all lines after that.
      for (var i = 0; i < item.items.length; i++) {
        var subItem = item.items[i];
        if (subItem.intertype == 'function') {
          item.functions.push(subItem);
          subItem.endLineNum = null;
          subItem.lines = [];
          subItem.labels = [];

          // no explicit 'entry' label in clang on LLVM 2.8 - most of the time, but not all the time! - so we add one if necessary
          if (LLVM_STYLE == 'new' && item.items[i+1].intertype !== 'label') {
            item.items.splice(i+1, 0, {
              intertype: 'label',
              ident: '%entry',
              lineNum: subItem.lineNum + '.5'
            });
          }
        } else if (subItem.intertype == 'functionEnd') {
          item.functions.slice(-1)[0].endLineNum = subItem.lineNum;
        } else if (subItem.intertype == 'label') {
          item.functions.slice(-1)[0].labels.push(subItem);
          subItem.lines = [];
          currLabelFinished = false;
        } else if (item.functions.length > 0 && item.functions.slice(-1)[0].endLineNum === null) {
          // Internal line
          if (!currLabelFinished) {
            item.functions.slice(-1)[0].lines.push(subItem);
            item.functions.slice(-1)[0].labels.slice(-1)[0].lines.push(subItem);
            if (subItem.intertype === 'branch') {
              currLabelFinished = true;
            }
          } else {
            print('// WARNING: content after a branch in a label, line: ' + subItem.lineNum);
          }
        } else {
          print("ERROR: what is this? " + JSON.stringify(subItem));
        }
      }
      delete item.items;
      this.forwardItem(item, 'Identinicer');
    }
  });

  // IdentiNicer
  substrate.addActor('Identinicer', {
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
    if (['internal', 'hidden', 'inbounds', 'void'].indexOf(type) != -1) return;
    if (Runtime.isNumberType(type)) return;

    // 'blocks': [14 x %struct.X] etc. If this is a pointer, we need
    // to look at the underlying type - it was not defined explicitly
    // anywhere else.
    var nonPointing = removeAllPointing(type);
    var check = new RegExp(/^\[(\d+)\ x\ (.*)\]$/g).exec(nonPointing);
    if (check && !data.types[nonPointing]) {
      var num = parseInt(check[1]);
      num = Math.max(num, 1); // [0 x something] is used not for allocations and such of course, but
                              // for indexing - for an |array of unknown length|, basically. So we
                              // define the 'type' as having a single field. TODO: Ensure as a sanity
                              // check that we never allocate with this (either as a child structure
                              // in the analyzer, or in calcSize in alloca).
      var subType = check[2];
      data.types[nonPointing] = {
        name_: nonPointing,
        fields: range(num).map(function() { return subType }),
        lineNum: '?'
      };
      // Also add a |[0 x type]| type
      var zerod = '[0 x ' + subType + ']';
      if (!data.types[zerod]) {
        data.types[zerod] = {
          name_: zerod,
          fields: [subType, subType], // Two, so we get the flatFactor right. We care about the flatFactor, not the size here
          lineNum: '?'
        };
      }
      return;
    }

    if (isPointerType(type)) return;
    if (['['].indexOf(type) != -1) return;
    data.types[type] = {
      name_: type,
      fields: [ 'i' + (QUANTUM_SIZE*8) ], // a single quantum size
      flatSize: 1,
      lineNum: '?'
    };
  }

  // Typevestigator
  substrate.addActor('Typevestigator', {
    processItem: function(data) {
      // Convert types list to dict
      if (data.types.length !== undefined) {
        var old = data.types;
        data.types = {};
        old.forEach(function(type) { data.types[type.name_] = type });
      }

      // Find additional types
      walkJSON(data, function(item) {
        if (!item) return;
        if (item.type) {
          addType(item.type, data);
        }
        if (item.type2) {
          addType(item.type2, data);
        }
        if (item.pointerType) {
          addType(item.pointerType, data);
        }
        if (item.valueType) {
          addType(item.valueType, data);
        }
      });
      this.forwardItem(data, 'Typeanalyzer');
    }
  });

  // Type analyzer
  substrate.addActor('Typeanalyzer', {
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

          Runtime.calculateStructAlignment(type, item.types);

          dprint('types', 'type: ' + type.name_ + ' : ' + JSON.stringify(type.fields));
          dprint('types', '                        has final size of ' + type.flatSize + ', flatting: ' + type.needsFlattening + ' ? ' + (type.flatFactor ? type.flatFactor : JSON.stringify(type.flatIndexes)));
        });
      }

      this.forwardItem(item, 'VariableAnalyzer');
    }
  });
  
  // Variable analyzer
  substrate.addActor('VariableAnalyzer', {
    processItem: function(item) {
      // Globals

      var old = item.globalVariables;
      item.globalVariables = {};
      old.forEach(function(variable) {
        variable.impl = 'emulated'; // All global variables are emulated, for now. Consider optimizing later if useful
        item.globalVariables[variable.ident] = variable;
      });

      // Function locals

      item.functions.forEach(function(func) {
        dprint('vars', 'Analyzing variables in ' + func.ident);

        func.variables = {};

        // LLVM is SSA, so we always have a single assignment/write. We care about
        // the reads/other uses.

        // Function parameters
        func.params.forEach(function(param) {
          if (param.intertype !== 'varargs') {
            func.variables[param.ident] = {
              ident: param.ident,
              type: param.type,
              origin: 'funcparam',
              lineNum: func.lineNum,
              uses: null
            };
          }
        });

        // Normal variables
        func.lines.forEach(function(item) {
          if (item.intertype in set('assign', 'fastgetelementptrload')) {
            if (!item.value.tokens.slice(-1)[0].item) throw 'Did you run llvm-dis with -show-annotations?';
            func.variables[item.ident] = {
              ident: item.ident,
              type: item.value.type,
              origin: item.intertype === 'assign' ? item.value.intertype : 'fastgetelementptrload',
              lineNum: item.lineNum,
              uses: parseInt(item.value.tokens.slice(-1)[0].item.tokens[0].text.split('=')[1])
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

          var pointedType = removePointing(variable.type);
          if (variable.origin == 'getelementptr') {
            // Use our implementation that emulates pointers etc.
            // TODO Can we perhaps nativize some of these? However to do so, we need to discover their
            //      true types; we have '?' for them now, as they cannot be discovered in the intertyper.
            variable.impl = VAR_EMULATED;
          } else if (variable.origin == 'funcparam') {
            variable.impl = VAR_EMULATED;
          } else if (OPTIMIZE && variable.pointingLevels === 0 && !variable.hasAddrTaken) {
            // A simple int value, can be implemented as a native variable
            variable.impl = VAR_NATIVE;
          } else if (OPTIMIZE && variable.origin === 'alloca' && !variable.hasAddrTaken && !variable.hasValueTaken &&
                     (Runtime.isNumberType(pointedType) || Runtime.isPointerType(pointedType))) {
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
    }
  });

  // Label analyzer
  substrate.addActor('LabelAnalyzer', {
    processItem: function(item) {
      item.functions.forEach(function(func) {
        func.labelsDict = {};
        func.labels.forEach(function(label) {
          func.labelsDict[label.ident] = label;
        });
        func.hasPhi = false;
        func.remarkableLabels = [];
        func.labels.forEach(function(label) {
          label.lines.forEach(function(line) {
            if (line.value && line.value.intertype == 'phi') {
              for (var i = 0; i < line.value.params.length; i++) {
                var remarkableLabelId = line.value.params[i].label;
                func.remarkableLabels.push(remarkableLabelId);
                var remarkableLabel = func.labelsDict[remarkableLabelId];
                var lastLine = remarkableLabel.lines.slice(-1)[0];
                if (lastLine.value) {
                  lastLine.value.currLabelId = remarkableLabelId;
                } else {
                  lastLine.currLabelId = remarkableLabelId;
                }
              }
              func.hasPhi = true;
            }
          });
        });
      });
      this.forwardItem(item, 'StackAnalyzer');
    }
  });

  // Stack analyzer - calculate the base stack usage
  substrate.addActor('StackAnalyzer', {
    processItem: function(data) {
      data.functions.forEach(function(func) {
        var total = 0;
        var lines = func.labels[0].lines;
        for (var i = 0; i < lines.length; i++) {
          var line = lines[i];
          var item = line.value;
          if (!item || item.intertype != 'alloca') break;
          assert(isNumber(item.allocatedNum));
          item.allocatedSize = func.variables[line.ident].impl === VAR_EMULATED ?
            calcAllocatedSize(item.allocatedType, data.types)*item.allocatedNum: 0;
          total += item.allocatedSize;
        }
        func.initialStack = total;
        var index = 0;
        for (var i = 0; i < lines.length; i++) {
          var item = lines[i].value;
          if (!item || item.intertype != 'alloca') break;
          item.allocatedIndex = index;
          index += item.allocatedSize;
          delete item.allocatedSize;
        }
      });
      this.forwardItem(data, 'Relooper');
    }
  });

  var BRANCH_INVOKE = set('branch', 'invoke');
  function operateOnLabels(line, func) {
    function process(item, id) {
      ['label', 'labelTrue', 'labelFalse', 'toLabel', 'unwindLabel', 'defaultLabel'].forEach(function(id) {
        if (item[id]) {
          func(item, id);
        }
      });
    }
    if (line.intertype in BRANCH_INVOKE) {
      process(line);
    } else if (line.intertype == 'assign' && line.value.intertype == 'invoke') {
      process(line.value);
    } else if (line.intertype == 'switch') {
      process(line);
      line.switchLabels.forEach(process);
    }
  }

  //! @param toLabelId If false, just a dry run - useful to search for labels
  function replaceLabels(line, labelIds, toLabelId) {
    var ret = [];

    var value = keys(labelIds)[0];
    var wildcard = value.indexOf('*') >= 0;
    assert(!wildcard || values(labelIds).length == 1); // For now, just handle that case
    var wildcardParts = null;
    if (wildcard) {
      wildcardParts = value.split('|');
    }
    function wildcardCheck(s) {
      var parts = s.split('|');
      for (var i = 0; i < 3; i++) {
        if (wildcardParts[i] !== '*' && wildcardParts[i] != parts[i]) return false;
      }
      return true;
    }

    operateOnLabels(line, function process(item, id) {
      if (item[id] in labelIds || (wildcard && wildcardCheck(item[id]))) {
        ret.push(item[id]);
        dprint('relooping', 'zz ' + id + ' replace ' + item[id] + ' with ' + toLabelId);
        if (toLabelId) {
          item[id] = toLabelId + '|' + item[id];
        }
      }
    });
    return ret;
  }

  function replaceLabelLabels(labels, labelIds, toLabelId) {
    ret = [];
    labels.forEach(function(label) {
      ret = ret.concat(replaceLabels(label.lines[label.lines.length-1], labelIds, toLabelId));
    });
    return ret;
  }

  // ReLooper - reconstruct nice loops, as much as possible
  substrate.addActor('Relooper', {
    processItem: function(item) {
      var that = this;
      function finish() {
        that.forwardItem(item, 'LoopOptimizer');
      }

      // Tools

      function calcLabelBranchingData(labels, labelsDict) {
        item.functions.forEach(function(func) {
          labels.forEach(function(label) {
            label.outLabels = [];
            label.inLabels = [];
            label.hasReturn = false;
            label.hasBreak = false;
            if (!label.originalOutlabels) {
              label.originalOutLabels = [];
              label.needOriginalOutLabels = true;
            }
          });
        });
        // Find direct branchings
        labels.forEach(function(label) {
          [label.lines[label.lines.length-1]].forEach(function(line) {
            operateOnLabels(line, function process(item, id) {
              if (item[id][0] == 'B') { // BREAK, BCONT, BNOPP, BJSET
                label.hasBreak = true;
              } else {
                label.outLabels.push(item[id]);
                labelsDict[item[id]].inLabels.push(label.ident);
              }
              if (label.needOriginalOutLabels) {
                label.originalOutLabels.push(item[id]);
              }
            });
            label.needOriginalOutLabels = false;
            label.hasReturn |= line.intertype == 'return';
          });
        });
        // Find all incoming and all outgoing - recursively
        labels.forEach(function(label) {
          label.allInLabels = [];
          label.allOutLabels = [];
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

        labels.forEach(function(label) {
          if (dcheck('relooping')) {
            dprint('// label: ' + label.ident + ' :out      : ' + JSON.stringify(label.outLabels));
            dprint('//        ' + label.ident + ' :in       : ' + JSON.stringify(label.inLabels));
            dprint('//        ' + label.ident + ' :ALL out  : ' + JSON.stringify(label.allOutLabels));
            dprint('//        ' + label.ident + ' :ALL in   : ' + JSON.stringify(label.allInLabels));
            dprint('//        ' + label.ident + ' :origOut  : ' + JSON.stringify(label.originalOutLabels));
          }

          // Convert to set, for speed (we mainly do lookups here) and code clarity (x in Xlabels)
          // Also removes duplicates (which we can get in llvm switches)
          // TODO do we need all these?
          label.outLabels = set(label.outLabels);
          label.inLabels = set(label.inLabels);
          label.allOutLabels = set(label.allOutLabels);
          label.allInLabels = set(label.allInLabels);
        });
      }

      var idCounter = 0;
      function makeBlockId(entries) {
        idCounter++;
        return entries.join('$') + '$' + idCounter;
      }

      // There are X main kinds of blocks:
      //
      //----------------------------------------------------------------------------------------
      //
      //  'emulated': A soup of labels, implemented as a barbaric switch in a loop. Any
      //              label can get to any label. No block follows this.
      //
      //  'reloop': That is a block of the following shape:
      //
      //       loopX: while(1) {
      //         // internal labels, etc. Labels are internal to the current one, if
      //         // they can return to it.
      //         //
      //         // Such labels can either do |continue loopX| to get back to the entry label,
      //         // or set __label__ and do |break loopX| to get to any of the external entries
      //         // they need to get to. External labels, of course, are those that cannot
      //         // get to the entry
      //       }
      //       // external labels
      //
      //  'multiple': A block that branches into multiple subblocks, each independent,
      //              finally leading outside into another block afterwards
      //              For now, we do this in a loop, so we can break out of it easily to get
      //              to the labels afterwards. TODO: Optimize that out
      //
      function makeBlock(labels, entries, labelsDict) {
        if (labels.length == 0) return null;
        dprint('relooping', 'prelooping: ' + entries + ',' + labels.length + ' labels');
        assert(entries && entries[0]); // need at least 1 entry

        var blockId = makeBlockId(entries);

        var emulated = {
          type: 'emulated',
          id: blockId,
          labels: labels,
          entries: entries.slice(0)
        };
        if (!RELOOP) return emulated;

        calcLabelBranchingData(labels, labelsDict);

        var s_entries = set(entries);
        dprint('relooping', 'makeBlock: ' + entries + ',' + labels.length + ' labels');

        var entryLabels = entries.map(function(entry) { return labelsDict[entry] });
        assert(entryLabels[0]);

        var canReturn = false, mustReturn = true;
        entryLabels.forEach(function(entryLabel) {
          var curr = values(entryLabel.inLabels).length > 0;
          canReturn = canReturn || curr;
          mustReturn = mustReturn && curr;
        });

        // Remove unreachables
        allOutLabels = {};
        entryLabels.forEach(function(entryLabel) {
          mergeInto(allOutLabels, entryLabel.allOutLabels);
        });
        labels = labels.filter(function(label) { return label.ident in s_entries || label.ident in allOutLabels });

        // === (simple) 'emulated' ===

        if (entries.length == 1 && !canReturn) {
          var entry = entries[0];
          var entryLabel = entryLabels[0];
          var others = labels.filter(function(label) { return label.ident != entry });

          var nextEntries = keys(entryLabel.outLabels);
          dprint('relooping', '   Creating simple emulated, outlabels: ' + nextEntries);
          nextEntries.forEach(function(nextEntry) {
            replaceLabelLabels([entryLabel], set(nextEntry), 'BJSET|' + nextEntry); // Just SET __label__ - no break or continue or whatnot
          });
          return {
            type: 'emulated',
            id: blockId,
            labels: [entryLabel],
            entries: entries,
            next: makeBlock(others, keys(entryLabel.outLabels), labelsDict)
          };
        }

        // === 'reloop' away a loop, if we need to ===

        function makeLoop() {
          var ret = {
            type: 'reloop',
            id: blockId,
            entries: entries,
            labels: labels
          };

          // Find internal and external labels
          var split_ = splitter(labels, function(label) {
            return !(label.ident in s_entries) && values(setIntersect(s_entries, label.allOutLabels)).length == 0;
          });
          var externals = split_.splitOut;
          var internals = split_.leftIn;
          var currExitLabels = set(getLabelIds(externals));

          dprint('relooping', function() { return '   Creating reloop: Inner: ' + dump(getLabelIds(internals)) + ', Exxer: ' + dump(currExitLabels) });

          // Verify that no external can reach an internal
          var inLabels = set(getLabelIds(internals));
          externals.forEach(function(external) {
            if (values(setIntersect(external.outLabels, inLabels)).length > 0) {
              dprint('relooping', 'Found an external that wants to reach an internal, fallback to emulated?');
              throw "Spaghetti label flow";
            }
          });

          // We will be in a loop, |continue| gets us back to the entry
          entries.forEach(function(entry) {
            replaceLabelLabels(internals, set(entries), 'BCONT|' + blockId);
          });

          // To get to any of our (not our parents') exit labels, we will break.
          dprint('relooping', 'for exit purposes, Replacing: ' + dump(currExitLabels));
          var enteredExitLabels = {};
          if (externals.length > 0) {
            entries.forEach(function(entry) {
              mergeInto(enteredExitLabels, set(replaceLabelLabels(internals, currExitLabels, 'BREAK|' + blockId)));
            });
            enteredExitLabels = keys(enteredExitLabels).map(cleanLabel);
            dprint('relooping', 'enteredExitLabels: ' + dump(enteredExitLabels));
            assert(enteredExitLabels.length > 0);
          }

          // inner
          ret.inner = makeBlock(internals, entries, labelsDict);

          if (externals.length > 0) {
            // outer
            ret.next = makeBlock(externals, enteredExitLabels, labelsDict);
          }

          return ret;
        }

        if (mustReturn) return makeLoop();

        // === handle multiple branches from the entry with a 'multiple' ===
        //
        // We cannot loop back to the entries, but aside from that we know nothing. We
        // try to create as much structure as possible, leaving subblocks to be |emulated|
        // if we can't do any better.

        // For each entry, try to 'build it out' as much as possible. Add labels, until
        //    * hit a post label
        //    * hit a label reachable by another actual entry

        dprint('relooping', 'trying multiple...');

        var shouldNotReach = entryLabels;
        var handlingNow = [];
        var actualEntryLabels = [];
        var postEntryLabels = {};
        entryLabels.forEach(function(entryLabel) {
          entryLabel.blockChildren = [];
          var visited = {};
          function tryAdd(label) {
            if (label.ident in visited) return;
            visited[label.ident] = true;
            function isReachable(label, otherLabels, ignoreLabel) { // is label reachable by otherLabels, ignoring ignoreLabel in those otherLabels
              var reachable = false;
              otherLabels.forEach(function(otherLabel) {
                reachable = reachable || (otherLabel !== ignoreLabel && (label.ident == otherLabel.ident ||
                                                                         label.ident in otherLabel.allOutLabels));
              });
              return reachable;
            }

            if (!isReachable(label, shouldNotReach, entryLabel)) {
              entryLabel.blockChildren.push(label);
              handlingNow.push(label);
              keys(label.outLabels).forEach(function(outLabelId) { tryAdd(labelsDict[outLabelId]) });
            } else {
              postEntryLabels[label.ident] = true; // This will be an entry in the next block
            }
          }
          tryAdd(entryLabel);
          if (entryLabel.blockChildren.length > 0) {
            dprint('relooping', '  Considering multiple, found a valid entry, ' + entryLabel.ident);
            actualEntryLabels.push(entryLabel);
          }
        });

        dprint('relooping', '  Considering multiple, canHandle: ' + getLabelIds(handlingNow));

        if (handlingNow.length == 0) {
          // Spaghetti - cannot even find a single label to do before the rest. What a mess.
          // But if there is looping, perhaps we can use that to simplify matters?
          if (canReturn) {
            return makeLoop();
          } else {
            throw "Spaghetti encountered in relooping.";
          }
        }

        // This is a 'multiple'

        var actualEntries = getLabelIds(actualEntryLabels);
        dprint('relooping', '   Creating multiple, with entries: ' + actualEntries + ', post entries: ' + dump(postEntryLabels));
        actualEntryLabels.forEach(function(actualEntryLabel) {
          dprint('relooping', '      creating sub-block in multiple for ' + actualEntryLabel.ident + ' : ' + getLabelIds(actualEntryLabel.blockChildren) + ' ::: ' + actualEntryLabel.blockChildren.length);

          keys(postEntryLabels).forEach(function(post) {
            replaceLabelLabels(actualEntryLabel.blockChildren, set(post), 'BREAK|' + blockId);
          });
          // Create child block
          actualEntryLabel.block = makeBlock(actualEntryLabel.blockChildren, [actualEntryLabel.blockChildren[0].ident], labelsDict);
        });
        return {
          type: 'multiple',
          id: blockId,
          entries: actualEntries,
          entryLabels: actualEntryLabels,
          labels: handlingNow,
          next: makeBlock(labels.filter(function(label) { return handlingNow.indexOf(label) == -1 }), keys(postEntryLabels), labelsDict)
        };
      }

      // TODO: each of these can be run in parallel
      item.functions.forEach(function(func) {
        dprint('relooping', "// relooping function: " + func.ident);
        func.block = makeBlock(func.labels, [toNiceIdent(func.labels[0].ident)], func.labelsDict);
      });

      return finish();
    }
  });

  // LoopOptimizer. The Relooper generates native loop structures, that are
  //       logically correct. The LoopOptimizer works on that, doing further optimizations
  //       like switching to BNOPP when possible, etc.

  substrate.addActor('LoopOptimizer', {
    processItem: function(item) {
      var that = this;
      function finish() {
        item.__finalResult__ = true;
        return [item];
      }
      if (!RELOOP) return finish();

      // Find where each block will 'naturally' get to, just by the flow of code
      function exploreBlock(block, endOfTheWorld) { // endoftheworld - where we will get, if we have nothing else to get to - 'fall off the face of the earth'
        if (!block) return;

        function singular(block) {
          if (!block) return endOfTheWorld;
          if (block.type === 'multiple') return null;
          if (block.entries.length == 1) {
            return block.entries[0];
          } else {
            return null;
          }
        }

        dprint('relooping', "//    exploring block: " + block.type + ' : ' + block.entries);

        if (block.type == 'reloop') {
          exploreBlock(block.inner, singular(block.inner));
        } else if (block.type == 'multiple') {
          block.entryLabels.forEach(function(entryLabel) { exploreBlock(entryLabel.block, singular(block.next)) });
        } else if (block.type === 'emulated' && block.next && block.next.type === 'multiple') {
          assert(block.labels.length == 1);
          var lastLine = block.labels[0].lines.slice(-1)[0];
          if (lastLine.intertype == 'branch' && lastLine.ident) { // TODO: handle switch, and other non-branch2 things
            // 'Steal' the condition
            block.next.stolenCondition = lastLine;
            dprint('relooping', 'steal condition: ' + block.next.stolenCondition.ident);
            lastLine.stolen = true;
          }
        }

        exploreBlock(block.next, endOfTheWorld);

        if (block.next) {
          block.willGetTo = singular(block.next);
        } else {
          block.willGetTo = endOfTheWorld;
        }

        dprint('relooping', "//    explored block: " + block.type + ' : ' + block.entries + ' , willGetTo: ' + block.willGetTo);
      }

      // Remove unneeded label settings, if we set it to where we will get anyhow
      function optimizeBlock(block) {
        if (!block) return;

        dprint('relooping', "//    optimizing block: " + block.type + ' : ' + block.entries);

        recurseBlock(block, optimizeBlock);

        if (block.type === 'emulated' && block.willGetTo) {
          dprint('relooping', '//         removing (trying): ' + block.willGetTo);
          replaceLabelLabels(block.labels, set('BJSET|' + block.willGetTo + '|' + block.willGetTo), 'BNOPP');
          replaceLabelLabels(block.labels, set('BCONT|' + block.willGetTo + '|' + block.willGetTo), 'BNOPP');
          replaceLabelLabels(block.labels, set('BREAK|*|' + block.willGetTo), 'BNOPP');
        } else if (block.type === 'multiple') {
          // Stolen conditions can be optimized further than the same branches in their original position
          var stolen = block.stolenCondition;
          if (stolen) {
            [stolen.labelTrue, stolen.labelFalse].forEach(function(entry) {
              entryLabel = block.entryLabels.filter(function(possible) { return possible.ident === getActualLabelId(entry) })[0];
              if (entryLabel) {
                replaceLabelLabels([{ lines: [stolen] }], set(entry), 'BNOPP');
              } else {
                replaceLabelLabels([{ lines: [stolen] }], set('BJSET|' + block.willGetTo + '|' + block.willGetTo), 'BNOPP');
              }
            });
          }

          // Check if the one-time loop (that allows breaking out) is actually needed
          if (replaceLabelLabels(block.labels, set('BREAK|' + block.id + '|*')).length === 0) {
            block.loopless = true;
          }
        }
      }

      // TODO: Parallelize
      item.functions.forEach(function(func) {
        dprint('relooping', "// loopOptimizing function: " + func.ident);
        exploreBlock(func.block);
        optimizeBlock(func.block);
      });
      return finish();
    }
  });

  substrate.addItem({
    items: data
  }, 'Sorter');

  return substrate.solve();
}

