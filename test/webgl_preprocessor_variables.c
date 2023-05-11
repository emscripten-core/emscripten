#include <emscripten/html5_webgl.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test(const char *src)
{
  GLuint shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  printf("%s\n", emscripten_webgl_get_shader_info_log_utf8(shader));
  assert(emscripten_webgl_get_shader_parameter_d(shader, GL_COMPILE_STATUS) == 1);
}

int main()
{
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = WEBGL_VERSION;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  emscripten_webgl_make_context_current(context);

  // Test the presence of GL_FRAGMENT_PRECISION_HIGH built-in preprocessor
  test("#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "void main() {}\n"
    "#endif");
  test("#if GL_FRAGMENT_PRECISION_HIGH == 1\n"
    "void main() {}\n"
    "#endif");

  // Test GL_ES built-in preprocessor directive
  test("#ifdef GL_ES\n"
    "void main() {}\n"
    "#endif");
  test("#if GL_ES == 1\n"
    "void main() {}\n"
    "#endif");
  test("#if GL_ES != 1\n" // Also check negation, i.e. that #if is not tautologically true
    "error!\n"
    "#else\n"
    "void main() {}\n"
    "#endif");

  // Test __VERSION__ built-in preprocessor directive.
  // Note that when WebGL 2 contexts are created, shaders
  // still default to #version 100, unless explicit
  // "#version 300 es" is specified.
  test("#ifdef __VERSION__\n"
    "void main() {}\n"
    "#endif");
  test("#if __VERSION__ == 100\n"
    "void main() {}\n"
    "#endif");
  test("#if defined(GL_ES) && __VERSION__ < 300\n"
    "void main() {}\n"
    "#endif");
  test("#if !defined(GL_ES) || __VERSION__ >= 300\n" // for good measure, check via negation
    "error!\n"
    "#else\n"
    "void main() {}\n"
    "#endif");

#if WEBGL_VERSION >= 2
  test("#version 100\n"
    "#if __VERSION__ == 300\n"
    "error!\n"
    "#endif\n"
    "void main() {}");
  test("#version 300 es\n"
    "#if __VERSION__ == 300\n"
    "void main() {}\n"
    "#endif");
#endif

  return 0;
}
