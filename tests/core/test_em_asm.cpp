#include <emscripten.h>
#include <stdio.h>

int main() {
  printf("BEGIN\n");
  EM_ASM({ Module['print']("no args works"); });
  printf("  EM_ASM_INT_V returned: %d\n", EM_ASM_INT_V({ Module['print']("no args returning int"); return 12; }));
  printf("  EM_ASM_DOUBLE_V returned: %f\n", EM_ASM_DOUBLE_V({ Module['print']("no args returning double"); return 12.25; }));

#define TEST() \
  FUNC({ Module['print']("  takes ints: " + $0);}, 5); \
  FUNC({ Module['print']("  takes doubles: " + $0);}, 5.0675); \
  FUNC({ Module['print']("  takes strings: " + Pointer_stringify($0)); return 7.75; }, "string arg"); \
  FUNC({ Module['print']("  takes multiple ints: " + $0 + ", " + $1); return 6; }, 5, 7); \
  FUNC({ Module['print']("  mixed arg types: " + $0 + ", " + Pointer_stringify($1) + ", " + $2); return 8.125; }, 3, "hello", 4.75); \
  FUNC({ Module['print']("  ignores unused args"); return 5.5; }, 0);     \
  FUNC({ Module['print']("  skips unused args: " + $1); return 6; }, 5, 7); \
  FUNC({ Module['print']("  " + $0 + " + " + $2); return $0 + $2; }, 5.5, 7.0, 14.375);

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
