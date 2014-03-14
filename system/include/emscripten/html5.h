#ifndef __emscripten_events_h__
#define __emscripten_events_h__

#ifdef __cplusplus
extern "C" {
#endif

/* This file defines Emscripten low-level glue bindings for interfacing with the following HTML5 APIs:
 *    - DOM Level 3 Events: Keyboard, Mouse, Mouse Wheel, Resize, Scroll, Focus. See https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html
 *    - DeviceOrientation Events for gyro and accelerometer. See http://www.w3.org/TR/orientation-event/
 *    - Screen Orientation Events for portrait/landscape handling. See https://dvcs.w3.org/hg/screen-orientation/raw-file/tip/Overview.html
 *    - Fullscreen Events for browser canvas fullscreen modes transitioning. See https://dvcs.w3.org/hg/fullscreen/raw-file/tip/Overview.html
 *    - Pointer Lock Events for relative-mode mouse motion control. See http://www.w3.org/TR/pointerlock/
 *    - Vibration API for mobile device haptic vibration feedback control. See http://dev.w3.org/2009/dap/vibration/
 *    - Page Visibility Events for power management control. See http://www.w3c-test.org/webperf/specs/PageVisibility/
 *    - Touch Events. See http://www.w3.org/TR/touch-events/
 *    - Gamepad API. See http://www.w3.org/TR/gamepad/
 *    - Beforeunload event. See http://www.whatwg.org/specs/web-apps/current-work/multipage/history.html#beforeunloadevent
 *    - WebGL context events. See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
 *
 * Most web APIs are event-based, which means that the functionality is accessed by registering a callback function to be called when the event occurs. The
 * Gamepad API is currently an exception, for which only a polling API is available. For some APIs, both an event-based and a polling-based API is exposed.
 *
 * Calling a callback registration function with a null pointer function causes an unregistration of that callback from the given target element. All event
 * handlers are also automatically unregistered when the C exit() function is invoked during the atexit handler pass. Use either the function 
 * emscripten_set_main_loop() or set Module.noExitRuntime = true; to make sure that leaving main() will not immediately cause an exit() and clean up the
 * event handlers.
 *
 * Throughout this file, the function signatures have a 'target' parameter. This parameter allows specifying the HTML Element ID to which the callback
 * registration is to be applied to. This field has the following special meanings:
 *    - 0 or NULL: A default element is chosen automatically based on the event type, which should be reasonable most of the time.
 *    - "#window": The event listener is applied to the JS 'window' object.
 *    - "#document": The event listener is applied to the JS 'document' object.
 *    - "#screen": The event listener is applied to the JS 'window.screen' object.
 *    - "#canvas": The event listener is applied to the Emscripten default WebGL canvas element.
 *    - Any other string without a leading hash "#" sign: The event listener is applied to the element by the given ID on the page.
 *
 * The callback hook functions also take in a 'userData' parameter. This is a custom user-defineable value that will be carried through unchanged to all 
 * invocations of the registered event callback. Use this e.g. to pass a pointer to a C++ class or similar to enclose the C API in a clean object-oriented manner.
 *
 * Callback handlers that return an EM_BOOL may return nonzero to signal that the default action for that event is to be suppressed. This will call 
 * the .preventDefault(); member on the event. Returning zero will cause the default browser event action to be carried out.
 *
 * Most functions return the result using the type EMSCRIPTEN_RESULT. Nonzero and positive values denote success. Negative values
 * signal failure. None of the functions fail or abort by throwing a JS or C++ exception. If a particular browser does not support the given feature,
 * the value EMSCRIPTEN_RESULT_NOT_SUPPORTED will be returned at the time the callback is registered.
 *
 * Due to web security purposes, the pointer lock and fullscreen requests can only be invoked from inside an user-originated event handler. Such requests
 * are automatically deferred until the user presses a keyboard or mouse button the next time.
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

#define EMSCRIPTEN_RESULT int

// The operation succeeded
#define EMSCRIPTEN_RESULT_SUCCESS              0

// For web security reasons, the requested operation cannot be completed now, but was deferred for completion in the next event handler.
#define EMSCRIPTEN_RESULT_DEFERRED             1

// The given operation is not supported by this browser or the target element.
#define EMSCRIPTEN_RESULT_NOT_SUPPORTED       -1

// For web security reasons, the requested operation could not be completed now, and it failed since the user requested the operation to not be deferred.
#define EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED -2

// The given target element for the operation is invalid.
#define EMSCRIPTEN_RESULT_INVALID_TARGET      -3

// The given target element for the operation was not found.
#define EMSCRIPTEN_RESULT_UNKNOWN_TARGET      -4

// An invalid parameter was passed to the function.
#define EMSCRIPTEN_RESULT_INVALID_PARAM       -5

// The operation failed due to some generic reason.
#define EMSCRIPTEN_RESULT_FAILED              -6

// Operation failed since no data is currently available.
#define EMSCRIPTEN_RESULT_NO_DATA             -7

#define EM_BOOL int
#define EM_UTF8 char

#define DOM_KEY_LOCATION int
#define DOM_KEY_LOCATION_STANDARD 0x00
#define DOM_KEY_LOCATION_LEFT     0x01
#define DOM_KEY_LOCATION_RIGHT    0x02
#define DOM_KEY_LOCATION_NUMPAD   0x03

// Values of keyCode as used in some implementations (mainly Firefox).
// See the list at https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent#Virtual_key_codes
#define DOM_VK_CANCEL              0x03
#define DOM_VK_HELP                0x06
#define DOM_VK_BACK_SPACE          0x08
#define DOM_VK_TAB                 0x09
#define DOM_VK_CLEAR               0x0C
#define DOM_VK_RETURN              0x0D
#define DOM_VK_SHIFT               0x10
#define DOM_VK_CONTROL             0x11
#define DOM_VK_ALT                 0x12
#define DOM_VK_PAUSE               0x13
#define DOM_VK_CAPS_LOCK           0x14
#define DOM_VK_KANA                0x15
#define DOM_VK_HANGUL              0x15
#define DOM_VK_EISU                0x16
#define DOM_VK_JUNJA               0x17
#define DOM_VK_FINAL               0x18
#define DOM_VK_HANJA               0x19
#define DOM_VK_KANJI               0x19
#define DOM_VK_ESCAPE              0x1B
#define DOM_VK_CONVERT             0x1C
#define DOM_VK_NONCONVERT          0x1D
#define DOM_VK_ACCEPT              0x1E
#define DOM_VK_MODECHANGE          0x1F
#define DOM_VK_SPACE               0x20
#define DOM_VK_PAGE_UP             0x21
#define DOM_VK_PAGE_DOWN           0x22
#define DOM_VK_END                 0x23
#define DOM_VK_HOME                0x24
#define DOM_VK_LEFT                0x25
#define DOM_VK_UP                  0x26
#define DOM_VK_RIGHT               0x27
#define DOM_VK_DOWN                0x28
#define DOM_VK_SELECT              0x29
#define DOM_VK_PRINT               0x2A
#define DOM_VK_EXECUTE             0x2B
#define DOM_VK_PRINTSCREEN         0x2C
#define DOM_VK_INSERT              0x2D
#define DOM_VK_DELETE              0x2E
#define DOM_VK_0                   0x30
#define DOM_VK_1                   0x31
#define DOM_VK_2                   0x32
#define DOM_VK_3                   0x33
#define DOM_VK_4                   0x34
#define DOM_VK_5                   0x35
#define DOM_VK_6                   0x36
#define DOM_VK_7                   0x37
#define DOM_VK_8                   0x38
#define DOM_VK_9                   0x39
#define DOM_VK_COLON               0x3A
#define DOM_VK_SEMICOLON           0x3B
#define DOM_VK_LESS_THAN           0x3C
#define DOM_VK_EQUALS              0x3D
#define DOM_VK_GREATER_THAN        0x3E
#define DOM_VK_QUESTION_MARK       0x3F
#define DOM_VK_AT                  0x40
#define DOM_VK_A                   0x41
#define DOM_VK_B                   0x42
#define DOM_VK_C                   0x43
#define DOM_VK_D                   0x44
#define DOM_VK_E                   0x45
#define DOM_VK_F                   0x46
#define DOM_VK_G                   0x47
#define DOM_VK_H                   0x48
#define DOM_VK_I                   0x49
#define DOM_VK_J                   0x4A
#define DOM_VK_K                   0x4B
#define DOM_VK_L                   0x4C
#define DOM_VK_M                   0x4D
#define DOM_VK_N                   0x4E
#define DOM_VK_O                   0x4F
#define DOM_VK_P                   0x50
#define DOM_VK_Q                   0x51
#define DOM_VK_R                   0x52
#define DOM_VK_S                   0x53
#define DOM_VK_T                   0x54
#define DOM_VK_U                   0x55
#define DOM_VK_V                   0x56
#define DOM_VK_W                   0x57
#define DOM_VK_X                   0x58
#define DOM_VK_Y                   0x59
#define DOM_VK_Z                   0x5A
#define DOM_VK_WIN                 0x5B
#define DOM_VK_CONTEXT_MENU        0x5D
#define DOM_VK_SLEEP               0x5F
#define DOM_VK_NUMPAD0             0x60
#define DOM_VK_NUMPAD1             0x61
#define DOM_VK_NUMPAD2             0x62
#define DOM_VK_NUMPAD3             0x63
#define DOM_VK_NUMPAD4             0x64
#define DOM_VK_NUMPAD5             0x65
#define DOM_VK_NUMPAD6             0x66
#define DOM_VK_NUMPAD7             0x67
#define DOM_VK_NUMPAD8             0x68
#define DOM_VK_NUMPAD9             0x69
#define DOM_VK_MULTIPLY            0x6A
#define DOM_VK_ADD                 0x6B
#define DOM_VK_SEPARATOR           0x6C
#define DOM_VK_SUBTRACT            0x6D
#define DOM_VK_DECIMAL             0x6E
#define DOM_VK_DIVIDE              0x6F
#define DOM_VK_F1                  0x70
#define DOM_VK_F2                  0x71
#define DOM_VK_F3                  0x72
#define DOM_VK_F4                  0x73
#define DOM_VK_F5                  0x74
#define DOM_VK_F6                  0x75
#define DOM_VK_F7                  0x76
#define DOM_VK_F8                  0x77
#define DOM_VK_F9                  0x78
#define DOM_VK_F10                 0x79
#define DOM_VK_F11                 0x7A
#define DOM_VK_F12                 0x7B
#define DOM_VK_F13                 0x7C
#define DOM_VK_F14                 0x7D
#define DOM_VK_F15                 0x7E
#define DOM_VK_F16                 0x7F
#define DOM_VK_F17                 0x80
#define DOM_VK_F18                 0x81
#define DOM_VK_F19                 0x82
#define DOM_VK_F20                 0x83
#define DOM_VK_F21                 0x84
#define DOM_VK_F22                 0x85
#define DOM_VK_F23                 0x86
#define DOM_VK_F24                 0x87
#define DOM_VK_NUM_LOCK            0x90
#define DOM_VK_SCROLL_LOCK         0x91
#define DOM_VK_WIN_OEM_FJ_JISHO    0x92
#define DOM_VK_WIN_OEM_FJ_MASSHOU  0x93
#define DOM_VK_WIN_OEM_FJ_TOUROKU  0x94
#define DOM_VK_WIN_OEM_FJ_LOYA     0x95
#define DOM_VK_WIN_OEM_FJ_ROYA     0x96
#define DOM_VK_CIRCUMFLEX          0xA0
#define DOM_VK_EXCLAMATION         0xA1
#define DOM_VK_DOUBLE_QUOTE        0xA2
#define DOM_VK_HASH                0xA3
#define DOM_VK_DOLLAR              0xA4
#define DOM_VK_PERCENT             0xA5
#define DOM_VK_AMPERSAND           0xA6
#define DOM_VK_UNDERSCORE          0xA7
#define DOM_VK_OPEN_PAREN          0xA8
#define DOM_VK_CLOSE_PAREN         0xA9
#define DOM_VK_ASTERISK            0xAA
#define DOM_VK_PLUS                0xAB
#define DOM_VK_PIPE                0xAC
#define DOM_VK_HYPHEN_MINUS        0xAD
#define DOM_VK_OPEN_CURLY_BRACKET  0xAE
#define DOM_VK_CLOSE_CURLY_BRACKET 0xAF
#define DOM_VK_TILDE               0xB0
#define DOM_VK_VOLUME_MUTE         0xB5
#define DOM_VK_VOLUME_DOWN         0xB6
#define DOM_VK_VOLUME_UP           0xB7
#define DOM_VK_COMMA               0xBC
#define DOM_VK_PERIOD              0xBE
#define DOM_VK_SLASH               0xBF
#define DOM_VK_BACK_QUOTE          0xC0
#define DOM_VK_OPEN_BRACKET        0xDB
#define DOM_VK_BACK_SLASH          0xDC
#define DOM_VK_CLOSE_BRACKET       0xDD
#define DOM_VK_QUOTE               0xDE
#define DOM_VK_META                0xE0
#define DOM_VK_ALTGR               0xE1
#define DOM_VK_WIN_ICO_HELP        0xE3
#define DOM_VK_WIN_ICO_00          0xE4
#define DOM_VK_WIN_ICO_CLEAR       0xE6
#define DOM_VK_WIN_OEM_RESET       0xE9
#define DOM_VK_WIN_OEM_JUMP        0xEA
#define DOM_VK_WIN_OEM_PA1         0xEB
#define DOM_VK_WIN_OEM_PA2         0xEC
#define DOM_VK_WIN_OEM_PA3         0xED
#define DOM_VK_WIN_OEM_WSCTRL      0xEE
#define DOM_VK_WIN_OEM_CUSEL       0xEF
#define DOM_VK_WIN_OEM_ATTN        0xF0
#define DOM_VK_WIN_OEM_FINISH      0xF1
#define DOM_VK_WIN_OEM_COPY        0xF2
#define DOM_VK_WIN_OEM_AUTO        0xF3
#define DOM_VK_WIN_OEM_ENLW        0xF4
#define DOM_VK_WIN_OEM_BACKTAB     0xF5
#define DOM_VK_ATTN                0xF6
#define DOM_VK_CRSEL               0xF7
#define DOM_VK_EXSEL               0xF8
#define DOM_VK_EREOF               0xF9
#define DOM_VK_PLAY                0xFA
#define DOM_VK_ZOOM                0xFB
#define DOM_VK_PA1                 0xFD
#define DOM_VK_WIN_OEM_CLEAR       0xFE

/*
 * The event structure passed in keyboard events keypress, keydown and keyup.
 * https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#keys
 */
typedef struct EmscriptenKeyboardEvent {
  // The printed representation of the pressed key.
  EM_UTF8 key[32];
  // A string that identifies the physical key being pressed. The value is not affected by the current keyboard
  // layout or modifier state, so a particular key will always return the same value.
  EM_UTF8 code[32];
  // Indicates the location of the key on the keyboard. One of the DOM_KEY_LOCATION_ values.
  unsigned long location;
  // Specifies which modifiers were active during the key event.
  EM_BOOL ctrlKey;
  EM_BOOL shiftKey;
  EM_BOOL altKey;
  EM_BOOL metaKey;
  // Specifies if this keyboard event represents a repeated press.
  EM_BOOL repeat;
  // A locale string indicating the locale the keyboard is configured for. This may be the empty string if the
  // browser or device doesn't know the keyboard's locale.
  EM_UTF8 locale[32];
  // The following fields are values from previous versions of the DOM key events specifications.
  // See https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent?redirectlocale=en-US&redirectslug=DOM%2FKeyboardEvent
  // The character representation of the key.
  EM_UTF8 charValue[32];
  // The Unicode reference number of the key; this attribute is used only by the keypress event. For keys whose char attribute
  // contains multiple characters, this is the Unicode value of the first character in that attribute.
  unsigned long charCode;
  // A system and implementation dependent numerical code identifying the unmodified value of the pressed key. In older versions of the DOM spec, this was the primary means of extracting keyboard information. Support for for other members of this struct is still spotty in many browsers.
  unsigned long keyCode;
  // A system and implementation dependent numeric code identifying the unmodified value of the pressed key; this is usually the same as keyCode.
  unsigned long which;
} EmscriptenKeyboardEvent;

/*
 * Registers a callback function for receiving browser-generated keyboard input events.
 * See https://developer.mozilla.org/en/DOM/Event/UIEvent/KeyEvent
 * and http://www.javascriptkit.com/jsref/eventkeyboardmouse.shtml
 */
extern EMSCRIPTEN_RESULT emscripten_set_keypress_callback(const char *target, void *userData, int useCapture, int (*func)(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_keydown_callback(const char *target, void *userData, int useCapture, int (*func)(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_keyup_callback(const char *target, void *userData, int useCapture, int (*func)(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData));

/*
 * The event structure passed in mouse events click, mousedown, mouseup, dblclick and mousemove.
 * https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#interface-MouseEvent
 */
typedef struct EmscriptenMouseEvent {
  // A timestamp of when this data was generated by the browser. This is an absolute wallclock time in milliseconds.
  double timestamp;
  // The coordinate relative to the browser screen coordinate system.
  long screenX;
  long screenY;
  // The coordinate relative to the viewport associate with the event.
  long clientX;
  long clientY;
  // Specifies which modifiers were active during the mouse event.
  EM_BOOL ctrlKey;
  EM_BOOL shiftKey;
  EM_BOOL altKey;
  EM_BOOL metaKey;
  // Which pointer device button changed state.
  unsigned short button;
  // A bitmask that indicates which combinations of mouse buttons were being held down at the time of the event.
  unsigned short buttons;
  // If pointer lock is active, these two extra fields give relative mouse movement since the last event.
  long movementX;
  long movementY;
  // Emscripten-specific extension: These fields give the mouse coordinates mapped to the Emscripten canvas client area.
  long canvasX;
  long canvasY;
  // Pad this struct to multiple of 8 bytes to make WheelEvent unambiguously align to 8 bytes.
  long padding;
} EmscriptenMouseEvent;

/*
 * Registers a callback function for receiving browser-generated mouse input events.
 * See https://developer.mozilla.org/en/DOM/MouseEvent
 */
extern EMSCRIPTEN_RESULT emscripten_set_click_callback(const char *target, void *userData, int useCapture, int (*func)(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_mousedown_callback(const char *target, void *userData, int useCapture, int (*func)(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_mouseup_callback(const char *target, void *userData, int useCapture, int (*func)(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_dblclick_callback(const char *target, void *userData, int useCapture, int (*func)(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_mousemove_callback(const char *target, void *userData, int useCapture, int (*func)(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData));
/*
 * Returns the most recently received mouse event state. Note that for this function call to succeed, emscripten_set_xx_callback must have first
 * been called with one of the mouse event types and a non-zero callback function pointer to enable the Mouse state capture.
 */
extern EMSCRIPTEN_RESULT emscripten_get_mouse_status(EmscriptenMouseEvent *mouseState);

#define DOM_DELTA_PIXEL 0x00
#define DOM_DELTA_LINE  0x01
#define DOM_DELTA_PAGE  0x02

/*
 * The event structure passed in mouse wheelevent.
 * https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#interface-WheelEvent
 */
typedef struct EmscriptenWheelEvent {
  // Specifies general mouse information related to this event.
  EmscriptenMouseEvent mouse;
  // Measures along different axes the movement of the wheel.
  double deltaX;
  double deltaY;
  double deltaZ;
  // One of the DOM_DELTA_ values that indicates the units of measurement for the delta values.
  unsigned long deltaMode;
} EmscriptenWheelEvent;

/*
 * Registers a callback function for receiving browser-generated mouse wheel events.
 * See http://www.w3.org/TR/DOM-Level-3-Events/#event-type-wheel
 */
extern EMSCRIPTEN_RESULT emscripten_set_wheel_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData));

/*
 * The event structure passed in DOM element resize and scroll events.
 * https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#interface-UIEvent
 */
typedef struct EmscriptenUiEvent {
  // Specifies detail information about this event.
  long detail;
  // The clientWidth/clientHeight of the document.body element.
  int documentBodyClientWidth;
  int documentBodyClientHeight;
  // The innerWidth/innerHeight of the window element.
  int windowInnerWidth;
  int windowInnerHeight;
  // The outerWidth/outerHeight of the window element.
  int windowOuterWidth;
  int windowOuterHeight;
  // The page scroll position.
  int scrollTop;
  int scrollLeft;
} EmscriptenUiEvent;

/*
 * Registers a callback function for receiving DOM element resize and scroll events.
 * See https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-resize
 */
extern EMSCRIPTEN_RESULT emscripten_set_resize_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenUiEvent *uiEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_scroll_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenUiEvent *uiEvent, void *userData));

/*
 * The event structure passed in DOM element blur, focus, focusin and focusout events.
 * https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#interface-FocusEvent
 */
typedef struct EmscriptenFocusEvent {
  // The nodeName of the target HTML Element. See https://developer.mozilla.org/en-US/docs/Web/API/Node.nodeName
  EM_UTF8 nodeName[128];
  // The HTML Element ID of the target element.
  EM_UTF8 id[128];
} EmscriptenFocusEvent;

/*
 * Registers a callback function for receiving DOM element blur, focus, focusin and focusout events.
 * See https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-blur
 */
extern EMSCRIPTEN_RESULT emscripten_set_blur_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_focus_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_focusin_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_focusout_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData));

/*
 * The event structure passed in the deviceorientation event.
 * http://dev.w3.org/geo/api/spec-source-orientation.html#deviceorientation
 */
typedef struct EmscriptenDeviceOrientationEvent {
  // Absolute wallclock time in msec units of when the event occurred.
  double timestamp;
  // The orientation of the device in terms of the transformation from a coordinate frame fixed on the Earth to a coordinate frame fixed in the device. 
  double alpha;
  double beta;
  double gamma;
  // If false, the orientation is only relative to some other bas orinetation, not to the fixed coordinate frame.
  EM_BOOL absolute;
} EmscriptenDeviceOrientationEvent;

/*
 * Registers a callback function for receiving the deviceorientation event.
 * See http://dev.w3.org/geo/api/spec-source-orientation.html
 */
extern EMSCRIPTEN_RESULT emscripten_set_deviceorientation_callback(void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenDeviceOrientationEvent *orientationEvent, void *userData));
/*
 * Returns the most recently received deviceorientation event state. Note that for this function call to succeed, emscripten_set_deviceorientation_callback
 * must have first been called with one of the mouse event types and a non-zero callback function pointer to enable the Device Orientation state capture.
 */
extern EMSCRIPTEN_RESULT emscripten_get_deviceorientation_status(EmscriptenDeviceOrientationEvent *orientationState);

/*
 * The event structure passed in the devicemotion event.
 * http://dev.w3.org/geo/api/spec-source-orientation.html#devicemotion
 */
typedef struct EmscriptenDeviceMotionEvent {
  // Absolute wallclock time in msec units of when the event occurred.
  double timestamp;
  // Acceleration of the device excluding gravity.
  double accelerationX;
  double accelerationY;
  double accelerationZ;
  // Acceleration of the device including gravity.
  double accelerationIncludingGravityX;
  double accelerationIncludingGravityY;
  double accelerationIncludingGravityZ;
  // The rotational delta of the device.
  double rotationRateAlpha;
  double rotationRateBeta;
  double rotationRateGamma;
} EmscriptenDeviceMotionEvent;

/*
 * Registers a callback function for receiving the devicemotion event.
 * See http://dev.w3.org/geo/api/spec-source-orientation.html
 */
extern EMSCRIPTEN_RESULT emscripten_set_devicemotion_callback(void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenDeviceMotionEvent *motionEvent, void *userData));
/*
 * Returns the most recently received deviceomotion event state. Note that for this function call to succeed, emscripten_set_devicemotion_callback
 * must have first been called with one of the mouse event types and a non-zero callback function pointer to enable the Device Motion state capture.
 */
extern EMSCRIPTEN_RESULT emscripten_get_devicemotion_status(EmscriptenDeviceMotionEvent *motionState);

#define EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY    1
#define EMSCRIPTEN_ORIENTATION_PORTRAIT_SECONDARY  2
#define EMSCRIPTEN_ORIENTATION_LANDSCAPE_PRIMARY   4
#define EMSCRIPTEN_ORIENTATION_LANDSCAPE_SECONDARY 8

/*
 * The event structure passed in the orientationchange event.
 * https://dvcs.w3.org/hg/screen-orientation/raw-file/tip/Overview.html
 */
typedef struct EmscriptenOrientationChangeEvent {
  // One of EM_ORIENTATION_PORTRAIT_xx fields, or -1 if unknown.
  int orientationIndex;
  // Emscripten-specific extension: Some browsers refer to 'window.orientation', so report that as well.
  // Orientation angle in degrees. 0: "default orientation", i.e. default upright orientation to hold the mobile device in. Could be either landscape or portrait.
  int orientationAngle;
} EmscriptenOrientationChangeEvent;

/*
 * Registers a callback function for receiving the orientationchange event.
 * https://dvcs.w3.org/hg/screen-orientation/raw-file/tip/Overview.html
 */
extern EMSCRIPTEN_RESULT emscripten_set_orientationchange_callback(void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenOrientationChangeEvent *orientationChangeEvent, void *userData));
/*
 * Returns the current device orientation state.
 */
extern EMSCRIPTEN_RESULT emscripten_get_orientation_status(EmscriptenOrientationChangeEvent *orientationStatus);
/*
 * Locks the screen orientation to the given set of allowed orientations.
 * allowedOrientations: A bitfield set of EM_ORIENTATION_xx flags.
 */
extern EMSCRIPTEN_RESULT emscripten_lock_orientation(int allowedOrientations);
/*
 * Allows the screen to turn again into any orientation.
 */
extern EMSCRIPTEN_RESULT emscripten_unlock_orientation(void);

/*
 * The event structure passed in the fullscreenchange event.
 * https://dvcs.w3.org/hg/fullscreen/raw-file/tip/Overview.html
 */
typedef struct EmscriptenFullscreenChangeEvent {
  // Specifies whether an element on the browser page is currently fullscreen.
  EM_BOOL isFullscreen;
  // Specifies if the current page has the ability to display elements fullscreen.
  EM_BOOL fullscreenEnabled;
  // The nodeName of the target HTML Element that is in full screen mode. See https://developer.mozilla.org/en-US/docs/Web/API/Node.nodeName
  // If isFullscreen is false, then nodeName, id and elementWidth/Height specify information about the element that just exited fullscreen mode.
  EM_UTF8 nodeName[128];
  // The HTML Element ID of the target HTML element that is in full screen mode.
  EM_UTF8 id[128];
  // The new pixel size of the element that changed fullscreen status.
  int elementWidth;
  int elementHeight;
  // The size of the whole screen, in pixels.
  int screenWidth;
  int screenHeight;
} EmscriptenFullscreenChangeEvent;

/*
 * Registers a callback function for receiving the fullscreenchange event.
 * https://dvcs.w3.org/hg/screen-orientation/raw-file/tip/Overview.html
 */
extern EMSCRIPTEN_RESULT emscripten_set_fullscreenchange_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenFullscreenChangeEvent *fullscreenChangeEvent, void *userData));
/*
 * Returns the current page fullscreen state.
 */
extern EMSCRIPTEN_RESULT emscripten_get_fullscreen_status(EmscriptenFullscreenChangeEvent *fullscreenStatus);
/*
 * Requests the given target element to transition to full screen mode.
 * Note: This function can only run inside a user-generated JavaScript event handler.
 * deferUntilInEventHandler: If true and you called this function outside an event callback, this request will
 *                           be queued to be executed the next time a JS event handler runs. If false, this
 *                           function will instead fail if not running inside a JS event handler.
 */
extern EMSCRIPTEN_RESULT emscripten_request_fullscreen(const char *target, int deferUntilInEventHandler);
/*
 * Returns back to windowed browsing mode.
 */
extern EMSCRIPTEN_RESULT emscripten_exit_fullscreen(void);

/*
 * The event structure passed in the pointerlockchange event.
 * http://www.w3.org/TR/pointerlock/
 */
typedef struct EmscriptenPointerlockChangeEvent {
  // Specifies whether an element on the browser page currently has pointer lock enabled.
  EM_BOOL isActive;
  // The nodeName of the target HTML Element that has the pointer lock active. See https://developer.mozilla.org/en-US/docs/Web/API/Node.nodeName
  EM_UTF8 nodeName[128];
  // The HTML Element ID of the target HTML element that has the pointer lock active.
  EM_UTF8 id[128];
} EmscriptenPointerlockChangeEvent;

/*
 * Registers a callback function for receiving the pointerlockchange event.
 * Pointer lock hides the mouse cursor and exclusively gives the target element relative mouse movement events via the mousemove event.
 * http://www.w3.org/TR/pointerlock/
 */
extern EMSCRIPTEN_RESULT emscripten_set_pointerlockchange_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenPointerlockChangeEvent *pointerlockChangeEvent, void *userData));
/*
 * Returns the current page pointerlock state.
 */
