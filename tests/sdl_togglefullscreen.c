#include <stdio.h>
#include <SDL.h>
#include <emscripten.h>
#include <html5.h>

static enum {
    STATE_INITIAL,      /* Initial state, click needed to enter full screen */
    STATE_FS_REQ,       /* After click, expecting to enter full screen */
    STATE_FS,           /* Should remain in full screen */
    STATE_FS_QUIT_REQ,  /* Second click, expecting to leave full screen */
    STATE_FS_QUIT,      /* Left full screen */
    STATE_SUCCESS,      /* Reported success, test finished */
    STATE_ERROR         /* Something went wrong, and an error was reported */
} state = STATE_INITIAL;

int result = 0;

#if SDL_VERSION_ATLEAST(2,0,0)
SDL_Window *window = 0;
SDL_Renderer *renderer = 0;
#else
SDL_Surface *screen = 0;
#endif

static void fail(const char *msg) {
  printf("%s Test failed.\n", msg);
  state = STATE_ERROR;
  result = 0;
}

static EM_BOOL mouseup(int eventType,
                     const EmscriptenMouseEvent *keyEvent, void *userData) {
  if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
    switch (state) {
    case STATE_INITIAL:
#if SDL_VERSION_ATLEAST(2,0,0)
      SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
#else
      SDL_WM_ToggleFullScreen(screen);
#endif
      state = STATE_FS_REQ;
      break;
    case STATE_FS:
#if SDL_VERSION_ATLEAST(2,0,0)
      SDL_SetWindowFullscreen(window, 0);
#else
      SDL_WM_ToggleFullScreen(screen);
#endif
      state = STATE_FS_QUIT_REQ;
      break;
    case STATE_FS_QUIT:
    case STATE_SUCCESS:
    case STATE_ERROR:
      break;
    default:
      fail("Unexpected click.");
      break;
    }
  }

  return 0;
}

static void render() {
#if SDL_VERSION_ATLEAST(2,0,0)
  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
#else
  int width, height, isfs;
  emscripten_get_canvas_size(&width, &height, &isfs);
  SDL_Rect rect = { 0, 0, width, height };
  SDL_FillRect(screen, &rect, 0xff00ffff);
#endif
}

static void mainloop() {
  render();
  int isInFullscreen = EM_ASM_INT_V(return !!(document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement));

  switch (state) {
  case STATE_INITIAL:
    if (isInFullscreen) fail("Unexpected full screen.");
    break;
  case STATE_FS_REQ:
    if (isInFullscreen) {
      state = STATE_FS;
      printf("Successfully transitioned to fullscreen mode.\n");
    }
    break;
  case STATE_FS:
    if (!isInFullscreen) fail("Unexpectedly left full screen.");
    break;
  case STATE_FS_QUIT_REQ:
    if (!isInFullscreen) {
      state = STATE_FS_QUIT;
      /* One more render() call is needed to make sure canvas is yellow,
       * so don't quit yet.
       */
    }
    break;
  case STATE_FS_QUIT:
    if (isInFullscreen) fail("Unexpected full screen.");
    state = STATE_SUCCESS;
    printf("Exited fullscreen. Test succeeded.\n");
    result = 1;
    break;
  case STATE_ERROR:
  case STATE_SUCCESS:
#ifdef REPORT_RESULT
    {
      REPORT_RESULT();
    }
#endif
    emscripten_cancel_main_loop();
    break;
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
#if SDL_VERSION_ATLEAST(2,0,0)
  window = SDL_CreateWindow(NULL, 0, 0, 600, 450, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, 0);
#else
  screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);
#endif

  /* SDL 1 limits which events can be used here.
   * Click and mouseup don't work.
   */
  if (emscripten_set_mouseup_callback(NULL, NULL, 1, mouseup) != EMSCRIPTEN_RESULT_SUCCESS) {
    printf("Couldn't to set mouse callback. Test failed.\n");
  }

  printf("You should see a yellow canvas.\n");
  printf("Click on the canvas to enter full screen, and then click on the canvas again to finish the test.\n");
  printf("When in full screen, you should see the whole screen filled yellow.\n");
  printf("After exiting, the yellow canvas should be restored in the window.\n");
  emscripten_set_main_loop(mainloop, 0, 0);
  return 0;
}
