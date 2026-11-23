// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Tests WebGL extensions that do not add any new functions, but only new
// enums, renderable formats or GLSL features. For each such extension that the
// browser supports, verifies that the extension is advertised in the
// GL_EXTENSIONS string, and that it was enabled automatically at context
// creation time, i.e. that the feature of the extension works.

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#include <webgl/webgl1_ext.h>
#include <webgl/webgl2_ext.h>

// The full list of extensions that the browser supports.
char *browser_extensions;

bool has_extension(const char *extensions, const char *name) {
  size_t len = strlen(name);
  for (const char *cur = extensions; (cur = strstr(cur, name)); cur += len) {
    if ((cur == extensions || cur[-1] == ' ') && (cur[len] == ' ' || !cur[len])) return true;
  }
  return false;
}

// Returns true if the browser supports the given extension. In that case
// Emscripten must also advertise it.
bool check_extension(const char *name) {
  bool supported = has_extension(browser_extensions, name);
  bool advertised = has_extension((const char *)glGetString(GL_EXTENSIONS), name);
  printf("%s: supported by browser: %d, advertised in GL_EXTENSIONS: %d\n", name, supported, advertised);
  assert(supported == advertised);

  // The indexed query must agree with the GL_EXTENSIONS string.
  GLint num_extensions = 0;
  glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
  bool advertised_indexed = false;
  for (GLint i = 0; i < num_extensions; ++i) {
    if (!strcmp((const char *)glGetStringi(GL_EXTENSIONS, i), name)) advertised_indexed = true;
  }
  assert(advertised_indexed == advertised);
  return supported;
}

void check_tex_parameter(GLenum pname, GLint param) {
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, pname, param);
  assert(glGetError() == GL_NO_ERROR);
  GLint value = 0;
  glGetTexParameteriv(GL_TEXTURE_2D, pname, &value);
  assert(glGetError() == GL_NO_ERROR);
  assert(value == param);
  glDeleteTextures(1, &tex);
}

void check_color_renderable(GLenum internal_format) {
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, 4, 4);
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
  assert(glGetError() == GL_NO_ERROR);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &tex);
}

void check_fragment_shader_compiles(const char *source) {
  GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    char log[1024];
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    printf("Shader compile failed: %s\n", log);
  }
  assert(compiled);
  glDeleteShader(shader);
}

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  assert(context > 0);
  EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);

  browser_extensions = emscripten_webgl_get_supported_extensions();
  assert(browser_extensions);

  if (check_extension("EXT_texture_mirror_clamp_to_edge")) {
    check_tex_parameter(GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE_EXT);
  }

  if (check_extension("WEBGL_stencil_texturing")) {
    check_tex_parameter(GL_DEPTH_STENCIL_TEXTURE_MODE_WEBGL, GL_STENCIL_INDEX_WEBGL);
  }

  if (check_extension("EXT_render_snorm")) {
    check_color_renderable(GL_RGBA8_SNORM);
  }

  if (check_extension("WEBGL_render_shared_exponent")) {
    check_color_renderable(GL_RGB9_E5);
  }

  if (check_extension("OES_sample_variables")) {
    check_fragment_shader_compiles(
      "#version 300 es\n"
      "#extension GL_OES_sample_variables : require\n"
      "precision mediump float;\n"
      "out vec4 color;\n"
      "void main() { color = vec4(float(gl_SampleID)); }\n");
  }

  if (check_extension("OES_shader_multisample_interpolation")) {
    check_fragment_shader_compiles(
      "#version 300 es\n"
      "#extension GL_OES_shader_multisample_interpolation : require\n"
      "precision mediump float;\n"
      "sample in vec4 v;\n"
      "out vec4 color;\n"
      "void main() { color = v; }\n");
    GLfloat min_offset = 0, max_offset = 0;
    glGetFloatv(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES, &min_offset);
    glGetFloatv(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES, &max_offset);
    GLint offset_bits = -1;
    glGetIntegerv(GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES, &offset_bits);
    assert(glGetError() == GL_NO_ERROR);
    assert(min_offset < max_offset);
    assert(offset_bits >= 0);
  }

  if (check_extension("WEBGL_compressed_texture_pvrtc")) {
    GLint num_formats = 0;
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &num_formats);
    assert(num_formats > 0);
    GLint *formats = malloc(num_formats * sizeof(GLint));
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats);
    assert(glGetError() == GL_NO_ERROR);
    bool found = false;
    for (GLint i = 0; i < num_formats; ++i) {
      if (formats[i] == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG) found = true;
    }
    assert(found);
    free(formats);
  }

  assert(glGetError() == GL_NO_ERROR);
  free(browser_extensions);
  printf("Test passed!\n");
  return 0;
}
