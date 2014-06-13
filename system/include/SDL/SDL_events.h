/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  Portions of these headers taken from SDL2 (where noted)
  Copyright (C) 1997-2013 Sam Lantinga <slouken@libsdl.org>

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
 *  \file SDL_events.h
 *  
 *  Include file for SDL event handling.
 */

#ifndef _SDL_events_h
#define _SDL_events_h

#include "SDL_stdinc.h"
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_keyboard.h"
#include "SDL_mouse.h"
#include "SDL_joystick.h"
#include "SDL_quit.h"
#include "SDL_gesture.h"
#include "SDL_touch.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* General keyboard/mouse state definitions */
#define SDL_RELEASED	0
#define SDL_PRESSED	1

/**
 * \brief The types of events that can be delivered.
 */
typedef enum
{
    SDL_NOEVENT        = 0,
    SDL_FIRSTEVENT     = 0,     /**< Unused (do not remove) */

    /* Application events */
    SDL_QUIT           = 0x100, /**< User-requested quit */

    /* Window events */
    SDL_WINDOWEVENT    = 0x200, /**< Window state change */
    SDL_SYSWMEVENT,             /**< System specific event */

    /* Keyboard events */
    SDL_KEYDOWN        = 0x300, /**< Key pressed */
    SDL_KEYUP,                  /**< Key released */
    SDL_TEXTEDITING,            /**< Keyboard text editing (composition) */
    SDL_TEXTINPUT,              /**< Keyboard text input */

    /* Mouse events */
    SDL_MOUSEMOTION    = 0x400, /**< Mouse moved */
    SDL_MOUSEBUTTONDOWN,        /**< Mouse button pressed */
    SDL_MOUSEBUTTONUP,          /**< Mouse button released */
    SDL_MOUSEWHEEL,             /**< Mouse wheel motion */

    /* Tablet or multiple mice input device events */
    SDL_INPUTMOTION    = 0x500, /**< Input moved */
    SDL_INPUTBUTTONDOWN,        /**< Input button pressed */
    SDL_INPUTBUTTONUP,          /**< Input button released */
    SDL_INPUTWHEEL,             /**< Input wheel motion */
    SDL_INPUTPROXIMITYIN,       /**< Input pen entered proximity */
    SDL_INPUTPROXIMITYOUT,      /**< Input pen left proximity */

    /* Joystick events */
    SDL_JOYAXISMOTION  = 0x600, /**< Joystick axis motion */
    SDL_JOYBALLMOTION,          /**< Joystick trackball motion */
    SDL_JOYHATMOTION,           /**< Joystick hat position change */
    SDL_JOYBUTTONDOWN,          /**< Joystick button pressed */
    SDL_JOYBUTTONUP,            /**< Joystick button released */

    /* Touch events */
    SDL_FINGERDOWN      = 0x700,
    SDL_FINGERUP,
    SDL_FINGERMOTION,
    SDL_TOUCHBUTTONDOWN,
    SDL_TOUCHBUTTONUP,    

    /* Gesture events */
    SDL_DOLLARGESTURE   = 0x800,
    SDL_DOLLARRECORD,
    SDL_MULTIGESTURE,

    /* Clipboard events */

    SDL_CLIPBOARDUPDATE = 0x900, /**< The clipboard changed */

    /* Obsolete events */
    SDL_EVENT_COMPAT1 = 0x7000, /**< SDL 1.2 events for compatibility */
    SDL_EVENT_COMPAT2,
    SDL_EVENT_COMPAT3,


    /** Events ::SDL_USEREVENT through ::SDL_LASTEVENT are for your use,
     *  and should be allocated with SDL_RegisterEvents()
     */
    SDL_USEREVENT    = 0x8000,

    /**
     *  This last event is only for bounding internal arrays
     */
    SDL_LASTEVENT    = 0xFFFF
} SDL_EventType;

