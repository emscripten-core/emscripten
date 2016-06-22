#ifndef __emscripten_events_h__
#define __emscripten_events_h__

#ifdef __cplusplus
extern "C" {
#endif

/* This file defines Emscripten low-level glue bindings for interfacing with HTML5 APIs
 * 
 * Documentation for the public APIs defined in this file must be updated in: 
 *    site/source/docs/api_reference/html5.h.rst
 * A prebuilt local version of the documentation is available at: 
 *    site/build/text/docs/api_reference/html5.h.txt
 * You can also build docs locally as HTML or other formats in site/
 * An online HTML version (which may be of a different version of Emscripten)
 *    is up at http://kripken.github.io/emscripten-site/docs/api_reference/html5.h.html
 */

#define EMSCRIPTEN_EVENT_KEYPRESS               1
#define EMSCRIPTEN_EVENT_KEYDOWN                2
#define EMSCRIPTEN_EVENT_KEYUP                  3
#define EMSCRIPTEN_EVENT_CLICK                  4
#define EMSCRIPTEN_EVENT_MOUSEDOWN              5
#define EMSCRIPTEN_EVENT_MOUSEUP                6
#define EMSCRIPTEN_EVENT_DBLCLICK               7
#define EMSCRIPTEN_EVENT_MOUSEMOVE              8
#define EMSCRIPTEN_EVENT_WHEEL                  9
#define EMSCRIPTEN_EVENT_RESIZE                10
#define EMSCRIPTEN_EVENT_SCROLL                11
#define EMSCRIPTEN_EVENT_BLUR                  12
#define EMSCRIPTEN_EVENT_FOCUS                 13
#define EMSCRIPTEN_EVENT_FOCUSIN               14
#define EMSCRIPTEN_EVENT_FOCUSOUT              15
#define EMSCRIPTEN_EVENT_DEVICEORIENTATION     16
#define EMSCRIPTEN_EVENT_DEVICEMOTION          17
#define EMSCRIPTEN_EVENT_ORIENTATIONCHANGE     18
#define EMSCRIPTEN_EVENT_FULLSCREENCHANGE      19
#define EMSCRIPTEN_EVENT_POINTERLOCKCHANGE     20
#define EMSCRIPTEN_EVENT_VISIBILITYCHANGE      21
#define EMSCRIPTEN_EVENT_TOUCHSTART            22
#define EMSCRIPTEN_EVENT_TOUCHEND              23
#define EMSCRIPTEN_EVENT_TOUCHMOVE             24
#define EMSCRIPTEN_EVENT_TOUCHCANCEL           25
#define EMSCRIPTEN_EVENT_GAMEPADCONNECTED      26
#define EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED   27
#define EMSCRIPTEN_EVENT_BEFOREUNLOAD          28
#define EMSCRIPTEN_EVENT_BATTERYCHARGINGCHANGE 29
#define EMSCRIPTEN_EVENT_BATTERYLEVELCHANGE    30
#define EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST      31
#define EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED  32
#define EMSCRIPTEN_EVENT_MOUSEENTER            33
#define EMSCRIPTEN_EVENT_MOUSELEAVE            34
#define EMSCRIPTEN_EVENT_MOUSEOVER             35
#define EMSCRIPTEN_EVENT_MOUSEOUT              36
#define EMSCRIPTEN_EVENT_CANVASRESIZED         37
#define EMSCRIPTEN_EVENT_POINTERLOCKERROR      38

#define EMSCRIPTEN_RESULT int


#define EMSCRIPTEN_RESULT_SUCCESS              0
#define EMSCRIPTEN_RESULT_DEFERRED             1
#define EMSCRIPTEN_RESULT_NOT_SUPPORTED       -1
#define EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED -2
#define EMSCRIPTEN_RESULT_INVALID_TARGET      -3
#define EMSCRIPTEN_RESULT_UNKNOWN_TARGET      -4
#define EMSCRIPTEN_RESULT_INVALID_PARAM       -5
#define EMSCRIPTEN_RESULT_FAILED              -6
#define EMSCRIPTEN_RESULT_NO_DATA             -7

#define EM_BOOL int
#define EM_TRUE 1
#define EM_FALSE 0
#define EM_UTF8 char

#define DOM_KEY_LOCATION int
#define DOM_KEY_LOCATION_STANDARD 0x00
#define DOM_KEY_LOCATION_LEFT     0x01
#define DOM_KEY_LOCATION_RIGHT    0x02
#define DOM_KEY_LOCATION_NUMPAD   0x03


typedef struct EmscriptenKeyboardEvent {
  EM_UTF8 key[32];
  EM_UTF8 code[32];
  unsigned long location;
  EM_BOOL ctrlKey;
  EM_BOOL shiftKey;
  EM_BOOL altKey;
  EM_BOOL metaKey;
  EM_BOOL repeat;
  EM_UTF8 locale[32];
  EM_UTF8 charValue[32];
  unsigned long charCode;
  unsigned long keyCode;
  unsigned long which;
} EmscriptenKeyboardEvent;


typedef EM_BOOL (*em_key_callback_func)(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_keypress_callback(const char *target, void *userData, EM_BOOL useCapture, em_key_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_keydown_callback(const char *target, void *userData, EM_BOOL useCapture, em_key_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_keyup_callback(const char *target, void *userData, EM_BOOL useCapture, em_key_callback_func callback);


typedef struct EmscriptenMouseEvent {
  double timestamp;
  long screenX;
  long screenY;
  long clientX;
  long clientY;
  EM_BOOL ctrlKey;
  EM_BOOL shiftKey;
  EM_BOOL altKey;
  EM_BOOL metaKey;
  unsigned short button;
  unsigned short buttons;
  long movementX;
  long movementY;
  long targetX;
  long targetY;
  long canvasX;
  long canvasY;
  long padding;
} EmscriptenMouseEvent;


typedef EM_BOOL (*em_mouse_callback_func)(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_click_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_mousedown_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_mouseup_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_dblclick_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_mousemove_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_mouseenter_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_mouseleave_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_mouseover_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_mouseout_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback);

extern EMSCRIPTEN_RESULT emscripten_get_mouse_status(EmscriptenMouseEvent *mouseState);

#define DOM_DELTA_PIXEL 0x00
#define DOM_DELTA_LINE  0x01
#define DOM_DELTA_PAGE  0x02

typedef struct EmscriptenWheelEvent {
  EmscriptenMouseEvent mouse;
  double deltaX;
  double deltaY;
  double deltaZ;
  unsigned long deltaMode;
} EmscriptenWheelEvent;


typedef EM_BOOL (*em_wheel_callback_func)(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_wheel_callback(const char *target, void *userData, EM_BOOL useCapture, em_wheel_callback_func callback);

typedef struct EmscriptenUiEvent {
  long detail;
  int documentBodyClientWidth;
  int documentBodyClientHeight;
  int windowInnerWidth;
  int windowInnerHeight;
  int windowOuterWidth;
  int windowOuterHeight;
  int scrollTop;
  int scrollLeft;
} EmscriptenUiEvent;


typedef EM_BOOL (*em_ui_callback_func)(int eventType, const EmscriptenUiEvent *uiEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_resize_callback(const char *target, void *userData, EM_BOOL useCapture, em_ui_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_scroll_callback(const char *target, void *userData, EM_BOOL useCapture, em_ui_callback_func callback);

typedef struct EmscriptenFocusEvent {
  EM_UTF8 nodeName[128];
  EM_UTF8 id[128];
} EmscriptenFocusEvent;

typedef EM_BOOL (*em_focus_callback_func)(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_blur_callback(const char *target, void *userData, EM_BOOL useCapture, em_focus_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_focus_callback(const char *target, void *userData, EM_BOOL useCapture, em_focus_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_focusin_callback(const char *target, void *userData, EM_BOOL useCapture, em_focus_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_focusout_callback(const char *target, void *userData, EM_BOOL useCapture, em_focus_callback_func callback);

typedef struct EmscriptenDeviceOrientationEvent {
  double timestamp;
  double alpha;
  double beta;
  double gamma;
  EM_BOOL absolute;
} EmscriptenDeviceOrientationEvent;


typedef EM_BOOL (*em_deviceorientation_callback_func)(int eventType, const EmscriptenDeviceOrientationEvent *deviceOrientationEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_deviceorientation_callback(void *userData, EM_BOOL useCapture, em_deviceorientation_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_get_deviceorientation_status(EmscriptenDeviceOrientationEvent *orientationState);


typedef struct EmscriptenDeviceMotionEvent {
  double timestamp;
  double accelerationX;
  double accelerationY;
  double accelerationZ;
  double accelerationIncludingGravityX;
  double accelerationIncludingGravityY;
  double accelerationIncludingGravityZ;
  double rotationRateAlpha;
  double rotationRateBeta;
  double rotationRateGamma;
} EmscriptenDeviceMotionEvent;


typedef EM_BOOL (*em_devicemotion_callback_func)(int eventType, const EmscriptenDeviceMotionEvent *deviceMotionEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_devicemotion_callback(void *userData, EM_BOOL useCapture, em_devicemotion_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_get_devicemotion_status(EmscriptenDeviceMotionEvent *motionState);

#define EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY    1
#define EMSCRIPTEN_ORIENTATION_PORTRAIT_SECONDARY  2
#define EMSCRIPTEN_ORIENTATION_LANDSCAPE_PRIMARY   4
#define EMSCRIPTEN_ORIENTATION_LANDSCAPE_SECONDARY 8

typedef struct EmscriptenOrientationChangeEvent {
  int orientationIndex;
  int orientationAngle;
} EmscriptenOrientationChangeEvent;


typedef EM_BOOL (*em_orientationchange_callback_func)(int eventType, const EmscriptenOrientationChangeEvent *orientationChangeEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_orientationchange_callback(void *userData, EM_BOOL useCapture, em_orientationchange_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_get_orientation_status(EmscriptenOrientationChangeEvent *orientationStatus);
extern EMSCRIPTEN_RESULT emscripten_lock_orientation(int allowedOrientations);
extern EMSCRIPTEN_RESULT emscripten_unlock_orientation(void);

typedef struct EmscriptenFullscreenChangeEvent {
  EM_BOOL isFullscreen;
  EM_BOOL fullscreenEnabled;
  EM_UTF8 nodeName[128];
  EM_UTF8 id[128];
  int elementWidth;
  int elementHeight;
  int screenWidth;
  int screenHeight;
} EmscriptenFullscreenChangeEvent;


typedef EM_BOOL (*em_fullscreenchange_callback_func)(int eventType, const EmscriptenFullscreenChangeEvent *fullscreenChangeEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_fullscreenchange_callback(const char *target, void *userData, EM_BOOL useCapture, em_fullscreenchange_callback_func callback);

extern EMSCRIPTEN_RESULT emscripten_get_fullscreen_status(EmscriptenFullscreenChangeEvent *fullscreenStatus);

#define EMSCRIPTEN_FULLSCREEN_SCALE int
#define EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT 0
#define EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH 1
#define EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT  2
#define EMSCRIPTEN_FULLSCREEN_SCALE_CENTER  3

#define EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE int
#define EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE   0
#define EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF 1
#define EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF  2

#define EMSCRIPTEN_FULLSCREEN_FILTERING int
#define EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT 0
#define EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST 1
#define EMSCRIPTEN_FULLSCREEN_FILTERING_BILINEAR 2

typedef EM_BOOL (*em_canvasresized_callback_func)(int eventType, const void *reserved, void *userData);

typedef struct EmscriptenFullscreenStrategy {
  EMSCRIPTEN_FULLSCREEN_SCALE scaleMode;
  EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE canvasResolutionScaleMode;
  EMSCRIPTEN_FULLSCREEN_FILTERING filteringMode;
  em_canvasresized_callback_func canvasResizedCallback;
  void *canvasResizedCallbackUserData;
} EmscriptenFullscreenStrategy;

extern EMSCRIPTEN_RESULT emscripten_request_fullscreen(const char *target, EM_BOOL deferUntilInEventHandler);
extern EMSCRIPTEN_RESULT emscripten_request_fullscreen_strategy(const char *target, EM_BOOL deferUntilInEventHandler, const EmscriptenFullscreenStrategy *fullscreenStrategy);

extern EMSCRIPTEN_RESULT emscripten_exit_fullscreen(void);

extern EMSCRIPTEN_RESULT emscripten_enter_soft_fullscreen(const char *target, const EmscriptenFullscreenStrategy *fullscreenStrategy);

extern EMSCRIPTEN_RESULT emscripten_exit_soft_fullscreen(void);

typedef struct EmscriptenPointerlockChangeEvent {
  EM_BOOL isActive;
  EM_UTF8 nodeName[128];
  EM_UTF8 id[128];
} EmscriptenPointerlockChangeEvent;


typedef EM_BOOL (*em_pointerlockchange_callback_func)(int eventType, const EmscriptenPointerlockChangeEvent *pointerlockChangeEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_pointerlockchange_callback(const char *target, void *userData, EM_BOOL useCapture, em_pointerlockchange_callback_func callback);

typedef EM_BOOL (*em_pointerlockerror_callback_func)(int eventType, const void *reserved, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_pointerlockerror_callback(const char *target, void *userData, EM_BOOL useCapture, em_pointerlockerror_callback_func callback);

extern EMSCRIPTEN_RESULT emscripten_get_pointerlock_status(EmscriptenPointerlockChangeEvent *pointerlockStatus);

extern EMSCRIPTEN_RESULT emscripten_request_pointerlock(const char *target, EM_BOOL deferUntilInEventHandler);

extern EMSCRIPTEN_RESULT emscripten_exit_pointerlock(void);

#define EMSCRIPTEN_VISIBILITY_HIDDEN    0
#define EMSCRIPTEN_VISIBILITY_VISIBLE   1
#define EMSCRIPTEN_VISIBILITY_PRERENDER 2
#define EMSCRIPTEN_VISIBILITY_UNLOADED  3

typedef struct EmscriptenVisibilityChangeEvent {
  EM_BOOL hidden;
  int visibilityState;
} EmscriptenVisibilityChangeEvent;

typedef EM_BOOL (*em_visibilitychange_callback_func)(int eventType, const EmscriptenVisibilityChangeEvent *visibilityChangeEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_visibilitychange_callback(void *userData, EM_BOOL useCapture, em_visibilitychange_callback_func callback);

extern EMSCRIPTEN_RESULT emscripten_get_visibility_status(EmscriptenVisibilityChangeEvent *visibilityStatus);


typedef struct EmscriptenTouchPoint
{
  long identifier;
  long screenX;
  long screenY;
  long clientX;
  long clientY;
  long pageX;
  long pageY;
  EM_BOOL isChanged;
  EM_BOOL onTarget;
  long targetX;
  long targetY;
  long canvasX;
  long canvasY;
} EmscriptenTouchPoint;

typedef struct EmscriptenTouchEvent {
  int numTouches;
  EM_BOOL ctrlKey;
  EM_BOOL shiftKey;
  EM_BOOL altKey;
  EM_BOOL metaKey;
  EmscriptenTouchPoint touches[32];
} EmscriptenTouchEvent;


typedef EM_BOOL (*em_touch_callback_func)(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_touchstart_callback(const char *target, void *userData, EM_BOOL useCapture, em_touch_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_touchend_callback(const char *target, void *userData, EM_BOOL useCapture, em_touch_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_touchmove_callback(const char *target, void *userData, EM_BOOL useCapture, em_touch_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_touchcancel_callback(const char *target, void *userData, EM_BOOL useCapture, em_touch_callback_func callback);


typedef struct EmscriptenGamepadEvent {
  double timestamp;
  int numAxes;
  int numButtons;
  double axis[64];
  double analogButton[64];
  EM_BOOL digitalButton[64];
  EM_BOOL connected;
  long index;
  EM_UTF8 id[64];
  EM_UTF8 mapping[64];
} EmscriptenGamepadEvent;


typedef EM_BOOL (*em_gamepad_callback_func)(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_gamepadconnected_callback(void *userData, EM_BOOL useCapture, em_gamepad_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_gamepaddisconnected_callback(void *userData, EM_BOOL useCapture, em_gamepad_callback_func callback);

extern int emscripten_get_num_gamepads(void);
extern EMSCRIPTEN_RESULT emscripten_get_gamepad_status(int index, EmscriptenGamepadEvent *gamepadState);

typedef struct EmscriptenBatteryEvent {
  double chargingTime;
  double dischargingTime;
  double level;
  EM_BOOL charging;
} EmscriptenBatteryEvent;

typedef EM_BOOL (*em_battery_callback_func)(int eventType, const EmscriptenBatteryEvent *batteryEvent, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_batterychargingchange_callback(void *userData, em_battery_callback_func callback);
extern EMSCRIPTEN_RESULT emscripten_set_batterylevelchange_callback(void *userData, em_battery_callback_func callback);

extern EMSCRIPTEN_RESULT emscripten_get_battery_status(EmscriptenBatteryEvent *batteryState);


extern EMSCRIPTEN_RESULT emscripten_vibrate(int msecs);
extern EMSCRIPTEN_RESULT emscripten_vibrate_pattern(int *msecsArray, int numEntries);

typedef const char *(*em_beforeunload_callback)(int eventType, const void *reserved, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_beforeunload_callback(void *userData, em_beforeunload_callback callback);

typedef int EMSCRIPTEN_WEBGL_CONTEXT_HANDLE;

typedef struct EmscriptenWebGLContextAttributes {
  EM_BOOL alpha;
  EM_BOOL depth;
  EM_BOOL stencil;
  EM_BOOL antialias;
  EM_BOOL premultipliedAlpha;
  EM_BOOL preserveDrawingBuffer;
  EM_BOOL preferLowPowerToHighPerformance;
  EM_BOOL failIfMajorPerformanceCaveat;

  int majorVersion;
  int minorVersion;

  EM_BOOL enableExtensionsByDefault;
} EmscriptenWebGLContextAttributes;

extern void emscripten_webgl_init_context_attributes(EmscriptenWebGLContextAttributes *attributes);

extern EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_create_context(const char *target, const EmscriptenWebGLContextAttributes *attributes);

extern EMSCRIPTEN_RESULT emscripten_webgl_make_context_current(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

extern EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_get_current_context();

extern EMSCRIPTEN_RESULT emscripten_webgl_destroy_context(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

extern EM_BOOL emscripten_webgl_enable_extension(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, const char *extension);

typedef EM_BOOL (*em_webgl_context_callback)(int eventType, const void *reserved, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_webglcontextlost_callback(const char *target, void *userData, EM_BOOL useCapture, em_webgl_context_callback callback);
extern EMSCRIPTEN_RESULT emscripten_set_webglcontextrestored_callback(const char *target, void *userData, EM_BOOL useCapture, em_webgl_context_callback callback);

extern EM_BOOL emscripten_is_webgl_context_lost(const char *target);

extern EMSCRIPTEN_RESULT emscripten_set_element_css_size(const char *target, double width, double height);
extern EMSCRIPTEN_RESULT emscripten_get_element_css_size(const char *target, double *width, double *height);

#ifdef __cplusplus
} // ~extern "C"
#endif

#endif
