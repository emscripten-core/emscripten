#include <emscripten.h>

int main() {
  EM_ASM({
    var size = 100;
    var before;
    before = stackSave();
    var x = stackAlloc(size);
    var y = stackAlloc(size);
    var direction = y > x ? 1 : -1;
    assert(x == Math.min(before, before + direction*size), "allocation must return the start of the range allocated");
    var z = stackAlloc(size);
    assert(x != y && y != z && x != z, "allocations must be unique");
    assert((y - x)*(z - y) > 0, "allocations must be in the same direction");
    // no overlaps
    function notInRange(value, oneSide, otherSide) {
      assert(!(Math.min(oneSide, otherSide) <= value &&
               value <= Math.max(oneSide, otherSide)), value + " must not be in the range " + [oneSide, otherSide]);
    }
    notInRange(x, y, y + direction*size);
    notInRange(x, z, z + direction*size);
    notInRange(y, x, x + direction*size);
    notInRange(y, z, z + direction*size);
    notInRange(z, x, x + direction*size);
    notInRange(z, y, y + direction*size);
    Module['print']('ok.');
  });
}

