// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <SDL/SDL.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#ifndef SINGLETHREADED
#include <emscripten/threading.h>
#endif
#endif

#ifdef __SSE__
#include <xmmintrin.h>
#endif

int ENVIRONMENT_IS_WEB = 0;

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
  h = log(h)*100.f;
  if (h < 0.f) h = 0.f;

  //h = fmod(h, 360.f); // fmod gives weird graphical artifacts?
  if (h >= 360.f) h -= ((int)(h / 360.f)) * 360.f;

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

unsigned long long ComputeMandelbrot(float *srcReal, float *srcImag, uint32_t *dst, int strideSrc, int strideDst, int x, int y, int yIncr, int w, int h, float left, float top, float incrX, float incrY, unsigned int numItersBefore, unsigned int numIters)
{
  for(int Y = y; Y < h; Y += yIncr)
  {
    float *sr = (float*)((uintptr_t)srcReal + strideSrc * Y) + x;
    float *si = (float*)((uintptr_t)srcImag + strideSrc * Y) + x;
    uint32_t *d = (uint32_t*)((uintptr_t)dst + strideDst * Y) + x;
    float imag = top + Y * incrY;
    for(int X = 0; X < w; ++X)
    {
      float real = left + (x + X) * incrX;
      float v_real = sr[X];
      if (v_real != INFINITY)
      {
        float v_imag = si[X];
        for(unsigned int i = 0; i < numIters; ++i)
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
        sr[X] = v_real;
        si[X] = v_imag;
      }
    }
  }
  return (unsigned long long)((h-y)/yIncr)*w*numIters;
}

#ifdef __SSE__
// Not strictly correct anyzero_ps, but faster, and depends on that color alpha channel is always either 0xFF or 0.
int anyzero_ps(__m128 m)
{
  __m128 y = _mm_shuffle_ps(m, m, _MM_SHUFFLE(2,3,0,1));
  m = _mm_and_ps(m, y);
  __m128 z = _mm_movehl_ps(m, m);
  m = _mm_and_ps(m, z);
  return _mm_ucomige_ss(m, m);
}

int any_ps(__m128 m)
{
  __m128 y = _mm_shuffle_ps(m, m, _MM_SHUFFLE(2,3,0,1));
  m = _mm_or_ps(m, y);
  __m128 z = _mm_movehl_ps(m, m);
  m = _mm_or_ps(m, z);
  return !_mm_ucomige_ss(m, m);
}

int xnan_ss(__m128 m) { return !_mm_ucomige_ss(m, m); }
int ynan_ss(__m128 m) { return xnan_ss(_mm_shuffle_ps(m, m, _MM_SHUFFLE(1,1,1,1))); }
int znan_ss(__m128 m) { return xnan_ss(_mm_movehl_ps(m, m)); }
int wnan_ss(__m128 m) { return xnan_ss(_mm_shuffle_ps(m, m, _MM_SHUFFLE(3,3,3,3))); }

