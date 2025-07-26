#ifdef __EMSCRIPTEN__
#define sigsetjmp(buf, x) setjmp((buf))
#endif