extern EMSCRIPTEN_RESULT emscripten_get_pointerlock_status(EmscriptenPointerlockChangeEvent *pointerlockStatus);
/*
 * Requests the given target element to grab pointerlock.
 * Note: This function can only run inside a user-generated JavaScript event handler.
 * deferUntilInEventHandler: If true and you called this function outside an event callback, this request will
 *                           be queued to be executed the next time a JS event handler runs. If false, this
 *                           function will instead fail if not running inside a JS event handler.
 */
extern EMSCRIPTEN_RESULT emscripten_request_pointerlock(const char *target, int deferUntilInEventHandler);
/*
 * Exits pointer lock state and restores the mouse cursor to be visible again.
 */
extern EMSCRIPTEN_RESULT emscripten_exit_pointerlock(void);

#define EMSCRIPTEN_VISIBILITY_HIDDEN    0
#define EMSCRIPTEN_VISIBILITY_VISIBLE   1
#define EMSCRIPTEN_VISIBILITY_PRERENDER 2
#define EMSCRIPTEN_VISIBILITY_UNLOADED  3

/*
 * The event structure passed in the visibilitychange event.
 * http://www.w3c-test.org/webperf/specs/PageVisibility/
 */
typedef struct EmscriptenVisibilityChangeEvent {
  // If true, the current browser page is now hidden.
  EM_BOOL hidden;
  // Specifies a more fine-grained state of the current page visibility status. One of the EMSCRIPTEN_VISIBILITY_ values.
  int visibilityState;
} EmscriptenVisibilityChangeEvent;

