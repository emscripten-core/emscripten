/**
 * An implementation of an 'enzymatic programming' paradigm.
 */

DEBUG = true;
DEBUG = false;

Substrate = function(name_) {
  this.name_ = name_;
  this.items = [];
  this.zymes = [];
  this.currUid = 1;
};

Substrate.prototype = {
  addItem: function(item) {
    if (!item.__uid__) {
      item.__uid__ = this.currUid;
      this.currUid ++;
    }
    this.items.push(item);
  },

  addZyme: function(zyme) {
    var name_ = '?';
    if (typeof zyme == 'string') {
      name_ = zyme;
      zyme = arguments[1];
    }
    zyme.name_ = name_;
    this.zymes.push(zyme);
    if (!zyme.select) zyme.select = Zyme.prototype.select;
    if (!zyme.process) zyme.process = Zyme.prototype.process;
  },

  solve: function() {
    print("// Solving " + this.name_ + "...");

    var startTime = Date.now();
    var midTime = startTime;
    var that = this;
    function midComment() {
      var curr = Date.now();
      if (curr - midTime > 1000) {
        print('// Working on ' + that.name_ + ', so far ' + ((curr-startTime)/1000).toString().substr(0,10) + ' seconds. Have ' + that.items.length + ' items.');
        midTime = curr;
      }
    }
    function finalComment() {
      print('// Completed ' + that.name_ + ' in ' + ((Date.now() - startTime)/1000).toString().substr(0,10) + ' seconds.');
    }

    // Naive solver - sheer brute force.
    // Assumes list of Zymes is non-changing.
    var results = [];
    while (true) {
      dprint('enzymatic', "Cycle start, " + this.items.length + " items.");
      var hadProcessing = false;
      for (var z = 0; z < this.zymes.length; z++) {
        midComment();
        var zyme = this.zymes[z];
        var selected = zyme.select(this.items);
        if (selected.length > 0) {
          if (DEBUG) print("Calling: " + (zyme.processItem ? zyme.processItem : zyme.process));
          if (DEBUG) {
            try {
              print("Inputs: \n---\n\n" + outputs.map(JSON.stringify).join('\n\n') + '\n\n---');
            } catch(e) {
              print("Inputs: \n---\n\n" + outputs + '\n\n---');
            }
          }
          hadProcessing = true;
          var outputs;
          try {
            dprint('Processing using ' + zyme.name_);
            PROF(true);
            outputs = zyme.process(selected);
            PROF();
            dprint('...complete');
          } catch (e) {
            print("Exception, current selected are: " + selected.map(dump).join('\n\n').substr(0,100));
            print("Stack: " + new Error().stack);
            throw e;
          }
          if (DEBUG) {
            try {
              print("Outputs: \n---\n\n" + outputs.map(JSON.stringify).join('\n\n') + '\n\n---');
            } catch(e) {
              print("Outputs: \n---\n\n" + outputs + '\n\n---');
            }
          }
          if (outputs.length === 1 && outputs[0].__finalResult__) {
            if (DEBUG) print("Solving complete: __finalResult__");
            delete outputs[0].__finalResult__; // Might recycle this
            delete outputs[0].__uid__;
            finalComment();
            return outputs[0];
          }
          results = results.concat(outputs.filter(function(output) { return !!output.__result__; }))
/*
          this.items = this.items.filter(function(item) { PROF(); return selected.indexOf(item) == -1 });
          outputs.filter(function(output) { return !output.__result__; }).forEach(this.addItem, this);
*/
          var nonResults = outputs.filter(function(output) { return !output.__result__; });

          var keptUids = {};
          nonResults.forEach(function(s) {
            if (s.__uid__) {
              keptUids[s.__uid__] = true;
            } else {
              this.addItem(s);
            }
          }, this);
          var droppedUids = {};
          selected.forEach(function(s) { if (!keptUids[s.__uid__]) droppedUids[s.__uid__] = true });
          this.items = this.items.filter(function(item) {
            if (!droppedUids[item.__uid__]) {
              return true;
            } else {
              delete item.__uid__;
            }
           });
        }
      }
      if (this.items.length === 0) {
        if (DEBUG) print("Solving complete: no remaining items");
        finalComment();
        results.forEach(function(output) {
          delete output.__result__; // Might recycle these
          delete output.__uid__;
        });
        return results;
      }
      if (!hadProcessing) {
        print("Reached a dead end.");
        this.items.forEach(function(item) {
          print("remaining item:" + dump(item));
        });
        throw "failure";
      }
      midComment();
      this.items = this.items.filter(function(item) { return item !== null; });
    }
  },
};

Zyme = function() { };
Zyme.prototype = {
  select: function(items) {
    return items.filter(this.selectItem, this);
  },
  process: function(items) {
    var ret = [];
    items.forEach(function(item) { ret = ret.concat(this.processItem(item)) }, this);
    return ret;
  },
};

