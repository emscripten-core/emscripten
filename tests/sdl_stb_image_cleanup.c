#include <stdlib.h>
#include <SDL/SDL_image.h>
#include <emscripten.h>

void loadImage(const char* fileName) {
  SDL_Surface *image = IMG_Load(fileName);
  if (!image) {
     printf("IMG_Load: %s\n", IMG_GetError());
     abort();
  }
  SDL_FreeSurface(image);
}

int main() {
  rename("screenshot.not", "screenshot.jpg");
  for (int i = 0; i < 20; ++i) {
    loadImage("screenshot.jpg");
  }

  int result = EM_ASM_INT({
    return Math.trunc(emscriptenMemoryProfiler.totalMemoryAllocated / 1024 / 1024);
  });

  printf("Total memory allocated %d Mb\n", result);
  REPORT_RESULT(result);
  return 0;
}