unsigned long long ComputeMandelbrot_SSE(float *srcReal, float *srcImag, uint32_t *dst, int strideSrc, int strideDst, int x, int y, int yIncr, int w, int h, float left, float top, float incrX, float incrY, unsigned int numItersBefore, unsigned int numIters)
{
  const __m128 four = _mm_set1_ps(4.f);
  for(int Y = y; Y < h; Y += yIncr)
  {
    float *sr = (float*)((uintptr_t)srcReal + strideSrc * Y) + x;
    float *si = (float*)((uintptr_t)srcImag + strideSrc * Y) + x;
    uint32_t *d = (uint32_t*)((uintptr_t)dst + strideDst * Y) + x;
    float imag = top + Y * incrY;
    __m128 Imag = _mm_set1_ps(imag);
    for(int X = 0; X < w; X += 4)
    {
      float real = left + (x + X) * incrX;
      __m128 Real = _mm_set_ps(real + 3*incrX, real + 2*incrX, real + incrX, real);
      __m128 v_real = _mm_loadu_ps(sr+X);
//      float v_real = sr[X];
//      if (v_real != INFINITY)
      {
        __m128 v_imag = _mm_loadu_ps(si+X);
//        float v_imag = si[X];

        __m128 oldColor = _mm_loadu_ps((float*)d+X);
        if (anyzero_ps(oldColor))
        //if (d[X] == 0 || d[X+1] == 0 || d[X+2] == 0 || d[X+3] == 0)
        {
          __m128 oldIterating = _mm_cmpeq_ps(oldColor, _mm_setzero_ps());
          for(unsigned int i = 0; i < numIters; ++i)
          {
            // (x+yi)^2 = x^2 - y^2 + 2xyi
            // ||x_+yi||^2 = x^2+y^2
            //float new_real = v_real*v_real - v_imag*v_imag + real;
            __m128 new_real = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(v_real, v_real), _mm_mul_ps(v_imag, v_imag)), Real);
            //v_imag = 2.f * v_real * v_imag + imag;
            __m128 v_ri = _mm_mul_ps(v_real, v_imag);
            v_imag = _mm_add_ps(_mm_add_ps(v_ri, v_ri), Imag);
            v_real = new_real;

  /*
            new_real = v_real*v_real - v_imag*v_imag + real;
            v_imag = 2.f * v_real * v_imag + imag;
            v_real = new_real;
  */
            __m128 len = _mm_add_ps(_mm_mul_ps(v_real, v_real), _mm_mul_ps(v_imag, v_imag));
            __m128 diverged = _mm_cmpgt_ps(len, four);
            __m128 divergedNow = _mm_and_ps(diverged, oldIterating);
            oldIterating = _mm_andnot_ps(divergedNow, oldIterating);
            //__m128 diverged = _mm_cmpge_ps(len, _mm_set1_ps(0));
            //__m128 old = _mm_loadu_ps((float*)d+X);

            if (any_ps(divergedNow))
            {
              uint32_t color = ColorMap(numItersBefore + i);
              if (xnan_ss(divergedNow)) d[X] = color;
              if (ynan_ss(divergedNow)) d[X+1] = color;
              if (znan_ss(divergedNow)) d[X+2] = color;
              if (wnan_ss(divergedNow)) d[X+3] = color;
//              _mm_storeu_ps((float*)d+X, _mm_or_ps(old, diverged));
            }
            /*
            if (v_real*v_real + v_imag*v_imag > 4.f)
            {
              d[X] = ColorMap(numItersBefore + i);
              v_real = INFINITY;
              break;
            }
            */
          }
          //sr[X] = v_real;
          //si[X] = v_imag;
          _mm_storeu_ps(sr+X, v_real);
          _mm_storeu_ps(si+X, v_imag);
        }
      }
//      real += incrX*4;
//      Real = _mm_set_ps(real + 3*incrX, real + 2*incrX, real + incrX, real);
    }
  }
  return (unsigned long long)((h-y)/yIncr)*w*numIters;
}
#endif

const int W = 512;
const int H = 512;
SDL_Surface *screen = 0;

int framesRendered = 0;
double lastFPSPrint = 0.0;

float incrX = 3.f / W;
float incrY = 3.f / W;
float left = -2.f;
float top = 0.f - incrY*H/2.f;

volatile unsigned int numItersDoneOnCanvas = 0;
unsigned int numItersPerFrame = 10;

#define MAX_NUM_THREADS 16
#define NUM_THREADS 2
int numTasks = NUM_THREADS;

float mandelReal[W*H] = {};
float mandelImag[W*H] = {};
uint32_t outputImage[W*H];

pthread_t thread[MAX_NUM_THREADS];
double timeSpentInMandelbrot[MAX_NUM_THREADS] = {};
unsigned long long numIters[MAX_NUM_THREADS] = {};

_Atomic uint32_t numThreadsRunning = 0;
uint32_t maxThreadsRunning = 1;

