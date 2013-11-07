#include <stdio.h>
#include <EGL/egl.h>

int main(int argc, char *argv[])
{
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    EGLint major, minor;
    eglInitialize(display, &major, &minor);

    EGLint numConfigs;
    eglGetConfigs(display, NULL, 0, &numConfigs);

    EGLint attribs[] = {
        EGL_RED_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_BLUE_SIZE, 5,
        EGL_NONE
    };
    EGLConfig config;
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    EGLNativeWindowType dummyWindow;
    EGLSurface surface = eglCreateWindowSurface(display, config, dummyWindow, NULL);

    EGLint width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    printf("(%d, %d)\n", width, height);

#ifdef REPORT_RESULT
    int result = 0;
    if(width == 300 && height == 150)
    {
        result = 1;
    }
    REPORT_RESULT();
#endif
}
