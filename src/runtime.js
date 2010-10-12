////////////QUANTUM_SIZE = GUARD_STACK = 1;
// Generates code that can be placed inline in generated code.
// This is not the cleanest way to write this kind of code - it is
// optimized for generating fast inline code.
RuntimeGenerator = {
  alloc: function(size, type) {
    var ret = type + 'TOP';
//    ret += '; for (var i = 0; i < ' + size + '; i++) HEAP[' + type + 'TOP+i] = 0';
    if (GUARD_MEMORY) {
      ret += '; assert(' + size + ' > 0)';
    }
    ret += '; ' + type + 'TOP += ' + size;
    if (QUANTUM_SIZE > 1) {
      ret += ';' + RuntimeGenerator.alignMemory(type + 'TOP', QUANTUM_SIZE);
    }
    return ret;
  },

  // An allocation that lives as long as the current function call
  stackAlloc: function(size) {
    var ret = RuntimeGenerator.alloc(size, 'STACK');
    if (GUARD_MEMORY) {
      ret += '; assert(STACKTOP < STACK_ROOT + STACK_MAX)';
    }
    return ret;
  },

  stackEnter: function(initial) {
    var ret = 'STACK_STACK.push(STACKTOP); STACKTOP += ' + initial;
    if (GUARD_MEMORY) {
      ret += '; assert(STACKTOP < STACK_MAX)';
    }
    return ret;
  },

  stackExit: function() {
    return 'STACKTOP = STACK_STACK.pop();';
  },

  // An allocation that cannot be free'd
  staticAlloc: function(size) {
    return RuntimeGenerator.alloc(size, 'STATIC');
  },

  alignMemory: function(target, quantum) {
    if (typeof quantum !== 'number') {
      quantum = '(quantum ? quantum : QUANTUM_SIZE)';
    }
    return target + ' = Math.ceil(' + target + '/' + quantum + ')*' + quantum + ';';
  },
};

function unInline(name_, params) {
  var src = '(function ' + name_ + '(' + params + ') { var ret = ' + RuntimeGenerator[name_].apply(null, params) + '; return ret; })';
  //print('src: ' + src);
  return eval(src);
}

// Uses the RuntimeGenerator during compilation, in order to
//  1. Let the compiler access and run those functions during compilation
//  2. We expose the entire Runtime object to generated code, so it can
//     use that functionality in a non-inline manner.
Runtime = {
  stackAlloc: unInline('stackAlloc', ['size']),
  staticAlloc: unInline('staticAlloc', ['size']),
  alignMemory: unInline('alignMemory', ['size', 'quantum']),
};

function getRuntime() {
  var ret = '';
  for (i in Runtime) {
    ret += Runtime[i].toString() + '\n';
  }
  return ret + '\n';
}

