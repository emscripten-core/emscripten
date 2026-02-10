#include "webgl.h"
#include <math.h>
#include <memory.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#define WIDTH 1024
#define HEIGHT 768

float clamp01(float v)
{
  return v < 0.f ? 0.f : (v > 1.f ? 1.f : v);
}

float mod_dist(float v, float t)
{
  float d = fabsf(v - t);
  return fminf(d, 6.f - d);
}
// Per-frame animation tick.
bool draw_frame(double t, void *unused)
{
  static double prevT;
  double dt = t - prevT;
  prevT = t;

  clear_screen(0.1f, 0.2f, 0.3f, 1.f);

  // moon
  fill_image(WIDTH-250.f, HEIGHT - 250.f, 2.f, 1.f, 1.f, 1.f, 1.f, "moon.png");

  // snow background
#define NUM_FLAKES 100
#define FLAKE_SIZE 10
#define FLAKE_SPEED 0.05f
#define SNOWINESS 0.998
  static struct { float x, y; } flakes[NUM_FLAKES] = {};

#define SIM do { \
      flakes[i].y -= dt*(FLAKE_SPEED+i*0.05f/NUM_FLAKES); \
      flakes[i].x += dt*(fmodf(i*345362.f, 0.02f) - 0.01f); \
      float c = 0.5f + i*0.5/NUM_FLAKES; \
      if (flakes[i].y > -FLAKE_SIZE) fill_solid_rectangle(flakes[i].x, flakes[i].y, flakes[i].x+FLAKE_SIZE, flakes[i].y+FLAKE_SIZE, c, c, c, 1.f); \
      else if (emscripten_random() > SNOWINESS) flakes[i].y = HEIGHT, flakes[i].x = WIDTH*emscripten_random(); \
    } while(0)
  for(int i = 0; i < NUM_FLAKES/2; ++i) SIM;

  // ground
  fill_solid_rectangle(0.f, 0.f, WIDTH, 20.f, 0.8f, 0.8f, 0.8f, 1.f);

  // text
  const char text[] = "HELLO WEBGL";
  for(size_t i = 0; i < strlen(text); ++i)
  {
    float c = fmodf(t*0.005f + i*0.3f, 6.f);
    fill_char(190.f + i*64.f, HEIGHT*2/6 + sinf(t*0.005f+i*0.3f)*100.f,
      clamp01(2.f - mod_dist(c, 0.f)),
      clamp01(2.f - mod_dist(c, 2.f)),
      clamp01(2.f - mod_dist(c, 4.f)),
      1.f, text[i], 64.f, true);
  }

  // snow foreground
  for(int i = NUM_FLAKES/2; i < NUM_FLAKES; ++i) SIM;

  return true;
}

int main()
{
  init_webgl(WIDTH, HEIGHT);
  emscripten_request_animation_frame_loop(&draw_frame, 0);
}
