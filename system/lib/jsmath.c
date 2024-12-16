//
// See JS_MATH setting in settings.js for details.
//

#include <emscripten/em_math.h>
#include <emscripten/em_js.h>
#include <math.h>
#include <stdlib.h>

#define CALL_JS_1(cname, jsname, type) \
  type cname(type x) { return (type)emscripten_math_##jsname(x); }

#define CALL_JS_1_TRIPLE(name) \
  CALL_JS_1(name, name, double) \
  CALL_JS_1(name##f, name, float)

CALL_JS_1_TRIPLE(cos)
CALL_JS_1_TRIPLE(sin)
CALL_JS_1_TRIPLE(tan)
CALL_JS_1_TRIPLE(acos)
CALL_JS_1_TRIPLE(asin)
CALL_JS_1_TRIPLE(atan)
CALL_JS_1_TRIPLE(exp)
CALL_JS_1_TRIPLE(log)
CALL_JS_1_TRIPLE(sqrt)

#define CALL_JS_2(cname, jsname, type) \
  type cname(type x, type y) { return (type)emscripten_math_##jsname(x, y); }

#define CALL_JS_2_TRIPLE(name) \
  CALL_JS_2(name, name, double) \
  CALL_JS_2(name##f, name, float)

CALL_JS_2_TRIPLE(atan2)
CALL_JS_2_TRIPLE(pow)

#define CALL_JS_1_IMPL(cname, type, impl) \
  EM_JS(type, JS_##cname, (type x), impl); \
  type cname(type x) { return JS_##cname(x); }

#define CALL_JS_1_IMPL_TRIPLE(cname, impl) \
  CALL_JS_1_IMPL(cname, double, impl) \
  CALL_JS_1_IMPL(cname##f, float, impl)

CALL_JS_1_IMPL_TRIPLE(round, {
  return x >= 0 ? Math.floor(x + 0.5) : Math.ceil(x - 0.5);
})