/**
 *  \brief Window state change event data (event.window.*)
 */
typedef struct SDL_WindowEvent
{
    Uint32 type;        /**< ::SDL_WINDOWEVENT */
    Uint32 windowID;    /**< The associated window */
    Uint8 event;        /**< ::SDL_WindowEventID */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    int data1;          /**< event dependent data */
    int data2;          /**< event dependent data */
} SDL_WindowEvent;

/**
 *  \brief Keyboard button event structure (event.key.*)
 */
typedef struct SDL_KeyboardEvent
{
    Uint32 type;        /**< ::SDL_KEYDOWN or ::SDL_KEYUP */
    Uint32 windowID;    /**< The window with keyboard focus, if any */
    Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
    Uint8 repeat;       /**< Non-zero if this is a key repeat */
    Uint8 padding2;
    Uint8 padding3;
    SDL_Keysym keysym;  /**< The key that was pressed or released */
} SDL_KeyboardEvent;

#define SDL_TEXTEDITINGEVENT_TEXT_SIZE (32)
/**
 *  \brief Keyboard text editing event structure (event.edit.*)
 */
typedef struct SDL_TextEditingEvent
{
    Uint32 type;                                /**< ::SDL_TEXTEDITING */
    Uint32 windowID;                            /**< The window with keyboard focus, if any */
    char text[SDL_TEXTEDITINGEVENT_TEXT_SIZE];  /**< The editing text */
    int start;                                  /**< The start cursor of selected editing text */
    int length;                                 /**< The length of selected editing text */
} SDL_TextEditingEvent;


#define SDL_TEXTINPUTEVENT_TEXT_SIZE (32)
/**
 *  \brief Keyboard text input event structure (event.text.*)
 */
typedef struct SDL_TextInputEvent
{
    Uint32 type;                              /**< ::SDL_TEXTINPUT */
    Uint32 windowID;                          /**< The window with keyboard focus, if any */
    char text[SDL_TEXTINPUTEVENT_TEXT_SIZE];  /**< The input text */
} SDL_TextInputEvent;

/**
 *  \brief Mouse motion event structure (event.motion.*)
 */
/*================================= IMPORTANT ================================
   The version of SDL_MouseMotionEvent that comes in these (emscripten)
   headers is taken from the finalized version of SDL2
  ============================================================================*/  

typedef struct SDL_MouseMotionEvent
{
    Uint32 type;        /**< ::SDL_MOUSEMOTION */
    Uint32 timestamp;
    Uint32 windowID;    /**< The window with mouse focus, if any */
    Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
    Uint32 state;       /**< The current button state */
    Sint32 x;           /**< X coordinate, relative to window */
    Sint32 y;           /**< Y coordinate, relative to window */
    Sint32 xrel;        /**< The relative motion in the X direction */
    Sint32 yrel;        /**< The relative motion in the Y direction */
} SDL_MouseMotionEvent;

/**
 *  \brief Mouse button event structure (event.button.*)
 */
/*================================= IMPORTANT ================================
   The version of SDL_MouseButtonEvent that comes in these (emscripten)
   headers is taken from the finalized version of SDL2
  ============================================================================*/  
typedef struct SDL_MouseButtonEvent
{
    Uint32 type;        /**< ::SDL_MOUSEBUTTONDOWN or ::SDL_MOUSEBUTTONUP */
    Uint32 timestamp;
    Uint32 windowID;    /**< The window with mouse focus, if any */
    Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
    Uint8 button;       /**< The mouse button index */
    Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
    Uint8 padding1;
    Uint8 padding2;
    Sint32 x;           /**< X coordinate, relative to window */
    Sint32 y;           /**< Y coordinate, relative to window */
} SDL_MouseButtonEvent;

/**
 *  \brief Mouse wheel event structure (event.wheel.*)
 */
