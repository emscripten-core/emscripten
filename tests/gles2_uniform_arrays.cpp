// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "SDL/SDL_opengl.h"
#include "SDL/SDL.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>

void RunTest(int testVariant)
{
    printf("Testing variant %d\n", testVariant);
    GLuint vs = 0;

    const char *vsCode = "#version 100\n"
        "attribute vec4 pos; void main() { gl_Position = pos; }";

    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsCode, NULL);
    glCompileShader(vs);

    GLuint ps = 0;

    const char *psCode = "#version 100\n"
        "precision lowp float;\n"
        "uniform vec3 color;\n"
        "uniform vec3 colors[3];\n"
        "void main() { gl_FragColor = vec4(color,1) + vec4(colors[0].r, colors[1].g, colors[2].b, 1); }";

    ps = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ps, 1, &psCode, NULL);
    glCompileShader(ps);

    GLuint program = 0;
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, ps);
    glBindAttribLocation(program, 0, "pos");
    glLinkProgram(program);

    // Also test that GL_ACTIVE_ATTRIBUTE_MAX_LENGTH and GL_ACTIVE_UNIFORM_MAX_LENGTH work. See https://github.com/emscripten-core/emscripten/issues/1796.
    GLint param;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &param);
    printf("active attrib max length: %d\n", param);
    assert(param == 4); // "pos"+null terminator
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &param);
    printf("active uniform max length: %d\n", param);
    assert(param == 10); // "colors[0]"+null terminator

    int color_loc = glGetUniformLocation(program, "color");
    assert(color_loc != -1);

    glUseProgram(program);
    float col[3] = { 0.2f, 0.2f, 0.2f };
    glUniform3fv(color_loc, 1, col);

    int loc = glGetUniformLocation(program, "colors");
    assert(loc != -1);
    // In previous Emscripten GL layer code, calling glGetUniformLocation would do extra caching operations that interacts how glUniform** after that will work, 
    // so to exhibit extra issues in old code (and to keep new code from regressing), must test both with and without excess glGetUniformLocation calls.
    if ((testVariant&1) != 0)
    {
        printf("check glGetUniformLocation with indexes\n");
        // Deliberately check in odd order to make sure any kind of lazy operations won't affect the indices we get.
        assert(glGetUniformLocation(program, "colors[2]") == loc+2);
        assert(glGetUniformLocation(program, "colors[0]") == loc);
        assert(glGetUniformLocation(program, "colors[3]") == -1);
        assert(glGetUniformLocation(program, "colors[1]") == loc+1);
        assert(glGetUniformLocation(program, "colors[]") == loc);
        assert(glGetUniformLocation(program, "colors[-1]") == -1);
        assert(glGetUniformLocation(program, "colors[-100]") == -1);
        printf("   ...ok\n");
    }

    float colors[4*3] = { 1,0,0, 0,0.5,0, 0,0,0.2, 1,1,1 };

    if ((testVariant&2)!=0)
    {
        glUniform3fv(loc+1, 3, colors+3); // Pass the actual colors (testing a nonzero location offset), but do a mistake by setting one index too many. Spec says this should be gracefully handled, and that excess elements are ignored.
        assert(glGetError() == GL_NO_ERROR);
        glUniform3fv(loc, 1, colors); // Set the first index as well.
        assert(glGetError() == GL_NO_ERROR);
    }
    else
    {
        glUniform3fv(loc, 4, colors); // Just directly set the full array.
        assert(glGetError() == GL_NO_ERROR);
    }

    assert(glGetError() == GL_NO_ERROR);

    GLuint vbo = 0;
    const GLfloat v[] = { -1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1 };
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    int in_worker = EM_ASM_INT({
      return typeof importScripts !== 'undefined'
    });

    if (!in_worker) {
      printf("Doing readpixels check\n");
      unsigned char pixel[4];
      glReadPixels(1,1,1,1,GL_RGBA,GL_UNSIGNED_BYTE, pixel);
      //printf("%d,%d,%d,%d\n", pixel[0], pixel[1], pixel[2], pixel[3]);
      assert(pixel[0] == 255);
      assert(pixel[1] == 178);
      assert(pixel[2] == 102);
      assert(pixel[3] == 255);
    }

    printf("OK: Case %d passed.\n", testVariant);
    // Lazy, don't clean up afterwards.
}

int main(int argc, char *argv[])
{
    SDL_Surface *screen;

    // Slightly different SDL initialization
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL ); // *changed*
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

    for(int i = 0; i < 4; ++i)
        RunTest(i);

#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif

    return 0;
}
