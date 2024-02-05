#undef __WORDSIZE
#ifdef __wasm64__
#define __WORDSIZE 64
#else
#define __WORDSIZE 32
#endif
