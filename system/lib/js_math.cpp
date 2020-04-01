#include <emscripten.h>
#include <math.h>
#include <stdlib.h>

#define CALL_JS_1(cname, jsname, type, casttype) \
  EM_JS(type, JS_##cname, (type x), { return jsname(x) }); \
  type cname(type x) { return JS_##cname((casttype)x); }

#define CALL_JS_1_TRIPLE(cname, jsname) \
  CALL_JS_1(cname, jsname, double, double) \
  CALL_JS_1(cname##f, jsname, float, float) \
  CALL_JS_1(cname##l, jsname, long double, double)

#define CALL_JS_2(cname, jsname, type, casttype) \
  EM_JS(type, JS_##cname, (type x, type y), { return jsname(x, y) }); \
  type cname(type x, type y) { return JS_##cname((casttype)x, (casttype)y); }

#define CALL_JS_2_TRIPLE(cname, jsname) \
  CALL_JS_2(cname, jsname, double, double) \
  CALL_JS_2(cname##f, jsname, float, float) \
  CALL_JS_2(cname##l, jsname, long double, double)

CALL_JS_1_TRIPLE(cos, Math.cos)
CALL_JS_1_TRIPLE(sin, Math.sin)
CALL_JS_1_TRIPLE(tan, Math.tan)
CALL_JS_1_TRIPLE(acos, Math.acos)
CALL_JS_1_TRIPLE(asin, Math.asin)
CALL_JS_1_TRIPLE(atan, Math.atan)
CALL_JS_2_TRIPLE(atan2, Math.atan2)
CALL_JS_1_TRIPLE(exp, Math.exp)
CALL_JS_1_TRIPLE(log, Math.log)
CALL_JS_1_TRIPLE(sqrt, Math.sqrt)
CALL_JS_1_TRIPLE(fabs, Math.abs)
CALL_JS_1_TRIPLE(ceil, Math.ceil)
CALL_JS_1_TRIPLE(floor, Math.floor)
CALL_JS_2_TRIPLE(pow, Math.pow)
CALL_JS_1_TRIPLE(round, Math.round)
CALL_JS_1_TRIPLE(rint, Math.round)
CALL_JS_1_TRIPLE(nearbyint, Math.round)

