#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <emscripten.h>
#include <math.h>
#include <sys/stat.h>

Mix_Chunk *sound;

void done() {
  int result = 1;
  REPORT_RESULT();
}

void pan() {
  static int frames = 0;
  frames++;

  float x = (sin(frames / 30.f) + 1) / 2;

  int channel = 0;
  int left = x * 255;
  int right = (1 - x) * 255;
  printf("%f %d %d\n", x, left, right);
  int panning = Mix_SetPanning(channel, left, right);
  assert(panning != 0);

  if (frames > 30 * 10)
      done();
}

int play() {
  int channel = Mix_PlayChannel(-1, sound, -1);
  assert(channel == 0);

  pan();

  return channel;
}

Mix_Chunk*
load(const char* filename)
{
  struct stat info;
  int result = stat(filename, &info);
  char * bytes = malloc( info.st_size );
  FILE * f = fopen( filename, "rb" );
  fread( bytes, 1, info.st_size, f  );
  fclose(f);

  SDL_RWops * ops = SDL_RWFromConstMem(bytes, info.st_size);
  Mix_Chunk * chunk = Mix_LoadWAV_RW(ops, 0);
  SDL_FreeRW(ops);
  free(bytes);

  return chunk;
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_AUDIO);

  int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..
  assert(ret == 0);

  sound = load("the_entertainer.wav");
  assert(sound);

  int channel = play();

  emscripten_set_main_loop(pan, 30, 0);

  emscripten_run_script("element = document.createElement('input');"
                        "element.setAttribute('type', 'button');"
                        "element.setAttribute('value', 'replay!');"
                        "element.setAttribute('onclick', 'Module[\"_play\"]()');"
                        "document.body.appendChild(element);");

  printf("you should hear the sound moving from left to right. press the button to replay!\n");

  return 0;
}
