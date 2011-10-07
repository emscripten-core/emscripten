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
 *  \defgroup Compatibility SDL 1.2 Compatibility API
 */
/*@{*/

/**
 *  \file SDL_compat.h
 *
 *  This file contains functions for backwards compatibility with SDL 1.2.
 */

/**
 *  \def SDL_NO_COMPAT
 *
 *  #define SDL_NO_COMPAT to prevent SDL_compat.h from being included.
 *  SDL_NO_COMPAT is intended to make it easier to covert SDL 1.2 code to
 *  SDL 1.3/2.0.
 */

 /*@}*/

#ifdef SDL_NO_COMPAT
#define _SDL_compat_h
#endif

#ifndef _SDL_compat_h
#define _SDL_compat_h

#include "SDL_video.h"
#include "SDL_version.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/**
 *  \addtogroup Compatibility
 */
/*@{*/

/* Platform */
#ifdef __WIN32__
#undef __WIN32__
#define __WIN32__   1
#endif

/**
 *  \name Surface flags
 */
/*@{*/
#define SDL_SWSURFACE       0x00000000  /**< \note Not used */
#define SDL_SRCALPHA        0x00010000
#define SDL_SRCCOLORKEY     0x00020000
#define SDL_ANYFORMAT       0x00100000
#define SDL_HWPALETTE       0x00200000
#define SDL_DOUBLEBUF       0x00400000
#define SDL_FULLSCREEN      0x00800000
#define SDL_RESIZABLE       0x01000000
#define SDL_NOFRAME         0x02000000
#define SDL_OPENGL          0x04000000
#define SDL_HWSURFACE       0x08000001  /**< \note Not used */
#define SDL_ASYNCBLIT       0x08000000  /**< \note Not used */
#define SDL_RLEACCELOK      0x08000000  /**< \note Not used */
#define SDL_HWACCEL         0x08000000  /**< \note Not used */
/*@}*//*Surface flags*/

#define SDL_APPMOUSEFOCUS	0x01
#define SDL_APPINPUTFOCUS	0x02
#define SDL_APPACTIVE		0x04

#define SDL_LOGPAL 0x01
#define SDL_PHYSPAL 0x02

#define SDL_ACTIVEEVENT	SDL_EVENT_COMPAT1
#define SDL_VIDEORESIZE	SDL_EVENT_COMPAT2
#define SDL_VIDEOEXPOSE	SDL_EVENT_COMPAT3
#define SDL_ACTIVEEVENTMASK	SDL_ACTIVEEVENT, SDL_ACTIVEEVENT
#define SDL_VIDEORESIZEMASK SDL_VIDEORESIZE, SDL_VIDEORESIZE
#define SDL_VIDEOEXPOSEMASK SDL_VIDEOEXPOSE, SDL_VIDEOEXPOSE
#define SDL_WINDOWEVENTMASK SDL_WINDOWEVENT, SDL_WINDOWEVENT
#define SDL_KEYDOWNMASK SDL_KEYDOWN, SDL_KEYDOWN
#define SDL_KEYUPMASK SDL_KEYUP, SDL_KEYUP
#define SDL_KEYEVENTMASK SDL_KEYDOWN, SDL_KEYUP
#define SDL_TEXTEDITINGMASK SDL_TEXTEDITING, SDL_TEXTEDITING
#define SDL_TEXTINPUTMASK SDL_TEXTINPUT, SDL_TEXTINPUT
#define SDL_MOUSEMOTIONMASK SDL_MOUSEMOTION, SDL_MOUSEMOTION
#define SDL_MOUSEBUTTONDOWNMASK SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONDOWN
#define SDL_MOUSEBUTTONUPMASK SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP
#define SDL_MOUSEWHEELMASK SDL_MOUSEWHEEL, SDL_MOUSEWHEEL
#define SDL_MOUSEEVENTMASK SDL_MOUSEMOTION, SDL_MOUSEBUTTONUP
#define SDL_JOYAXISMOTIONMASK SDL_JOYAXISMOTION, SDL_JOYAXISMOTION
#define SDL_JOYBALLMOTIONMASK SDL_JOYBALLMOTION, SDL_JOYBALLMOTION
#define SDL_JOYHATMOTIONMASK SDL_JOYHATMOTION, SDL_JOYHATMOTION
#define SDL_JOYBUTTONDOWNMASK SDL_JOYBUTTONDOWN, SDL_JOYBUTTONDOWN
#define SDL_JOYBUTTONUPMASK SDL_JOYBUTTONUP, SDL_JOYBUTTONUP
#define SDL_JOYEVENTMASK SDL_JOYAXISMOTION, SDL_JOYBUTTONUP
#define SDL_QUITMASK SDL_QUIT, SDL_QUIT
#define SDL_SYSWMEVENTMASK SDL_SYSWMEVENT, SDL_SYSWMEVENT
#define SDL_PROXIMITYINMASK SDL_PROXIMITYIN, SDL_PROXIMITYIN
#define SDL_PROXIMITYOUTMASK SDL_PROXIMITYOUT, SDL_PROXIMITYOUT
#define SDL_ALLEVENTS SDL_FIRSTEVENT, SDL_LASTEVENT

