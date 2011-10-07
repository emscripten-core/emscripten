/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _SDL_main_h
#define _SDL_main_h

#include "SDL_stdinc.h"

/**
 *  \file SDL_main.h
 *  
 *  Redefine main() on some platforms so that it is called by SDL.
 */

#if defined(__WIN32__) || defined(__IPHONEOS__) || defined(__ANDROID__)
#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_NEEDED
#endif
#endif

#ifdef __cplusplus
#define C_LINKAGE	"C"
#else
#define C_LINKAGE
#endif /* __cplusplus */

/**
 *  \file SDL_main.h
 *
 *  The application's main() function must be called with C linkage,
 *  and should be declared like this:
 *  \code
 *  #ifdef __cplusplus
 *  extern "C"
 *  #endif
 *  int main(int argc, char *argv[])
 *  {
 *  }
 *  \endcode
 */

#ifdef SDL_MAIN_NEEDED
#define main	SDL_main
#endif

/**
 *  The prototype for the application's main() function
 */
extern C_LINKAGE int SDL_main(int argc, char *argv[]);


#include "begin_code.h"
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#ifdef __WIN32__

/**
 *  This can be called to set the application class at startup
 */
extern DECLSPEC int SDLCALL SDL_RegisterApp(char *name, Uint32 style,
                                            void *hInst);
extern DECLSPEC void SDLCALL SDL_UnregisterApp(void);

#endif /* __WIN32__ */


#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_main_h */

/* vi: set ts=4 sw=4 expandtab: */
