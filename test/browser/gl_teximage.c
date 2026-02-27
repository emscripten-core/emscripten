/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

/*
 * GLES2 test for glTexImage2D parameters
 *
 * Original author: Jason Green <jason@transgaming.com>
 *
 */
#include "GLES2/gl2.h"
#include "SDL/SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <unistd.h>

typedef enum {
    TEST_STATUS_SUCCESS = 0,
    TEST_STATUS_FAILURE = 1
} TestStatus;

/* Report success or failure (1 or 0) to Emscripten's test harness. Also, exit
 * with the given error code. */
static void exit_with_status(TestStatus code) {
#ifdef REPORT_RESULT
    int result = (code == TEST_STATUS_SUCCESS) ? 1 : 0;
    REPORT_RESULT(result);
#endif

    exit(code);
}

/* Loop over all glGetError() results until GL reports GL_NO_ERROR */
static void clear_gl_errors() {
    GLenum err;
    do {
        err = glGetError();
    } while (err != GL_NO_ERROR);
}

int main(int argc, char *argv[]) {
    TestStatus passed = TEST_STATUS_SUCCESS;
    SDL_Surface *screen;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed with %s\n", SDL_GetError());
        exit_with_status(TEST_STATUS_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    screen = SDL_SetVideoMode(640, 480, 16, SDL_OPENGL);
    if (!screen) {
        printf("SDL_SetVideoMode failed with %s\n", SDL_GetError());
        exit_with_status(TEST_STATUS_FAILURE);
    }

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Allocate space for a 32x32 image with 4 bytes per pixel.
    // No need to fill it with any useful information, as these tests are
    // only designed to make sure glTexImage2D doesn't crash on unsupported
    // formats.
    void* pixels = malloc(4 * 32 * 32);
    if (pixels == NULL) {
        printf("Unable to allocate pixel data\n");
        exit_with_status(TEST_STATUS_FAILURE);
    }

    // First, try 0xffff for the internal format - should fail
    glTexImage2D(GL_TEXTURE_2D, 0, 0xffff, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) {
        printf("internal format == 0xffff succeeded, but should have failed\n");
        passed = TEST_STATUS_FAILURE;
    }
    clear_gl_errors();

    // Try 0xffff for the format - should fail
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, 0xffff, GL_UNSIGNED_BYTE, pixels);
    err = glGetError();
    if (err == GL_NO_ERROR) {
        printf("format == 0xffff succeeded, but should have failed\n");
        passed = TEST_STATUS_FAILURE;
    }
    clear_gl_errors();

    // Try 0xffff for the type - should fail
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, 0xffff, pixels);
    err = glGetError();
    if (err == GL_NO_ERROR) {
        printf("type == 0xffff succeeded, but should have failed\n");
        passed = TEST_STATUS_FAILURE;
    }
    clear_gl_errors();

    // Try GL_RGBA/GL_UNSIGNED_BYTE - should succeed
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("GL_RGBA/GL_UNSIGNED_BYTE failed with %x, but should have succeeded\n", err);
        passed = TEST_STATUS_FAILURE;
    }
    clear_gl_errors();

    // Clean up objects
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texture);
    free(pixels);

    // 'screen' is freed implicitly by SDL_Quit()
    SDL_Quit();

    exit_with_status(passed);
}
