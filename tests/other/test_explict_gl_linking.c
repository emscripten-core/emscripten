#include <EGL/egl.h>

int main() {
    return (int)eglGetProcAddress("foo");
}
