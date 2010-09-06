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
    var str = String_copy(pstr);
    var set = String_copy(pset);
    var i = 0;
    while (set.indexOf(str[i]) != -1) i++; // Must halt, as 0 is in both
    return i;
  },
};

// Aliases
Snippets.__cxa_atexit = Snippets.atexit;

