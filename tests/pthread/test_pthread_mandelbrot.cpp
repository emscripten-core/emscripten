#include <stdio.h>
#include <math.h>
#include <string.h>
#include <SDL/SDL.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/threading.h>
#include <emscripten/html5.h>
#endif

// h: 0,360
// s: 0,1
// v: 0,1
void HSVtoRGB(float *r, float *g, float *b, float h, float s, float v)
{
  int i;
  float f, p, q, t;
  if (s == 0)
  {
    // achromatic (grey)
    *r = *g = *b = v;
    return;
  }
  h /= 60;      // sector 0 to 5
  i = floor(h);
  f = h - i;      // factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));
  switch(i)
  {
  case 0:
    *r = v;
    *g = t;
    *b = p;
    break;
  case 1:
    *r = q;
    *g = v;
    *b = p;
    break;
  case 2:
    *r = p;
    *g = v;
    *b = t;
    break;
  case 3:
    *r = p;
    *g = q;
    *b = v;
    break;
  case 4:
    *r = t;
    *g = p;
    *b = v;
    break;
  default:    // case 5:
    *r = v;
    *g = p;
    *b = q;
    break;
  }
}

int smallestIterOut = 0x7FFFFFFF;
uint32_t ColorMap(int iter)
{
//  int si = smallestIterOut;
//  if (iter < si)
//    emscripten_atomic_cas_u32(&smallestIterOut, si, iter);

  float r,g,b;
  float h=(float)iter;
  //h = sqrtf(h);
  h = log(h)*10.f;
  h = fmod(h, 360.f);
  float s = 0.5f;
  float v = 0.5f;
  HSVtoRGB(&r, &g, &b, h, s, v);
  int R = r*255.f;
  int G = g*255.f;
  int B = b*255.f;
  return 0xFF000000 | (B) | (G << 8) | (R << 16);


  /*
  unsigned int i = (iter)*10;
//  unsigned int i = (iter-si)*10;
  if (i > 255) i = 255;
  i = 255 - i;
  if (i < 30) i = 30;
  return 0xFF000000 | (i) | (i << 8) | (i << 16);
  */
}

int ComputeMandelbrot(float *src, uint32_t *dst, int strideSrc, int strideDst, int x, int y, int w, int h, float left, float top, float incrX, float incrY, int numItersBefore, int numIters)
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

/*
          new_real = v_real*v_real - v_imag*v_imag + real;
          v_imag = 2.f * v_real * v_imag + imag;
          v_real = new_real;
*/
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
  return h*w*numIters;
}

const int W = 800;
const int H = 500;
SDL_Surface *screen = 0;

int framesRendered = 0;
double lastFPSPrint = 0.0;

float incrX = 3.f / W;
float incrY = 3.f / W;
float left = -2.f;
float top = 0.f - incrY*H/2.f;

unsigned long numIters = 0;

volatile int numItersBefore = 0;
int numItersPerFrame = 10;

#define MAX_NUM_THREADS 8
#define NUM_THREADS 2
int numTasks = NUM_THREADS;

float mandel[W*H*2] = {};
uint32_t outputImage[W*H];

pthread_t thread[MAX_NUM_THREADS];
double timeSpentInMandelbrot[MAX_NUM_THREADS] = {};

bool use_sse = true;

int tasksDone = 0;
int tasksPending[MAX_NUM_THREADS] = {};
void *mandelbrot_thread(void *arg)
{
  int idx = (int)arg;

  for(;;)
  {
    emscripten_futex_wait(&tasksPending[idx], 0, INFINITY);
    emscripten_atomic_store_u32(&tasksPending[idx], 0);
    double t0 = emscripten_get_now();
    int ni = ComputeMandelbrot(mandel, outputImage, sizeof(float)*2*W, sizeof(uint32_t)*W, W*idx/numTasks, 0, W/numTasks, H, left, top, incrX, incrY, numItersBefore, numItersPerFrame);
    emscripten_atomic_add_u32(&numIters, ni);
    double t1 = emscripten_get_now();
    timeSpentInMandelbrot[idx] += t1-t0;
    emscripten_atomic_add_u32(&tasksDone, 1);
    emscripten_futex_wake(&tasksDone, 9999);
  }
}

float hScroll = 0;
float vScroll = 0;
float zoom = 0.f;

//#define NO_SDL

double prevT = 0;

void register_tasks()
{
#ifdef SINGLETHREADED
  // Single-threaded
  for(int i = 0; i < numTasks; ++i)
  {
    double t0 = emscripten_get_now();
    numIters += ComputeMandelbrot(mandel, outputImage, sizeof(float)*2*W, sizeof(uint32_t)*W, W*i/numTasks, 0, W/numTasks, H, left, top, incrX, incrY, numItersBefore, numItersPerFrame);
    double t1 = emscripten_get_now();
    timeSpentInMandelbrot[0] += t1-t0;
  }
#else
  emscripten_atomic_fence();

  numTasks = EM_ASM_INT_V(return parseInt(document.getElementById('num_threads').value));
  if (numTasks < 1) numTasks = 1;
  if (numTasks > MAX_NUM_THREADS) numTasks = MAX_NUM_THREADS;

  // Register tasks.
  emscripten_atomic_store_u32(&tasksDone, 0);
  emscripten_atomic_fence();
  for(int i = 0; i < numTasks; ++i)
  {
      emscripten_atomic_store_u32(&tasksPending[i], 1);
      emscripten_futex_wake(&tasksPending[i], 999);
  }
#endif
}

