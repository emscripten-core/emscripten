var Snippets = {
  _vsnprintf: function(dst, num, src, ptr) {
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
};

