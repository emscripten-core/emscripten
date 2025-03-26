/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <EGL/egl.h>

#include <GLES2/gl2.h>

int main(int argc, char *argv[]) {
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(display != EGL_NO_DISPLAY);
    assert(eglGetError() == EGL_SUCCESS);

    EGLint major = 0, minor = 0;
    EGLBoolean ret = eglInitialize(display, &major, &minor);
    assert(eglGetError() == EGL_SUCCESS);
    assert(ret == EGL_TRUE);
    assert(major * 10000 + minor >= 10004);

    EGLint numConfigs;
    ret = eglGetConfigs(display, NULL, 0, &numConfigs);
    assert(eglGetError() == EGL_SUCCESS);
    assert(ret == EGL_TRUE);

    EGLint attribs[] = {
        EGL_RED_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_BLUE_SIZE, 5,
        EGL_NONE
    };
    EGLConfig config;
    ret = eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    assert(eglGetError() == EGL_SUCCESS);
    assert(ret == EGL_TRUE);

    EGLNativeWindowType dummyWindow;
    EGLSurface surface = eglCreateWindowSurface(display, config, dummyWindow, NULL);
    assert(eglGetError() == EGL_SUCCESS);
    assert(surface != 0);

    // WebGL maps to GLES2. GLES1 is not supported.
    EGLint contextAttribsOld[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 1,
        EGL_NONE
    };
    EGLContext context = eglCreateContext(display, config, NULL, contextAttribsOld);
    assert(eglGetError() != EGL_SUCCESS);

    //Test for invalid attribs
    EGLint contextInvalidAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        0xFFFF, -1,
        EGL_NONE
    };
    context = eglCreateContext(display, config, NULL, contextInvalidAttribs);
    assert(eglGetError() != EGL_SUCCESS);
    assert(context == 0);
    //Test for missing terminator
    EGLint contextAttribsMissingTerm[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
    };
    context = eglCreateContext(display, config, NULL, contextAttribsMissingTerm);
    assert(eglGetError() != EGL_SUCCESS);
    assert(context == 0);
    //Test for null terminator
    EGLint contextAttribsNullTerm[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        0
    };
    context = eglCreateContext(display, config, NULL, contextAttribsNullTerm);
    assert(eglGetError() != EGL_SUCCESS);
    assert(context == 0);
    //Test for invalid and null terminator
    EGLint contextAttribsNullTermInvalid[] =
    {
        0,
    };
    context = eglCreateContext(display, config, NULL, contextAttribsNullTermInvalid);
    assert(eglGetError() != EGL_SUCCESS);
    assert(context == 0);

    // The correct attributes, should create a good EGL context
    EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    context = eglCreateContext(display, config, NULL, contextAttribs);
    assert(eglGetError() == EGL_SUCCESS);
    assert(context != 0);

    assert(eglGetCurrentContext() == 0); // Creating a context does not yet activate it.
    assert(eglGetError() == EGL_SUCCESS);

    ret = eglMakeCurrent(display, surface, surface, context);
    assert(eglGetError() == EGL_SUCCESS);
    assert(ret == EGL_TRUE);
    assert(eglGetCurrentContext() == context);
    assert(eglGetCurrentSurface(EGL_READ) == surface);
    assert(eglGetCurrentSurface(EGL_DRAW) == surface);

    glClearColor(1.0,0.0,0.0,0.5);
    glClear(GL_COLOR_BUFFER_BIT);

    ret = eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    assert(eglGetError() == EGL_SUCCESS);
    assert(ret == EGL_TRUE);
    assert(eglGetCurrentContext() == EGL_NO_CONTEXT);
    assert(eglGetCurrentSurface(EGL_READ) == EGL_NO_SURFACE);
    assert(eglGetCurrentSurface(EGL_DRAW) == EGL_NO_SURFACE);

    assert(eglSwapInterval(display, 0) == EGL_TRUE);
    assert(eglGetError() == EGL_SUCCESS);
    assert(eglSwapInterval(display, 1) == EGL_TRUE);
    assert(eglGetError() == EGL_SUCCESS);
    assert(eglSwapInterval(display, 2) == EGL_TRUE);
    assert(eglGetError() == EGL_SUCCESS);

    ret = eglTerminate(display);
    assert(eglGetError() == EGL_SUCCESS);
    assert(ret == EGL_TRUE);

    // eglGetProcAddress() without active GL context and/or connected EGL display (after eglTerminate) is required to work, even though the returned function
    // pointers cannot be called unless an active context is available:
    // "return value of NULL indicates that the specified function does not exist for the implementation."
    // "Client API function pointers returned by eglGetProcAddress are independent of the display and the currently bound client API context, and may be used by any client API context which supports the function."
    // At https://www.khronos.org/registry/EGL/specs/eglspec.1.5.pdf, pages 82-32.
    assert(eglGetProcAddress("glClear") != 0);
    assert(eglGetProcAddress("glWakaWaka") == 0);

    return 0;
}
