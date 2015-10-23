#include <features.h>

#undef assert

#ifdef NDEBUG
#define	assert(x) (void)0
#else
#define assert(x) ((void)((x) || (__assert_fail(#x, __FILE__, __LINE__, __func__),0)))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if __EMSCRIPTEN__
_Noreturn
#endif
void __assert_fail (const char *, const char *, int, const char *);

#ifdef __cplusplus
}
#endif
