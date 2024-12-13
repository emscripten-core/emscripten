//
// See JS_MATH setting in settings.js for details.
//

#include <emscripten.h>
#include <math.h>
#include <stdlib.h>

#define CALL_JS_1(cname, jsname, type) \
  EM_JS(type, JS_##cname, (type x), { return jsname(x) }); \
  type cname(type x) { return JS_##cname(x); }

#define CALL_JS_1_TRIPLE(cname, jsname) \
  CALL_JS_1(cname, jsname, double) \
  CALL_JS_1(cname##f, jsname, float)

CALL_JS_1_TRIPLE(cos, Math.cos)
CALL_JS_1_TRIPLE(sin, Math.sin)
CALL_JS_1_TRIPLE(tan, Math.tan)
CALL_JS_1_TRIPLE(acos, Math.acos)
CALL_JS_1_TRIPLE(asin, Math.asin)
CALL_JS_1_TRIPLE(atan, Math.atan)
CALL_JS_1_TRIPLE(exp, Math.exp)
CALL_JS_1_TRIPLE(log, Math.log)
CALL_JS_1_TRIPLE(sqrt, Math.sqrt)

#define CALL_JS_2(cname, jsname, type) \
  EM_JS(type, JS_##cname, (type x, type y), { return jsname(x, y) }); \
  type cname(type x, type y) { return JS_##cname(x, y); }

#define CALL_JS_2_TRIPLE(cname, jsname) \
  CALL_JS_2(cname, jsname, double) \
  CALL_JS_2(cname##f, jsname, float)

CALL_JS_2_TRIPLE(atan2, Math.atan2)
CALL_JS_2_TRIPLE(pow, Math.pow)

#define CALL_JS_1_IMPL(cname, type, impl) \
  EM_JS(type, JS_##cname, (type x), impl); \
  type cname(type x) { return JS_##cname(x); }

#define CALL_JS_1_IMPL_TRIPLE(cname, impl) \
  CALL_JS_1_IMPL(cname, double, impl) \
  CALL_JS_1_IMPL(cname##f, float, impl)

CALL_JS_1_IMPL_TRIPLE(round, {
  return x >= 0 ? Math.floor(x + 0.5) : Math.ceil(x - 0.5);
})
CALL_JS_1_IMPL_TRIPLE(rint,  {
  function round(x) {
    return x >= 0 ? Math.floor(x + 0.5) : Math.ceil(x - 0.5);
  }
  return (x - Math.floor(x) != .5) ? round(x) : round(x / 2) * 2;
})
