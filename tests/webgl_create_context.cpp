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
#include <html5.h>

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

GLint GetInt(GLenum param)
{
  GLint value;
  glGetIntegerv(param, &value);
  return value;
}

void final(void*) {
  #ifdef REPORT_RESULT
  int result = 0;
  REPORT_RESULT();
  #endif
}

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

void loop()
{
  EMSCRIPTEN_RESULT res;
  if (!context) {
    // new rendering frame started without a context
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    EM_ASM(
      var canvas2 = Module.canvas.cloneNode();
      Module.canvas.parentElement.appendChild(canvas2);
      Module.canvas = canvas2;
    );
    assert(emscripten_webgl_get_current_context() == 0);
    context = emscripten_webgl_create_context(0, &attrs);

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

int main()
{
  bool first = true;
  EmscriptenWebGLContextAttributes attrs;
  for(int depth = 0; depth <= 1; ++depth)
  for(int stencil = 0; stencil <= 1; ++stencil)
  for(int antialias = 0; antialias <= 1; ++antialias)
  {
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.depth = depth;
    attrs.stencil = stencil;
    attrs.antialias = antialias;
    printf("Requesting depth: %d, stencil: %d, antialias: %d\n", depth, stencil, antialias);

    if (!first)
    {
      EM_ASM(var canvas2 = Module.canvas.cloneNode();
        Module.canvas.parentElement.appendChild(canvas2);
   //   Module.canvas.parentElement.removeChild(canvas);
      Module.canvas = canvas2;
      );
    }
    first = false;
    
    assert(emscripten_webgl_get_current_context() == 0);
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(0, &attrs);
    assert(context > 0); // Must have received a valid context.
    EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context);
    assert(res == EMSCRIPTEN_RESULT_SUCCESS);
    assert(emscripten_webgl_get_current_context() == context);

    // Let's try enabling all extensions.
    const char *extensions = (const char*)glGetString(GL_EXTENSIONS);
    std::vector<std::string> exts = split(extensions, ' ');
    for(size_t i = 0; i < exts.size(); ++i)
    {
      EM_BOOL supported = emscripten_webgl_enable_extension(context, exts[i].c_str());
      assert(supported);
    }

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
    
    if (!depth && stencil && numDepthBits && numStencilBits && EM_ASM_INT_V(navigator.userAgent.toLowerCase().indexOf('firefox')) > -1)
    {
      numDepthBits = 0;
      printf("Applying workaround to ignore Firefox bug https://bugzilla.mozilla.org/show_bug.cgi?id=982477\n");
    }
    assert(!!numDepthBits == !!depth);
    assert(!!numStencilBits == !!stencil);
    assert(!!numSamples == !!antialias);
    printf("\n");

    // Test that deleting the context works.
    res = emscripten_webgl_destroy_context(context);
    assert(res == 0);
    assert(emscripten_webgl_get_current_context() == 0);
  }
  
  // result will be reported when mainLoop completes
  emscripten_set_main_loop(loop, 0, 0);

#ifndef REPORT_RESULT
  return 0;
#endif
}
