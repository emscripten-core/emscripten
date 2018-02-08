#ifndef __em_js_h__
#define __em_js_h__

#ifdef __cplusplus
#define _EM_JS_CPP_BEGIN extern "C" {
#define _EM_JS_CPP_END   }
#else // __cplusplus
#define _EM_JS_CPP_BEGIN
#define _EM_JS_CPP_END
#endif // __cplusplus

#define EM_JS(ret, name, params, ...)          \
  _EM_JS_CPP_BEGIN                             \
  ret name params;                             \
  __attribute__((used, visibility("default"))) \
  const char* __em_js__##name() {              \
    return #params "<::>" #__VA_ARGS__;        \
  }                                            \
  _EM_JS_CPP_END

#endif // __em_js_h__
