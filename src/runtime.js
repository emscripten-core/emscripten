// Implementation details for the 'runtime environment' we generate in
// JavaScript. The Runtime object itself is used both during compilation,
// and is available at runtime (dynamic compilation). The RuntimeGenerator
// helps to create the Runtime object (written so that the Runtime object
// itself is as optimized as possible - no unneeded runtime checks).

RuntimeGenerator = {
  alloc: function(size, type, init) {
    var ret = type + 'TOP';
    if (ASSERTIONS) {
      ret += '; assert(' + size + ' > 0, "Trying to allocate 0")';
    }
    if (init) {
      ret += '; _memset(' + type + 'TOP, 0, ' + size + ')';
    }
    ret += '; ' + type + 'TOP += ' + size;
    if (QUANTUM_SIZE > 1) {
      ret += ';' + RuntimeGenerator.alignMemory(type + 'TOP', QUANTUM_SIZE);
    }
    return ret;
  },

  // An allocation that lives as long as the current function call
  stackAlloc: function(size) {
    var ret = RuntimeGenerator.alloc(size, 'STACK', INIT_STACK);
    if (ASSERTIONS) {
      ret += '; assert(STACKTOP < STACK_ROOT + STACK_MAX, "Ran out of stack")';
    }
    return ret;
  },

  stackEnter: function(initial) {
    if (initial === 0 && SKIP_STACK_IN_SMALL) return '';
    var ret = 'var __stackBase__  = STACKTOP; STACKTOP += ' + initial;
    if (ASSERTIONS) {
      ret += '; assert(STACKTOP < STACK_MAX)';
    }
    if (INIT_STACK) {
      ret += '; _memset(__stackBase__, 0, ' + initial + ')';
    }
    return ret;
  },

  stackExit: function(initial) {
    if (initial === 0 && SKIP_STACK_IN_SMALL) return '';
    var ret = '';
    if (SAFE_HEAP) {
      ret += 'for (var i = __stackBase__; i < STACKTOP; i++) SAFE_HEAP_CLEAR(i);';
    }
    return ret += 'STACKTOP = __stackBase__';
  },

  // An allocation that cannot be free'd
  staticAlloc: function(size) {
    return RuntimeGenerator.alloc(size, 'STATIC', INIT_HEAP);
  },

  alignMemory: function(target, quantum) {
    if (typeof quantum !== 'number') {
      quantum = '(quantum ? quantum : QUANTUM_SIZE)';
    }
    return target + ' = Math.ceil(' + target + '/' + quantum + ')*' + quantum + ';';
  }
};

function unInline(name_, params) {
  var src = '(function ' + name_ + '(' + params + ') { var ret = ' + RuntimeGenerator[name_].apply(null, params) + '; return ret; })';
  //print('src: ' + src);
  return eval(src);
}

Runtime = {
  stackAlloc: unInline('stackAlloc', ['size']),
  staticAlloc: unInline('staticAlloc', ['size']),
  alignMemory: unInline('alignMemory', ['size', 'quantum']),

  // TODO: cleanup
  isNumberType: function(type) {
    return type in Runtime.INT_TYPES || type in Runtime.FLOAT_TYPES;
  },

  isPointerType: isPointerType,
  isStructType: isStructType,

  INT_TYPES: set('i1', 'i8', 'i16', 'i32', 'i64'),
  FLOAT_TYPES: set('float', 'double'),

  getNativeFieldSize: getNativeFieldSize,
  dedup: dedup,

  set: set,

  // Calculate aligned size, just like C structs should be. TODO: Consider
  // requesting that compilation be done with #pragma pack(push) /n #pragma pack(1),
  // which would remove much of the complexity here.
  calculateStructAlignment: function calculateStructAlignment(type) {
    type.flatSize = 0;
    type.alignSize = 0;
    var diffs = [];
    var prev = -1;
    type.flatIndexes = type.fields.map(function(field) {
      var size, alignSize;
      if (Runtime.isNumberType(field) || Runtime.isPointerType(field)) {
        size = Runtime.getNativeFieldSize(field, true); // pack char; char; in structs, also char[X]s.
        alignSize = size;
      } else if (Runtime.isStructType(field)) {
        size = Types.types[field].flatSize;
        alignSize = Types.types[field].alignSize;
      } else {
        dprint('Unclear type in struct: ' + field + ', in ' + type.name_);
        assert(0);
      }
      alignSize = type.packed ? 1 : Math.min(alignSize, QUANTUM_SIZE);
      type.alignSize = Math.max(type.alignSize, alignSize);
      var curr = Runtime.alignMemory(type.flatSize, alignSize); // if necessary, place this on aligned memory
      type.flatSize = curr + size;
      if (prev >= 0) {
        diffs.push(curr-prev);
      }
      prev = curr;
      return curr;
    });
    type.flatSize = Runtime.alignMemory(type.flatSize, type.alignSize);
    if (diffs.length == 0) {
      type.flatFactor = type.flatSize;
    } else if (Runtime.dedup(diffs).length == 1) {
      type.flatFactor = diffs[0];
    }
    type.needsFlattening = (type.flatFactor != 1);
    return type.flatIndexes;
  },

  // Given details about a structure, returns its alignment. For example,
  // generateStructInfo(
  //    [
  //      ['i32', 'field1'],
  //      ['i8', 'field2']
  //    ]
  // ) will return
  //    { field1: 0, field2: 4 } (depending on QUANTUM_SIZE)
  generateStructInfo: function(struct) {
    var fields = struct.map(function(item) { return item[0] });
    var type = { fields: fields };
    var alignment = Runtime.calculateStructAlignment(type);
    var ret = {
      __size__: type.flatSize
    };
    struct.forEach(function(item, i) {
      ret[item[1]] = alignment[i];
    });
    return ret;
  }
};

function getRuntime() {
  var ret = 'var Runtime = {\n';
  for (i in Runtime) {
    var item = Runtime[i];
    ret += '  ' + i + ': ';
    if (typeof item === 'function') {
      ret += item.toString();
    } else {
      ret += JSON.stringify(item);
    }
    ret += ',\n';
  }
  return ret + '  __dummy__: 0\n}\n';
}

