#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifndef __cplusplus

// These work for both C and javascript.
// In C !!0 ==> 0 and in javascript !!0 ==> false
// In C !!1 ==> 1 and in javascript !!1 ==> true
#define true (!!1)
#define false (!!0)
#define bool _Bool

#endif

#define __bool_true_false_are_defined 1

#endif
