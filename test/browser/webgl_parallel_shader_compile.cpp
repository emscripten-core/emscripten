/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <webgl/webgl1.h>
#include <webgl/webgl1_ext.h>
#include <vector>

#define NUM_SHADERS_TO_LINK 100

GLuint compile_shader(GLenum shaderType, const char *src)
{
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  return shader;
}

GLuint create_program(GLuint vertexShader, GLuint fragmentShader)
{
   GLuint program = glCreateProgram();
   glAttachShader(program, vertexShader);
   glAttachShader(program, fragmentShader);
   glBindAttribLocation(program, 0, "apos");
   glBindAttribLocation(program, 1, "acolor");
   glLinkProgram(program);
   return program;
}

bool check_program_link_completed(GLuint program)
{
  GLint completed = 0;
  glGetProgramiv(program, GL_COMPLETION_STATUS_KHR, &completed);
  return completed;
}

double linkStart = 0;
int numRafFramesElapsed = 0;
int numShadersPending = 0;
std::vector<std::pair<GLuint, double> > pendingLinks;

int parallel_shader_compile_is_working = 0;

bool tick(double, void*)
{
  for(size_t i = 0; i < pendingLinks.size(); ++i)
  {
    if (pendingLinks[i].first && check_program_link_completed(pendingLinks[i].first))
    {
      double elapsed = emscripten_get_now() - pendingLinks[i].second;
      printf("Shader %d link completed in %d rAF frames/%f msecs.\n", (int)i, numRafFramesElapsed, elapsed);
      pendingLinks[i].first = 0;
      --numShadersPending;

      // This is how we detect that parallel shader compilation must be working: spawning NUM_SHADERS_TO_LINK shader
      // compiles cannot all finish within a single rAF() period, so some of them must finish after the first rAF.
      // If they all finished within the first rAF(), they must have gotten synchronously linked in main().
      if (numRafFramesElapsed > 0)
        parallel_shader_compile_is_working = 1;
    }
  }

  if (numShadersPending == 0)
  {
    printf("All shaders linked in %f msecs. parallel_shader_compile_is_working=%d\n", emscripten_get_now() - linkStart, parallel_shader_compile_is_working);
    assert(parallel_shader_compile_is_working);
    emscripten_force_exit(0);
  }

  ++numRafFramesElapsed;
  return true;
}

int main()
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  bool supported = emscripten_webgl_enable_extension(ctx, "KHR_parallel_shader_compile");
  if (!supported)
  {
    printf("Skipping test, KHR_parallel_shader_compile WebGL extension is not supported.\n");
    return 0;
  }

  linkStart = emscripten_get_now();

  for(int i = 0; i < NUM_SHADERS_TO_LINK; ++i)
  {
    char shader[256];
    sprintf(shader,
      "attribute vec4 apos;"
      "attribute vec4 acolor;"
      "varying vec4 color;"
      "void main() {"
        "color = acolor;"
        "gl_Position = apos + vec4(%f,%f,%f,0.0);"
      "}", (float)i, (float)i, (float)i);

    GLuint vs = compile_shader(GL_VERTEX_SHADER, shader);

    sprintf(shader,
      "precision lowp float;"
      "varying vec4 color;"
      "void main() {"
        "gl_FragColor = color + vec4(%f,%f,%f,0.0);"
      "}", (float)i, (float)i, (float)i);

    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, shader);

    GLuint program = create_program(vs, fs);

    pendingLinks.push_back(std::make_pair(program, emscripten_get_now()));
  }
  numShadersPending = pendingLinks.size();
  printf("Issuing %d shader links took %f msecs.\n", numShadersPending, emscripten_get_now() - linkStart);

  emscripten_request_animation_frame_loop(tick, 0);
}