typedef struct SDL_MouseWheelEvent
{
    Uint32 type;        /**< ::SDL_MOUSEWHEEL */
    Uint32 timestamp;
    Uint32 windowID;    /**< The window with mouse focus, if any */
    Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
    Sint32 x;           /**< The amount scrolled horizontally */
    Sint32 y;           /**< The amount scrolled vertically */
} SDL_MouseWheelEvent;

/**
 *  \brief Joystick axis motion event structure (event.jaxis.*)
 */
typedef struct SDL_JoyAxisEvent
{
    Uint32 type;        /**< ::SDL_JOYAXISMOTION */
    Uint8 which;        /**< The joystick device index */
    Uint8 axis;         /**< The joystick axis index */
    Uint8 padding1;
    Uint8 padding2;
    int value;          /**< The axis value (range: -32768 to 32767) */
} SDL_JoyAxisEvent;

/**
 *  \brief Joystick trackball motion event structure (event.jball.*)
 */
typedef struct SDL_JoyBallEvent
{
    Uint32 type;        /**< ::SDL_JOYBALLMOTION */
    Uint8 which;        /**< The joystick device index */
    Uint8 ball;         /**< The joystick trackball index */
    Uint8 padding1;
    Uint8 padding2;
    int xrel;           /**< The relative motion in the X direction */
    int yrel;           /**< The relative motion in the Y direction */
} SDL_JoyBallEvent;

/**
 *  \brief Joystick hat position change event structure (event.jhat.*)
 */
typedef struct SDL_JoyHatEvent
{
    Uint32 type;        /**< ::SDL_JOYHATMOTION */
    Uint8 which;        /**< The joystick device index */
    Uint8 hat;          /**< The joystick hat index */
    Uint8 value;        /**< The hat position value.
                         *   \sa ::SDL_HAT_LEFTUP ::SDL_HAT_UP ::SDL_HAT_RIGHTUP
                         *   \sa ::SDL_HAT_LEFT ::SDL_HAT_CENTERED ::SDL_HAT_RIGHT
                         *   \sa ::SDL_HAT_LEFTDOWN ::SDL_HAT_DOWN ::SDL_HAT_RIGHTDOWN
                         *   
                         *   Note that zero means the POV is centered.
                         */
    Uint8 padding1;
} SDL_JoyHatEvent;

/**
 *  \brief Joystick button event structure (event.jbutton.*)
 */
typedef struct SDL_JoyButtonEvent
{
    Uint32 type;        /**< ::SDL_JOYBUTTONDOWN or ::SDL_JOYBUTTONUP */
    Uint8 which;        /**< The joystick device index */
    Uint8 button;       /**< The joystick button index */
    Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
    Uint8 padding1;
} SDL_JoyButtonEvent;


/**
 *  \brief Touch finger motion/finger event structure (event.tfinger.*)
 */

/*================================= IMPORTANT ================================
   The version of SDL_TouchFingerEvent that comes in these (emscripten)
   headers is taken from the finalized version of SDL2
  ============================================================================*/  

typedef struct SDL_TouchFingerEvent
{
    Uint32 type;        /**< ::SDL_FINGERMOTION or ::SDL_FINGERDOWN or ::SDL_FINGERUP */
    Uint32 timestamp;
    SDL_TouchID touchId; /**< The touch device id */
    SDL_FingerID fingerId;
    float x;            /**< Normalized in the range 0...1 */
    float y;            /**< Normalized in the range 0...1 */
    float dx;           /**< Normalized in the range 0...1 */
    float dy;           /**< Normalized in the range 0...1 */
    float pressure;     /**< Normalized in the range 0...1 */
} SDL_TouchFingerEvent;


/**
 *  \brief Touch finger motion/finger event structure (event.tmotion.*)
 */