bool use_sse = true;

_Atomic uint32_t tasksDone = 0;
_Atomic uint32_t tasksPending[MAX_NUM_THREADS] = {};
#ifndef SINGLETHREADED
void *mandelbrot_thread(void *arg)
{
  long idx = (long)arg;
  numThreadsRunning++;

  char threadName[32];
  sprintf(threadName, "Worker %ld", idx);
  emscripten_set_thread_name(pthread_self(), threadName);

  for(;;)
  {
    emscripten_futex_wait(&tasksPending[idx], 0, INFINITY);
    tasksPending[idx] = 0;
    double t0 = emscripten_get_now();
    int ni;
#ifdef TEST_THREAD_PROFILING
    // If building as part of the harness, do silly things that show up in --threadprofiler,
    // such as sleeping and proxied file i/o ops
    usleep(2000);
    FILE *handle = fopen("a.txt", "w");
    fputs("hello", handle);
    fclose(handle);
#endif
#ifdef __SSE__
    if (use_sse)
      ni = ComputeMandelbrot_SSE(mandelReal, mandelImag, outputImage, sizeof(float)*W, sizeof(uint32_t)*W, 0, (int)idx, numTasks, W, H, left, top, incrX, incrY, numItersDoneOnCanvas, numItersPerFrame);
    else
#endif
      ni = ComputeMandelbrot(mandelReal, mandelImag, outputImage, sizeof(float)*W, sizeof(uint32_t)*W, 0, (int)idx, numTasks, W, H, left, top, incrX, incrY, numItersDoneOnCanvas, numItersPerFrame);
    double t1 = emscripten_get_now();
    numIters[idx] += ni;
    timeSpentInMandelbrot[idx] += t1-t0;
    tasksDone++;
    emscripten_futex_wake(&tasksDone, 9999);
  }
}
#endif

float hScroll = 0;
float vScroll = 0;
float zoom = 0.f;

//#define NO_SDL

double prevT = 0;

void register_tasks()
{
    numTasks = EM_ASM_INT(return (typeof document !== 'undefined' && document.getElementById('num_threads')) ? parseInt(document.getElementById('num_threads').value) : 1);

#ifdef SINGLETHREADED
  // Single-threaded
  for(int i = 0; i < numTasks; ++i)
  {
    double t0 = emscripten_get_now();
#ifdef __SSE__
    if (use_sse)
      numIters[0] += ComputeMandelbrot_SSE(mandelReal, mandelImag, outputImage, sizeof(float)*W, sizeof(uint32_t)*W, W*i/numTasks, 0, 1, W/numTasks, H, left, top, incrX, incrY, numItersDoneOnCanvas, numItersPerFrame);
    else
#endif
      numIters[0] += ComputeMandelbrot(mandelReal, mandelImag, outputImage, sizeof(float)*W, sizeof(uint32_t)*W, W*i/numTasks, 0, 1, W/numTasks, H, left, top, incrX, incrY, numItersDoneOnCanvas, numItersPerFrame);
    double t1 = emscripten_get_now();
    timeSpentInMandelbrot[0] += t1-t0;
  }
#else
  emscripten_atomic_fence();

  numTasks = EM_ASM_INT(return (typeof document !== 'undefined' && document.getElementById('num_threads')) ? parseInt(document.getElementById('num_threads').value) : 1);
  if (numTasks < 1) numTasks = 1;
  if (numTasks > emscripten_num_logical_cores()) numTasks = emscripten_num_logical_cores();

  // Register tasks.
  tasksDone = 0;
  emscripten_atomic_fence();
  for(int i = 0; i < numTasks; ++i)
  {
      tasksPending[i] = 1;
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
    emscripten_futex_wait(&tasksDone, td, 1);
    emscripten_main_thread_process_queued_calls();
  }
#endif
}

