#ifndef __em_js_h__
#define __em_js_h__

#ifdef __cplusplus
#define _EM_JS_CPP_BEGIN extern "C" {
#define _EM_JS_CPP_END   }
#else // __cplusplus
#define _EM_JS_CPP_BEGIN
#define _EM_JS_CPP_END
#endif // __cplusplus

#define EM_JS(ret, name, params, code)           \
  ret name params;                               \
  _EM_JS_CPP_BEGIN                               \
  __attribute__((used))                          \
  const char* __em_js__##name() {                \
    return #ret " " #name " " #params " " #code; \
  }                                              \
  _EM_JS_CPP_END

#endif // __em_js_h__
