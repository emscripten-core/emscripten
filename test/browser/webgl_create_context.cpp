// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

GLint GetInt(GLenum param) {
  GLint value;
  glGetIntegerv(param, &value);
  return value;
}

void final(void*) {
  emscripten_force_exit(0);
}

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

void loop() {
  EMSCRIPTEN_RESULT res;
  if (!context) {
    // new rendering frame started without a context
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    EM_ASM(
      var canvas2 = Module['canvas'].cloneNode(true);
      Module['canvas'].parentElement.appendChild(canvas2);
      Module['canvas'] = canvas2;
    );
    assert(emscripten_webgl_get_current_context() == 0);
    context = emscripten_webgl_create_context("#canvas", &attrs);

    assert(context > 0); // Must have received a valid context.
    res = emscripten_webgl_make_context_current(context);
    assert(res == EMSCRIPTEN_RESULT_SUCCESS);
    assert(emscripten_webgl_get_current_context() == context);
  } else {
    res = emscripten_webgl_destroy_context(context);
    assert(res == EMSCRIPTEN_RESULT_SUCCESS);
    assert(emscripten_webgl_get_current_context() == 0);

    emscripten_cancel_main_loop();
    emscripten_async_call(final, (void*)0, 10);
  }
}

int main() {
  bool first = true;
  EmscriptenWebGLContextAttributes attrs;
  int depth = 0;
  int stencil = 0;
  int antialias = 0;
#ifndef NO_DEPTH
  for(depth = 0; depth <= 1; ++depth)
#endif
#ifndef NO_STENCIL
  for(stencil = 0; stencil <= 1; ++stencil)
#endif
#ifndef NO_ANTIALIAS
  for(antialias = 0; antialias <= 1; ++antialias)
#endif
  {
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.depth = depth;
    attrs.stencil = stencil;
    attrs.antialias = antialias;
    printf("Requesting depth: %d, stencil: %d, antialias: %d\n", depth, stencil, antialias);

    EM_ASM(
      var canvas2 = document.createElement('canvas');
      Module['canvas'].parentElement.appendChild(canvas2);
      canvas2.id = 'customCanvas';
    );

    assert(emscripten_webgl_get_current_context() == 0);
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#customCanvas", &attrs);
    assert(context > 0); // Must have received a valid context.
    EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context);
    assert(res == EMSCRIPTEN_RESULT_SUCCESS);
    assert(emscripten_webgl_get_current_context() == context);

    // Let's try enabling all extensions.
    const char *extensions = (const char*)glGetString(GL_EXTENSIONS);
    std::vector<std::string> exts = split(extensions, ' ');
    for(size_t i = 0; i < exts.size(); ++i)
    {
      bool supported = emscripten_webgl_enable_extension(context, exts[i].c_str());
      printf("%s\n", exts[i].c_str());
      assert(supported);
    }

    int drawingBufferWidth = -1;
    int drawingBufferHeight = -1;
    res = emscripten_webgl_get_drawing_buffer_size(context, &drawingBufferWidth, &drawingBufferHeight);
    assert(res == EMSCRIPTEN_RESULT_SUCCESS);
    printf("drawingBufferWidth x Height: %dx%d\n", drawingBufferWidth, drawingBufferHeight);
    assert(drawingBufferWidth == 300);
    assert(drawingBufferHeight == 150);

    // Try with a simple glClear() that we got a context.
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    unsigned char pixels[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    assert(pixels[0] == 0xFF);
    assert(pixels[1] == 0);
    assert(pixels[2] == 0);
    assert(pixels[3] == 0xFF);

    int numDepthBits = GetInt(GL_DEPTH_BITS);
    int numStencilBits = GetInt(GL_STENCIL_BITS);
    int numSamples = GetInt(GL_SAMPLES);
    printf("RGBA: %d%d%d%d, Depth: %d, Stencil: %d, Samples: %d\n",
      GetInt(GL_RED_BITS), GetInt(GL_GREEN_BITS), GetInt(GL_BLUE_BITS), GetInt(GL_ALPHA_BITS),
      numDepthBits, numStencilBits, numSamples);

    if (!depth && stencil && numDepthBits && numStencilBits && EM_ASM_INT(navigator.userAgent.toLowerCase().indexOf('firefox')) > -1)
    {
      numDepthBits = 0;
      printf("Applying workaround to ignore Firefox bug https://bugzilla.mozilla.org/show_bug.cgi?id=982477\n");
    }
    assert(!!numDepthBits == !!depth);
    assert(!!numStencilBits == !!stencil);
    assert(!!numSamples == !!antialias);
    printf("\n");

    // Test bug https://github.com/emscripten-core/emscripten/issues/1330:
    unsigned vb;
    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    unsigned vb2;
    glGenBuffers(1, &vb2);
    glBindBuffer(GL_ARRAY_BUFFER, vb2);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    int vb3;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vb3);
    if (vb != vb3) printf("Index 0: Generated VB: %d, read back VB: %d\n", vb, vb3);
    assert(vb == vb3);

    int vb4;
    glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vb4);
    if (vb2 != vb4) printf("Index 1: Generated VB: %d, read back VB: %d\n", vb2, vb4);
    assert(vb2 == vb4);

    // Test bug https://github.com/emscripten-core/emscripten/issues/7472:
    GLint enabled = 0;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    assert(enabled == 0);

    // Test that deleting the context works.
    res = emscripten_webgl_destroy_context(context);
    assert(res == 0);
    assert(emscripten_webgl_get_current_context() == 0);

    EM_ASM(
      var canvas2 = document.getElementById('customCanvas');
      canvas2.parentElement.removeChild(canvas2);
    );
  }

  // result will be reported when mainLoop completes
  emscripten_set_main_loop(loop, 0, 0);
  return 99;
}
