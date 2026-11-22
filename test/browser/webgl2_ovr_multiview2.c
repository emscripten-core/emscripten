// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Tests the WebGL 2 extension OVR_multiview2, which allows rendering to
// several layers ("views") of a 2D array texture with a single draw call.
// See https://github.com/emscripten-core/emscripten/issues/20173
//
// The extension is not available in all browsers and platforms. If it is not
// supported, the test only verifies that it cannot be enabled.

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <emscripten/html5.h>

// The function glFramebufferTextureMultiviewOVR() is declared both by the
// Khronos GLES extension header, and by the Emscripten WebGL 2 extension
// header. Include both to verify that the two can be used together.
#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <webgl/webgl2_ext.h>

#define WIDTH 4
#define HEIGHT 4
#define NUM_VIEWS 2

static bool has_extension(const char *name) {
  GLint num_extensions = 0;
  glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
  for (GLint i = 0; i < num_extensions; ++i) {
    if (!strcmp((const char *)glGetStringi(GL_EXTENSIONS, i), name)) return true;
  }
  return false;
}

static GLuint compile_shader(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    char log[1024];
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    printf("Shader compile failed: %s\n", log);
  }
  assert(compiled);
  return shader;
}

static GLuint create_array_texture(GLenum internal_format) {
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internal_format, WIDTH, HEIGHT, NUM_VIEWS);
  return texture;
}

static void assert_attachment(GLenum attachment, GLint type, GLint num_views, GLint base_view_index) {
  GLint value = -1;
  glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &value);
  assert(value == type);
  if (type != GL_NONE) {
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR, &value);
    assert(value == num_views);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR, &value);
    assert(value == base_view_index);
  }
  assert(glGetError() == GL_NO_ERROR);
}

// Reads back a pixel from the given layer of the texture. Reading from a
// multiview framebuffer is not allowed, so do this via a regular framebuffer
// that has the layer as its attachment.
static void assert_layer_color(GLuint texture, int layer, int r, int g, int b) {
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, layer);
  assert(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  unsigned char pixel[4] = { 0x12, 0x34, 0x56, 0x78 };
  glReadPixels(WIDTH / 2, HEIGHT / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  printf("Layer %d: %d,%d,%d,%d\n", layer, pixel[0], pixel[1], pixel[2], pixel[3]);
  assert(pixel[0] == r && pixel[1] == g && pixel[2] == b && pixel[3] == 255);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbo);
  assert(glGetError() == GL_NO_ERROR);
}

int main() {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  assert(ctx > 0);
  emscripten_webgl_make_context_current(ctx);

  bool supported = has_extension("GL_OVR_multiview2");
  printf("OVR_multiview2 supported: %d\n", supported);

#if defined(TEST_EXPLICIT_ENABLE)
  // Enable the extension with its dedicated function.
  assert(emscripten_webgl_enable_OVR_multiview2(ctx) == supported);
#elif defined(TEST_SIMPLE_ENABLE)
  // Enable the extension with the generic extension enable function.
  assert(emscripten_webgl_enable_extension(ctx, "OVR_multiview2") == supported);
#else
  // By default the extension is enabled automatically at context creation
  // time, if the browser supports it (-sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS).
#endif

  if (!supported) {
    printf("OVR_multiview2 is not supported, skipping test.\n");
    return 0;
  }

  GLint max_views = 0;
  glGetIntegerv(GL_MAX_VIEWS_OVR, &max_views);
  printf("GL_MAX_VIEWS_OVR: %d\n", max_views);
  assert(max_views >= NUM_VIEWS);
  assert(glGetError() == GL_NO_ERROR);

  GLuint color = create_array_texture(GL_RGBA8);
  GLuint depth = create_array_texture(GL_DEPTH_COMPONENT24);

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color, 0, 0, NUM_VIEWS);
  assert(glGetError() == GL_NO_ERROR);

  // Attach the depth buffer via a function pointer to also test
  // that the function can be looked up dynamically.
  void (*framebufferTextureMultiview)(GLenum, GLenum, GLuint, GLint, GLint, GLsizei) =
    (void (*)(GLenum, GLenum, GLuint, GLint, GLint, GLsizei))emscripten_webgl_get_proc_address("glFramebufferTextureMultiviewOVR");
  assert(framebufferTextureMultiview);
  framebufferTextureMultiview(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth, 0, 0, NUM_VIEWS);
  assert(glGetError() == GL_NO_ERROR);

  assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  assert_attachment(GL_COLOR_ATTACHMENT0, GL_TEXTURE, NUM_VIEWS, 0);
  assert_attachment(GL_DEPTH_ATTACHMENT, GL_TEXTURE, NUM_VIEWS, 0);

  // Attaching the null texture detaches the attachment.
  glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0, NUM_VIEWS);
  assert_attachment(GL_DEPTH_ATTACHMENT, GL_NONE, 0, 0);
  glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth, 0, 0, NUM_VIEWS);
  assert_attachment(GL_DEPTH_ATTACHMENT, GL_TEXTURE, NUM_VIEWS, 0);

  // Invalid parameters are reported as GL errors.
  glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color, 0, 0, max_views + 1);
  assert(glGetError() == GL_INVALID_VALUE);
  assert_attachment(GL_COLOR_ATTACHMENT0, GL_TEXTURE, NUM_VIEWS, 0);
  assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  // Render a fullscreen triangle to both views with a single draw call. The
  // output color depends on the view that is being rendered to.
  GLuint vs = compile_shader(GL_VERTEX_SHADER,
    "#version 300 es\n"
    "#extension GL_OVR_multiview2 : require\n"
    "layout(num_views = 2) in;\n"
    "flat out vec4 color;\n"
    "void main() {\n"
    "  color = (gl_ViewID_OVR == 0u) ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0, 1.0, 0.0, 1.0);\n"
    "  gl_Position = vec4(float((gl_VertexID << 1) & 2) * 2.0 - 1.0, float(gl_VertexID & 2) * 2.0 - 1.0, 0.0, 1.0);\n"
    "}\n");

  GLuint fs = compile_shader(GL_FRAGMENT_SHADER,
    "#version 300 es\n"
    "precision mediump float;\n"
    "flat in vec4 color;\n"
    "out vec4 outColor;\n"
    "void main() { outColor = color; }\n");

  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  GLint linked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  assert(linked);
  glUseProgram(program);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glViewport(0, 0, WIDTH, HEIGHT);
  glClearColor(0.f, 0.f, 1.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  assert(glGetError() == GL_NO_ERROR);

  // Clearing a multiview framebuffer clears all of the views.
  assert_layer_color(color, 0, 0, 0, 255);
  assert_layer_color(color, 1, 0, 0, 255);

  glDrawArrays(GL_TRIANGLES, 0, 3);
  assert(glGetError() == GL_NO_ERROR);

  assert_layer_color(color, 0, 255, 0, 0);
  assert_layer_color(color, 1, 0, 255, 0);

  printf("Test passed!\n");
  return 0;
}
