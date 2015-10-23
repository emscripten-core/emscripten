#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include <emscripten.h>

int result = 1;

static char *TouchFingerTypeToString(int type) {
  if (type == SDL_FINGERMOTION) return "SDL_FINGERMOTION";
  if (type == SDL_FINGERDOWN) return "SDL_FINGERDOWN";
  if (type == SDL_FINGERUP) return "SDL_FINGERUP";
  return "UNKNOWN";
}

int got_down = 0;
int got_move = 0;
int got_up = 0;

void progress() {
  if (!got_down) printf("Hold down a finger to generate a touch down event.\n");
  else if (!got_move) printf("Drag a finger to generate a touch move event.\n");
  else if (!got_up) printf("Release a finger to generate a touch up event.\n");
  else
  {
    int result = 0;
#ifdef REPORT_RESULT
    REPORT_RESULT();
#endif
  }
}

void loop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_FINGERMOTION:
      case SDL_FINGERDOWN:
      case SDL_FINGERUP: {
        SDL_TouchFingerEvent *t = (SDL_TouchFingerEvent*)&event;
        printf("type: %s, timestamp: %u, touchId: %llu, fingerId: %llu, x: %f, y: %f, dx: %f, dy: %f, pressure: %f\n",
          TouchFingerTypeToString(event.type), t->timestamp, t->touchId, t->fingerId, t->x, t->y, t->dx, t->dy, t->pressure);

        if (t->timestamp != 0 && t->x >= 0.f && t->x <= 1.f && t->y >= 0.f && t->y <= 1.f && t->pressure >= 0.f && t->pressure <= 1.f) {
          if (event.type == SDL_FINGERDOWN) { got_down = 1; progress(); }
          if (event.type == SDL_FINGERMOTION) { got_move = 1; progress(); }
          if (event.type == SDL_FINGERDOWN) { got_up = 1; progress(); }
        }
        break;
      }
    }
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  progress();

#ifdef AUTOMATE_SUCCESS
  EM_ASM(
    function sendEvent(type, data) {
      var event = document.createEvent('Event');
      event.initEvent(type, true, true);
      for(var d in data) event[d] = data[d];
      Module['canvas'].dispatchEvent(event);
    }
    // Pass test coordinates in canvas element coordinate frame.
    var x = Module['canvas'].getBoundingClientRect().left;
    var y = Module['canvas'].getBoundingClientRect().top;
    sendEvent('touchstart', { touches: [ { pageX: x+300, pageY: y+225, deviceID: 1, identifier: 1, force: 1 } ] });
    sendEvent('touchmove', { touches: [ { pageX: x+400, pageY: y+225, deviceID: 1, identifier: 1, force: 1 } ] });
    sendEvent('touchend', { changedTouches: [ { pageX: x+400, pageY: y+225, deviceID: 1, identifier: 1, force: 1 } ] });
  );
#endif

  emscripten_set_main_loop(loop, 0, 0);

  return 0;
}