void main_tick()
{
#ifndef SINGLETHREADED
  const int threadsRunning = numThreadsRunning;
  if (threadsRunning < maxThreadsRunning) return;
#endif

  wait_tasks();
  numItersDoneOnCanvas += numItersPerFrame;

#if defined(TEST_THREAD_PROFILING)
  static bool reported = false;
  if (!reported && numItersDoneOnCanvas > 50000)
  {
    reported = true;
    emscripten_force_exit(0);
  }
#endif

  double t = emscripten_get_now();
  double dt = t - prevT;

#ifndef NO_SDL
  if (ENVIRONMENT_IS_WEB) {
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
  }
#endif

  float iterSize = 1.f / (incrX < incrY ? incrX : incrY);
  unsigned int minItersBeforeDisplaying = 50 + (int)(iterSize / 10000.f);
  prevT = t;
  if (numItersDoneOnCanvas >= minItersBeforeDisplaying)
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

    if (incrXNew > 1.f / 20000000.f && incrYNew > 1.f / 20000000.f) // Stop zooming in when single-precision floating point accuracy starts to visibly break apart.
    {
      left += (incrX - incrXNew) * W / 2.f;
      top += (incrY - incrYNew) * H / 2.f;

      incrX = incrXNew;
      incrY = incrYNew;
    }
  }

#ifndef NO_SDL
  if (ENVIRONMENT_IS_WEB && numItersDoneOnCanvas >= minItersBeforeDisplaying)
  {
    if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
    memcpy(screen->pixels, outputImage, sizeof(outputImage));
    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
    SDL_Flip(screen);
  }
#endif

  int new_use_sse = EM_ASM_INT(return (typeof document !== 'undefined' && document.getElementById('use_sse')) ? document.getElementById('use_sse').checked : false);

  if (numItersDoneOnCanvas >= minItersBeforeDisplaying || new_use_sse != use_sse)
  {
    if (hScroll != 0.f || vScroll != 0.f || zoom != 0.f || new_use_sse != use_sse)
    {
      for(int i = 0; i < W*H; ++i)
        outputImage[i] = 0x00000000;
      numItersDoneOnCanvas = 0;
      smallestIterOut = 0x7FFFFFFF;
      memset(mandelReal, 0, sizeof(mandelReal));
      memset(mandelImag, 0, sizeof(mandelImag));
    }
  }
  use_sse = new_use_sse;

  numItersPerFrame = EM_ASM_INT({
    if (typeof location !== 'undefined') {
      var updatesPerFrame = (new RegExp("[\\?&]updates=([^&#]*)")).exec(location.href);
      if (updatesPerFrame) return updatesPerFrame[1];
    }
    if (arguments_ && arguments_.length >= 1) return parseInt(arguments_[0]);
    if (typeof document !== 'undefined' && document.getElementById('updates_per_frame')) return parseInt(document.getElementById('updates_per_frame').value);
    return 50;
  });
  if (numItersPerFrame < 10) numItersPerFrame = 10;
  if (numItersPerFrame > 50000) numItersPerFrame = 50000;

  ++framesRendered;
  t = emscripten_get_now();
  if (t - lastFPSPrint > 1000.0)
  {
    double msecsPerFrame = (t - lastFPSPrint) / framesRendered;
    double mbTime = 0.0;
    unsigned long long numItersAllThreads = 0;
    for(int i = 0; i < numTasks; ++i)
    {
      mbTime += timeSpentInMandelbrot[i];
      timeSpentInMandelbrot[i] = 0;
      numItersAllThreads += numIters[i];
      numIters[i] = 0;
    }
#ifndef SINGLETHREADED
    mbTime /= numTasks;
#endif
    double fps = 1000.0 / msecsPerFrame;
    double itersPerSecond = numItersAllThreads * 1000.0 / (t-lastFPSPrint);
    char str[256];
    const char *suffix = "";
    static double ItersSmoothed = 0;
    ItersSmoothed = ItersSmoothed * 0.8 + itersPerSecond * 0.2;
    double itersNum = ItersSmoothed;

    if (ItersSmoothed > 0.9 * 1000 * 1000 * 1000)
    {
      suffix = "G";
      itersNum = ItersSmoothed / 1000000000.0;
    }
    else if (ItersSmoothed > 0.9 * 1000 * 1000)
    {
      suffix = "M";
      itersNum = ItersSmoothed / 1000000.0;
    }
    else if (ItersSmoothed > 0.9 * 1000)
    {
      suffix = "K";
      itersNum = ItersSmoothed / 1000.0;
    }
    double cpuUsageSeconds = mbTime/1000.0;
    double cpuUsageRatio = mbTime * 100.0 / (t-lastFPSPrint);

    if (ENVIRONMENT_IS_WEB) {
      sprintf(str, "%.3f%s iterations/second. FPS: %.2f. CPU usage: %.2f%%", itersNum, suffix, fps, cpuUsageRatio);
//    sprintf(str, "%.3f%s iterations/second. FPS: %.2f. Zoom: %f", itersNum, suffix, fps, 1.f / (incrX < incrY ? incrX : incrY));
      char str2[256];
      sprintf(str2, "document.getElementById('performance').innerHTML = '%s';", str);
      emscripten_run_script_string(str2);
      //EM_ASM({document.getElementById('performance').innerHTML = $0;}, str);
    }
    printf("%.2f msecs/frame, FPS: %.2f. %f iters/second. Time spent in Mandelbrot: %f secs. (%.2f%%)\n", msecsPerFrame, fps, itersPerSecond,
      cpuUsageSeconds, cpuUsageRatio);
    lastFPSPrint = t;
    framesRendered = 0;
  }

  register_tasks();
}

