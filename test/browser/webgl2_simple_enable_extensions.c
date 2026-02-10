// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include <webgl/webgl1.h>

#ifndef WEBGL_CONTEXT_VERSION
#error Build with -DWEBGL_CONTEXT_VERSION=1 or -DWEBGL_CONTEXT_VERSION=2!
#endif

#ifndef WEBGL_SIMPLE_ENABLE_EXTENSION
#error Build with -DWEBGL_SIMPLE_ENABLE_EXTENSION=1 or -DWEBGL_SIMPLE_ENABLE_EXTENSION=0!
#endif

int hasext(const char *exts, const char *ext) // from cube2, zlib licensed
{
    int len = strlen(ext);
    if(len) for(const char *cur = exts; (cur = strstr(cur, ext)); cur += len)
    {
        if((cur == exts || cur[-1] == ' ') && (cur[len] == ' ' || !cur[len])) return 1;
    }
    return 0;
}

int main()
{
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
  EMSCRIPTEN_RESULT res;
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = WEBGL_CONTEXT_VERSION;
  context = emscripten_webgl_create_context("#canvas", &attrs);
  assert(context > 0); // Must have received a valid context.
  res = emscripten_webgl_make_context_current(context);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);

  const char *exts = (const char *)glGetString(GL_EXTENSIONS);

  if (hasext(exts, "ANGLE_instanced_arrays"))
    assert(emscripten_webgl_enable_ANGLE_instanced_arrays(context));

  if (hasext(exts, "OES_vertex_array_object"))
    assert(emscripten_webgl_enable_OES_vertex_array_object(context));

  if (hasext(exts, "WEBGL_draw_buffers"))
    assert(emscripten_webgl_enable_WEBGL_draw_buffers(context));

  if (hasext(exts, "WEBGL_draw_instanced_base_vertex_base_instance"))
    assert(emscripten_webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(context));

  if (hasext(exts, "WEBGL_multi_draw"))
    assert(emscripten_webgl_enable_WEBGL_multi_draw(context));

  if (hasext(exts, "WEBGL_multi_draw_instanced_base_vertex_base_instance"))
    assert(emscripten_webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(context));

  if (hasext(exts, "EXT_polygon_offset_clamp"))
    assert(emscripten_webgl_enable_EXT_polygon_offset_clamp(context));

  if (hasext(exts, "EXT_clip_control"))
    assert(emscripten_webgl_enable_EXT_clip_control(context));

  if (hasext(exts, "WEBGL_polygon_mode"))
    assert(emscripten_webgl_enable_WEBGL_polygon_mode(context));

#if WEBGL_SIMPLE_ENABLE_EXTENSION
  assert(hasext(exts, "ANGLE_instanced_arrays") == emscripten_webgl_enable_extension(context, "ANGLE_instanced_arrays"));
  assert(hasext(exts, "OES_vertex_array_object") == emscripten_webgl_enable_extension(context, "OES_vertex_array_object"));
  assert(hasext(exts, "WEBGL_draw_buffers") == emscripten_webgl_enable_extension(context, "WEBGL_draw_buffers"));
  assert(hasext(exts, "WEBGL_draw_instanced_base_vertex_base_instance") == emscripten_webgl_enable_extension(context, "WEBGL_draw_instanced_base_vertex_base_instance"));
  assert(hasext(exts, "WEBGL_multi_draw") == emscripten_webgl_enable_extension(context, "WEBGL_multi_draw"));
  assert(hasext(exts, "WEBGL_multi_draw_instanced_base_vertex_base_instance") == emscripten_webgl_enable_extension(context, "WEBGL_multi_draw_instanced_base_vertex_base_instance"));
#endif

  return 0;
}
