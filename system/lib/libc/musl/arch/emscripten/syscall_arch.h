#include <wasi/api.h>
#include <wasi/wasi-helpers.h>
#include <emscripten/syscalls.h>

// Compile as if we can pass uint64 values directly to the host.  Binaryen will
// take care of splitting any i64 params into a pair of i32 values if needed.
#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)