/*
 * Registers a callback function for receiving the visibilitychange event.
 * http://www.w3c-test.org/webperf/specs/PageVisibility/
 */
extern EMSCRIPTEN_RESULT emscripten_set_visibilitychange_callback(void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenVisibilityChangeEvent *visibilityChangeEvent, void *userData));
/*
 * Returns the current page visibility state.
 */
extern EMSCRIPTEN_RESULT emscripten_get_visibility_status(EmscriptenVisibilityChangeEvent *visibilityStatus);

/*
 * Specifies the status of a single touch point on the page.
 * See http://www.w3.org/TR/touch-events/#touch-interface
 */
typedef struct EmscriptenTouchPoint
{
  // An identification number for each touch point.
  long identifier;
  // The touch coordinate relative to the whole screen origin, in pixels.
  long screenX;
  long screenY;
  // The touch coordinate relative to the viewport, in pixels.
  long clientX;
  long clientY;
  // The touch coordinate relative to the viewport, in pixels, and including any scroll offset.
  long pageX;
  long pageY;
  // Specifies whether this touch point changed during this event.
  EM_BOOL isChanged;
  // Specifies whether this touch point is still above the original target on which it was initially pressed against.
  EM_BOOL onTarget;
  // The touch coordinates mapped to the Emscripten canvas client area, in pixels.
  long canvasX;
  long canvasY;
} EmscriptenTouchPoint;

