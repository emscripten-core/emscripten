/**
 * An implementation of an 'enzymatic programming' paradigm.
 */

DEBUG = true;
DEBUG = false;

Substrate = function(name_) {
  this.name_ = name_;
  this.zymes = {};
  this.currUid = 1;
};

Substrate.prototype = {
  addItem: function(item, targetZyme) {
    this.addItems([item], targetZyme);
  },

  addItems: function(items, targetZyme) {
    assert(targetZyme);
    if (targetZyme == '/dev/null') return;
    if (targetZyme == '/dev/stdout') {
      this.results = this.results.concat(items);
      return;
    }
    assert(this.zymes[targetZyme]);
    for (var i = 0; i < items.length; i++) {
      var item = items[i];
      if (!item.__uid__) {
        item.__uid__ = this.currUid;
        this.currUid ++;
      }
    }
    this.zymes[targetZyme].items = this.zymes[targetZyme].items.concat(items);
  },

  addZyme: function(name_, zyme) {
    assert(name_ && zyme);
    zyme.name_ = name_;
    zyme.items = [];
    zyme.forwardItem  = bind(this, this.addItem);
    zyme.forwardItems = bind(this, this.addItems);
    this.zymes[name_] = zyme;
    if (!zyme.process) zyme.process = Zyme.prototype.process;
    return zyme;
  },

  solve: function() {
    print("// Solving " + this.name_ + "...");

    var startTime = Date.now();
    var midTime = startTime;
    var that = this;
    function midComment(force) {
      var curr = Date.now();
      if (curr - midTime > 1000 || force) {
        print('// Working on ' + that.name_ + ', so far ' + ((curr-startTime)/1000).toString().substr(0,10) + ' seconds.');
        midTime = curr;
      }
    }
    function finalComment() {
      print('// Completed ' + that.name_ + ' in ' + ((Date.now() - startTime)/1000).toString().substr(0,10) + ' seconds.');
    }

    var finalResult = null;
    this.results = [];
    var finished = false;
    var that = this;
    while (!finished) {
      dprint('enzymatic', "Cycle start, items: ");// + values(this.zymes).map(function(zyme) zyme.items).reduce(function(x,y) x+y, 0));
      var hadProcessing = false;
      values(this.zymes).forEach(function(zyme) {
        midComment();

        if (zyme.items.length == 0) return;

        var inputs = zyme.items.slice(0);
        var outputs;
        var currResultCount = that.results.length;
        try {
          dprint('Processing using ' + zyme.name_ + ': ' + inputs.length);
          zyme.items = []; // More may be added in process(); we'll get to them next time
          outputs = zyme.process(inputs);
          dprint('New results: ' + (outputs.length + that.results.length - currResultCount) + ' out of ' + (that.results.length + outputs.length));
        } catch (e) {
          //print("Exception, current selected are: " + inputs.map(dump).join('\n\n'));
          print("Stack: " + new Error().stack);
          throw e;
        }
        hadProcessing = true;

        if (outputs) {
          if (outputs.length === 1 && outputs[0].__finalResult__) {
            if (DEBUG) print("Solving complete: __finalResult__");
            delete outputs[0].__finalResult__; // Might recycle this
            delete outputs[0].__uid__;
            finalComment();
            finished = true;
            finalResult = outputs[0];
          } else {
            that.results = that.results.concat(outputs);
          }
        }
      });
      if (!hadProcessing) {
        if (DEBUG) print("Solving complete: no remaining items");
        finalComment();
        this.results.forEach(function(output) {
          delete output.__result__; // Might recycle these
          delete output.__uid__;
        });
        return this.results;
      }
      if (finalResult) {
        return finalResult;
      }
      midComment();
    }
  },
};

Zyme = function() { };
Zyme.prototype = {
  process: function(items) {
    var ret = [];
    for (var i = 0; i < items.length; i++) {
      var item = items[i];
      try {
        var outputs = this.processItem(item);
        if (outputs) {
          ret = ret.concat(outputs);
        }
      } catch (e) {
        print("Exception in process(), current item is: " + dump(item));
        throw e;
      }
    }
    return ret;
  },
  processPairs: function(items, func) {
    var ret = [];
    for (var i = 0; i < items.length; i += 2) {
      try {
        ret = ret.concat(func(items[i], items[i+1]));
      } catch (e) {
        print("Exception in processPairs(), current items are: " + dump(items[i]) + ' :::: ' + dump(items[i+1]));
        throw e;
      }
    }
    return ret;
  },
};

