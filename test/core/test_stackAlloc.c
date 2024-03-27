// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>

EM_JS_DEPS(deps, "$stackSave,$stackAlloc");

int main() {
  EM_ASM({
    var size = 128;
    var before;
    before = stackSave();
    var x = stackAlloc(size);
    var y = stackAlloc(size);
    var direction = y > x ? 1 : -1;
    assert(x % 16 == 0, "allocation must have 16-byte alignment");
    assert(x == Math.min(before, before + direction*size), "allocation must return the start of the range allocated");
    var z = stackAlloc(size);
    assert(x != y && y != z && x != z, "allocations must be unique");
    assert((y - x)*(z - y) > 0, "allocations must be in the same direction");
    // no overlaps
    function notInRange(value, begin, end) {
      function errormsg() { return value + " must not be in the range (" + begin + ", " + end + "]"; }
      if (begin < end) assert(!(value >= begin && value < end), errormsg());
      else assert(!(value <= begin && value > end), errormsg());
    }
    notInRange(x, y, y + direction*size);
    notInRange(x, z, z + direction*size);
    notInRange(y, x, x + direction*size);
    notInRange(y, z, z + direction*size);
    notInRange(z, x, x + direction*size);
    notInRange(z, y, y + direction*size);
    out('ok.');
  });
}
