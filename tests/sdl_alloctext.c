#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>

int main(int argc, char **argv)
{
    int result = 0;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

    TTF_Font *font = TTF_OpenFont("myfont.ttf", 40);

    if (argc == 12) font = (TTF_Font*)malloc(1024);
    if (argc % 3) free(font);

    int i = 0;
    while (i < 200)
    {
        printf("%i\n", i);
        i++;

        SDL_Color color = { 0xff, 0x99, 0x00, 0xff };
        SDL_Surface *text = TTF_RenderText_Solid(font, "hello world", color);
        SDL_BlitSurface (text, NULL, screen, NULL);

        SDL_FreeSurface(text);
    }

#if __EMSCRIPTEN__
    result = 1;
    REPORT_RESULT();
#endif
}

