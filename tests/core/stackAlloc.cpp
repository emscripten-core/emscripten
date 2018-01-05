#include <emscripten.h>

int main() {
  EM_ASM({
    var size = 10;
    var x = stackAlloc(size);
    var y = stackAlloc(size);
    var z = stackAlloc(size);
    assert(x != y && y != z && x != z);
    var direction = y > x ? 1 : -1;
    assert((y - x)*(z - y) > 0); // same direction
    // no overlaps
    function notInRange(value, oneSide, otherSide) {
      assert(!(Math.min(oneSide, otherSide) <= value &&
               value <= Math.max(oneSide, otherSide)));
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