#define SDL_BUTTON_WHEELUP	4
#define SDL_BUTTON_WHEELDOWN	5

#define SDL_DEFAULT_REPEAT_DELAY	500
#define SDL_DEFAULT_REPEAT_INTERVAL	30

typedef struct SDL_VideoInfo
{
    Uint32 hw_available:1;
    Uint32 wm_available:1;
    Uint32 UnusedBits1:6;
    Uint32 UnusedBits2:1;
    Uint32 blit_hw:1;
    Uint32 blit_hw_CC:1;
    Uint32 blit_hw_A:1;
    Uint32 blit_sw:1;
    Uint32 blit_sw_CC:1;
    Uint32 blit_sw_A:1;
    Uint32 blit_fill:1;
    Uint32 UnusedBits3:16;
    Uint32 video_mem;

    SDL_PixelFormat *vfmt;

    int current_w;
    int current_h;
} SDL_VideoInfo;

/**
 *  \name Overlay formats
 *
 *  The most common video overlay formats.
 *  
 *  For an explanation of these pixel formats, see:
 *  http://www.webartz.com/fourcc/indexyuv.htm
 *  
 *  For information on the relationship between color spaces, see:
 *  http://www.neuro.sfc.keio.ac.jp/~aly/polygon/info/color-space-faq.html
 */
/*@{*/
#define SDL_YV12_OVERLAY  0x32315659    /**< Planar mode: Y + V + U  (3 planes) */
#define SDL_IYUV_OVERLAY  0x56555949    /**< Planar mode: Y + U + V  (3 planes) */
#define SDL_YUY2_OVERLAY  0x32595559    /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
#define SDL_UYVY_OVERLAY  0x59565955    /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
#define SDL_YVYU_OVERLAY  0x55595659    /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
/*@}*//*Overlay formats*/

/**
 *  The YUV hardware video overlay.
 */
typedef struct SDL_Overlay
{
    Uint32 format;              /**< Read-only */
    int w, h;                   /**< Read-only */
    int planes;                 /**< Read-only */
    Uint16 *pitches;            /**< Read-only */
    Uint8 **pixels;             /**< Read-write */

    /** 
     *  \name Hardware-specific surface info
     */
    /*@{*/
    struct private_yuvhwfuncs *hwfuncs;
    struct private_yuvhwdata *hwdata;
    /*@}*//*Hardware-specific surface info*/

    /** 
     *  \name Special flags
     */
    /*@{*/
    Uint32 hw_overlay:1;        /**< Flag: This overlay hardware accelerated? */
    Uint32 UnusedBits:31;
    /*@}*//*Special flags*/
} SDL_Overlay;