void wait_tasks()
{
#ifndef SINGLETHREADED
  // Wait for each task to finish.
  for(;;)
  {
    int td = tasksDone;
    if (td >= numTasks)
      break;
    emscripten_futex_wait(&tasksDone, td, INFINITY);
    emscripten_main_thread_process_queued_calls();
  }
#endif
}

void main_tick()
{
  wait_tasks();
  numItersBefore += numItersPerFrame;

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
#endif

  prevT = t;
  if (numItersBefore > 2*numItersPerFrame)
  {
    top += dt * vScroll * incrX / 5.f;
    left += dt * hScroll * incrY / 5.f;

    // ctrX = left + incrX * W / 2.f;
    // ctrXNew = leftNew + incrXNew * W / 2.f;
    // ctrXNew == ctrX
    // left + incrX * W / 2.f == leftNew + incrXNew * W / 2.f
    // leftNew = left + (incrX - incrXNew) * W / 2.f;
    float incrXNew = incrX + dt * zoom * incrX / 1000.0;
    float incrYNew = incrY + dt * zoom * incrX / 1000.0;

    left += (incrX - incrXNew) * W / 2.f;
    top += (incrY - incrYNew) * H / 2.f;

    incrX = incrXNew;
    incrY = incrYNew;
  }

#ifndef NO_SDL
  if (numItersBefore > 2*numItersPerFrame)
  {
    if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
    memcpy(screen->pixels, outputImage, sizeof(outputImage));
    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
    SDL_Flip(screen);
  }
#endif

  if (numItersBefore > 2*numItersPerFrame)
  {
    if (hScroll != 0.f || vScroll != 0.f || zoom != 0.f)
    {
      for(int i = 0; i < W*H; ++i)
        outputImage[i] = 0xFF000000;
      numItersBefore = 0;
      smallestIterOut = 0x7FFFFFFF;
      memset(mandel, 0, sizeof(mandel));
    }
  }

  numItersPerFrame = EM_ASM_INT_V(return parseInt(document.getElementById('updates_per_frame').value););
  if (numItersPerFrame < 10) numItersPerFrame = 10;
  if (numItersPerFrame > 2000) numItersPerFrame = 2000;

  ++framesRendered;
  t = emscripten_get_now();
  if (t - lastFPSPrint > 1000.0)
  {
    double msecsPerFrame = (t - lastFPSPrint) / framesRendered;
    double mbTime = 0.0;
    for(int i = 0; i < numTasks; ++i)
    {
      mbTime += timeSpentInMandelbrot[i];
      timeSpentInMandelbrot[i] = 0;
    }
    mbTime /= numTasks;
    double fps = 1000.0 / msecsPerFrame;
    double itersPerSecond = numIters * 1000.0 / (t-lastFPSPrint);
    char str[256];
    if (itersPerSecond > 0.9 * 1000 * 1000 * 1000)
      sprintf(str, "%.3fG iterations/second", itersPerSecond / 1000000000.0);
    else if (itersPerSecond > 0.9 * 1000 * 1000)
      sprintf(str, "%.3fM iterations/second", itersPerSecond / 1000000.0);
    else if (itersPerSecond > 0.9 * 1000)
      sprintf(str, "%.3fK iterations/second", itersPerSecond / 1000.0);
    else if (itersPerSecond > 1000)
      sprintf(str, "%.3f iterations/second", itersPerSecond / 1000.0);
    char str2[256];
    sprintf(str2, "document.getElementById('performance').innerHTML = '%s';", str);
    emscripten_run_script_string(str2);
    //EM_ASM({document.getElementById('performance').innerHTML = $0;}, str);
    printf("%.2f msecs/frame, FPS: %.2f. %f iters/second. Time spent in Mandelbrot: %f secs. (%.2f%%)\n", msecsPerFrame, fps, itersPerSecond,
      mbTime/1000.0, mbTime * 100.0 / (t-lastFPSPrint));
    lastFPSPrint = t;
    framesRendered = 0;
    numIters = 0;
  }

  register_tasks();
}

int main(int argc, char** argv)
{
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(W, H, 32, SDL_SWSURFACE);
  for(int i = 0; i < W*H; ++i)
    outputImage[i] = 0xFF000000;

  for(int i = 0; i < MAX_NUM_THREADS; ++i)
  {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc = pthread_create(&thread[i], &attr, mandelbrot_thread, (void*)i);
    assert(rc == 0);
    pthread_attr_destroy(&attr);
  }

  EM_ASM("SDL.defaults.copyOnLock = false; SDL.defaults.discardOnLock = true; SDL.defaults.opaqueFrontBuffer = false;");

  register_tasks();
  emscripten_set_main_loop(main_tick, 0, 0);

  return 0;
}
