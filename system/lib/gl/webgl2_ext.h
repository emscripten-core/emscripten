#pragma once

#include "webgl2.h"

// 33. https://www.khronos.org/registry/webgl/extensions/EXT_disjoint_timer_query_webgl2/
#define GL_QUERY_COUNTER_BITS_EXT 0x8864
#define GL_TIME_ELAPSED_EXT 0x88BF
#define GL_TIMESTAMP_EXT 0x8E28
#define GL_GPU_DISJOINT_EXT 0x8FBB
void emscripten_webgl2_queryCounterEXT(GLuint query, GLenum target);