int main(int argc, char** argv)
{
  ENVIRONMENT_IS_WEB = EM_ASM_INT(return ENVIRONMENT_IS_WEB);

#ifndef NO_SDL
  if (ENVIRONMENT_IS_WEB) {
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(W, H, 32, SDL_SWSURFACE);
  }
#endif
  for(int i = 0; i < W*H; ++i)
    outputImage[i] = 0x00000000;

#ifndef SINGLETHREADED
  maxThreadsRunning = emscripten_num_logical_cores() < MAX_NUM_THREADS ? emscripten_num_logical_cores() : MAX_NUM_THREADS;
  for(int i = 0; i < maxThreadsRunning; ++i)
  {
    int rc = pthread_create(&thread[i], NULL, mandelbrot_thread, (void*)i);
    assert(rc == 0);
  }
#endif

#ifndef SINGLETHREADED
  emscripten_set_thread_name(pthread_self(), "Mandelbrot main");
#endif

#ifndef NO_SDL
  EM_ASM("SDL.defaults.copyOnLock = false; SDL.defaults.discardOnLock = true; SDL.defaults.opaqueFrontBuffer = false;");
#endif

  register_tasks();
  if (ENVIRONMENT_IS_WEB) {
    emscripten_set_main_loop(main_tick, 0, 0);
  } else {
    int numTotalFrames = EM_ASM_INT(return (arguments_ && arguments_.length >= 2) ? parseInt(arguments_[1]) : 1000);
    printf("Rendering %d frames of Mandelbrot. Invoke \"node|js mandelbrot.js numItersPerFrame numFrames\" to configure.\n", numTotalFrames);
    double t0 = emscripten_get_now();
    for(int i = 0; i < numTotalFrames; ++i) {
      main_tick();
    }
    double t1 = emscripten_get_now();
    printf("Rendered %d frames (%d total iterations) of Mandelbrot.\n", numTotalFrames, numItersDoneOnCanvas);
    printf("Total time: %f seconds, or %f seconds/frame, or %f seconds/iteration.\n", (t1-t0)/1000.0, (t1-t0)/(1000.0*numTotalFrames), (t1-t0)/(1000.0*numItersDoneOnCanvas));
  }

  return 0;
}
