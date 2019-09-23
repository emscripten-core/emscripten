#pragma once

#include <GLES3/gl31.h>

#if USE_WEBGL2_COMPUTE
#if GL_MAX_FEATURE_LEVEL != 30
#error "cannot resolve conflicting USE_WEBGL* settings"
#endif
#endif

// TODO: add declaration of emscripten_gl*() here
