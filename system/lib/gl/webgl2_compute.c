#include <emscripten/threading.h>

#include "webgl1.h"
#include "webgl1_ext.h"
#include "webgl2.h"
#include "webgl2_compute.h"

#if defined(__EMSCRIPTEN_PTHREADS__) && defined(__EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__)

// TODO: add threaded implementation of gl*() here

#endif // ~(__EMSCRIPTEN_PTHREADS__ && __EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__)

// Returns a function pointer to the given WebGL 2 Compute extension function, when queried without
// a GL extension suffix such as "EXT", "OES", or "ANGLE". This function is used by
// emscripten_GetProcAddress() to implement legacy GL emulation semantics for portability.
void *_webgl2_compute_match_ext_proc_address_without_suffix(const char *name)
{
  return 0;
}

void *emscripten_webgl2_compute_get_proc_address(const char *name)
{
  // TODO: add RETURN_FN(gl*) here

  // WebGL 2 Compute extensions:
  // (currently none)

  return 0;
}
