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

      function replaceLabels(line, labelIds, toLabelId) {
        var ret = [];
        function process(item) {
          ['label', 'labelTrue', 'labelFalse', 'toLabel', 'unwindLabel', 'defaultLabel'].forEach(function(id) {
            if (item[id] && item[id] in labelIds) {
              ret.push(item[id]);
              assert(!item['old_' + id]);
              item['old_' + id] = item[id]; // Save it; we need this later for labels before breaks, when we have multiple entries later
              dprint('relooping', 'zz ' + id + ' replace ' + item[id] + ' with ' + toLabelId + '; old: ' + item['old_' + id]);
              item[id] = toLabelId;
            }
          });
        }
        if (['branch', 'invoke'].indexOf(line.intertype) != -1) {
          process(line);
        } else if (line.intertype == 'assign' && line.value.intertype == 'invoke') {
          process(line.value);
        } else if (line.intertype == 'switch') {
          process(line);
          line.switchLabels.forEach(process);
        }
        return ret;
      }

      function replaceLabelLabels(labels, labelIds, toLabelId) {
        ret = [];
        labels.forEach(function(label) {
          ret = ret.concat(replaceLabels(label.lines[label.lines.length-1], labelIds, toLabelId));
        });
        return ret;
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
              ['label', 'labelTrue', 'labelFalse', 'toLabel', 'unwindLabel', 'defaultLabel'].forEach(function(id) {
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
            } else if (line.intertype == 'assign' && line.value.intertype == 'invoke') {
              process(line.value);
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

        if (dcheck('relooping')) {
          labels.forEach(function(label) {
            dprint('// label: ' + label.ident + ' :out      : ' + JSON.stringify(label.outLabels));
            dprint('//        ' + label.ident + ' :in       : ' + JSON.stringify(label.inLabels));
            dprint('//        ' + label.ident + ' :ALL out  : ' + JSON.stringify(label.allOutLabels));
            dprint('//        ' + label.ident + ' :ALL in   : ' + JSON.stringify(label.allInLabels));

            // Convert to searchables, for speed (we mainly do lookups here) and code clarity (x in Xlabels)
            // FIXME TODO XXX do we need all these?
            label.outLabels = searchable(label.outLabels);
            label.inLabels = searchable(label.inLabels);
            label.allOutLabels = searchable(label.allOutLabels);
            label.allInLabels = searchable(label.allInLabels);
          });
        }
      }

      // There are X main kinds of blocks:
      //
      //  'emulated': A soup of labels, implemented as a barbaric switch in a loop
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
      //  'multiple': A block that branches into multiple subblocks, somehow.
      //
      // @param exitLabels Labels which we should not implement; our parent block will
      //                   do them. These are the external labels for our parent. Note that
      //                   they include BCONT etc., as the labels have been replaced to be that way.
      // @param exitLabelsHit Exit labels which were actually encountered - we update that. XXX do we need this?!
      function makeBlock(labels, entries, labelsDict, exitLabels, exitLabelsHit) {
        dprint('relooping', 'prelooping: ' + entries + ',' + labels.length + ' labels');
        assert(entries);

        calcLabelBranchingData(labels, labelsDict);

        function emulated() {
          labels.forEach(function(label) {
            for (l in label.outLabels) {
              exitLabelsHit[l] = true;
            }
          });
          assert(entries[0]);
          return {
            type: 'emulated',
            labels: labels,
            entries: entries.slice(0),
          };
        }
        if (!RELOOP) return emulated();

        var s_entries = searchable(entries);
        assert(entries[0]); // need at least 1 entry
        dprint('relooping', 'makeBlock: ' + entries + ',' + labels.length + ' labels');

        var entryLabels = entries.map(function(entry) { return labelsDict[entry] });
        assert(entryLabels[0]);

        var canReturn = false;
        entryLabels.forEach(function(entryLabel) {
          canReturn = canReturn || values(entryLabel.inLabels).length > 0;
        });

        // Remove unreachables
        allOutLabels = {};
        entryLabels.forEach(function(entryLabel) {
          mergeInto(allOutLabels, entryLabel.allOutLabels);
        });
        labels = labels.filter(function(label) { return label.ident in s_entries || label.ident in allOutLabels });

        // === (simple) 'emulated' ===

        if (entries.length == 1 && !canReturn && values(entryLabels[0].outLabels).length == 1) {
          var entry = entries[0];
          var entryLabel = entryLabels[0];
          var others = labels.filter(function(label) { return label.ident != entry });

          dprint('relooping', '   Creating simple emulated, outlabels: ' + keys(entryLabel.outLabels));
          var next = keys(entryLabel.outLabels)[0];
          if (next in exitLabels) {
            exitLabelsHit[next] = true;
            next = null;
          } else {
            replaceLabelLabels(others, searchable(entry));
          }
          dprint('relooping', '      next: ' + next);
          replaceLabelLabels([entryLabel], searchable(next), 'BNOPP'); // remove unneeded branch
          return {
            type: 'emulated',
            labels: [entryLabel],
            entries: entries,
            next: next ? makeBlock(others, [next], labelsDict, exitLabels, exitLabelsHit) : null,
          };
        }

        // === 'reloop' away a loop, if there is one ===

        if (entries.length == 1 && canReturn) {
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
              dprint('relooping', 'Found an external that wants to reach an internal, fallback to |return emulated()|?');
              throw "Spaghetti label flow";
            }
          });

          // We will be in a loop, |continue| gets us back to the entry
          entries.forEach(function(entry) {
            replaceLabelLabels(internals, searchable(entries), 'BCONT' + entries[0]); // entries[0] is the name of the loop, see walkBlock
          });

          // To get to any of our (not our parents') exit labels, we will break.
          dprint('relooping', 'for exit purposes, Replacing: ' + dump(currExitLabels));
          var enteredExitLabels = {};
          if (externals.length > 0) {
            entries.forEach(function(entry) {
              mergeInto(enteredExitLabels, set(replaceLabelLabels(internals, currExitLabels, 'BREAK' + entries[0]))); // see comment on entries[0] above
            });
            enteredExitLabels = keys(enteredExitLabels).map(cleanLabel);
            dprint('relooping', 'enteredExitLabels: ' + dump(enteredExitLabels));
            assert(enteredExitLabels.length > 0);
          }

          // inner
          var allExitLabels = mergeInto(set(currExitLabels), exitLabels);
          var currExitLabelsHit = {};
          ret.inner = makeBlock(internals, entries, labelsDict, allExitLabels, currExitLabelsHit);

          if (externals.length > 0) {
            // outer
            ret.outer = makeBlock(externals, enteredExitLabels, labelsDict, exitLabels, currExitLabelsHit);
            mergeInto(exitLabelsHit, setSub(currExitLabelsHit, currExitLabels)); // Don't really need setSub, but nicer
          }

          return ret;
        }

        // === handle multiple branches from the entry with a 'multiple' ===

        // TODO

        // Give up on this structure - emulate it
        dprint('relooping', '   Creating complex emulated');
        return emulated();
      }

      // TODO: each of these can be run in parallel
      item.functions.forEach(function(func) {
        dprint('relooping', "// relooping function: " + func.ident);
        func.labelsDict = {};
        func.labels.forEach(function(label) {
          func.labelsDict[label.ident] = label;
        });
        func.block = makeBlock(func.labels, [toNiceIdent('%entry')], func.labelsDict, {}, {});
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

