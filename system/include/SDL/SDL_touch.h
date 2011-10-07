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

/**
 *  \file SDL_touch.h
 *  
 *  Include file for SDL touch event handling.
 */

#ifndef _SDL_touch_h
#define _SDL_touch_h

#include "SDL_stdinc.h"
#include "SDL_error.h"
#include "SDL_video.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif


typedef Sint64 SDL_TouchID;
typedef Sint64 SDL_FingerID;


struct SDL_Finger {
  SDL_FingerID id;
  Uint16 x;
  Uint16 y;
  Uint16 pressure;
  Uint16 xdelta;
  Uint16 ydelta;
  Uint16 last_x, last_y,last_pressure;  /* the last reported coordinates */
  SDL_bool down;
};

typedef struct SDL_Touch SDL_Touch;
typedef struct SDL_Finger SDL_Finger;


struct SDL_Touch {
  
  /* Free the touch when it's time */
  void (*FreeTouch) (SDL_Touch * touch);
  
  /* data common for tablets */
  float pressure_max, pressure_min;
  float x_max,x_min;
  float y_max,y_min;
  Uint16 xres,yres,pressureres;
  float native_xres,native_yres,native_pressureres;
  float tilt;                   /* for future use */
  float rotation;               /* for future use */
  
  /* Data common to all touch */
  SDL_TouchID id;
  SDL_Window *focus;
  
  char *name;
  Uint8 buttonstate;
  SDL_bool relative_mode;
  SDL_bool flush_motion;

  int num_fingers;
  int max_fingers;
  SDL_Finger** fingers;
    
  void *driverdata;
};



/* Function prototypes */

/**
 *  \brief Get the touch object at the given id.
 *
 *
 */
  extern DECLSPEC SDL_Touch* SDLCALL SDL_GetTouch(SDL_TouchID id);



/**
 *  \brief Get the finger object of the given touch, at the given id.
 *
 *
 */
  extern 
  DECLSPEC SDL_Finger* SDLCALL SDL_GetFinger(SDL_Touch *touch, SDL_FingerID id);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_mouse_h */

/* vi: set ts=4 sw=4 expandtab: */
