#ifndef  _COMPAT_MATH_H_
#define  _COMPAT_MATH_H_

#ifndef isinff
  #define isinff isinf
#endif

#ifndef isnanf
  #define isnanf isnan
#endif

#include_next <math.h>

#endif /* _COMPAT_MATH_H_ */
