#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

int main()
{
    int result = 0;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

    TTF_Init();
    TTF_Font *font = TTF_OpenFont("myfont.ttf", 40);

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

#ifdef __EMSCRIPTEN__
    result = 1;
    REPORT_RESULT();
#endif
}

