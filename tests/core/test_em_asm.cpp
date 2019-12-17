// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

int main() {
  printf("BEGIN\n");
  EM_ASM({ out("no args works"); });

  // The following two lines are deprecated, test them still.
  printf("  EM_ASM_INT_V returned: %d\n", EM_ASM_INT_V({ out("no args returning int"); return 12; }));
  printf("  EM_ASM_DOUBLE_V returned: %f\n", EM_ASM_DOUBLE_V({ out("no args returning double"); return 12.25; }));

  printf("  EM_ASM_INT returned: %d\n", EM_ASM_INT({ out("no args returning int"); return 12; }));
  printf("  EM_ASM_DOUBLE returned: %f\n", EM_ASM_DOUBLE({ out("no args returning double"); return 12.25; }));

#define TEST() \
  FUNC({ out("  takes ints: " + $0);}, 5); \
  FUNC({ out("  takes doubles: " + $0);}, 5.0675); \
  FUNC({ out("  takes strings: " + UTF8ToString($0)); return 7.75; }, "string arg"); \
  FUNC({ out("  takes multiple ints: " + $0 + ", " + $1); return 6; }, 5, 7); \
  FUNC({ out("  mixed arg types: " + $0 + ", " + UTF8ToString($1) + ", " + $2); return 8.125; }, 3, "hello", 4.75); \
  FUNC({ out("  ignores unused args"); return 5.5; }, 0);     \
  FUNC({ out("  skips unused args: " + $1); return 6; }, 5, 7); \
  FUNC({ out("  " + $0 + " + " + $2); return $0 + $2; }, 5.5, 7.0, 14.375);

#define FUNC_WITH(macro, format, ...) printf("    returned: " format "\n", macro(__VA_ARGS__));
#define FUNC(...) FUNC_WITH(EM_ASM_, "%d", __VA_ARGS__)
  printf("EM_ASM_ :\n");
  TEST()
#undef FUNC

#define FUNC(...) FUNC_WITH(EM_ASM_INT, "%d", __VA_ARGS__)
  printf("EM_ASM_INT :\n");
  TEST()
#undef FUNC

#define FUNC(...) FUNC_WITH(EM_ASM_ARGS, "%d", __VA_ARGS__)
  printf("EM_ASM_ARGS :\n");
  TEST()
#undef FUNC

#define FUNC(...) FUNC_WITH(EM_ASM_DOUBLE, "%f", __VA_ARGS__)
  printf("EM_ASM_DOUBLE :\n");
  TEST()
#undef FUNC

  printf("END\n");
  return 0;
}