typedef enum
{
    SDL_GRAB_QUERY = -1,
    SDL_GRAB_OFF = 0,
    SDL_GRAB_ON = 1
} SDL_GrabMode;

struct SDL_SysWMinfo;

/**
 *  \name Obsolete or renamed key codes
 */
/*@{*/

#define SDL_keysym		SDL_Keysym
#define SDL_KeySym		SDL_Keysym
#define SDL_scancode	SDL_Scancode
#define SDL_ScanCode	SDL_Scancode
#define SDLKey          SDL_Keycode
#define SDLMod          SDL_Keymod

/** 
 *  \name Renamed keys
 *
 *  These key constants were renamed for clarity or consistency. 
 */
/*@{*/
#define SDLK_KP0 SDLK_KP_0
#define SDLK_KP1 SDLK_KP_1
#define SDLK_KP2 SDLK_KP_2
#define SDLK_KP3 SDLK_KP_3
#define SDLK_KP4 SDLK_KP_4
#define SDLK_KP5 SDLK_KP_5
#define SDLK_KP6 SDLK_KP_6
#define SDLK_KP7 SDLK_KP_7
#define SDLK_KP8 SDLK_KP_8
#define SDLK_KP9 SDLK_KP_9
#define SDLK_NUMLOCK SDLK_NUMLOCKCLEAR
#define SDLK_SCROLLOCK SDLK_SCROLLLOCK
#define SDLK_PRINT SDLK_PRINTSCREEN
#define SDLK_LMETA SDLK_LGUI
#define SDLK_RMETA SDLK_RGUI
/*@}*//*Renamed keys*/

/**
 *  \name META modifier
 *  
 *  The META modifier is equivalent to the GUI modifier from the USB standard.
 */
/*@{*/
#define KMOD_LMETA KMOD_LGUI
#define KMOD_RMETA KMOD_RGUI
#define KMOD_META KMOD_GUI
/*@}*//*META modifier*/

/** 
 *  \name Not in USB
 *
 *  These keys don't appear in the USB specification (or at least not under 
 *  those names). I'm unsure if the following assignments make sense or if these
 *  codes should be defined as actual additional SDLK_ constants.
 */
/*@{*/
#define SDLK_LSUPER SDLK_LMETA
#define SDLK_RSUPER SDLK_RMETA
#define SDLK_COMPOSE SDLK_APPLICATION
#define SDLK_BREAK SDLK_STOP
#define SDLK_EURO SDLK_2
/*@}*//*Not in USB*/

/*@}*//*Obsolete or renamed key codes*/

#define SDL_SetModuleHandle(x)
#define SDL_AllocSurface    SDL_CreateRGBSurface

extern DECLSPEC const SDL_version *SDLCALL SDL_Linked_Version(void);
extern DECLSPEC const char *SDLCALL SDL_AudioDriverName(char *namebuf, int maxlen);
extern DECLSPEC const char *SDLCALL SDL_VideoDriverName(char *namebuf, int maxlen);
extern DECLSPEC const SDL_VideoInfo *SDLCALL SDL_GetVideoInfo(void);
extern DECLSPEC int SDLCALL SDL_VideoModeOK(int width,
                                            int height,
                                            int bpp, Uint32 flags);
extern DECLSPEC SDL_Rect **SDLCALL SDL_ListModes(const SDL_PixelFormat *
                                                 format, Uint32 flags);
extern DECLSPEC SDL_Surface *SDLCALL SDL_SetVideoMode(int width, int height,
                                                      int bpp, Uint32 flags);
extern DECLSPEC SDL_Surface *SDLCALL SDL_GetVideoSurface(void);
extern DECLSPEC void SDLCALL SDL_UpdateRects(SDL_Surface * screen,
                                             int numrects, SDL_Rect * rects);
extern DECLSPEC void SDLCALL SDL_UpdateRect(SDL_Surface * screen,
                                            Sint32 x,
                                            Sint32 y, Uint32 w, Uint32 h);
