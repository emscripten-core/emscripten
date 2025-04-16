#include <emscripten/html5_webgl.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

GLuint compile_shader(GLenum shaderType, const char *src) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  return shader;
}

GLuint create_program(GLuint vertexShader, GLuint fragmentShader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glBindAttribLocation(program, 0, "apos");
  glBindAttribLocation(program, 1, "acolor");
  glLinkProgram(program);
  return program;
}

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  attrs.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK;
  attrs.renderViaOffscreenBackBuffer = true;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  emscripten_webgl_make_context_current(context);

  // Test emscripten_webgl_get_supported_extensions() API
  char *extensions = emscripten_webgl_get_supported_extensions();
  assert(extensions);
  assert(strlen(extensions) > 0);
  assert(strstr(extensions, "WEBGL") != 0);
  free(extensions);

  // Test emscripten_webgl_get_parameter_d() API
  assert(emscripten_webgl_get_parameter_d(GL_BLUE_BITS) == 8);

  // Test emscripten_webgl_get_parameter_o() API
  GLuint buf;
  glGenBuffers(1, &buf);
  assert(buf != 0);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  assert(emscripten_webgl_get_parameter_o(GL_ARRAY_BUFFER_BINDING) == buf);

  // Test emscripten_webgl_get_parameter_utf8() API
  char *version = emscripten_webgl_get_parameter_utf8(GL_VERSION);
  assert(version);
  assert(strstr(version, "WebGL") != 0);
  free(version);

  // Test emscripten_webgl_get_parameter_i64v API
  int64_t components = 0;
  emscripten_webgl_get_parameter_i64v(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, &components);
  assert(components > 0);

  // Test emscripten_webgl_get_parameter_v API with truncated read length
  glViewport(1, 2, 3, 4);
  int viewport[4] = { 0, 0, 0, 0 };
  int numElements = emscripten_webgl_get_parameter_v(GL_VIEWPORT, viewport, 3, EMSCRIPTEN_WEBGL_PARAM_TYPE_INT);
  assert(numElements == 4);
  assert(viewport[0] == 1);
  assert(viewport[1] == 2);
  assert(viewport[2] == 3);
  assert(viewport[3] == 0);

  // Test emscripten_webgl_get_parameter_v API with full read length
  numElements = emscripten_webgl_get_parameter_v(GL_VIEWPORT, viewport, 4, EMSCRIPTEN_WEBGL_PARAM_TYPE_INT);
  assert(numElements == 4);
  assert(viewport[0] == 1);
  assert(viewport[1] == 2);
  assert(viewport[2] == 3);
  assert(viewport[3] == 4);

  // Test emscripten_webgl_get_vertex_attrib_d() API
  glEnableVertexAttribArray(0);
  double enabled = emscripten_webgl_get_vertex_attrib_d(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED);
  assert(enabled == 1.0);

  // Test emscripten_webgl_get_vertex_attrib_v() API with truncated read length
  glVertexAttrib4f(0, 0.2f, 0.4f, 0.6f, 0.8f);
  float data[4] = { 0, 0, 0, 0 };
  numElements = emscripten_webgl_get_vertex_attrib_v(0, GL_CURRENT_VERTEX_ATTRIB, data, 2, EMSCRIPTEN_WEBGL_PARAM_TYPE_FLOAT);
  assert(numElements == 4);
  assert(data[0] == 0.2f);
  assert(data[1] == 0.4f);
  assert(data[2] == 0.0f);
  assert(data[3] == 0.0f);

  // Test emscripten_webgl_get_vertex_attrib_v() API with full read length
  numElements = emscripten_webgl_get_vertex_attrib_v(0, GL_CURRENT_VERTEX_ATTRIB, data, 4, EMSCRIPTEN_WEBGL_PARAM_TYPE_FLOAT);
  assert(numElements == 4);
  assert(data[0] == 0.2f);
  assert(data[1] == 0.4f);
  assert(data[2] == 0.6f);
  assert(data[3] == 0.8f);

  // Test emscripten_webgl_get_vertex_attrib_o() API
  glBufferData(GL_ARRAY_BUFFER, 4, data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
  assert(emscripten_webgl_get_vertex_attrib_o(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING) == buf);

  static const char vertex_shader[] =
    "#version 100\n"
    "attribute vec4 apos;"
    "attribute vec4 acolor;"
    "uniform float vsUniformFloat;"
    "varying vec4 color;"
    "void main() {"
      "color = acolor * vsUniformFloat;"
      "gl_Position = apos;"
    "}";
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);

  // Test emscripten_webgl_get_shader_parameter_d() API
  assert(emscripten_webgl_get_shader_parameter_d(vs, GL_DELETE_STATUS) == 0);
  assert(emscripten_webgl_get_shader_parameter_d(vs, GL_COMPILE_STATUS) == 1);
  assert(emscripten_webgl_get_shader_parameter_d(vs, GL_SHADER_TYPE) == GL_VERTEX_SHADER);

  // Test emscripten_webgl_get_shader_source_utf8() API
  char *src = emscripten_webgl_get_shader_source_utf8(vs);
  assert(src);
  assert(strstr(src, "attribute") != 0);
  free(src);

  static const char failing_shader[] =
    "#version 100\n"
    "attribute vec4 apos;"
    "attr_error_ibute vec4 acolor;"
    "varying vec4 color;"
    "void main() {"
      "color = acolor;"
      "gl_Position = apos;"
    "}";
  GLuint vs2 = compile_shader(GL_VERTEX_SHADER, failing_shader);

  // Test emscripten_webgl_get_shader_info_log_utf8() API
  src = emscripten_webgl_get_shader_info_log_utf8(vs2);
  assert(src);
  assert(strstr(src, "attr_error_ibute") != 0);
  free(src);

  static const char fragment_shader[] =
    "#version 100\n"
    "precision lowp float;"
    "varying vec4 color;"
    "uniform vec4 psUniformVec;"
    "void main() {"
      "gl_FragColor = color + psUniformVec;"
    "}";
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

  // Test emscripten_webgl_get_shader_parameter_d() API
  assert(emscripten_webgl_get_shader_parameter_d(fs, GL_DELETE_STATUS) == 0);
  assert(emscripten_webgl_get_shader_parameter_d(fs, GL_COMPILE_STATUS) == 1);
  assert(emscripten_webgl_get_shader_parameter_d(fs, GL_SHADER_TYPE) == GL_FRAGMENT_SHADER);

  // Test emscripten_webgl_get_shader_source_utf8() API
  src = emscripten_webgl_get_shader_source_utf8(fs);
  assert(src);
  assert(strstr(src, "varying") != 0);
  free(src);

  GLuint program = create_program(vs, fs);

  // Test emscripten_webgl_get_program_parameter_d() API
  assert(emscripten_webgl_get_program_parameter_d(program, GL_ACTIVE_ATTRIBUTES) == 2);

  // Test emscripten_webgl_get_program_info_log_utf8() API
  GLuint program2 = create_program(vs2, fs);
  char *programLog = emscripten_webgl_get_program_info_log_utf8(program2);
  assert(programLog);
  // Firefox says "Must have a compiled vertex shader attached."
  // Chrome says "invalid shaders"
  // -> test that word "shader" is present
  assert(strstr(programLog, "shader") != 0);
  free(programLog);

  glUseProgram(program);
  glUniform1f(glGetUniformLocation(program, "vsUniformFloat"), 2.f);
  glUniform4f(glGetUniformLocation(program, "psUniformVec"), 1.f, 2.f, 3.f, 4.f);

  // Test emscripten_webgl_get_uniform_d() API
  double vsUniformFloat = emscripten_webgl_get_uniform_d(program, glGetUniformLocation(program, "vsUniformFloat"));
  assert(vsUniformFloat == 2.f);

  float psUniformVec[4] = { 0, 0, 0, 0 };

  // Test emscripten_webgl_get_uniform_v() API truncated read
  numElements = emscripten_webgl_get_uniform_v(program, glGetUniformLocation(program, "psUniformVec"), psUniformVec, 3, EMSCRIPTEN_WEBGL_PARAM_TYPE_FLOAT);
  assert(numElements == 4);
  assert(psUniformVec[0] == 1.0f);
  assert(psUniformVec[1] == 2.0f);
  assert(psUniformVec[2] == 3.0f);
  assert(psUniformVec[3] == 0.0f);

  // Test emscripten_webgl_get_uniform_v() API full read
  numElements = emscripten_webgl_get_uniform_v(program, glGetUniformLocation(program, "psUniformVec"), psUniformVec, 4, EMSCRIPTEN_WEBGL_PARAM_TYPE_FLOAT);
  assert(numElements == 4);
  assert(psUniformVec[0] == 1.0f);
  assert(psUniformVec[1] == 2.0f);
  assert(psUniformVec[2] == 3.0f);
  assert(psUniformVec[3] == 4.0f);

  return 0;
}
