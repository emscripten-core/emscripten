#ifdef __EMSCRIPTEN__
#define siglongjmp(buf, val) longjmp(buf, val)
#endif