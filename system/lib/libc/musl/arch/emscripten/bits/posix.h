#ifdef __wasm64__
// 64-bit wide pointers under wasm64
#define _POSIX_V6_LP64_OFF64  1
#define _POSIX_V7_LP64_OFF64  1
#else
// 32-bit wide pointers under wasm32
#define _POSIX_V6_ILP32_OFFBIG  1
#define _POSIX_V7_ILP32_OFFBIG  1
#endif
