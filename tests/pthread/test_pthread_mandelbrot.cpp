#include <stdio.h>
#include <math.h>
#include <string.h>
#include <SDL/SDL.h>
#include <pthread.h>
#include <assert.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/threading.h>
#include <emscripten/html5.h>
#endif

/*volatile*/ int smallestIterOut = 0x7FFFFFFF;
uint32_t ColorMap(int iter)
{
//  if (iter < smallestIterOut)
//    smallestIterOut = iter;
  unsigned int i = (iter/*-smallestIterOut*/)*10;
  if (i > 255) i = 255;
  i = 255 - i;
  if (i < 30) i = 30;
  return 0xFF000000 | (i) | (i << 8) | (i << 16);
}

void ComputeMandelbrot(float *src, uint32_t *dst, int strideSrc, int strideDst, int x, int y, int w, int h, float left, float top, float incrX, float incrY, int numItersBefore, int numIters)
{
  for(int Y = y; Y < y+h; ++Y)
  {
    float *s = (float*)((uintptr_t)src + strideSrc * Y) + 2*x;
    uint32_t *d = (uint32_t*)((uintptr_t)dst + strideDst * Y) + x;
    float imag = top + Y * incrY;
    float real = left + x * incrX;
    for(int X = 0; X < w; ++X)
    {
      float v_real = s[2*X];
      if (v_real != INFINITY)
      {
        float v_imag = s[2*X+1];
        for(int i = 0; i < numIters; ++i)
        {
          // (x+yi)^2 = x^2 - y^2 + 2xyi
          // ||x_+yi||^2 = x^2+y^2
          float new_real = v_real*v_real - v_imag*v_imag + real;
          v_imag = 2.f * v_real * v_imag + imag;
          v_real = new_real;
          if (v_real*v_real + v_imag*v_imag > 4.f)
          {
            d[X] = ColorMap(numItersBefore + i);
            v_real = INFINITY;
            break;
          }
        }
        s[2*X] = v_real;
        s[2*X+1] = v_imag;
      }
      real += incrX;
    }
  }
}

const int W = 384;
const int H = 384;
SDL_Surface *screen = 0;

int framesRendered = 0;
double lastFPSPrint = 0.0;

float incrX = 3.f / W;
float incrY = 3.f / W;
float left = -2.f;
float top = -1.5f;

volatile int numItersBefore = 0;
const int numItersPerFrame = 100;

#define NUM_THREADS 8
const int numTasks = NUM_THREADS;

float mandel[W*H*2] = {};
uint32_t outputImage[W*H];

pthread_t thread[NUM_THREADS];

int tasksPending[NUM_THREADS] = {};
void *mandelbrot_thread(void *arg)
{
  int idx = (int)arg;

  for(;;)
  {
    int oldVal = emscripten_atomic_cas_u32(&tasksPending[idx], 1, 2);
    if (oldVal == 1)
      ComputeMandelbrot(mandel, outputImage, sizeof(float)*2*W, sizeof(uint32_t)*W, W*idx/numTasks, 0, W/numTasks, H, left, top, incrX, incrY, numItersBefore, numItersPerFrame);
    emscripten_atomic_cas_u32(&tasksPending[idx], 2, 3);
  }

//  pthread_exit(0);
}

float hScroll = 0.f;
float vScroll = 0.f;
float zoom = 0.f;

//#define NO_SDL

double prevT = 0;

void main_tick()
{
  double t = emscripten_get_now();
  double dt = t - prevT;

#ifndef NO_SDL
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_RIGHT: hScroll = 1.f; break;
          case SDLK_LEFT: hScroll = -1.f; break;
          case SDLK_DOWN: vScroll = 1.f; break;
          case SDLK_UP: vScroll = -1.f; break;
          case SDLK_a: zoom = -1.f; break;
          case SDLK_z: zoom = 1.f; break;
          }
        break;
      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
          case SDLK_RIGHT: 
          case SDLK_LEFT: hScroll = 0.f; break;
          case SDLK_DOWN: 
          case SDLK_UP: vScroll = 0.f; break;
          case SDLK_a:
          case SDLK_z: zoom = 0.f; break;
          }
        break;
    }
  }

  if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
#endif

  prevT = t;
  top += dt * vScroll * incrX / 5.f;
  left += dt * hScroll * incrY / 5.f;

  // ctrX = left + incrX * W / 2.f;
  // ctrXNew = leftNew + incrXNew * W / 2.f;
  // ctrXNew == ctrX
  // left + incrX * W / 2.f == leftNew + incrXNew * W / 2.f
  // leftNew = left + (incrX - incrXNew) * W / 2.f;
  float incrXNew = incrX + dt * zoom / 1000000.0;
  float incrYNew = incrY + dt * zoom / 1000000.0;

  left += (incrX - incrXNew) * W / 2.f;
  top += (incrY - incrYNew) * H / 2.f;

  incrX = incrXNew;
  incrY = incrYNew;

  if (hScroll != 0.f || vScroll != 0.f || zoom != 0.f)
  {
    for(int i = 0; i < W*H; ++i)
      outputImage[i] = 0xFF000000;
    numItersBefore = 0;
    smallestIterOut = 0x7FFFFFFF;
    memset(mandel, 0, sizeof(mandel));
  }
  emscripten_atomic_fence();
#if 0
  // Single-threaded
  for(int i = 0; i < numTasks; ++i)
  {
//    ComputeMandelbrot(mandel, outputImage, sizeof(float)*2*W, sizeof(uint32_t)*W, W*i/numTasks, 0, W/numTasks, H, left, top, incrX, incrY, numItersBefore, numItersPerFrame);
  }
#endif

  // Register tasks.
  for(int i = 0; i < NUM_THREADS; ++i)
  {
    for(;;)
    {
      int oldVal = emscripten_atomic_cas_u32(&tasksPending[i], 0, 1);
      if (oldVal == 0)
        break;
    }
  }

  // Wait for each task to finish.
  for(int i = 0; i < NUM_THREADS; ++i)
  {
    for(;;)
    {
      int oldVal = emscripten_atomic_cas_u32(&tasksPending[i], 3, 0);
      if (oldVal == 3)
        break;
    }
  }

  numItersBefore += numItersPerFrame;
#ifndef NO_SDL
  memcpy(screen->pixels, outputImage, sizeof(outputImage));
  if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  SDL_Flip(screen); 
#endif

  ++framesRendered;
  t = emscripten_get_now();
  if (t - lastFPSPrint > 1000.0)
  {
    double msecsPerFrame = (t - lastFPSPrint) / framesRendered;
    double fps = 1000.0 / msecsPerFrame;
    printf("%.2f msecs/frame, FPS: %.2f\n", msecsPerFrame, fps);
    lastFPSPrint = t;
    framesRendered = 0;
  }
}

int main(int argc, char** argv)
{
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(W, H, 32, SDL_SWSURFACE);
  for(int i = 0; i < W*H; ++i)
    outputImage[i] = 0xFF000000;

  for(int i = 0; i < numTasks; ++i)
  {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc = pthread_create(&thread[i], &attr, mandelbrot_thread, (void*)i);
    assert(rc == 0);
    pthread_attr_destroy(&attr);
  }

  EM_ASM("SDL.defaults.copyOnLock = false; SDL.defaults.discardOnLock = true; SDL.defaults.opaqueFrontBuffer = false;");

  emscripten_set_main_loop(main_tick, 0, 0);
//  SDL_Quit();

  return 0;
}

