/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <ftw.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

GLFWwindow* g_window;

void render();

void render() {
  glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

int display_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
  printf("%-3s %2d %s\n", 
    (tflag == FTW_D) ? "d" : "f", // Type: directory or file
    ftwbuf->level,                // Depth level
    fpath                         // Full path
  );
  if ( tflag != FTW_D ) {
    FILE *fp = fopen(fpath, "rb");
    if (!fp) {
        printf("failed to open %s\n", fpath);
        perror("fopen");
        return -1;
    }
    int c;
    long size = 0;
    bool dump = strstr(fpath, ".txt") != 0;
    if (dump) printf("text file contents (first 100 bytes): ");
    while ((c = fgetc(fp)) != -1) {
        ++size;
        if (dump && size <= 100) putchar(c);
    }
    if (dump) putchar('\n');
    printf("read %ld bytes from %s\n", size, fpath);

    fclose(fp);
  }
  return 0; // Return 0 to continue traversal
}

void on_file_drop(GLFWwindow *window, int count, const char **paths) {
  for (int i = 0; i < count; ++i) {
    printf("dropped file %s\n", paths[i]);
    // FTW_PHYS: Do not follow symbolic links
    if (nftw(paths[i], display_info, 20, FTW_PHYS) == -1) {
      printf("failed to traverse %s\n", paths[i]);
      perror("nftw");
      assert(false);
    }
#ifdef __EMSCRIPTEN__
    // Emscripten copies the contents of the dropped file into the
    // in-browser filesystem. Delete after usage to free up memory.
    printf("unlinking %s\n", paths[i]);
    unlink(paths[i]);
#endif

  }
  emscripten_force_exit(0);
}

int main() {
  if (!glfwInit())
  {
    printf("Could not create window. Test failed.\n");      
    return 1;
  }
  glfwWindowHint(GLFW_RESIZABLE , 1);
  g_window = glfwCreateWindow(600, 450, "GLFW drop file", NULL, NULL);
  if (!g_window)
  {
    printf("Could not create window. Test failed.\n");      
    glfwTerminate();
    return 2;
  }
  glfwMakeContextCurrent(g_window);

  // Install callbacks
  glfwSetDropCallback(g_window, on_file_drop);

  // Main loop
  printf("Drag and drop a file or directory from your desktop onto the green canvas.\n");
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(render, 0, 1);
#else
  while (!glfwWindowShouldClose(g_window)) {
    render();
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }
#endif

  glfwTerminate();

  return 0;
}
