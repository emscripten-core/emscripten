#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <emscripten.h>

static Mix_Chunk *sound = NULL;
static Mix_Music *music = NULL;

static int soundChannel = 0;

void one_iter();
void one_iter() {
  static int frames = 0;
  frames++;
  
  switch( frames ) {
    case 1:
      soundChannel = Mix_PlayChannel(-1, sound, 0);
      assert(soundChannel != -1);
      break;
    case 2:
      assert(Mix_Playing(soundChannel));
      break;
    case 30:
      Mix_Pause(soundChannel);
      Mix_PlayMusic(music, 1);
      break;
    case 31:
      assert(Mix_Paused(soundChannel));
      assert(Mix_PlayingMusic());
      break;
    case 60:
      Mix_Resume(soundChannel);
      Mix_PauseMusic();
      break;
    case 61:
      assert(Mix_Playing(soundChannel));
      assert(Mix_PausedMusic());
      break;
    case 90:
      Mix_ResumeMusic();
      break;
    case 91:
      assert(Mix_PlayingMusic());
      break;
    case 120:
      Mix_HaltChannel(soundChannel);
      Mix_HaltMusic();
      break;
  };
}


int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_AUDIO);

  int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..
  assert(ret == 0);

  sound = Mix_LoadWAV("sound.ogg");
  assert(sound);
  music = Mix_LoadMUS("music.ogg");
  assert(music);

  emscripten_set_main_loop(one_iter, 30, 0);

  // force a quit
  while(Mix_Init(0))
    Mix_Quit();
  Mix_CloseAudio();

  return 0;
}

