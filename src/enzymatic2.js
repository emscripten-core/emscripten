/**
 * A 2nd implementation of an 'enzymatic programming' paradigm.
 */

Zyme2 = function(reqs) {
  //! List of items that we want. Each element in this list is
  //! a list of features, we want elements that have them all.
  //! We will only call this Zyme if we find *all* of the items
  //! it wants.
  this.reqs = reqs;
};

Substrate2 = function(name_) {
  this.name_ = name_;
  this.elems = {};
  this.currUid = 1;
};

Substrate2.prototype = {
  _addElement: function(elem) {
    elem.__uid__ = this.currUid;
    this.elems[elem.__uid__] = elem;
    this.currUid ++;
  },

  _removeElement: function(elem) {
    assertTrue(!!this.elems[elem.__uid__]);
    this.elems[elem.__uid__] = null;
    delete elem.__uid__;
  },

  _getElement: function(uid) { return this.elems[uid]; },

  getItems: function() {
    return values(this.elems).filter(function(elem) { return elem.isItem });
  },

  getZymes: function() {
    return values(this.elems).filter(function(elem) { return elem.isZyme });
  },

  addItem: function(item) {
    this._addElement(item);
    item.isItem = true;

    this.getZymes().forEach(function(zyme) { this.noticeItem(zyme, item) });
  },

  addZyme: function(zyme) {
    this._addElement(zyme);
    zyme.isZyme = true;
    zyme.potentials = zyme.reqs.map(function() { return [] }); // for each required item, a list of matching items

    this.getItems().forEach(function(item) { this.noticeItem(zyme, item); });
  },

  removeItem: function(item) {
    this.getZymes().forEach(function(zyme) { this.forgetItem(zyme, item) });

    this._removeElement(item);
    delete item.__result__;
    delete item.__finalResult__;
  },

  removeZyme: function(zyme) {
    this._removeElement(zyme);
  },

  _getFits: function(zyme, item) {
    return fits = zyme.reqs.map(function(req) {
      return req.filter(function(feat) { return !item[feat] }).length == 0;
    });
  },

  noticeItem: function(zyme, item) {
    loopOn(this._getFits(zyme, item), function(i, fit) {
      if (fit) {
        zyme.potentials[i].push(item);
      }
    });
  },

  forgetItem: function(zyme, item) {
    loopOn(this._getFits(zyme, item), function(i, fit) {
      if (fit) {
        zyme.potentials = zyme.potentials.filter(function(item2) { return item2 !== item });
      }
    });
  },

  processAction: function(zyme, items) {
    items.forEach(this.removeItem, this);
    var splat = splitter(zyme.process(items), function(item) {
      return item.__result__ || item.__finalResult__;
    });
    splat.leftIn.forEach(this.addItem, this);
    return splat.splitOut;
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

    var results = [];
    var done = false;
    while (!done) {
      var hadProcessing = false;
      this.getZymes().forEach(function(zyme) {
        while (!done) {
          midComment();
          var selected = zyme.potentials.map(function(potential) { return potential[0] });
          if (sumTruthy(selected) == zyme.reqs.length) {
            var outputs = this.processAction(zyme, selected);
            hadProcessing = true;
            if (outputs.length === 1 && outputs[0].__finalResult__) {
              results = outputs;
              done = true;
            } else {
              results = results.concat(outputs);
            }
          } else {
            break;
          }
        }
      }, this);
      if (this.items.length === 0) {
        finalComment();
        done = true;
      }
      if (!hadProcessing) {
        print("Reached a dead end.");
        this.getItems().forEach(function(item) {
          print("remaining item:" + dump(item));
        });
        throw "failure";
      }
      midComment();
    }
    if (results[0].__finalResult__) {
      return results[0];
    } else {
      return results;
    }
  },
};

