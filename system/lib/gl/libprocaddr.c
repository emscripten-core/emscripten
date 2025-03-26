/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// GL proc address library integration

#if GL_ENABLE_GET_PROC_ADDRESS

extern void* emscripten_GetProcAddress(const char *name);

__attribute__((weak)) // SDL2 will link in its own version of this
void* SDL_GL_GetProcAddress(const char* name) {
  return emscripten_GetProcAddress(name);
}

void* eglGetProcAddress(const char* name) {
  return emscripten_GetProcAddress(name);
}

void* glfwGetProcAddress(const char* name) {
  return emscripten_GetProcAddress(name);
}

#endif
