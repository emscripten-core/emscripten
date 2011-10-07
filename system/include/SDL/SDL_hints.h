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
 *  \file SDL_hints.h
 *  
 *  Official documentation for SDL configuration variables
 *
 *  This file contains functions to set and get configuration hints,
 *  as well as listing each of them alphabetically.
 *
 *  The convention for naming hints is SDL_HINT_X, where "SDL_X" is
 *  the environment variable that can be used to override the default.
 *
 *  In general these hints are just that - they may or may not be
 *  supported or applicable on any given platform, but they provide
 *  a way for an application or user to give the library a hint as
 *  to how they would like the library to work.
 */

#ifndef _SDL_hints_h
#define _SDL_hints_h

#include "SDL_stdinc.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/**
 *  \brief  A variable controlling how 3D acceleration is used to accelerate the SDL 1.2 screen surface. 
 *
 *  SDL can try to accelerate the SDL 1.2 screen surface by using streaming
 *  textures with a 3D rendering engine.  This variable controls whether and
 *  how this is done.
 *
 *  This variable can be set to the following values:
 *    "0"       - Disable 3D acceleration
 *    "1"       - Enable 3D acceleration, using the default renderer.
 *    "X"       - Enable 3D acceleration, using X where X is one of the valid rendering drivers.  (e.g. "direct3d", "opengl", etc.)
 *
 *  By default SDL tries to make a best guess for each platform whether
 *  to use acceleration or not.
 */
#define SDL_HINT_FRAMEBUFFER_ACCELERATION   "SDL_FRAMEBUFFER_ACCELERATION"

/**
 *  \brief  A variable specifying which render driver to use.
 *
 *  If the application doesn't pick a specific renderer to use, this variable
 *  specifies the name of the preferred renderer.  If the preferred renderer
 *  can't be initialized, the normal default renderer is used.
 *
 *  This variable is case insensitive and can be set to the following values:
 *    "direct3d"
 *    "opengl"
 *    "opengles2"
 *    "opengles"
 *    "software"
 *
 *  The default varies by platform, but it's the first one in the list that
 *  is available on the current platform.
 */
#define SDL_HINT_RENDER_DRIVER              "SDL_RENDER_DRIVER"

/**
 *  \brief  A variable controlling whether the OpenGL render driver uses shaders if they are available.
 *
 *  This variable can be set to the following values:
 *    "0"       - Disable shaders
 *    "1"       - Enable shaders
 *
 *  By default shaders are used if OpenGL supports them.
 */
#define SDL_HINT_RENDER_OPENGL_SHADERS      "SDL_RENDER_OPENGL_SHADERS"

/**
 *  \brief  A variable controlling the scaling quality
 *
 *  This variable can be set to the following values:
 *    "0" or "nearest" - Nearest pixel sampling
 *    "1" or "linear"  - Linear filtering (supported by OpenGL and Direct3D)
 *    "2" or "best"    - Anisotropic filtering (supported by Direct3D)
 *
 *  By default nearest pixel sampling is used
 */
#define SDL_HINT_RENDER_SCALE_QUALITY       "SDL_RENDER_SCALE_QUALITY"

/**
 *  \brief  A variable controlling whether updates to the SDL 1.2 screen surface should be synchronized with the vertical refresh, to avoid tearing.
 *
 *  This variable can be set to the following values:
 *    "0"       - Disable vsync
 *    "1"       - Enable vsync
 *
 *  By default SDL does not sync screen surface updates with vertical refresh.
 */
#define SDL_HINT_RENDER_VSYNC               "SDL_RENDER_VSYNC"
	
/**
 *  \brief  A variable controlling whether the idle timer is disabled on iOS.
 *
 *  When an iOS app does not receive touches for some time, the screen is
 *  dimmed automatically. For games where the accelerometer is the only input
 *  this is problematic. This functionality can be disabled by setting this
 *  hint.
 *
 *  This variable can be set to the following values:
 *    "0"       - Enable idle timer
 *    "1"       - Disable idle timer
 */
#define SDL_HINT_IDLE_TIMER_DISABLED "SDL_IOS_IDLE_TIMER_DISABLED"
	
/**
 *  \brief  A variable controlling which orientations are allowed on iOS.
 *
 *  In some circumstances it is necessary to be able to explicitly control
 *  which UI orientations are allowed.
 *
 *  This variable is a space delimited list of the following values:
 *    "LandscapeLeft", "LandscapeRight", "Portrait" "PortraitUpsideDown"
 */
#define SDL_HINT_ORIENTATIONS "SDL_IOS_ORIENTATIONS"


/**
 *  \brief  An enumeration of hint priorities
 */
typedef enum
{
    SDL_HINT_DEFAULT,
    SDL_HINT_NORMAL,
    SDL_HINT_OVERRIDE
} SDL_HintPriority;


/**
 *  \brief Set a hint with a specific priority
 *
 *  The priority controls the behavior when setting a hint that already
 *  has a value.  Hints will replace existing hints of their priority and
 *  lower.  Environment variables are considered to have override priority.
 * 
 *  \return SDL_TRUE if the hint was set, SDL_FALSE otherwise
 */
extern DECLSPEC SDL_bool SDLCALL SDL_SetHintWithPriority(const char *name,
                                                         const char *value,
                                                         SDL_HintPriority priority);

/**
 *  \brief Set a hint with normal priority
 * 
 *  \return SDL_TRUE if the hint was set, SDL_FALSE otherwise
 */
extern DECLSPEC SDL_bool SDLCALL SDL_SetHint(const char *name,
                                             const char *value);


/**
 *  \brief Get a hint
 *  
 *  \return The string value of a hint variable.
 */
extern DECLSPEC const char * SDLCALL SDL_GetHint(const char *name);

/**
 *  \brief  Clear all hints
 *
 *  This function is called during SDL_Quit() to free stored hints.
 */
extern DECLSPEC void SDLCALL SDL_ClearHints(void);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_hints_h */

/* vi: set ts=4 sw=4 expandtab: */
