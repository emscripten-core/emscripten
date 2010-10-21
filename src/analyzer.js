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

          // no explicit 'entry' label in clang on LLVM 2.8 - most of the time, but not all the time! - so we add one if necessary
          if (LLVM_STYLE == 'new' && item.items[i+1].intertype !== 'label') {
            item.items.splice(i+1, 0, {
              intertype: 'label',
              ident: '%entry',
              lineNum: subItem.lineNum + '.5',
            });
          }
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
    if (isNumberType(type)) return;

    // 'blocks': [14 x %struct.X] etc. If this is a pointer, we need
    // to look at the underlying type - it was not defined explicitly
    // anywhere else.
    var nonPointing = removeAllPointing(type);
    var check = new RegExp(/^\[(\d+)\ x\ (.*)\]$/g).exec(nonPointing);
    if (check && !data.types[nonPointing]) {
      var num = parseInt(check[1]);
      var subType = check[2];
      data.types[nonPointing] = {
        name_: nonPointing,
        fields: range(num).map(function() { return subType }),
        lineNum: '?',
      };
      return;
    }

    if (isPointerType(type)) return;
    if (['['].indexOf(type) != -1) return;
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
          // Calculate aligned size, just like C structs should be. TODO: Consider
          // requesting that compilation be done with #pragma pack(push) /n #pragma pack(1),
          // which would remove much of the complexity here.
          type.flatSize = 0;
          var diffs = [];
          var prev = -1, maxSize = -1;
          type.flatIndexes = type.fields.map(function(field) {
            var size;
            if (isNumberType(field) || isPointerType(field)) {
              size = getNativeFieldSize(field, true); // pack char; char; in structs, also char[X]s.
              maxSize = Math.max(maxSize, size);
            } else if (isStructType(field)) {
              size = item.types[field].flatSize;
              maxSize = Math.max(maxSize, QUANTUM_SIZE);
            } else {
              dprint('Unclear type in struct: ' + field + ', in ' + type.name_);
              assert(0);
            }
            var curr = Runtime.alignMemory(type.flatSize, Math.min(QUANTUM_SIZE, size)); // if necessary, place this on aligned memory
            type.flatSize = curr + size;
            if (prev >= 0) {
              diffs.push(curr-prev);
            }
            prev = curr;
            return curr;
          });
          type.flatSize = Runtime.alignMemory(type.flatSize, maxSize);
          if (diffs.length == 0) {
            type.flatFactor = type.flatSize;
          } else if (dedup(diffs).length == 1) {
            type.flatFactor = diffs[0];
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
        dprint('vars', 'Analyzing variables in ' + func.ident);

        func.variables = {};

        // LLVM is SSA, so we always have a single assignment/write. We care about
        // the reads/other uses.
        walkJSON(func.lines, function(item) {
          if (item && item.intertype == 'assign' && ['alloca', 'load', 'call', 'bitcast', 'mathop', 'getelementptr', 'fastgetelementptrload'].indexOf(item.value.intertype) != -1) {
            func.variables[item.ident] = {
              ident: item.ident,
              type: item.value.type,
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
          } else if (OPTIMIZE && variable.pointingLevels === 0 && !variable.hasAddrTaken) {
            // A simple int value, can be implemented as a native variable
            variable.impl = VAR_NATIVE;
          } else if (OPTIMIZE && variable.origin === 'alloca' && !variable.hasAddrTaken && !variable.hasValueTaken) {
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
      this.forwardItem(item, 'StackAnalyzer');
    },
  });

  // Stack analyzer - calculate the base stack usage
  substrate.addZyme('StackAnalyzer', {
    processItem: function(data) {
      data.functions.forEach(function(func) {
        var total = 0;
        var lines = func.labels[0].lines;
        for (var i = 0; i < lines.length; i++) {
          var line = lines[i];
          var item = line.value;
          if (!item || item.intertype != 'alloca') continue;
          assert(item.allocatedNum === 1);
          item.allocatedSize = func.variables[line.ident].impl === VAR_EMULATED ?
            calcAllocatedSize(item.allocatedType, data.types) : 0;
          total += item.allocatedSize;
        }
        func.initialStack = total;
        var index = 0;
        for (var i = 0; i < lines.length; i++) {
          var item = lines[i].value;
          if (!item || item.intertype != 'alloca') continue;
          item.allocatedIndex = index;
          index += item.allocatedSize;
          delete item.allocatedSize;
        }
      });
      this.forwardItem(data, 'Relooper');
    },
  });

  var BRANCH_INVOKE = searchable('branch', 'invoke');
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
  substrate.addZyme('Relooper', {
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

          // Convert to searchables, for speed (we mainly do lookups here) and code clarity (x in Xlabels)
          // Also removes duplicates (which we can get in llvm switches)
          // FIXME TODO XXX do we need all these?
          label.outLabels = searchable(label.outLabels);
          label.inLabels = searchable(label.inLabels);
          label.allOutLabels = searchable(label.allOutLabels);
          label.allInLabels = searchable(label.allInLabels);
        });
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

        var emulated = {
          type: 'emulated',
          labels: labels,
          entries: entries.slice(0),
        };
        if (!RELOOP) return emulated;

        calcLabelBranchingData(labels, labelsDict);

        var s_entries = searchable(entries);
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
          //if (nextEntries.length == 1) {
          //  replaceLabelLabels([entryLabel], set(nextEntries), 'BNOPP|XXX'); // remove unneeded branch XXX - this is dangerous, as we may
          //                                                               // have 1 next entry, but 1 or more B-labels...
          //} else {
            nextEntries.forEach(function(nextEntry) {
              replaceLabelLabels([entryLabel], set(nextEntry), 'BJSET|' + nextEntry); // Just SET __label__ - no break or continue or whatnot
            });
          //}
          return {
            type: 'emulated',
            labels: [entryLabel],
            entries: entries,
            next: makeBlock(others, keys(entryLabel.outLabels), labelsDict),
          };
        }

        // === 'reloop' away a loop, if we need to ===

        function makeLoop() {
          var ret = {
            type: 'reloop',
            entries: entries,
            labels: labels,
          };

          // Find internal and external labels
          var split_ = splitter(labels, function(label) {
            return !(label.ident in s_entries) && values(setIntersect(s_entries, label.allOutLabels)).length == 0
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
            replaceLabelLabels(internals, searchable(entries), 'BCONT|' + entries[0]); // entries[0] is the name of the loop, see walkBlock
          });

          // To get to any of our (not our parents') exit labels, we will break.
          dprint('relooping', 'for exit purposes, Replacing: ' + dump(currExitLabels));
          var enteredExitLabels = {};
          if (externals.length > 0) {
            entries.forEach(function(entry) {
              mergeInto(enteredExitLabels, set(replaceLabelLabels(internals, currExitLabels, 'BREAK|' + entries[0]))); // see comment on entries[0] above
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
            replaceLabelLabels(actualEntryLabel.blockChildren, set(post), 'BREAK|' + actualEntries[0]);
          });
          // Create child block
          actualEntryLabel.block = makeBlock(actualEntryLabel.blockChildren, [actualEntryLabel.blockChildren[0].ident], labelsDict);
        });
        return {
          type: 'multiple',
          entries: actualEntries,
          entryLabels: actualEntryLabels,
          labels: handlingNow,
          next: makeBlock(labels.filter(function(label) { return handlingNow.indexOf(label) == -1 }), keys(postEntryLabels), labelsDict),
        };
      }

      // TODO: each of these can be run in parallel
      item.functions.forEach(function(func) {
        dprint('relooping', "// relooping function: " + func.ident);
        func.labelsDict = {};
        func.labels.forEach(function(label) {
          func.labelsDict[label.ident] = label;
        });
        func.block = makeBlock(func.labels, [toNiceIdent(func.labels[0].ident)], func.labelsDict);
      });

      return finish();
    },
  });

  // LoopOptimizer. The Relooper generates native loop structures, that are
  //       logically correct. The LoopOptimizer works on that, doing further optimizations
  //       like switching to BNOPP when possible, etc.

  substrate.addZyme('LoopOptimizer', {
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
          if (replaceLabelLabels(block.labels, set('BREAK|' + block.entries[0] + '|*')).length === 0) {
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
    },
  });

  substrate.addItem({
    items: data,
  }, 'Sorter');

  return substrate.solve();
}

