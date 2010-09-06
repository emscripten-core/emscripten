var Snippets = {
  vsnprintf: function(dst, num, src, ptr) {
    var args = [];
    while (HEAP[ptr] != 0) {
      args.push(HEAP[ptr]);
      ptr ++;
    }
    var text = __formatString.apply(null, [src].concat(args));
    for (var i = 0; i < num; i++) {
      HEAP[dst+i] = HEAP[text+i];
      if (HEAP[dst+i] == 0) break;
    }
  },

  atexit: function(func) {
    __ATEXIT__.push(func);
  },

  strspn: function(pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset);
    var i = 0;
    while (set.indexOf(str[i]) != -1) i++; // Must halt, as 0 is in str but not set
    return i;
  },

  strcspn: function(pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset, true);
    var i = 0;
    while (set.indexOf(str[i]) == -1) i++; // Must halt, as 0 is in both
    return i;
  },

  strncpy: function(pdest, psrc, num) {
    var padding = false;
    for (var i = 0; i < num; i++) {
      HEAP[pdest+i] = padding ? 0 : HEAP[psrc+i];
      padding = padding || HEAP[psrc+i] == 0;
    }
  },

  strcmp: function(px, py) {
    var i = 0;
    while (true) {
      var x = HEAP[px+i];
      var y = HEAP[py+i];
      if (x == y && x == 0) return 0;
      if (x == 0) return -1;
      if (y == 0) return 1;
      if (x == y) {
        i ++;
        continue;
      } else {
        return x > y ? 1 : -1;
      }
    }
  },

  __assert_fail: function(condition, file, line) {
    throw 'Assertion failed: ' + Pointer_stringify(condition);//JSON.stringify(arguments)//condition;
  },
};

// Aliases
Snippets.__cxa_atexit = Snippets.atexit;

