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
 *  \file SDL_input.h
 *  
 *  Include file for lowlevel SDL input device handling.
 *
 *  This talks about individual devices, and not the system cursor. If you
 *  just want to know when the user moves the pointer somewhere in your
 *  window, this is NOT the API you want. This one handles things like
 *  multi-touch, drawing tablets, and multiple, separate mice.
 *
 *  The other API is in SDL_mouse.h
 */

#ifndef _SDL_input_h
#define _SDL_input_h

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


/* Function prototypes */

/* !!! FIXME: real documentation
 * - Redetect devices
 * - This invalidates all existing device information from previous queries!
 * - There is an implicit (re)detect upon SDL_Init().
 */
extern DECLSPEC int SDLCALL SDL_RedetectInputDevices(void);

/**
 *  \brief Get the number of mouse input devices available.
 */
extern DECLSPEC int SDLCALL SDL_GetNumInputDevices(void);

/**
 *  \brief Gets the name of a device with the given index.
 *  
 *  \param index is the index of the device, whose name is to be returned.
 *  
 *  \return the name of the device with the specified index
 */
extern DECLSPEC const char *SDLCALL SDL_GetInputDeviceName(int index);


extern DECLSPEC int SDLCALL SDL_IsDeviceDisconnected(int index);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_mouse_h */

/* vi: set ts=4 sw=4 expandtab: */