typedef struct SDL_TouchButtonEvent
{
    Uint32 type;        /**< ::SDL_TOUCHBUTTONUP OR SDL_TOUCHBUTTONDOWN */
    Uint32 windowID;    /**< The window with mouse focus, if any */
    SDL_TouchID touchId;        /**< The touch device index */
    Uint8 state;        /**< The current button state */
    Uint8 button;        /**< The button changing state */
    Uint8 padding1;
    Uint8 padding2;
} SDL_TouchButtonEvent;


/**
 *  \brief Multiple Finger Gesture Event (event.mgesture.*)
 */
typedef struct SDL_MultiGestureEvent
{
    Uint32 type;        /**< ::SDL_MULTIGESTURE */
    Uint32 windowID;    /**< The window with mouse focus, if any */
    SDL_TouchID touchId;        /**< The touch device index */
    float dTheta;
    float dDist;
    float x;  //currently 0...1. Change to screen coords?
    float y;  
    Uint16 numFingers;
    Uint16 padding;
} SDL_MultiGestureEvent;

/* (event.dgesture.*) */
typedef struct SDL_DollarGestureEvent
{
    Uint32 type;        /**< ::SDL_DOLLARGESTURE */
    Uint32 windowID;    /**< The window with mouse focus, if any */
    SDL_TouchID touchId;        /**< The touch device index */
    SDL_GestureID gestureId;
    Uint32 numFingers;
    float error;
  /*
    //TODO: Enable to give location?
    float x;  //currently 0...1. Change to screen coords?
    float y;  
  */
} SDL_DollarGestureEvent;


/**
 *  \brief The "quit requested" event
 */
typedef struct SDL_QuitEvent
{
    Uint32 type;        /**< ::SDL_QUIT */
} SDL_QuitEvent;


/**
 *  \brief A user-defined event type (event.user.*)
 */
typedef struct SDL_UserEvent
{
    Uint32 type;        /**< ::SDL_USEREVENT through ::SDL_NUMEVENTS-1 */
    Uint32 windowID;    /**< The associated window if any */
    int code;           /**< User defined event code */
    void *data1;        /**< User defined data pointer */
    void *data2;        /**< User defined data pointer */
} SDL_UserEvent;


struct SDL_SysWMmsg;
typedef struct SDL_SysWMmsg SDL_SysWMmsg;

/**
 *  \brief A video driver dependent system event (event.syswm.*)
 *  
 *  \note If you want to use this event, you should include SDL_syswm.h.
 */
typedef struct SDL_SysWMEvent
{
    Uint32 type;        /**< ::SDL_SYSWMEVENT */
    SDL_SysWMmsg *msg;  /**< driver dependent data, defined in SDL_syswm.h */
} SDL_SysWMEvent;

#ifndef SDL_NO_COMPAT
/**
 *  \addtogroup Compatibility 
 */
/*@{*/

/**
 *  \name Typedefs for backwards compatibility
 */
/*@{*/
typedef struct SDL_ActiveEvent
{
    Uint32 type;
    Uint8 gain;
    Uint8 state;
} SDL_ActiveEvent;

typedef struct SDL_ResizeEvent
{
    Uint32 type;
    int w;
    int h;
} SDL_ResizeEvent;
/*@}*/

/*@}*//*Compatibility*/
#endif

/**
 *  \brief General event structure
 */
