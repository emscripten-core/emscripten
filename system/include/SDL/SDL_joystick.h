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
 *  \file SDL_joystick.h
 *  
 *  Include file for SDL joystick event handling
 */

#ifndef _SDL_joystick_h
#define _SDL_joystick_h

#include "SDL_stdinc.h"
#include "SDL_error.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/**
 *  \file SDL_joystick.h
 *
 *  In order to use these functions, SDL_Init() must have been called
 *  with the ::SDL_INIT_JOYSTICK flag.  This causes SDL to scan the system
 *  for joysticks, and load appropriate drivers.
 */

/* The joystick structure used to identify an SDL joystick */
struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;


/* Function prototypes */
/**
 *  Count the number of joysticks attached to the system
 */
extern DECLSPEC int SDLCALL SDL_NumJoysticks(void);

/**
 *  Get the implementation dependent name of a joystick.
 *  This can be called before any joysticks are opened.
 *  If no name can be found, this function returns NULL.
 */
extern DECLSPEC const char *SDLCALL SDL_JoystickName(int device_index);

/**
 *  Open a joystick for use.  
 *  The index passed as an argument refers tothe N'th joystick on the system.  
 *  This index is the value which will identify this joystick in future joystick
 *  events.
 *  
 *  \return A joystick identifier, or NULL if an error occurred.
 */
extern DECLSPEC SDL_Joystick *SDLCALL SDL_JoystickOpen(int device_index);

/**
 *  Returns 1 if the joystick has been opened, or 0 if it has not.
 */
extern DECLSPEC int SDLCALL SDL_JoystickOpened(int device_index);

/**
 *  Get the device index of an opened joystick.
 */
extern DECLSPEC int SDLCALL SDL_JoystickIndex(SDL_Joystick * joystick);

/**
 *  Get the number of general axis controls on a joystick.
 */
extern DECLSPEC int SDLCALL SDL_JoystickNumAxes(SDL_Joystick * joystick);

/**
 *  Get the number of trackballs on a joystick.
 *  
 *  Joystick trackballs have only relative motion events associated
 *  with them and their state cannot be polled.
 */
extern DECLSPEC int SDLCALL SDL_JoystickNumBalls(SDL_Joystick * joystick);

/**
 *  Get the number of POV hats on a joystick.
 */
extern DECLSPEC int SDLCALL SDL_JoystickNumHats(SDL_Joystick * joystick);

/**
 *  Get the number of buttons on a joystick.
 */
extern DECLSPEC int SDLCALL SDL_JoystickNumButtons(SDL_Joystick * joystick);

/**
 *  Update the current state of the open joysticks.
 *  
 *  This is called automatically by the event loop if any joystick
 *  events are enabled.
 */
extern DECLSPEC void SDLCALL SDL_JoystickUpdate(void);

/**
 *  Enable/disable joystick event polling.
 *  
 *  If joystick events are disabled, you must call SDL_JoystickUpdate()
 *  yourself and check the state of the joystick when you want joystick
 *  information.
 *  
 *  The state can be one of ::SDL_QUERY, ::SDL_ENABLE or ::SDL_IGNORE.
 */
extern DECLSPEC int SDLCALL SDL_JoystickEventState(int state);

/**
 *  Get the current state of an axis control on a joystick.
 *  
 *  The state is a value ranging from -32768 to 32767.
 *  
 *  The axis indices start at index 0.
 */
extern DECLSPEC Sint16 SDLCALL SDL_JoystickGetAxis(SDL_Joystick * joystick,
                                                   int axis);

/**
 *  \name Hat positions
 */
/*@{*/
#define SDL_HAT_CENTERED	0x00
#define SDL_HAT_UP		0x01
#define SDL_HAT_RIGHT		0x02
#define SDL_HAT_DOWN		0x04
#define SDL_HAT_LEFT		0x08
#define SDL_HAT_RIGHTUP		(SDL_HAT_RIGHT|SDL_HAT_UP)
#define SDL_HAT_RIGHTDOWN	(SDL_HAT_RIGHT|SDL_HAT_DOWN)
#define SDL_HAT_LEFTUP		(SDL_HAT_LEFT|SDL_HAT_UP)
#define SDL_HAT_LEFTDOWN	(SDL_HAT_LEFT|SDL_HAT_DOWN)
/*@}*/

/**
 *  Get the current state of a POV hat on a joystick.
 *
 *  The hat indices start at index 0.
 *  
 *  \return The return value is one of the following positions:
 *           - ::SDL_HAT_CENTERED
 *           - ::SDL_HAT_UP
 *           - ::SDL_HAT_RIGHT
 *           - ::SDL_HAT_DOWN
 *           - ::SDL_HAT_LEFT
 *           - ::SDL_HAT_RIGHTUP
 *           - ::SDL_HAT_RIGHTDOWN
 *           - ::SDL_HAT_LEFTUP
 *           - ::SDL_HAT_LEFTDOWN
 */
extern DECLSPEC Uint8 SDLCALL SDL_JoystickGetHat(SDL_Joystick * joystick,
                                                 int hat);

/**
 *  Get the ball axis change since the last poll.
 *  
 *  \return 0, or -1 if you passed it invalid parameters.
 *  
 *  The ball indices start at index 0.
 */
extern DECLSPEC int SDLCALL SDL_JoystickGetBall(SDL_Joystick * joystick,
                                                int ball, int *dx, int *dy);

/**
 *  Get the current state of a button on a joystick.
 *  
 *  The button indices start at index 0.
 */
extern DECLSPEC Uint8 SDLCALL SDL_JoystickGetButton(SDL_Joystick * joystick,
                                                    int button);

/**
 *  Close a joystick previously opened with SDL_JoystickOpen().
 */
extern DECLSPEC void SDLCALL SDL_JoystickClose(SDL_Joystick * joystick);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_joystick_h */

/* vi: set ts=4 sw=4 expandtab: */