/*
 * Specifies the data of a single touch event.
 * See http://www.w3.org/TR/touch-events/#touchevent-interface
 */
typedef struct EmscriptenTouchEvent {
  // The number of valid elements in the touches array.
  int numTouches;
  // Specifies which modifiers were active during the key event.
  EM_BOOL ctrlKey;
  EM_BOOL shiftKey;
  EM_BOOL altKey;
  EM_BOOL metaKey;
  // An array of currently active touches, one for each finger.
  EmscriptenTouchPoint touches[32];
} EmscriptenTouchEvent;

/*
 * Registers a callback function for receiving the touchstart, touchend, touchmove and touchcancel events.
 * http://www.w3.org/TR/touch-events/
 */
extern EMSCRIPTEN_RESULT emscripten_set_touchstart_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_touchend_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_touchmove_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_touchcancel_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData));

/*
 * Represents the current snapshot state of a gamepad.
 * http://www.w3.org/TR/gamepad/#gamepad-interface
 */
typedef struct EmscriptenGamepadEvent {
  // Absolute wallclock time in msec units of when the data was recorded.
  double timestamp;
  // The number of valid axes entries in the axis array.
  int numAxes;
  // The number of valid button entries in the analogButton and digitalButton arrays.
  int numButtons;
  // The analog state of the gamepad axes, in the range [-1, 1].
  double axis[64];
  // The analog state of the gamepad buttons, in the range [0, 1].
  double analogButton[64];
  // The digital state of the gamepad buttons, either 0 or 1.
  EM_BOOL digitalButton[64];
  // Specifies whether this gamepad is connected to the browser page.
  EM_BOOL connected;
  // An ordinal associated with this gamepad, zero-based.
  long index;
  // An ID for the brand or style of the connected gamepad device. Typically, this will include the USB vendor and a product ID.
  EM_UTF8 id[64];
  // A string that identifies the layout or control mapping of this device.
  EM_UTF8 mapping[64];
} EmscriptenGamepadEvent;