typedef union SDL_Event
{
    Uint32 type;                    /**< Event type, shared with all events */
    SDL_WindowEvent window;         /**< Window event data */
    SDL_KeyboardEvent key;          /**< Keyboard event data */
    SDL_TextEditingEvent edit;      /**< Text editing event data */
    SDL_TextInputEvent text;        /**< Text input event data */
    SDL_MouseMotionEvent motion;    /**< Mouse motion event data */
    SDL_MouseButtonEvent button;    /**< Mouse button event data */
    SDL_MouseWheelEvent wheel;      /**< Mouse wheel event data */
    SDL_JoyAxisEvent jaxis;         /**< Joystick axis event data */
    SDL_JoyBallEvent jball;         /**< Joystick ball event data */
    SDL_JoyHatEvent jhat;           /**< Joystick hat event data */
    SDL_JoyButtonEvent jbutton;     /**< Joystick button event data */
    SDL_QuitEvent quit;             /**< Quit request event data */
    SDL_UserEvent user;             /**< Custom event data */
    SDL_SysWMEvent syswm;           /**< System dependent window event data */
    SDL_TouchFingerEvent tfinger;   /**< SDL2 Touch finger event data */
    SDL_TouchButtonEvent tbutton;   /**< Touch button event data */
    SDL_MultiGestureEvent mgesture; /**< Multi Finger Gesture data */
    SDL_DollarGestureEvent dgesture; /**< Multi Finger Gesture data */

    /** Temporarily here for backwards compatibility */
    /*@{*/
#ifndef SDL_NO_COMPAT
    SDL_ActiveEvent active;
    SDL_ResizeEvent resize;
#endif
    /*@}*/
} SDL_Event;


/* Function prototypes */

/**
 *  Pumps the event loop, gathering events from the input devices.
 *  
 *  This function updates the event queue and internal input device state.
 *  
 *  This should only be run in the thread that sets the video mode.
 */
extern DECLSPEC void SDLCALL SDL_PumpEvents(void);

/*@{*/
typedef enum
{
    SDL_ADDEVENT,
    SDL_PEEKEVENT,
    SDL_GETEVENT
} SDL_eventaction;

/**
 *  Checks the event queue for messages and optionally returns them.
 *  
 *  If \c action is ::SDL_ADDEVENT, up to \c numevents events will be added to
 *  the back of the event queue.
 *  
 *  If \c action is ::SDL_PEEKEVENT, up to \c numevents events at the front
 *  of the event queue, within the specified minimum and maximum type,
 *  will be returned and will not be removed from the queue.
 *  
 *  If \c action is ::SDL_GETEVENT, up to \c numevents events at the front 
 *  of the event queue, within the specified minimum and maximum type,
 *  will be returned and will be removed from the queue.
 *  
 *  \return The number of events actually stored, or -1 if there was an error.
 *  
 *  This function is thread-safe.
 */
extern DECLSPEC int SDLCALL SDL_PeepEvents(SDL_Event * events, int numevents,
                                           SDL_eventaction action,
                                           Uint32 minType, Uint32 maxType);
/*@}*/

/**
 *  Checks to see if certain event types are in the event queue.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_HasEvent(Uint32 type);
extern DECLSPEC SDL_bool SDLCALL SDL_HasEvents(Uint32 minType, Uint32 maxType);

/**
 *  This function clears events from the event queue
 */
extern DECLSPEC void SDLCALL SDL_FlushEvent(Uint32 type);
extern DECLSPEC void SDLCALL SDL_FlushEvents(Uint32 minType, Uint32 maxType);

/**
 *  \brief Polls for currently pending events.
 *  
 *  \return 1 if there are any pending events, or 0 if there are none available.
 *  
 *  \param event If not NULL, the next event is removed from the queue and 
 *               stored in that area.
 */
extern DECLSPEC int SDLCALL SDL_PollEvent(SDL_Event * event);

/**
 *  \brief Waits indefinitely for the next available event.
 *  
 *  \return 1, or 0 if there was an error while waiting for events.
 *   
 *  \param event If not NULL, the next event is removed from the queue and 
 *               stored in that area.
 */
extern DECLSPEC int SDLCALL SDL_WaitEvent(SDL_Event * event);

/**
 *  \brief Waits until the specified timeout (in milliseconds) for the next 
 *         available event.
 *  
 *  \return 1, or 0 if there was an error while waiting for events.
 *  
 *  \param event If not NULL, the next event is removed from the queue and 
 *               stored in that area.
 */
extern DECLSPEC int SDLCALL SDL_WaitEventTimeout(SDL_Event * event,
                                                 int timeout);

