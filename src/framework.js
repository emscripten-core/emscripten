//
// A framework to make building Emscripten easier. Lets you write modular
// code to handle specific issues.
//
// Actor - Some code that processes items.
// Item - Some data that is processed by actors.
// Substrate - A 'world' with some actors and some items.
//
// The idea is to create a substrate, fill it with the proper items
// and actors, and then run it to completion. Actors will process
// the items they are given, and forward the results to other actors,
// until we are finished. Some of the results are then the final
// output of the entire calculation done in the substrate.
//

var DEBUG = 0;
var DEBUG_MEMORY = 0;

var MemoryDebugger = {
  time: Date.now(),
  datas: {},
  last: 0,

  tick: function(name) {
    var now = Date.now();
    if (now - this.time > 1000) {
      // ..
      this.time = now;
    }

    // assume |name| exists from now on

    print('zzz timer gc...');
    var raw = gc().replace('\n', '');
    print('zzz       gc: ' + raw);
    var info = /before (\d+), after (\d+),.*/.exec(raw);
    var before = info[1];
    var after = info[2];
    var garbage = before - after;
    var real = after - MemoryDebugger.last;
    MemoryDebugger.last = after;

    if (Math.abs(garbage) + Math.abs(real) > 0) {
      var data = MemoryDebugger.datas[name];
      if (!data) {
        data = MemoryDebugger.datas[name] = {
          name: name,
          count: 0,
          garbage: 0,
          real: 0
        };
      }
      data.garbage += garbage;
      data.real += real;
    }

    MemoryDebugger.dump();
  },

  dump: function() {
    var vals = values(MemoryDebugger.datas);
    print('zz real:');
    vals.sort(function(x, y) { return y.real - x.real });
    vals.forEach(function(v) { if (v.real > 1024*1024) print('zz    ' + v.name + ' real = ' + (v.real/(1024*1024)).toFixed(3) + ' mb'); });
    print('zz garbage:');
    vals.sort(function(x, y) { return y.garbage - x.garbage });
    vals.forEach(function(v) { if (v.garbage > 1024*1024) print('zz    ' + v.name + ' garbage = ' + (v.garbage/(1024*1024)).toFixed(3) + ' mb'); });
  }
}

Substrate = function(name_) {
  this.name_ = name_;
  this.actors = {};
  this.currUid = 1;
};

Substrate.prototype = {
  addItem: function(item, targetActor) {
    if (targetActor == '/dev/null') return;
    if (targetActor == '/dev/stdout') {
      this.results.push(item);
      return;
    }
    this.actors[targetActor].inbox.push(item);
  },

  addItems: function(items, targetActor) {
    if (targetActor == '/dev/null') return;
    if (targetActor == '/dev/stdout') {
      this.results = this.results.concat(items);
      return;
    }
    this.actors[targetActor].inbox = this.actors[targetActor].inbox.concat(items);
  },

  checkInbox: function(actor) {
    var items = actor.inbox;
    for (var i = 0; i < items.length; i++) {
      var item = items[i];
      if (!item.__uid__) {
        item.__uid__ = this.currUid;
        this.currUid ++;
      }
    }
    actor.inbox = [];
    actor.items = actor.items.concat(items);
  },

  addActor: function(name_, actor) {
    assert(name_ && actor);
    actor.name_ = name_;
    actor.items = [];
    actor.inbox = [];
    actor.forwardItem  = bind(this, this.addItem);
    actor.forwardItems = bind(this, this.addItems);
    this.actors[name_] = actor;
    if (!actor.process) actor.process = Actor.prototype.process;
    return actor;
  },

  solve: function() {
    dprint('framework', "// Solving " + this.name_ + "...");

    var startTime = Date.now();
    var midTime = startTime;
    var that = this;
    function midComment() {
      var curr = Date.now();
      if (curr - midTime > 500) {
        dprint('framework', '// Working on ' + that.name_ + ', so far ' + ((curr-startTime)/1000).toString().substr(0,10) + ' seconds.');
        midTime = curr;
      }
    }
    function finalComment() {
      dprint('framework', '// Completed ' + that.name_ + ' in ' + ((Date.now() - startTime)/1000).toString().substr(0,10) + ' seconds.');
    }

    var ret = null;
    var finalResult = null;
    this.results = [];
    var finished = false;
    var that = this;
    var actors = values(this.actors); // Assumes actors are not constantly added
    while (!finished) {
      dprint('framework', "Cycle start, items: ");// + values(this.actors).map(function(actor) actor.items).reduce(function(x,y) x+y, 0));
      var hadProcessing = false;
      actors.forEach(function(actor) {
        midComment();

        that.checkInbox(actor);
        if (actor.items.length == 0) return;

        var inputs = actor.items;
        var outputs;
        var currResultCount = that.results.length;
        dprint('framework', 'Processing using ' + actor.name_ + ': ' + inputs.length);
        actor.items = [];
        outputs = actor.process(inputs);
        if (DEBUG_MEMORY) MemoryDebugger.tick('actor ' + actor.name_);
        dprint('framework', 'New results: ' + (outputs.length + that.results.length - currResultCount) + ' out of ' + (that.results.length + outputs.length));
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
            outputs.forEach(function(output) { delete output.tokens }); // clean up tokens to save memory
            that.results = that.results.concat(outputs);
          }
        }
      });
      if (!hadProcessing) {
        if (DEBUG) print("Solving complete: no remaining items");
        finalComment();
        this.results.forEach(function(output) {
          delete output.__uid__; // Might recycle these
        });
        ret =  this.results;
        break;
      }
      if (finalResult) {
        ret = finalResult;
        break;
      }
      midComment();
    }

    // Clear the actors. Do not forget about the actors, though, to make this substrate reusable.
    actors.forEach(function(actor) {
      actor.items = null;
      actor.inbox = null;
    });

    return ret;
  }
};

// Global access to the currently-being processed item.
// Note that if you overload process in Actor, this will need to be set if you rely on it.
var Framework = {
  currItem: null
};

Actor = function() { };
Actor.prototype = {
  process: function(items) {
    var ret = [];
    for (var i = 0; i < items.length; i++) {
      var item = items[i];
      items[i] = null; // items may be very very large. Allow GCing to occur in the loop by releasing refs here
      dprint('frameworkLines', 'Processing item for llvm line ' + item.lineNum);
      Framework.currItem = item;
      var outputs = this.processItem(item);
      Framework.currItem = null;
      if (outputs) {
        ret = ret.concat(outputs);
      }
    }
    return ret;
  }
};

