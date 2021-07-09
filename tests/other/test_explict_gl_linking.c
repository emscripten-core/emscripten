#include <EGL/egl.h>

int main() {
    return (int)(long)eglGetProcAddress("foo");
}