/**
 *  \brief Add an event to the event queue.
 *  
 *  \return 1 on success, 0 if the event was filtered, or -1 if the event queue 
 *          was full or there was some other error.
 */
extern DECLSPEC int SDLCALL SDL_PushEvent(SDL_Event * event);

typedef int (SDLCALL * SDL_EventFilter) (void *userdata, SDL_Event * event);

/**
 *  Sets up a filter to process all events before they change internal state and
 *  are posted to the internal event queue.
 *  
 *  The filter is protypted as:
 *  \code
 *      int SDL_EventFilter(void *userdata, SDL_Event * event);
 *  \endcode
 *
 *  If the filter returns 1, then the event will be added to the internal queue.
 *  If it returns 0, then the event will be dropped from the queue, but the 
 *  internal state will still be updated.  This allows selective filtering of
 *  dynamically arriving events.
 *  
 *  \warning  Be very careful of what you do in the event filter function, as 
 *            it may run in a different thread!
 *  
 *  There is one caveat when dealing with the ::SDL_QUITEVENT event type.  The
 *  event filter is only called when the window manager desires to close the
 *  application window.  If the event filter returns 1, then the window will
 *  be closed, otherwise the window will remain open if possible.
 *
 *  If the quit event is generated by an interrupt signal, it will bypass the
 *  internal queue and be delivered to the application at the next event poll.
 */
extern DECLSPEC void SDLCALL SDL_SetEventFilter(SDL_EventFilter filter,
                                                void *userdata);

/**
 *  Return the current event filter - can be used to "chain" filters.
 *  If there is no event filter set, this function returns SDL_FALSE.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_GetEventFilter(SDL_EventFilter * filter,
                                                    void **userdata);

/**
 *  Add a function which is called when an event is added to the queue.
 */
extern DECLSPEC void SDLCALL SDL_AddEventWatch(SDL_EventFilter filter,
                                               void *userdata);

/**
 *  Remove an event watch function added with SDL_AddEventWatch()
 */
extern DECLSPEC void SDLCALL SDL_DelEventWatch(SDL_EventFilter filter,
                                               void *userdata);

/**
 *  Run the filter function on the current event queue, removing any
 *  events for which the filter returns 0.
 */
extern DECLSPEC void SDLCALL SDL_FilterEvents(SDL_EventFilter filter,
                                              void *userdata);

/**
 *  An Emscripten-specific extension to SDL: Some browser APIs require that they are called from within an event handler function.
 *  Allow recording a callback that will be called for each received event. This is used in place of SDL_PollEvent.
 *  Your application will be called whenever there are events available.
 */
extern DECLSPEC void SDLCALL emscripten_SDL_SetEventHandler(SDL_EventFilter handler,
                                                            void *userdata);

/*@{*/
#define SDL_QUERY	-1
#define SDL_IGNORE	 0
#define SDL_DISABLE	 0
#define SDL_ENABLE	 1

/**
 *  This function allows you to set the state of processing certain events.
 *   - If \c state is set to ::SDL_IGNORE, that event will be automatically 
 *     dropped from the event queue and will not event be filtered.
 *   - If \c state is set to ::SDL_ENABLE, that event will be processed 
 *     normally.
 *   - If \c state is set to ::SDL_QUERY, SDL_EventState() will return the 
 *     current processing state of the specified event.
 */
extern DECLSPEC Uint8 SDLCALL SDL_EventState(Uint32 type, int state);
/*@}*/
#define SDL_GetEventState(type) SDL_EventState(type, SDL_QUERY)

/**
 *  This function allocates a set of user-defined events, and returns
 *  the beginning event number for that set of events.
 *
 *  If there aren't enough user-defined events left, this function
 *  returns (Uint32)-1
 */
extern DECLSPEC Uint32 SDLCALL SDL_RegisterEvents(int numevents);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_events_h */

/* vi: set ts=4 sw=4 expandtab: */
