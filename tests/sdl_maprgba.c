#include <stdio.h>
#include <SDL/SDL.h>
#include <emscripten.h>

/* Do a fill by locking the surface and filling the region */
void lock_fill(SDL_Surface *s, SDL_Rect *rect, Uint32 c)
{
  int x,y;
  Uint32 *pixels;

  SDL_LockSurface(s);
  pixels = s->pixels;

  for(y=rect->y; y<rect->y+rect->h; y++) {
    for(x=rect->x; x<rect->x+rect->w; x++) {
      pixels[x+y*s->pitch/sizeof(Uint32)] = c;
    }
  }

  SDL_UnlockSurface(s);
}

int main() {
  Uint32 c;
  SDL_Rect rect = {0,0,300,225};

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  c = SDL_MapRGB(screen->format, 0xff, 0x00, 0x00); // OPAQUE RED
  SDL_FillRect(screen, &rect, c);
  rect.y = 225;
  lock_fill(screen, &rect, c);
  rect.x = 300;
  rect.y = 0;
  c = SDL_MapRGB(screen->format, 0x7f, 0x7f, 0x00); // OPAQUE MUSTARD
  SDL_FillRect(screen, &rect, c);
  rect.y = 225;
  lock_fill(screen, &rect, c);
  rect.x = 150;
  rect.y = 112;
  rect.w = 300;
  rect.h = 225;
  c = SDL_MapRGBA(screen->format, 0xff, 0xff, 0xff, 0xff); // OPAQUE WHITE
  SDL_FillRect(screen, &rect, c);
  c = SDL_MapRGBA(screen->format, 0x00, 0x00, 0x00, 0x00); // TRANSPARENT BLACK
  SDL_FillRect(screen, &rect, c);
  SDL_UpdateRect(screen, 0, 0, 600, 450);

  printf("The left half should be red and the right half mustard.\n");
  printf("There should be a white rectangle in the center.\n");

  SDL_Quit();

  return 0;
}