/*
 * Registers a callback function for receiving the gamepadconnected and gamepaddisconnected events.
 * http://www.w3.org/TR/gamepad/
 */
extern EMSCRIPTEN_RESULT emscripten_set_gamepadconnected_callback(void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_gamepaddisconnected_callback(void *userData, int useCapture, EM_BOOL (*func)(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData));

/*
 * Returns the number of gamepads connected to the system or EMSCRIPTEN_RESULT_NOT_SUPPORTED if the current browser does not support gamepads.
 * Note: A gamepad does not show up as connected until a button on it is pressed.
 */
extern int emscripten_get_num_gamepads(void);
/*
 * Returns a snapshot of the current gamepad state.
 */
extern EMSCRIPTEN_RESULT emscripten_get_gamepad_status(int index, EmscriptenGamepadEvent *gamepadState);

/*
 * The event structure passed in the battery chargingchange and levelchange event.
 * http://www.w3.org/TR/battery-status/#batterymanager-interface
 */
typedef struct EmscriptenBatteryEvent {
  double chargingTime;
  double dischargingTime;
  double level;
  EM_BOOL charging;
} EmscriptenBatteryEvent;

/*
 * Registers a callback function for receiving the battery chargingchange and levelchange events.
 * http://www.w3.org/TR/battery-status/
 */
extern EMSCRIPTEN_RESULT emscripten_set_batterychargingchange_callback(void *userData, EM_BOOL (*func)(int eventType, const EmscriptenBatteryEvent *batteryEvent, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_batterylevelchange_callback(void *userData, EM_BOOL (*func)(int eventType, const EmscriptenBatteryEvent *batteryEvent, void *userData));
/*
 * Returns the current battery status.
 */
extern EMSCRIPTEN_RESULT emscripten_get_battery_status(EmscriptenBatteryEvent *batteryState);

/*
 * Produces a vibration feedback for given msecs.
 * http://dev.w3.org/2009/dap/vibration/
 */
extern EMSCRIPTEN_RESULT emscripten_vibrate(int msecs);
/*
 * Produces a complex vibration feedback pattern.
 * msecsArray: An array of timing entries [on, off, on, off, on, off, ...] where every second one specifies a duration of vibration, and 
 *             every other one specifies a duration of silence.
 * numEntries: The number of integers in the array msecsArray.
 */
extern EMSCRIPTEN_RESULT emscripten_vibrate_pattern(int *msecsArray, int numEntries);

/*
 * Registers a callback function for receiving the page beforeunload event.
 * Hook onto this event to perform process right prior to page close, and/or display a confirmation notification asking if the user really wants to leave the page.
 * http://www.whatwg.org/specs/web-apps/current-work/multipage/history.html#beforeunloadevent
 */
extern EMSCRIPTEN_RESULT emscripten_set_beforeunload_callback(void *userData, const char *(*func)(int eventType, const void *reserved, void *userData));

/*
 * Registers a callback function for the canvas webgl context webglcontextlost and webglcontextrestored events.
 * See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
 */
extern EMSCRIPTEN_RESULT emscripten_set_webglcontextlost_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const void *reserved, void *userData));
extern EMSCRIPTEN_RESULT emscripten_set_webglcontextrestored_callback(const char *target, void *userData, int useCapture, EM_BOOL (*func)(int eventType, const void *reserved, void *userData));

#ifdef __cplusplus
} // ~extern "C"
#endif

#endif