extern DECLSPEC int SDLCALL SDL_Flip(SDL_Surface * screen);
extern DECLSPEC int SDLCALL SDL_SetAlpha(SDL_Surface * surface,
                                         Uint32 flag, Uint8 alpha);
extern DECLSPEC SDL_Surface *SDLCALL SDL_DisplayFormat(SDL_Surface * surface);
extern DECLSPEC SDL_Surface *SDLCALL SDL_DisplayFormatAlpha(SDL_Surface *
                                                            surface);
extern DECLSPEC void SDLCALL SDL_WM_SetCaption(const char *title,
                                               const char *icon);
extern DECLSPEC void SDLCALL SDL_WM_GetCaption(const char **title,
                                               const char **icon);
extern DECLSPEC void SDLCALL SDL_WM_SetIcon(SDL_Surface * icon, Uint8 * mask);
extern DECLSPEC int SDLCALL SDL_WM_IconifyWindow(void);
extern DECLSPEC int SDLCALL SDL_WM_ToggleFullScreen(SDL_Surface * surface);
extern DECLSPEC SDL_GrabMode SDLCALL SDL_WM_GrabInput(SDL_GrabMode mode);
extern DECLSPEC int SDLCALL SDL_SetPalette(SDL_Surface * surface,
                                           int flags,
                                           const SDL_Color * colors,
                                           int firstcolor, int ncolors);
extern DECLSPEC int SDLCALL SDL_SetColors(SDL_Surface * surface,
                                          const SDL_Color * colors,
                                          int firstcolor, int ncolors);
extern DECLSPEC int SDLCALL SDL_GetWMInfo(struct SDL_SysWMinfo *info);
extern DECLSPEC Uint8 SDLCALL SDL_GetAppState(void);
extern DECLSPEC void SDLCALL SDL_WarpMouse(Uint16 x, Uint16 y);
extern DECLSPEC SDL_Overlay *SDLCALL SDL_CreateYUVOverlay(int width,
                                                          int height,
                                                          Uint32 format,
                                                          SDL_Surface *
                                                          display);
extern DECLSPEC int SDLCALL SDL_LockYUVOverlay(SDL_Overlay * overlay);
extern DECLSPEC void SDLCALL SDL_UnlockYUVOverlay(SDL_Overlay * overlay);
extern DECLSPEC int SDLCALL SDL_DisplayYUVOverlay(SDL_Overlay * overlay,
                                                  SDL_Rect * dstrect);
extern DECLSPEC void SDLCALL SDL_FreeYUVOverlay(SDL_Overlay * overlay);
extern DECLSPEC void SDLCALL SDL_GL_SwapBuffers(void);
extern DECLSPEC int SDLCALL SDL_SetGamma(float red, float green, float blue);
extern DECLSPEC int SDLCALL SDL_SetGammaRamp(const Uint16 * red,
                                             const Uint16 * green,
                                             const Uint16 * blue);
extern DECLSPEC int SDLCALL SDL_GetGammaRamp(Uint16 * red, Uint16 * green,
                                             Uint16 * blue);
extern DECLSPEC int SDLCALL SDL_EnableKeyRepeat(int delay, int interval);
extern DECLSPEC void SDLCALL SDL_GetKeyRepeat(int *delay, int *interval);
extern DECLSPEC int SDLCALL SDL_EnableUNICODE(int enable);

typedef SDL_Window* SDL_WindowID;

#define SDL_KillThread(X)

/* The timeslice and timer resolution are no longer relevant */
#define SDL_TIMESLICE		10
#define TIMER_RESOLUTION	10

typedef Uint32 (SDLCALL * SDL_OldTimerCallback) (Uint32 interval);
extern DECLSPEC int SDLCALL SDL_SetTimer(Uint32 interval, SDL_OldTimerCallback callback);

extern DECLSPEC int SDLCALL SDL_putenv(const char *variable);

/*@}*//*Compatibility*/

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_compat_h */

/* vi: set ts=4 sw=4 expandtab: */
