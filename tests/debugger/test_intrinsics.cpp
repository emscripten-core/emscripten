#include <vector>
#include <set>
#include <map>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emscripten.h>


void test_1() {
  EM_ASM({
    Module['cyberdwarf_test_data'] = {
      "i": 0
    };

    Module['cyberdwarf_debug_value'] = function(value, type_id, offset, dwarf_md, name) {
      var decoded = Module["cyberdwarf"].decode_var_by_type_name(value, type_id, 6, dwarf_md);
      if (name === "i") {
        test_assert("i loop value", decoded["int"] == Module['cyberdwarf_test_data']["i"]);
        Module['cyberdwarf_test_data']["i"]++;
      } else if (name === "j") {
        test_assert("j inner value", decoded["int volatile"] == (Module['cyberdwarf_test_data']["i"] - 1) * 2);
      }
    };
    Module['cyberdwarf_debug_constant'] = Module['cyberdwarf_debug_value'];
  });

  for (int i = 0; i < 100; i++) {
    // Volatile to prevent code removal
    volatile int j = i * 2;
  }

}

int main(int argc, char *argv[]) {
  EM_ASM(init_cd_test("test_intrinsics"));
  test_1();
}
