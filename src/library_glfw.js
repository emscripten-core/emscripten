/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * EMSCRIPTEN GLFW 2.x-3.x emulation.
 * It tries to emulate the behavior described in
 * http://www.glfw.org/docs/latest/
 *
 * This also implements parts of GLFW 2.x on top of GLFW 3.x.
 *
 * What it does:
 * - Creates a GL context.
 * - Manage keyboard and mouse events.
 * - GL Extensions support.
 *
 * What it does not but should probably do:
 * - Transmit events when glfwPollEvents, glfwWaitEvents or glfwSwapBuffers is
 *    called. Events callbacks are called as soon as event are received.
 * - Input modes.
 * - Gamma ramps.
 * - Video modes.
 * - Monitors.
 * - Clipboard (not possible from javascript?).
 * - Multiple windows.
 * - Error codes && messages through callback.
 * - Thread emulation. (removed in GLFW3).
 * - Image/Texture I/O support (removed in GLFW 3).
 *
 * Authors:
 * - Jari Vetoniemi <mailroxas@gmail.com>
 * - Éloi Rivard <eloi.rivard@gmail.com>
 * - Thomas Borsos <thomasborsos@gmail.com>
 */

var LibraryGLFW = {
  $GLFW_Window__docs: '/** @constructor */',
  $GLFW_Window: function(id, width, height, framebufferWidth, framebufferHeight, title, monitor, share) {
      this.id = id;
      this.x = 0;
      this.y = 0;
      this.fullscreen = false; // Used to determine if app in fullscreen mode
      this.storedX = 0; // Used to store X before fullscreen
      this.storedY = 0; // Used to store Y before fullscreen
      this.width = width;
      this.height = height;
      this.framebufferWidth = framebufferWidth;
      this.framebufferHeight = framebufferHeight;
      this.storedWidth = width; // Used to store width before fullscreen
      this.storedHeight = height; // Used to store height before fullscreen
      this.title = title;
      this.monitor = monitor;
      this.share = share;
      this.attributes = Object.assign({}, GLFW.hints);
      this.inputModes = {
        0x00033001:0x00034001, // GLFW_CURSOR (GLFW_CURSOR_NORMAL)
        0x00033002:0, // GLFW_STICKY_KEYS
        0x00033003:0, // GLFW_STICKY_MOUSE_BUTTONS
      };
      this.buttons = 0;
      this.keys = new Array();
      this.domKeys = new Array();
      this.shouldClose = 0;
      this.title = null;
      this.windowPosFunc = 0; // GLFWwindowposfun
      this.windowSizeFunc = 0; // GLFWwindowsizefun
      this.windowCloseFunc = 0; // GLFWwindowclosefun
      this.windowRefreshFunc = 0; // GLFWwindowrefreshfun
      this.windowFocusFunc = 0; // GLFWwindowfocusfun
      this.windowIconifyFunc = 0; // GLFWwindowiconifyfun
      this.windowMaximizeFunc = 0; // GLFWwindowmaximizefun
      this.framebufferSizeFunc = 0; // GLFWframebuffersizefun
      this.windowContentScaleFunc = 0; // GLFWwindowcontentscalefun
      this.mouseButtonFunc = 0; // GLFWmousebuttonfun
      this.cursorPosFunc = 0; // GLFWcursorposfun
      this.cursorEnterFunc = 0; // GLFWcursorenterfun
      this.scrollFunc = 0; // GLFWscrollfun
      this.dropFunc = 0; // GLFWdropfun
      this.keyFunc = 0; // GLFWkeyfun
      this.charFunc = 0; // GLFWcharfun
      this.userptr = 0;
    },

  $GLFW__deps: ['emscripten_get_now', '$GL', '$Browser', '$GLFW_Window',
    '$MainLoop',
    '$stringToNewUTF8',
    'emscripten_set_window_title',
#if FILESYSTEM
    '$FS',
#endif
  ],
  $GLFW: {
    WindowFromId: (id) => {
      if (id <= 0 || !GLFW.windows) return null;
      return GLFW.windows[id - 1];
    },

    joystickFunc: 0, // GLFWjoystickfun
    errorFunc: 0, // GLFWerrorfun
    monitorFunc: 0, // GLFWmonitorfun
    active: null, // active window
    scale: null,
    windows: null,
    monitors: null,
    monitorString: null,
    versionString: null,
    initialTime: null,
    extensions: null,
    devicePixelRatioMQL: null, // MediaQueryList from window.matchMedia
    hints: null,
    primaryTouchId: null,
    defaultHints: {
      0x00020001:0, // GLFW_FOCUSED
      0x00020002:0, // GLFW_ICONIFIED
      0x00020003:1, // GLFW_RESIZABLE
      0x00020004:1, // GLFW_VISIBLE
      0x00020005:1, // GLFW_DECORATED
      0x0002000A:0, // GLFW_TRANSPARENT_FRAMEBUFFER
      0x0002200C:0, // GLFW_SCALE_TO_MONITOR

      0x00021001:8, // GLFW_RED_BITS
      0x00021002:8, // GLFW_GREEN_BITS
      0x00021003:8, // GLFW_BLUE_BITS
      0x00021004:8, // GLFW_ALPHA_BITS
      0x00021005:24, // GLFW_DEPTH_BITS
      0x00021006:8, // GLFW_STENCIL_BITS
      0x00021007:0, // GLFW_ACCUM_RED_BITS
      0x00021008:0, // GLFW_ACCUM_GREEN_BITS
      0x00021009:0, // GLFW_ACCUM_BLUE_BITS
      0x0002100A:0, // GLFW_ACCUM_ALPHA_BITS
      0x0002100B:0, // GLFW_AUX_BUFFERS
      0x0002100C:0, // GLFW_STEREO
      0x0002100D:0, // GLFW_SAMPLES
      0x0002100E:0, // GLFW_SRGB_CAPABLE
      0x0002100F:0, // GLFW_REFRESH_RATE

      0x00022001:0x00030001, // GLFW_CLIENT_API (GLFW_OPENGL_API)
      0x00022002:1, // GLFW_CONTEXT_VERSION_MAJOR
      0x00022003:0, // GLFW_CONTEXT_VERSION_MINOR
      0x00022004:0, // GLFW_CONTEXT_REVISION
      0x00022005:0, // GLFW_CONTEXT_ROBUSTNESS
      0x00022006:0, // GLFW_OPENGL_FORWARD_COMPAT
      0x00022007:0, // GLFW_OPENGL_DEBUG_CONTEXT
      0x00022008:0, // GLFW_OPENGL_PROFILE
    },

/*******************************************************************************
 * DOM EVENT CALLBACKS
 ******************************************************************************/

    /* https://developer.mozilla.org/en/Document_Object_Model_%28DOM%29/KeyboardEvent and GLFW/glfw3.h */
    DOMToGLFWKeyCode: (keycode) => {
      switch (keycode) {
        // these keycodes are only defined for GLFW3, assume they are the same for GLFW2
        case 0x20:return 32; // DOM_VK_SPACE -> GLFW_KEY_SPACE
        case 0xDE:return 39; // DOM_VK_QUOTE -> GLFW_KEY_APOSTROPHE
        case 0xBC:return 44; // DOM_VK_COMMA -> GLFW_KEY_COMMA
        case 0xAD:return 45; // DOM_VK_HYPHEN_MINUS -> GLFW_KEY_MINUS
        case 0xBD:return 45; // DOM_VK_MINUS -> GLFW_KEY_MINUS
        case 0xBE:return 46; // DOM_VK_PERIOD -> GLFW_KEY_PERIOD
        case 0xBF:return 47; // DOM_VK_SLASH -> GLFW_KEY_SLASH
        case 0x30:return 48; // DOM_VK_0 -> GLFW_KEY_0
        case 0x31:return 49; // DOM_VK_1 -> GLFW_KEY_1
        case 0x32:return 50; // DOM_VK_2 -> GLFW_KEY_2
        case 0x33:return 51; // DOM_VK_3 -> GLFW_KEY_3
        case 0x34:return 52; // DOM_VK_4 -> GLFW_KEY_4
        case 0x35:return 53; // DOM_VK_5 -> GLFW_KEY_5
        case 0x36:return 54; // DOM_VK_6 -> GLFW_KEY_6
        case 0x37:return 55; // DOM_VK_7 -> GLFW_KEY_7
        case 0x38:return 56; // DOM_VK_8 -> GLFW_KEY_8
        case 0x39:return 57; // DOM_VK_9 -> GLFW_KEY_9
        case 0x3B:return 59; // DOM_VK_SEMICOLON -> GLFW_KEY_SEMICOLON
        case 0x3D:return 61; // DOM_VK_EQUALS -> GLFW_KEY_EQUAL
        case 0xBB:return 61; // DOM_VK_EQUALS -> GLFW_KEY_EQUAL
        case 0x41:return 65; // DOM_VK_A -> GLFW_KEY_A
        case 0x42:return 66; // DOM_VK_B -> GLFW_KEY_B
        case 0x43:return 67; // DOM_VK_C -> GLFW_KEY_C
        case 0x44:return 68; // DOM_VK_D -> GLFW_KEY_D
        case 0x45:return 69; // DOM_VK_E -> GLFW_KEY_E
        case 0x46:return 70; // DOM_VK_F -> GLFW_KEY_F
        case 0x47:return 71; // DOM_VK_G -> GLFW_KEY_G
        case 0x48:return 72; // DOM_VK_H -> GLFW_KEY_H
        case 0x49:return 73; // DOM_VK_I -> GLFW_KEY_I
        case 0x4A:return 74; // DOM_VK_J -> GLFW_KEY_J
        case 0x4B:return 75; // DOM_VK_K -> GLFW_KEY_K
        case 0x4C:return 76; // DOM_VK_L -> GLFW_KEY_L
        case 0x4D:return 77; // DOM_VK_M -> GLFW_KEY_M
        case 0x4E:return 78; // DOM_VK_N -> GLFW_KEY_N
        case 0x4F:return 79; // DOM_VK_O -> GLFW_KEY_O
        case 0x50:return 80; // DOM_VK_P -> GLFW_KEY_P
        case 0x51:return 81; // DOM_VK_Q -> GLFW_KEY_Q
        case 0x52:return 82; // DOM_VK_R -> GLFW_KEY_R
        case 0x53:return 83; // DOM_VK_S -> GLFW_KEY_S
        case 0x54:return 84; // DOM_VK_T -> GLFW_KEY_T
        case 0x55:return 85; // DOM_VK_U -> GLFW_KEY_U
        case 0x56:return 86; // DOM_VK_V -> GLFW_KEY_V
        case 0x57:return 87; // DOM_VK_W -> GLFW_KEY_W
        case 0x58:return 88; // DOM_VK_X -> GLFW_KEY_X
        case 0x59:return 89; // DOM_VK_Y -> GLFW_KEY_Y
        case 0x5a:return 90; // DOM_VK_Z -> GLFW_KEY_Z
        case 0xDB:return 91; // DOM_VK_OPEN_BRACKET -> GLFW_KEY_LEFT_BRACKET
        case 0xDC:return 92; // DOM_VK_BACKSLASH -> GLFW_KEY_BACKSLASH
        case 0xDD:return 93; // DOM_VK_CLOSE_BRACKET -> GLFW_KEY_RIGHT_BRACKET
        case 0xC0:return 96; // DOM_VK_BACK_QUOTE -> GLFW_KEY_GRAVE_ACCENT

#if USE_GLFW == 2
        //#define GLFW_KEY_SPECIAL      256
        case 0x1B:return (256+1); // DOM_VK_ESCAPE -> GLFW_KEY_ESC
        case 0x70:return (256+2); // DOM_VK_F1 -> GLFW_KEY_F1
        case 0x71:return (256+3); // DOM_VK_F2 -> GLFW_KEY_F2
        case 0x72:return (256+4); // DOM_VK_F3 -> GLFW_KEY_F3
        case 0x73:return (256+5); // DOM_VK_F4 -> GLFW_KEY_F4
        case 0x74:return (256+6); // DOM_VK_F5 -> GLFW_KEY_F5
        case 0x75:return (256+7); // DOM_VK_F6 -> GLFW_KEY_F6
        case 0x76:return (256+8); // DOM_VK_F7 -> GLFW_KEY_F7
        case 0x77:return (256+9); // DOM_VK_F8 -> GLFW_KEY_F8
        case 0x78:return (256+10); // DOM_VK_F9 -> GLFW_KEY_F9
        case 0x79:return (256+11); // DOM_VK_F10 -> GLFW_KEY_F10
        case 0x7A:return (256+12); // DOM_VK_F11 -> GLFW_KEY_F11
        case 0x7B:return (256+13); // DOM_VK_F12 -> GLFW_KEY_F12
        case 0x7C:return (256+14); // DOM_VK_F13 -> GLFW_KEY_F13
        case 0x7D:return (256+15); // DOM_VK_F14 -> GLFW_KEY_F14
        case 0x7E:return (256+16); // DOM_VK_F15 -> GLFW_KEY_F15
        case 0x7F:return (256+17); // DOM_VK_F16 -> GLFW_KEY_F16
        case 0x80:return (256+18); // DOM_VK_F17 -> GLFW_KEY_F17
        case 0x81:return (256+19); // DOM_VK_F18 -> GLFW_KEY_F18
        case 0x82:return (256+20); // DOM_VK_F19 -> GLFW_KEY_F19
        case 0x83:return (256+21); // DOM_VK_F20 -> GLFW_KEY_F20
        case 0x84:return (256+22); // DOM_VK_F21 -> GLFW_KEY_F21
        case 0x85:return (256+23); // DOM_VK_F22 -> GLFW_KEY_F22
        case 0x86:return (256+24); // DOM_VK_F23 -> GLFW_KEY_F23
        case 0x87:return (256+25); // DOM_VK_F24 -> GLFW_KEY_F24
        case 0x88:return (256+26); // 0x88 (not used?) -> GLFW_KEY_F25
        case 0x27:return (256+30); // DOM_VK_RIGHT -> GLFW_KEY_RIGHT
        case 0x25:return (256+29); // DOM_VK_LEFT -> GLFW_KEY_LEFT
        case 0x28:return (256+28); // DOM_VK_DOWN -> GLFW_KEY_DOWN
        case 0x26:return (256+27); // DOM_VK_UP -> GLFW_KEY_UP
        case 0x10:return (256+31); // DOM_VK_SHIFT -> GLFW_KEY_LSHIFT
        // #define GLFW_KEY_RSHIFT       (GLFW_KEY_SPECIAL+32)
        case 0x11:return (256+33); // DOM_VK_CONTROL -> GLFW_KEY_LCTRL
        // #define GLFW_KEY_RCTRL        (GLFW_KEY_SPECIAL+34)
        case 0x12:return (256+35); // DOM_VK_ALT -> GLFW_KEY_LALT
        // #define GLFW_KEY_RALT         (GLFW_KEY_SPECIAL+36)
        case 0x09:return (256+37); // DOM_VK_TAB -> GLFW_KEY_TAB
        case 0x0D:return (256+38); // DOM_VK_RETURN -> GLFW_KEY_ENTER
        case 0x08:return (256+39); // DOM_VK_BACK -> GLFW_KEY_BACKSPACE
        case 0x2D:return (256+40); // DOM_VK_INSERT -> GLFW_KEY_INSERT
        case 0x2E:return (256+41); // DOM_VK_DELETE -> GLFW_KEY_DEL
        case 0x21:return (256+42); // DOM_VK_PAGE_UP -> GLFW_KEY_PAGEUP
        case 0x22:return (256+43); // DOM_VK_PAGE_DOWN -> GLFW_KEY_PAGEDOWN
        case 0x24:return (256+44); // DOM_VK_HOME -> GLFW_KEY_HOME
        case 0x23:return (256+45); // DOM_VK_END -> GLFW_KEY_END
        case 0x60:return (256+46); // DOM_VK_NUMPAD0 -> GLFW_KEY_KP_0
        case 0x61:return (256+47); // DOM_VK_NUMPAD1 -> GLFW_KEY_KP_1
        case 0x62:return (256+48); // DOM_VK_NUMPAD2 -> GLFW_KEY_KP_2
        case 0x63:return (256+49); // DOM_VK_NUMPAD3 -> GLFW_KEY_KP_3
        case 0x64:return (256+50); // DOM_VK_NUMPAD4 -> GLFW_KEY_KP_4
        case 0x65:return (256+51); // DOM_VK_NUMPAD5 -> GLFW_KEY_KP_5
        case 0x66:return (256+52); // DOM_VK_NUMPAD6 -> GLFW_KEY_KP_6
        case 0x67:return (256+53); // DOM_VK_NUMPAD7 -> GLFW_KEY_KP_7
        case 0x68:return (256+54); // DOM_VK_NUMPAD8 -> GLFW_KEY_KP_8
        case 0x69:return (256+55); // DOM_VK_NUMPAD9 -> GLFW_KEY_KP_9
        case 0x6F:return (256+56); // DOM_VK_DIVIDE -> GLFW_KEY_KP_DIVIDE
        case 0x6A:return (256+57); // DOM_VK_MULTIPLY -> GLFW_KEY_KP_MULTIPLY
        case 0x6D:return (256+58); // DOM_VK_SUBTRACT -> GLFW_KEY_KP_SUBTRACT
        case 0x6B:return (256+59); // DOM_VK_ADD -> GLFW_KEY_KP_ADD
        case 0x6E:return (256+60); // DOM_VK_DECIMAL -> GLFW_KEY_KP_DECIMAL
        // #define GLFW_KEY_KP_EQUAL     (GLFW_KEY_SPECIAL+61)
        // #define GLFW_KEY_KP_ENTER     (GLFW_KEY_SPECIAL+62)
        case 0x90:return (256+63); // DOM_VK_NUM_LOCK -> GLFW_KEY_KP_NUM_LOCK
        case 0x14:return (256+64); // DOM_VK_CAPS_LOCK -> GLFW_KEY_CAPS_LOCK
        case 0x91:return (256+65); // DOM_VK_SCROLL_LOCK -> GLFW_KEY_SCROLL_LOCK
        case 0x13:return (256+66); // DOM_VK_PAUSE -> GLFW_KEY_PAUSE
        case 0x5B:return (256+67); // DOM_VK_WIN -> GLFW_KEY_LSUPER
        // #define GLFW_KEY_RSUPER       (GLFW_KEY_SPECIAL+68)
        case 0x5D:return (256+69); // DOM_VK_CONTEXT_MENU -> GLFW_KEY_MENU
#endif

#if USE_GLFW == 3
        case 0x1B:return 256; // DOM_VK_ESCAPE -> GLFW_KEY_ESCAPE
        case 0x0D:return 257; // DOM_VK_RETURN -> GLFW_KEY_ENTER
        case 0x09:return 258; // DOM_VK_TAB -> GLFW_KEY_TAB
        case 0x08:return 259; // DOM_VK_BACK -> GLFW_KEY_BACKSPACE
        case 0x2D:return 260; // DOM_VK_INSERT -> GLFW_KEY_INSERT
        case 0x2E:return 261; // DOM_VK_DELETE -> GLFW_KEY_DELETE
        case 0x27:return 262; // DOM_VK_RIGHT -> GLFW_KEY_RIGHT
        case 0x25:return 263; // DOM_VK_LEFT -> GLFW_KEY_LEFT
        case 0x28:return 264; // DOM_VK_DOWN -> GLFW_KEY_DOWN
        case 0x26:return 265; // DOM_VK_UP -> GLFW_KEY_UP
        case 0x21:return 266; // DOM_VK_PAGE_UP -> GLFW_KEY_PAGE_UP
        case 0x22:return 267; // DOM_VK_PAGE_DOWN -> GLFW_KEY_PAGE_DOWN
        case 0x24:return 268; // DOM_VK_HOME -> GLFW_KEY_HOME
        case 0x23:return 269; // DOM_VK_END -> GLFW_KEY_END
        case 0x14:return 280; // DOM_VK_CAPS_LOCK -> GLFW_KEY_CAPS_LOCK
        case 0x91:return 281; // DOM_VK_SCROLL_LOCK -> GLFW_KEY_SCROLL_LOCK
        case 0x90:return 282; // DOM_VK_NUM_LOCK -> GLFW_KEY_NUM_LOCK
        case 0x2C:return 283; // DOM_VK_SNAPSHOT -> GLFW_KEY_PRINT_SCREEN
        case 0x13:return 284; // DOM_VK_PAUSE -> GLFW_KEY_PAUSE
        case 0x70:return 290; // DOM_VK_F1 -> GLFW_KEY_F1
        case 0x71:return 291; // DOM_VK_F2 -> GLFW_KEY_F2
        case 0x72:return 292; // DOM_VK_F3 -> GLFW_KEY_F3
        case 0x73:return 293; // DOM_VK_F4 -> GLFW_KEY_F4
        case 0x74:return 294; // DOM_VK_F5 -> GLFW_KEY_F5
        case 0x75:return 295; // DOM_VK_F6 -> GLFW_KEY_F6
        case 0x76:return 296; // DOM_VK_F7 -> GLFW_KEY_F7
        case 0x77:return 297; // DOM_VK_F8 -> GLFW_KEY_F8
        case 0x78:return 298; // DOM_VK_F9 -> GLFW_KEY_F9
        case 0x79:return 299; // DOM_VK_F10 -> GLFW_KEY_F10
        case 0x7A:return 300; // DOM_VK_F11 -> GLFW_KEY_F11
        case 0x7B:return 301; // DOM_VK_F12 -> GLFW_KEY_F12
        case 0x7C:return 302; // DOM_VK_F13 -> GLFW_KEY_F13
        case 0x7D:return 303; // DOM_VK_F14 -> GLFW_KEY_F14
        case 0x7E:return 304; // DOM_VK_F15 -> GLFW_KEY_F15
        case 0x7F:return 305; // DOM_VK_F16 -> GLFW_KEY_F16
        case 0x80:return 306; // DOM_VK_F17 -> GLFW_KEY_F17
        case 0x81:return 307; // DOM_VK_F18 -> GLFW_KEY_F18
        case 0x82:return 308; // DOM_VK_F19 -> GLFW_KEY_F19
        case 0x83:return 309; // DOM_VK_F20 -> GLFW_KEY_F20
        case 0x84:return 310; // DOM_VK_F21 -> GLFW_KEY_F21
        case 0x85:return 311; // DOM_VK_F22 -> GLFW_KEY_F22
        case 0x86:return 312; // DOM_VK_F23 -> GLFW_KEY_F23
        case 0x87:return 313; // DOM_VK_F24 -> GLFW_KEY_F24
        case 0x88:return 314; // 0x88 (not used?) -> GLFW_KEY_F25
        case 0x60:return 320; // DOM_VK_NUMPAD0 -> GLFW_KEY_KP_0
        case 0x61:return 321; // DOM_VK_NUMPAD1 -> GLFW_KEY_KP_1
        case 0x62:return 322; // DOM_VK_NUMPAD2 -> GLFW_KEY_KP_2
        case 0x63:return 323; // DOM_VK_NUMPAD3 -> GLFW_KEY_KP_3
        case 0x64:return 324; // DOM_VK_NUMPAD4 -> GLFW_KEY_KP_4
        case 0x65:return 325; // DOM_VK_NUMPAD5 -> GLFW_KEY_KP_5
        case 0x66:return 326; // DOM_VK_NUMPAD6 -> GLFW_KEY_KP_6
        case 0x67:return 327; // DOM_VK_NUMPAD7 -> GLFW_KEY_KP_7
        case 0x68:return 328; // DOM_VK_NUMPAD8 -> GLFW_KEY_KP_8
        case 0x69:return 329; // DOM_VK_NUMPAD9 -> GLFW_KEY_KP_9
        case 0x6E:return 330; // DOM_VK_DECIMAL -> GLFW_KEY_KP_DECIMAL
        case 0x6F:return 331; // DOM_VK_DIVIDE -> GLFW_KEY_KP_DIVIDE
        case 0x6A:return 332; // DOM_VK_MULTIPLY -> GLFW_KEY_KP_MULTIPLY
        case 0x6D:return 333; // DOM_VK_SUBTRACT -> GLFW_KEY_KP_SUBTRACT
        case 0x6B:return 334; // DOM_VK_ADD -> GLFW_KEY_KP_ADD
        // case 0x0D:return 335; // DOM_VK_RETURN -> GLFW_KEY_KP_ENTER (DOM_KEY_LOCATION_RIGHT)
        // case 0x61:return 336; // DOM_VK_EQUALS -> GLFW_KEY_KP_EQUAL (DOM_KEY_LOCATION_RIGHT)
        case 0x10:return 340; // DOM_VK_SHIFT -> GLFW_KEY_LEFT_SHIFT
        case 0x11:return 341; // DOM_VK_CONTROL -> GLFW_KEY_LEFT_CONTROL
        case 0x12:return 342; // DOM_VK_ALT -> GLFW_KEY_LEFT_ALT
        case 0x5B:return 343; // DOM_VK_WIN -> GLFW_KEY_LEFT_SUPER
        case 0xE0:return 343; // DOM_VK_META -> GLFW_KEY_LEFT_SUPER
        // case 0x10:return 344; // DOM_VK_SHIFT -> GLFW_KEY_RIGHT_SHIFT (DOM_KEY_LOCATION_RIGHT)
        // case 0x11:return 345; // DOM_VK_CONTROL -> GLFW_KEY_RIGHT_CONTROL (DOM_KEY_LOCATION_RIGHT)
        // case 0x12:return 346; // DOM_VK_ALT -> GLFW_KEY_RIGHT_ALT (DOM_KEY_LOCATION_RIGHT)
        // case 0x5B:return 347; // DOM_VK_WIN -> GLFW_KEY_RIGHT_SUPER (DOM_KEY_LOCATION_RIGHT)
        case 0x5D:return 348; // DOM_VK_CONTEXT_MENU -> GLFW_KEY_MENU
        // XXX: GLFW_KEY_WORLD_1, GLFW_KEY_WORLD_2 what are these?
#endif
        default:return -1; // GLFW_KEY_UNKNOWN
      };
    },

    getModBits: (win) => {
      var mod = 0;
      if (win.keys[340]) mod |= 0x0001; // GLFW_MOD_SHIFT
      if (win.keys[341]) mod |= 0x0002; // GLFW_MOD_CONTROL
      if (win.keys[342]) mod |= 0x0004; // GLFW_MOD_ALT
      if (win.keys[343] || win.keys[348]) mod |= 0x0008; // GLFW_MOD_SUPER
      // add caps and num lock keys? only if lock_key_mod is set
      return mod;
    },

    onKeyPress: (event) => {
      if (!GLFW.active || !GLFW.active.charFunc) return;
      if (event.ctrlKey || event.metaKey) return;

      // correct unicode charCode is only available with onKeyPress event
      var charCode = event.charCode;
      if (charCode == 0 || (charCode >= 0x00 && charCode <= 0x1F)) return;

#if USE_GLFW == 2
      {{{ makeDynCall('vii', 'GLFW.active.charFunc') }}}(charCode, 1);
#endif
#if USE_GLFW == 3
      {{{ makeDynCall('vpi', 'GLFW.active.charFunc') }}}(GLFW.active.id, charCode);
#endif
    },

    onKeyChanged: (keyCode, status) => {
      if (!GLFW.active) return;

      var key = GLFW.DOMToGLFWKeyCode(keyCode);
      if (key == -1) return;

#if USE_GLFW == 3
      var repeat = status && GLFW.active.keys[key];
#endif
      GLFW.active.keys[key] = status;
      GLFW.active.domKeys[keyCode] = status;

      if (GLFW.active.keyFunc) {
#if USE_GLFW == 2
        {{{ makeDynCall('vii', 'GLFW.active.keyFunc') }}}(key, status);
#endif
#if USE_GLFW == 3
        if (repeat) status = 2; // GLFW_REPEAT
        {{{ makeDynCall('vpiiii', 'GLFW.active.keyFunc') }}}(GLFW.active.id, key, keyCode, status, GLFW.getModBits(GLFW.active));
#endif
      }
    },

    onGamepadConnected: (event) => {
      GLFW.refreshJoysticks();
    },

    onGamepadDisconnected: (event) => {
      GLFW.refreshJoysticks();
    },

    onKeydown: (event) => {
      GLFW.onKeyChanged(event.keyCode, 1); // GLFW_PRESS or GLFW_REPEAT

      // This logic comes directly from the sdl implementation. We cannot
      // call preventDefault on all keydown events otherwise onKeyPress will
      // not get called
      if (event.keyCode === 8 /* backspace */ || event.keyCode === 9 /* tab */) {
        event.preventDefault();
      }
    },

    onKeyup: (event) => {
      GLFW.onKeyChanged(event.keyCode, 0); // GLFW_RELEASE
    },

    onBlur: (event) => {
      if (!GLFW.active) return;

      for (var i = 0; i < GLFW.active.domKeys.length; ++i) {
        if (GLFW.active.domKeys[i]) {
          GLFW.onKeyChanged(i, 0); // GLFW_RELEASE
        }
      }
    },

    onMousemove: (event) => {
      if (!GLFW.active) return;

      if (event.type === 'touchmove') {
        // Handling for touch events that are being converted to mouse input.

        // Don't let the browser fire a duplicate mouse event.
        event.preventDefault();

        let primaryChanged = false;
        for (let i of event.changedTouches) {
          // If our chosen primary touch moved, update Browser mouse coords
          if (GLFW.primaryTouchId === i.identifier) {
            Browser.setMouseCoords(i.pageX, i.pageY);
            primaryChanged = true;
            break;
          }
        }

        if (!primaryChanged) {
          // Do not send mouse events if some touch other than the primary triggered this.
          return;
        }

      } else {
        // Handling for non-touch mouse input events.
        Browser.calculateMouseEvent(event);
      }

      if (event.target != Module["canvas"] || !GLFW.active.cursorPosFunc) return;

      if (GLFW.active.cursorPosFunc) {
#if USE_GLFW == 2
        {{{ makeDynCall('vii', 'GLFW.active.cursorPosFunc') }}}(Browser.mouseX, Browser.mouseY);
#endif
#if USE_GLFW == 3
        {{{ makeDynCall('vpdd', 'GLFW.active.cursorPosFunc') }}}(GLFW.active.id, Browser.mouseX, Browser.mouseY);
#endif
      }
    },

    DOMToGLFWMouseButton: (event) => {
      // DOM and glfw have different button codes.
      // See http://www.w3schools.com/jsref/event_button.asp.
      var eventButton = event['button'];
      if (eventButton > 0) {
        if (eventButton == 1) {
          eventButton = 2;
        } else {
          eventButton = 1;
        }
      }
      return eventButton;
    },

    onMouseenter: (event) => {
      if (!GLFW.active) return;

      if (event.target != Module["canvas"]) return;

#if USE_GLFW == 3
      if (GLFW.active.cursorEnterFunc) {
        {{{ makeDynCall('vpi', 'GLFW.active.cursorEnterFunc') }}}(GLFW.active.id, 1);
      }
#endif
    },

    onMouseleave: (event) => {
      if (!GLFW.active) return;

      if (event.target != Module["canvas"]) return;

#if USE_GLFW == 3
      if (GLFW.active.cursorEnterFunc) {
        {{{ makeDynCall('vpi', 'GLFW.active.cursorEnterFunc') }}}(GLFW.active.id, 0);
      }
#endif
    },

    onMouseButtonChanged: (event, status) => {
      if (!GLFW.active) return;

      if (event.target != Module["canvas"]) return;

      // Is this from a touch event?
      const isTouchType = event.type === 'touchstart' || event.type === 'touchend' || event.type === 'touchcancel';

      // Only emulating mouse left-click behavior for touches.
      let eventButton = 0;
      if (isTouchType) {
        // Handling for touch events that are being converted to mouse input.

        // Don't let the browser fire a duplicate mouse event.
        event.preventDefault();

        let primaryChanged = false;

        // Set a primary touch if we have none.
        if (GLFW.primaryTouchId === null && event.type === 'touchstart' && event.targetTouches.length > 0) {
          // Pick the first touch that started in the canvas and treat it as primary.
          const chosenTouch = event.targetTouches[0];
          GLFW.primaryTouchId = chosenTouch.identifier;

          Browser.setMouseCoords(chosenTouch.pageX, chosenTouch.pageY);
          primaryChanged = true;
        } else if (event.type === 'touchend' || event.type === 'touchcancel') {
          // Clear the primary touch if it ended.
          for (let i of event.changedTouches) {
            // If our chosen primary touch ended, remove it.
            if (GLFW.primaryTouchId === i.identifier) {
              GLFW.primaryTouchId = null;
              primaryChanged = true;
              break;
            }
          }
        }

        if (!primaryChanged) {
          // Do not send mouse events if some touch other than the primary triggered this.
          return;
        }

      } else {
        // Handling for non-touch mouse input events.
        Browser.calculateMouseEvent(event);
        eventButton = GLFW.DOMToGLFWMouseButton(event);
      }

      if (status == 1) { // GLFW_PRESS
        GLFW.active.buttons |= (1 << eventButton);
        try {
          event.target.setCapture();
        } catch (e) {}
      } else {  // GLFW_RELEASE
        GLFW.active.buttons &= ~(1 << eventButton);
      }

      // Send mouse event to GLFW.
      if (GLFW.active.mouseButtonFunc) {
#if USE_GLFW == 2
        {{{ makeDynCall('vii', 'GLFW.active.mouseButtonFunc') }}}(eventButton, status);
#endif
#if USE_GLFW == 3
        {{{ makeDynCall('vpiii', 'GLFW.active.mouseButtonFunc') }}}(GLFW.active.id, eventButton, status, GLFW.getModBits(GLFW.active));
#endif
      }
    },

    onMouseButtonDown: (event) => {
      if (!GLFW.active) return;
      GLFW.onMouseButtonChanged(event, 1); // GLFW_PRESS
    },

    onMouseButtonUp: (event) => {
      if (!GLFW.active) return;
      GLFW.onMouseButtonChanged(event, 0); // GLFW_RELEASE
    },

    onMouseWheel: (event) => {
      // Note the minus sign that flips browser wheel direction (positive direction scrolls page down) to native wheel direction (positive direction is mouse wheel up)
      var delta = -Browser.getMouseWheelDelta(event);
      delta = (delta == 0) ? 0 : (delta > 0 ? Math.max(delta, 1) : Math.min(delta, -1)); // Quantize to integer so that minimum scroll is at least +/- 1.
      GLFW.wheelPos += delta;

      if (!GLFW.active || !GLFW.active.scrollFunc || event.target != Module['canvas']) return;
#if USE_GLFW == 2
      {{{ makeDynCall('vi', 'GLFW.active.scrollFunc') }}}(GLFW.wheelPos);
#endif
#if USE_GLFW == 3
      var sx = 0;
      var sy = delta;
      if (event.type == 'mousewheel') {
        sx = event.wheelDeltaX;
      } else {
        sx = event.deltaX;
      }

      {{{ makeDynCall('vpdd', 'GLFW.active.scrollFunc') }}}(GLFW.active.id, sx, sy);
#endif

      event.preventDefault();
    },

    // width/height are the dimensions in screen coordinates the user interact with (ex: drawing, mouse coordinates...)
    // framebufferWidth/framebufferHeight are the dimensions in pixel coordinates used for rendering
    // in a HiDPI scenario framebufferWidth = devicePixelRatio * width
    onCanvasResize: (width, height, framebufferWidth, framebufferHeight) => {
      if (!GLFW.active) return;

      var resizeNeeded = false;

      // If the client is requesting fullscreen mode
      if (document["fullscreen"] || document["fullScreen"] || document["mozFullScreen"] || document["webkitIsFullScreen"]) {
        if (!GLFW.active.fullscreen) {
          resizeNeeded = width != screen.width || height != screen.height;
          GLFW.active.storedX = GLFW.active.x;
          GLFW.active.storedY = GLFW.active.y;
          GLFW.active.storedWidth = GLFW.active.width;
          GLFW.active.storedHeight = GLFW.active.height;
          GLFW.active.x = GLFW.active.y = 0;
          GLFW.active.width = screen.width;
          GLFW.active.height = screen.height;
          GLFW.active.fullscreen = true;
        }
      // If the client is reverting from fullscreen mode
      } else if (GLFW.active.fullscreen == true) {
        resizeNeeded = width != GLFW.active.storedWidth || height != GLFW.active.storedHeight;
        GLFW.active.x = GLFW.active.storedX;
        GLFW.active.y = GLFW.active.storedY;
        GLFW.active.width = GLFW.active.storedWidth;
        GLFW.active.height = GLFW.active.storedHeight;
        GLFW.active.fullscreen = false;
      }

      if (resizeNeeded) {
        // width or height is changed (fullscreen / exit fullscreen) which will call this listener back
        // with proper framebufferWidth/framebufferHeight
        Browser.setCanvasSize(GLFW.active.width, GLFW.active.height);
      } else if (GLFW.active.width != width ||
                 GLFW.active.height != height ||
                 GLFW.active.framebufferWidth != framebufferWidth ||
                 GLFW.active.framebufferHeight != framebufferHeight) {
        GLFW.active.width = width;
        GLFW.active.height = height;
        GLFW.active.framebufferWidth = framebufferWidth;
        GLFW.active.framebufferHeight = framebufferHeight;
        GLFW.onWindowSizeChanged();
        GLFW.onFramebufferSizeChanged();
      }
    },

    onWindowSizeChanged: () => {
      if (!GLFW.active) return;

      if (GLFW.active.windowSizeFunc) {
#if USE_GLFW == 2
        {{{ makeDynCall('vii', 'GLFW.active.windowSizeFunc') }}}(GLFW.active.width, GLFW.active.height);
#endif
#if USE_GLFW == 3
        {{{ makeDynCall('vpii', 'GLFW.active.windowSizeFunc') }}}(GLFW.active.id, GLFW.active.width, GLFW.active.height);
#endif
      }
    },

    onFramebufferSizeChanged: () => {
      if (!GLFW.active) return;

#if USE_GLFW == 3
      if (GLFW.active.framebufferSizeFunc) {
        {{{ makeDynCall('vpii', 'GLFW.active.framebufferSizeFunc') }}}(GLFW.active.id, GLFW.active.framebufferWidth, GLFW.active.framebufferHeight);
      }
#endif
    },

    onWindowContentScaleChanged: (scale) => {
      GLFW.scale = scale;
      if (!GLFW.active) return;

#if USE_GLFW == 3
      if (GLFW.active.windowContentScaleFunc) {
        {{{ makeDynCall('vpff', 'GLFW.active.windowContentScaleFunc') }}}(GLFW.active.id, GLFW.scale, GLFW.scale);
      }
#endif
    },

    getTime: () => _emscripten_get_now() / 1000,

    /* GLFW2 wrapping */

    setWindowTitle: (winid, title) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;

      win.title = title;
      if (GLFW.active.id == win.id) {
        _emscripten_set_window_title(title);
      }
    },

    setJoystickCallback: (cbfun) => {
      var prevcbfun = GLFW.joystickFunc;
      GLFW.joystickFunc = cbfun;
      GLFW.refreshJoysticks();
      return prevcbfun;
    },

    joys: {}, // glfw joystick data
    lastGamepadState: [],
    lastGamepadStateFrame: null, // The integer value of MainLoop.currentFrameNumber of when the last gamepad state was produced.

    refreshJoysticks: () => {
      // Produce a new Gamepad API sample if we are ticking a new game frame, or if not using emscripten_set_main_loop() at all to drive animation.
      if (MainLoop.currentFrameNumber !== GLFW.lastGamepadStateFrame || !MainLoop.currentFrameNumber) {
        GLFW.lastGamepadState = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads || []);
        GLFW.lastGamepadStateFrame = MainLoop.currentFrameNumber;

        for (var joy = 0; joy < GLFW.lastGamepadState.length; ++joy) {
          var gamepad = GLFW.lastGamepadState[joy];

          if (gamepad) {
            if (!GLFW.joys[joy]) {
              out('glfw joystick connected:',joy);
              GLFW.joys[joy] = {
                id: stringToNewUTF8(gamepad.id),
                buttonsCount: gamepad.buttons.length,
                axesCount: gamepad.axes.length,
                buttons: _malloc(gamepad.buttons.length),
                axes: _malloc(gamepad.axes.length*4),
              };

              if (GLFW.joystickFunc) {
                {{{ makeDynCall('vii', 'GLFW.joystickFunc') }}}(joy, 0x00040001); // GLFW_CONNECTED
              }
            }

            var data = GLFW.joys[joy];

            for (var i = 0; i < gamepad.buttons.length;  ++i) {
              {{{ makeSetValue('data.buttons + i', '0', 'gamepad.buttons[i].pressed', 'i8') }}};
            }

            for (var i = 0; i < gamepad.axes.length; ++i) {
              {{{ makeSetValue('data.axes + i*4', '0', 'gamepad.axes[i]', 'float') }}};
            }
          } else {
            if (GLFW.joys[joy]) {
              out('glfw joystick disconnected',joy);

              if (GLFW.joystickFunc) {
                {{{ makeDynCall('vii', 'GLFW.joystickFunc') }}}(joy, 0x00040002); // GLFW_DISCONNECTED
              }

              _free(GLFW.joys[joy].id);
              _free(GLFW.joys[joy].buttons);
              _free(GLFW.joys[joy].axes);

              delete GLFW.joys[joy];
            }
          }
        }
      }
    },

    setKeyCallback: (winid, cbfun) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return null;
      var prevcbfun = win.keyFunc;
      win.keyFunc = cbfun;
      return prevcbfun;
    },

    setCharCallback: (winid, cbfun) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return null;
      var prevcbfun = win.charFunc;
      win.charFunc = cbfun;
      return prevcbfun;
    },

    setMouseButtonCallback: (winid, cbfun) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return null;
      var prevcbfun = win.mouseButtonFunc;
      win.mouseButtonFunc = cbfun;
      return prevcbfun;
    },

    setCursorPosCallback: (winid, cbfun) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return null;
      var prevcbfun = win.cursorPosFunc;
      win.cursorPosFunc = cbfun;
      return prevcbfun;
    },

    setScrollCallback: (winid, cbfun) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return null;
      var prevcbfun = win.scrollFunc;
      win.scrollFunc = cbfun;
      return prevcbfun;
    },

    setDropCallback: (winid, cbfun) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return null;
      var prevcbfun = win.dropFunc;
      win.dropFunc = cbfun;
      return prevcbfun;
    },

    onDrop: (event) => {
      if (!GLFW.active || !GLFW.active.dropFunc) return;
      if (!event.dataTransfer || !event.dataTransfer.files || event.dataTransfer.files.length == 0) return;

      event.preventDefault();

#if FILESYSTEM
      var filenames = _malloc(event.dataTransfer.files.length*4);
      var filenamesArray = [];
      var count = event.dataTransfer.files.length;

      // Read and save the files to emscripten's FS
      var written = 0;
      var drop_dir = '.glfw_dropped_files';
      FS.createPath('/', drop_dir);

      function save(file) {
        var path = '/' + drop_dir + '/' + file.name.replace(/\//g, '_');
        var reader = new FileReader();
        reader.onloadend = (e) => {
          if (reader.readyState != 2) { // not DONE
            ++written;
            out('failed to read dropped file: '+file.name+': '+reader.error);
            return;
          }

          var data = e.target.result;
          FS.writeFile(path, new Uint8Array(data));
          if (++written === count) {
            {{{ makeDynCall('vpii', 'GLFW.active.dropFunc') }}}(GLFW.active.id, count, filenames);

            for (var i = 0; i < filenamesArray.length; ++i) {
              _free(filenamesArray[i]);
            }
            _free(filenames);
          }
        };
        reader.readAsArrayBuffer(file);

        var filename = stringToNewUTF8(path);
        filenamesArray.push(filename);
        {{{ makeSetValue('filenames + i*4', '0', 'filename', POINTER_TYPE) }}};
      }

      for (var i = 0; i < count; ++i) {
        save(event.dataTransfer.files[i]);
      }
#endif // FILESYSTEM

      return false;
    },

    onDragover: (event) => {
      if (!GLFW.active || !GLFW.active.dropFunc) return;

      event.preventDefault();
      return false;
    },

    setWindowSizeCallback: (winid, cbfun) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return null;
      var prevcbfun = win.windowSizeFunc;
      win.windowSizeFunc = cbfun;

#if USE_GLFW == 2
      // As documented in GLFW2 API (http://www.glfw.org/GLFWReference27.pdf#page=22), when size
      // callback function is set, it will be called with the current window size before this
      // function returns.
      // GLFW3 on the over hand doesn't have this behavior (https://github.com/glfw/glfw/issues/62).
      if (!win.windowSizeFunc) return null;
      {{{ makeDynCall('vii', 'win.windowSizeFunc') }}}(win.width, win.height);
#endif

      return prevcbfun;
    },

    setWindowCloseCallback: (winid, cbfun) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return null;
      var prevcbfun = win.windowCloseFunc;
      win.windowCloseFunc = cbfun;
      return prevcbfun;
    },

    setWindowRefreshCallback: (winid, cbfun) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return null;
      var prevcbfun = win.windowRefreshFunc;
      win.windowRefreshFunc = cbfun;
      return prevcbfun;
    },

    onClickRequestPointerLock: (e) => {
      if (!Browser.pointerLock && Module['canvas'].requestPointerLock) {
        Module['canvas'].requestPointerLock();
        e.preventDefault();
      }
    },

    setInputMode: (winid, mode, value) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;

      switch (mode) {
        case 0x00033001: { // GLFW_CURSOR
          switch (value) {
            case 0x00034001: { // GLFW_CURSOR_NORMAL
              win.inputModes[mode] = value;
              Module['canvas'].removeEventListener('click', GLFW.onClickRequestPointerLock, true);
              Module['canvas'].exitPointerLock();
              break;
            }
            case 0x00034002: { // GLFW_CURSOR_HIDDEN
              err('glfwSetInputMode called with GLFW_CURSOR_HIDDEN value not implemented');
              break;
            }
            case 0x00034003: { // GLFW_CURSOR_DISABLED
              win.inputModes[mode] = value;
              Module['canvas'].addEventListener('click', GLFW.onClickRequestPointerLock, true);
              Module['canvas'].requestPointerLock();
              break;
            }
            default: {
              err(`glfwSetInputMode called with unknown value parameter value: ${value}`);
              break;
            }
          }
          break;
        }
        case 0x00033002: { // GLFW_STICKY_KEYS
          err('glfwSetInputMode called with GLFW_STICKY_KEYS mode not implemented');
          break;
        }
        case 0x00033003: { // GLFW_STICKY_MOUSE_BUTTONS
          err('glfwSetInputMode called with GLFW_STICKY_MOUSE_BUTTONS mode not implemented');
          break;
        }
        case 0x00033004: { // GLFW_LOCK_KEY_MODS
          err('glfwSetInputMode called with GLFW_LOCK_KEY_MODS mode not implemented');
          break;
        }
        case 0x000330005: { // GLFW_RAW_MOUSE_MOTION
          err('glfwSetInputMode called with GLFW_RAW_MOUSE_MOTION mode not implemented');
          break;
        }
        default: {
          err(`glfwSetInputMode called with unknown mode parameter value: ${mode}`);
          break;
        }
      }
    },

    getKey: (winid, key) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return 0;
      return win.keys[key];
    },

    getMouseButton: (winid, button) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return 0;
      return (win.buttons & (1 << button)) > 0;
    },

    getCursorPos: (winid, x, y) => {
      {{{ makeSetValue('x', '0', 'Browser.mouseX', 'double') }}};
      {{{ makeSetValue('y', '0', 'Browser.mouseY', 'double') }}};
    },

    getMousePos: (winid, x, y) => {
      {{{ makeSetValue('x', '0', 'Browser.mouseX', 'i32') }}};
      {{{ makeSetValue('y', '0', 'Browser.mouseY', 'i32') }}};
    },

    setCursorPos: (winid, x, y) => {
    },

    getWindowPos: (winid, x, y) => {
      var wx = 0;
      var wy = 0;

      var win = GLFW.WindowFromId(winid);
      if (win) {
        wx = win.x;
        wy = win.y;
      }

      if (x) {
        {{{ makeSetValue('x', '0', 'wx', 'i32') }}};
      }

      if (y) {
        {{{ makeSetValue('y', '0', 'wy', 'i32') }}};
      }
    },

    setWindowPos: (winid, x, y) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.x = x;
      win.y = y;
    },

    getWindowSize: (winid, width, height) => {
      var ww = 0;
      var wh = 0;

      var win = GLFW.WindowFromId(winid);
      if (win) {
        ww = win.width;
        wh = win.height;
      }

      if (width) {
        {{{ makeSetValue('width', '0', 'ww', 'i32') }}};
      }

      if (height) {
        {{{ makeSetValue('height', '0', 'wh', 'i32') }}};
      }
    },

    setWindowSize: (winid, width, height) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;

      if (GLFW.active.id == win.id) {
        Browser.setCanvasSize(width, height); // triggers the listener (onCanvasResize) + windowSizeFunc
      }
    },

    defaultWindowHints: () => {
      GLFW.hints = Object.assign({}, GLFW.defaultHints);
    },

    createWindow: (width, height, title, monitor, share) => {
      var i, id;
      for (i = 0; i < GLFW.windows.length && GLFW.windows[i] !== null; i++) {
        // no-op
      }
      if (i > 0) throw "glfwCreateWindow only supports one window at time currently";

      // id for window
      id = i + 1;

      // not valid
      if (width <= 0 || height <= 0) return 0;

      if (monitor) {
        Browser.requestFullscreen();
      } else {
        Browser.setCanvasSize(width, height);
      }

      // Create context when there are no existing alive windows
      for (i = 0; i < GLFW.windows.length && GLFW.windows[i] == null; i++) {
        // no-op
      }
      var useWebGL = GLFW.hints[0x00022001] > 0; // Use WebGL when we are told to based on GLFW_CLIENT_API
      if (i == GLFW.windows.length) {
        if (useWebGL) {
          var contextAttributes = {
            antialias: (GLFW.hints[0x0002100D] > 1), // GLFW_SAMPLES
            depth: (GLFW.hints[0x00021005] > 0),     // GLFW_DEPTH_BITS
            stencil: (GLFW.hints[0x00021006] > 0),   // GLFW_STENCIL_BITS
            alpha: (GLFW.hints[0x00021004] > 0)      // GLFW_ALPHA_BITS
          }
#if OFFSCREEN_FRAMEBUFFER
          // TODO: Make GLFW explicitly aware of whether it is being proxied or not, and set these to true only when proxying is being performed.
          GL.enableOffscreenFramebufferAttributes(contextAttributes);
#endif
          Module.ctx = Browser.createContext(Module['canvas'], true, true, contextAttributes);
        } else {
          Browser.init();
        }
      }

      // If context creation failed, do not return a valid window
      if (!Module.ctx && useWebGL) return 0;

      // Get non alive id
      const canvas = Module['canvas'];
      var win = new GLFW_Window(id, canvas.clientWidth, canvas.clientHeight, canvas.width, canvas.height, title, monitor, share);

      // Set window to array
      if (id - 1 == GLFW.windows.length) {
        GLFW.windows.push(win);
      } else {
        GLFW.windows[id - 1] = win;
      }

      GLFW.active = win;
      GLFW.adjustCanvasDimensions();
      return win.id;
    },

    destroyWindow: (winid) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;

#if USE_GLFW == 3
      if (win.windowCloseFunc) {
        {{{ makeDynCall('vp', 'win.windowCloseFunc') }}}(win.id);
      }
#endif

      GLFW.windows[win.id - 1] = null;
      if (GLFW.active.id == win.id)
        GLFW.active = null;

      // Destroy context when no alive windows
      for (var i = 0; i < GLFW.windows.length; i++)
        if (GLFW.windows[i] !== null) return;

      delete Module.ctx;
    },

    swapBuffers: (winid) => {
    },

    // Overrides Browser.requestFullscreen to notify listeners even if Browser.resizeCanvas is false
    requestFullscreen(lockPointer, resizeCanvas) {
      Browser.lockPointer = lockPointer;
      Browser.resizeCanvas = resizeCanvas;
      if (typeof Browser.lockPointer == 'undefined') Browser.lockPointer = true;
      if (typeof Browser.resizeCanvas == 'undefined') Browser.resizeCanvas = false;

      var canvas = Module['canvas'];
      function fullscreenChange() {
        Browser.isFullscreen = false;
        var canvasContainer = canvas.parentNode;
        if ((document['fullscreenElement'] || document['mozFullScreenElement'] ||
          document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
          document['webkitCurrentFullScreenElement']) === canvasContainer) {
          canvas.exitFullscreen = Browser.exitFullscreen;
          if (Browser.lockPointer) canvas.requestPointerLock();
          Browser.isFullscreen = true;
          if (Browser.resizeCanvas) {
            Browser.setFullscreenCanvasSize();
          } else {
            Browser.updateCanvasDimensions(canvas);
            Browser.updateResizeListeners();
          }
        } else {
          // remove the full screen specific parent of the canvas again to restore the HTML structure from before going full screen
          canvasContainer.parentNode.insertBefore(canvas, canvasContainer);
          canvasContainer.parentNode.removeChild(canvasContainer);

          if (Browser.resizeCanvas) {
            Browser.setWindowedCanvasSize();
          } else {
            Browser.updateCanvasDimensions(canvas);
            Browser.updateResizeListeners();
          }
        }
        Module['onFullScreen']?.(Browser.isFullscreen);
        Module['onFullscreen']?.(Browser.isFullscreen);
      }

      if (!Browser.fullscreenHandlersInstalled) {
        Browser.fullscreenHandlersInstalled = true;
        document.addEventListener('fullscreenchange', fullscreenChange, false);
        document.addEventListener('mozfullscreenchange', fullscreenChange, false);
        document.addEventListener('webkitfullscreenchange', fullscreenChange, false);
        document.addEventListener('MSFullscreenChange', fullscreenChange, false);
      }

      // create a new parent to ensure the canvas has no siblings. this allows browsers to optimize full screen performance when its parent is the full screen root
      var canvasContainer = document.createElement("div");
      canvas.parentNode.insertBefore(canvasContainer, canvas);
      canvasContainer.appendChild(canvas);

      // use parent of canvas as full screen root to allow aspect ratio correction (Firefox stretches the root to screen size)
      canvasContainer.requestFullscreen = canvasContainer['requestFullscreen'] ||
        canvasContainer['mozRequestFullScreen'] ||
        canvasContainer['msRequestFullscreen'] ||
        (canvasContainer['webkitRequestFullscreen'] ? () => canvasContainer['webkitRequestFullscreen'](Element['ALLOW_KEYBOARD_INPUT']) : null) ||
        (canvasContainer['webkitRequestFullScreen'] ? () => canvasContainer['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) : null);

      canvasContainer.requestFullscreen();
    },

    // Overrides Browser.updateCanvasDimensions to account for hi dpi scaling
    updateCanvasDimensions(canvas, wNative, hNative) {
      const scale = GLFW.getHiDPIScale();

      if (wNative && hNative) {
        canvas.widthNative = wNative;
        canvas.heightNative = hNative;
      } else {
        wNative = canvas.widthNative;
        hNative = canvas.heightNative;
      }
      var w = wNative;
      var h = hNative;
      if (Module['forcedAspectRatio'] && Module['forcedAspectRatio'] > 0) {
        if (w/h < Module['forcedAspectRatio']) {
          w = Math.round(h * Module['forcedAspectRatio']);
        } else {
          h = Math.round(w / Module['forcedAspectRatio']);
        }
      }
      if (((document['fullscreenElement'] || document['mozFullScreenElement'] ||
        document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
        document['webkitCurrentFullScreenElement']) === canvas.parentNode) && (typeof screen != 'undefined')) {
        var factor = Math.min(screen.width / w, screen.height / h);
        w = Math.round(w * factor);
        h = Math.round(h * factor);
      }
      if (Browser.resizeCanvas) {
        wNative = w;
        hNative = h;
      }
      const wNativeScaled = Math.floor(wNative * scale);
      const hNativeScaled = Math.floor(hNative * scale);
      if (canvas.width  != wNativeScaled) canvas.width  = wNativeScaled;
      if (canvas.height != hNativeScaled) canvas.height = hNativeScaled;
      if (typeof canvas.style != 'undefined') {
        if (wNativeScaled != wNative || hNativeScaled != hNative) {
          canvas.style.setProperty( "width", wNative + "px", "important");
          canvas.style.setProperty("height", hNative + "px", "important");
        } else {
          canvas.style.removeProperty( "width");
          canvas.style.removeProperty("height");
        }
      }
    },

    // Overrides Browser.calculateMouseCoords to account for hi dpi scaling
    calculateMouseCoords(pageX, pageY) {
      // Calculate the movement based on the changes
      // in the coordinates.
      var rect = Module["canvas"].getBoundingClientRect();
      var cw = Module["canvas"].clientWidth;
      var ch = Module["canvas"].clientHeight;

      // Neither .scrollX or .pageXOffset are defined in a spec, but
      // we prefer .scrollX because it is currently in a spec draft.
      // (see: http://www.w3.org/TR/2013/WD-cssom-view-20131217/)
      var scrollX = ((typeof window.scrollX != 'undefined') ? window.scrollX : window.pageXOffset);
      var scrollY = ((typeof window.scrollY != 'undefined') ? window.scrollY : window.pageYOffset);
#if ASSERTIONS
      // If this assert lands, it's likely because the browser doesn't support scrollX or pageXOffset
      // and we have no viable fallback.
      assert((typeof scrollX != 'undefined') && (typeof scrollY != 'undefined'), 'Unable to retrieve scroll position, mouse positions likely broken.');
#endif
      var adjustedX = pageX - (scrollX + rect.left);
      var adjustedY = pageY - (scrollY + rect.top);

      // the canvas might be CSS-scaled compared to its backbuffer;
      // SDL-using content will want mouse coordinates in terms
      // of backbuffer units.
      adjustedX = adjustedX * (cw / rect.width);
      adjustedY = adjustedY * (ch / rect.height);

      return { x: adjustedX, y: adjustedY };
    },

    setWindowAttrib: (winid, attrib, value) => {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      const isHiDPIAware = GLFW.isHiDPIAware();
      win.attributes[attrib] = value;
      if (isHiDPIAware !== GLFW.isHiDPIAware())
        GLFW.adjustCanvasDimensions();
    },

    getDevicePixelRatio() {
      return (typeof devicePixelRatio == 'number' && devicePixelRatio) || 1.0;
    },

    isHiDPIAware() {
      if (GLFW.active)
        return GLFW.active.attributes[0x0002200C] > 0; // GLFW_SCALE_TO_MONITOR
      else
        return false;
    },

    adjustCanvasDimensions() {
      const canvas = Module['canvas'];
      Browser.updateCanvasDimensions(canvas, canvas.clientWidth, canvas.clientHeight);
      Browser.updateResizeListeners();
    },

    getHiDPIScale() {
      return GLFW.isHiDPIAware() ? GLFW.scale : 1.0;
    },

    onDevicePixelRatioChange() {
      GLFW.onWindowContentScaleChanged(GLFW.getDevicePixelRatio());
      GLFW.adjustCanvasDimensions();
    },

    GLFW2ParamToGLFW3Param: (param) => {
      var table = {
        0x00030001:0, // GLFW_MOUSE_CURSOR
        0x00030002:0, // GLFW_STICKY_KEYS
        0x00030003:0, // GLFW_STICKY_MOUSE_BUTTONS
        0x00030004:0, // GLFW_SYSTEM_KEYS
        0x00030005:0, // GLFW_KEY_REPEAT
        0x00030006:0, // GLFW_AUTO_POLL_EVENTS
        0x00020001:0, // GLFW_OPENED
        0x00020002:0, // GLFW_ACTIVE
        0x00020003:0, // GLFW_ICONIFIED
        0x00020004:0, // GLFW_ACCELERATED
        0x00020005:0x00021001, // GLFW_RED_BITS
        0x00020006:0x00021002, // GLFW_GREEN_BITS
        0x00020007:0x00021003, // GLFW_BLUE_BITS
        0x00020008:0x00021004, // GLFW_ALPHA_BITS
        0x00020009:0x00021005, // GLFW_DEPTH_BITS
        0x0002000A:0x00021006, // GLFW_STENCIL_BITS
        0x0002000B:0x0002100F, // GLFW_REFRESH_RATE
        0x0002000C:0x00021007, // GLFW_ACCUM_RED_BITS
        0x0002000D:0x00021008, // GLFW_ACCUM_GREEN_BITS
        0x0002000E:0x00021009, // GLFW_ACCUM_BLUE_BITS
        0x0002000F:0x0002100A, // GLFW_ACCUM_ALPHA_BITS
        0x00020010:0x0002100B, // GLFW_AUX_BUFFERS
        0x00020011:0x0002100C, // GLFW_STEREO
        0x00020012:0, // GLFW_WINDOW_NO_RESIZE
        0x00020013:0x0002100D, // GLFW_FSAA_SAMPLES
        0x00020014:0x00022002, // GLFW_OPENGL_VERSION_MAJOR
        0x00020015:0x00022003, // GLFW_OPENGL_VERSION_MINOR
        0x00020016:0x00022006, // GLFW_OPENGL_FORWARD_COMPAT
        0x00020017:0x00022007, // GLFW_OPENGL_DEBUG_CONTEXT
        0x00020018:0x00022008, // GLFW_OPENGL_PROFILE
      };
      return table[param];
    }
  },

/*******************************************************************************
 * GLFW FUNCTIONS
 ******************************************************************************/
  glfwInit__deps: ['malloc', 'free'],
  glfwInit: () => {
    if (GLFW.windows) return 1; // GL_TRUE

    GLFW.initialTime = GLFW.getTime();
    GLFW.defaultWindowHints();
    GLFW.windows = new Array()
    GLFW.active = null;
    GLFW.scale  = GLFW.getDevicePixelRatio();


    window.addEventListener("gamepadconnected", GLFW.onGamepadConnected, true);
    window.addEventListener("gamepaddisconnected", GLFW.onGamepadDisconnected, true);
    window.addEventListener("keydown", GLFW.onKeydown, true);
    window.addEventListener("keypress", GLFW.onKeyPress, true);
    window.addEventListener("keyup", GLFW.onKeyup, true);
    window.addEventListener("blur", GLFW.onBlur, true);

    // watch for devicePixelRatio changes
    GLFW.devicePixelRatioMQL = window.matchMedia('(resolution: ' + GLFW.getDevicePixelRatio() + 'dppx)');
    GLFW.devicePixelRatioMQL.addEventListener('change', GLFW.onDevicePixelRatioChange);

    Module["canvas"].addEventListener("touchmove", GLFW.onMousemove, true);
    Module["canvas"].addEventListener("touchstart", GLFW.onMouseButtonDown, true);
    Module["canvas"].addEventListener("touchcancel", GLFW.onMouseButtonUp, true);
    Module["canvas"].addEventListener("touchend", GLFW.onMouseButtonUp, true);
    Module["canvas"].addEventListener("mousemove", GLFW.onMousemove, true);
    Module["canvas"].addEventListener("mousedown", GLFW.onMouseButtonDown, true);
    Module["canvas"].addEventListener("mouseup", GLFW.onMouseButtonUp, true);
    Module["canvas"].addEventListener('wheel', GLFW.onMouseWheel, true);
    Module["canvas"].addEventListener('mousewheel', GLFW.onMouseWheel, true);
    Module["canvas"].addEventListener('mouseenter', GLFW.onMouseenter, true);
    Module["canvas"].addEventListener('mouseleave', GLFW.onMouseleave, true);
    Module["canvas"].addEventListener('drop', GLFW.onDrop, true);
    Module["canvas"].addEventListener('dragover', GLFW.onDragover, true);

    // Overriding implementation to account for HiDPI
    Browser.requestFullscreen = GLFW.requestFullscreen;
    Browser.calculateMouseCoords = GLFW.calculateMouseCoords;
    Browser.updateCanvasDimensions = GLFW.updateCanvasDimensions;

    Browser.resizeListeners.push((width, height) => {
      if (GLFW.isHiDPIAware()) {
        var canvas = Module['canvas'];
        GLFW.onCanvasResize(canvas.clientWidth, canvas.clientHeight, width, height);
      } else {
        GLFW.onCanvasResize(width, height, width, height);
      }
    });

    return 1; // GL_TRUE
  },

  glfwTerminate: () => {
    window.removeEventListener("gamepadconnected", GLFW.onGamepadConnected, true);
    window.removeEventListener("gamepaddisconnected", GLFW.onGamepadDisconnected, true);
    window.removeEventListener("keydown", GLFW.onKeydown, true);
    window.removeEventListener("keypress", GLFW.onKeyPress, true);
    window.removeEventListener("keyup", GLFW.onKeyup, true);
    window.removeEventListener("blur", GLFW.onBlur, true);
    Module["canvas"].removeEventListener("touchmove", GLFW.onMousemove, true);
    Module["canvas"].removeEventListener("touchstart", GLFW.onMouseButtonDown, true);
    Module["canvas"].removeEventListener("touchcancel", GLFW.onMouseButtonUp, true);
    Module["canvas"].removeEventListener("touchend", GLFW.onMouseButtonUp, true);
    Module["canvas"].removeEventListener("mousemove", GLFW.onMousemove, true);
    Module["canvas"].removeEventListener("mousedown", GLFW.onMouseButtonDown, true);
    Module["canvas"].removeEventListener("mouseup", GLFW.onMouseButtonUp, true);
    Module["canvas"].removeEventListener('wheel', GLFW.onMouseWheel, true);
    Module["canvas"].removeEventListener('mousewheel', GLFW.onMouseWheel, true);
    Module["canvas"].removeEventListener('mouseenter', GLFW.onMouseenter, true);
    Module["canvas"].removeEventListener('mouseleave', GLFW.onMouseleave, true);
    Module["canvas"].removeEventListener('drop', GLFW.onDrop, true);
    Module["canvas"].removeEventListener('dragover', GLFW.onDragover, true);

    if (GLFW.devicePixelRatioMQL)
      GLFW.devicePixelRatioMQL.removeEventListener('change', GLFW.onDevicePixelRatioChange);

    Module["canvas"].width = Module["canvas"].height = 1;
    GLFW.windows = null;
    GLFW.active = null;
  },

  glfwGetVersion: (major, minor, rev) => {
#if USE_GLFW == 2
    {{{ makeSetValue('major', '0', '2', 'i32') }}};
    {{{ makeSetValue('minor', '0', '7', 'i32') }}};
    {{{ makeSetValue('rev', '0', '7', 'i32') }}};
#endif

#if USE_GLFW == 3
    {{{ makeSetValue('major', '0', '3', 'i32') }}};
    {{{ makeSetValue('minor', '0', '2', 'i32') }}};
    {{{ makeSetValue('rev', '0', '1', 'i32') }}};
#endif
  },

  glfwPollEvents: () => {},

  glfwWaitEvents: () => {},

  glfwGetTime: () => GLFW.getTime() - GLFW.initialTime,

  glfwSetTime: (time) => {
    GLFW.initialTime = GLFW.getTime() - time;
  },

  glfwExtensionSupported__deps: ['glGetString', '$webglGetExtensions'],
  glfwExtensionSupported: (extension) => {
    GLFW.extensions ||= webglGetExtensions();

    if (GLFW.extensions.includes(extension)) return 1;

    // extensions from GLEmulations do not come unprefixed
    // so, try with prefix
    return (GLFW.extensions.includes("GL_" + extension));
  },

  glfwSwapInterval__deps: ['emscripten_set_main_loop_timing'],
  glfwSwapInterval: (interval) => {
    interval = Math.abs(interval); // GLFW uses negative values to enable GLX_EXT_swap_control_tear, which we don't have, so just treat negative and positive the same.
    if (interval == 0) _emscripten_set_main_loop_timing({{{ cDefs.EM_TIMING_SETTIMEOUT }}}, 0);
    else _emscripten_set_main_loop_timing({{{ cDefs.EM_TIMING_RAF }}}, interval);
  },

#if USE_GLFW == 3
  glfwGetVersionString: () => {
    GLFW.versionString ||= stringToNewUTF8("3.2.1 JS WebGL Emscripten");
    return GLFW.versionString;
  },

  glfwSetErrorCallback: (cbfun) => {
    var prevcbfun = GLFW.errorFunc;
    GLFW.errorFunc = cbfun;
    return prevcbfun;
  },

  glfwWaitEventsTimeout: (timeout) => {},

  glfwPostEmptyEvent: () => {},

  glfwGetMonitors__deps: ['malloc'],
  glfwGetMonitors: (count) => {
    {{{ makeSetValue('count', '0', '1', 'i32') }}};
    if (!GLFW.monitors) {
      GLFW.monitors = _malloc({{{ POINTER_SIZE }}});
      {{{ makeSetValue('GLFW.monitors', '0', '1', 'i32') }}};
    }
    return GLFW.monitors;
  },

  glfwGetPrimaryMonitor: () => 1,

  glfwGetMonitorPos: (monitor, x, y) => {
    {{{ makeSetValue('x', '0', '0', 'i32') }}};
    {{{ makeSetValue('y', '0', '0', 'i32') }}};
  },

  glfwGetMonitorWorkarea: (monitor, x, y, w, h) => {
    {{{ makeSetValue('x', '0', '0', 'i32') }}};
    {{{ makeSetValue('y', '0', '0', 'i32') }}};

    {{{ makeSetValue('w', '0', 'screen.availWidth', 'i32') }}};
    {{{ makeSetValue('h', '0', 'screen.availHeight', 'i32') }}};
  },

  glfwGetMonitorPhysicalSize: (monitor, width, height) => {
    // AFAIK there is no way to do this in javascript
    // Maybe with platform specific ccalls?
    //
    // Lets report 0 now which is wrong as it can get for end user.
    {{{ makeSetValue('width', '0', '0', 'i32') }}};
    {{{ makeSetValue('height', '0', '0', 'i32') }}};
  },

  glfwGetMonitorContentScale: (monitor, x, y) => {
    {{{ makeSetValue('x', '0', 'GLFW.scale', 'float') }}};
    {{{ makeSetValue('y', '0', 'GLFW.scale', 'float') }}};
  },

  glfwGetMonitorName: (mon) => {
    GLFW.monitorString ||= stringToNewUTF8("HTML5 WebGL Canvas");
    return GLFW.monitorString;
  },

  glfwSetMonitorCallback: (cbfun) => {
    var prevcbfun = GLFW.monitorFunc;
    GLFW.monitorFunc = cbfun;
    return prevcbfun;
  },

  // TODO: implement
  glfwGetVideoModes: (monitor, count) => {
    {{{ makeSetValue('count', '0', '0', 'i32') }}};
    return 0;
  },

  // TODO: implement
  glfwGetVideoMode: (monitor) => 0,

  // TODO: implement
  glfwSetGamma: (monitor, gamma) => {},

  glfwGetGammaRamp: (monitor) => { throw "glfwGetGammaRamp not implemented."; },

  glfwSetGammaRamp: (monitor, ramp) => { throw "glfwSetGammaRamp not implemented."; },

  glfwDefaultWindowHints: () => GLFW.defaultWindowHints(),

  glfwWindowHint: (target, hint) => {
    GLFW.hints[target] = hint;
  },

  glfwWindowHintString: (hint, value) => {
    // from glfw docs -> we just ignore this.
    // Some hints are platform specific.  These may be set on any platform but they
    // will only affect their specific platform.  Other platforms will ignore them.
  },

  glfwCreateWindow: (width, height, title, monitor, share) => GLFW.createWindow(width, height, title, monitor, share),

  glfwDestroyWindow: (winid) => GLFW.destroyWindow(winid),

  glfwWindowShouldClose: (winid) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return 0;
    return win.shouldClose;
  },

  glfwSetWindowShouldClose: (winid, value) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    win.shouldClose = value;
  },

  glfwSetWindowTitle: (winid, title) => GLFW.setWindowTitle(winid, title),

  glfwGetWindowPos: (winid, x, y) => GLFW.getWindowPos(winid, x, y),

  glfwSetWindowPos: (winid, x, y) => GLFW.setWindowPos(winid, x, y),

  glfwGetWindowSize: (winid, width, height) => GLFW.getWindowSize(winid, width, height),

  glfwSetWindowSize: (winid, width, height) => GLFW.setWindowSize(winid, width, height),

  glfwGetFramebufferSize: (winid, width, height) => {
    var ww = 0;
    var wh = 0;

    var win = GLFW.WindowFromId(winid);
    if (win) {
      ww = win.framebufferWidth;
      wh = win.framebufferHeight;
    }

    if (width) {
      {{{ makeSetValue('width', '0', 'ww', 'i32') }}};
    }

    if (height) {
      {{{ makeSetValue('height', '0', 'wh', 'i32') }}};
    }
  },

  glfwGetWindowContentScale: (winid, x, y) => {
    // winid doesn't matter. all windows will use same scale anyway.
    // hope i used this makeSetValue correctly
    {{{ makeSetValue('x', '0', 'GLFW.scale', 'float') }}};
    {{{ makeSetValue('y', '0', 'GLFW.scale', 'float') }}};
  },

  glfwGetWindowOpacity: (winid) => 1.0,

  glfwSetWindowOpacity: (winid, opacity) => { /* error */ },

  glfwIconifyWindow: (winid) => {
#if ASSERTIONS
    warnOnce('glfwIconifyWindow is not implemented');
#endif
  },

  glfwRestoreWindow: (winid) => {
#if ASSERTIONS
    warnOnce('glfwRestoreWindow is not implemented');
#endif
  },

  glfwShowWindow: (winid) => {},

  glfwHideWindow: (winid) => {},

  glfwGetWindowMonitor: (winid) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return 0;
    return win.monitor;
  },

  glfwGetWindowAttrib: (winid, attrib) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return 0;
    return win.attributes[attrib];
  },

  glfwSetWindowAttrib: (winid, attrib, value) => GLFW.setWindowAttrib(winid, attrib, value),

  glfwSetWindowUserPointer: (winid, ptr) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    win.userptr = ptr;
  },

  glfwGetWindowUserPointer: (winid) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return 0;
    return win.userptr;
  },

  glfwSetWindowPosCallback: (winid, cbfun) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return null;
    var prevcbfun = win.windowPosFunc;
    win.windowPosFunc = cbfun;
    return prevcbfun;
  },

  glfwSetWindowSizeCallback: (winid, cbfun) => GLFW.setWindowSizeCallback(winid, cbfun),

  glfwSetWindowCloseCallback: (winid, cbfun) => GLFW.setWindowCloseCallback(winid, cbfun),

  glfwSetWindowRefreshCallback: (winid, cbfun) => GLFW.setWindowRefreshCallback(winid, cbfun),

  glfwSetWindowFocusCallback: (winid, cbfun) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return null;
    var prevcbfun = win.windowFocusFunc;
    win.windowFocusFunc = cbfun;
    return prevcbfun;
  },

  glfwSetWindowIconifyCallback: (winid, cbfun) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return null;
    var prevcbfun = win.windowIconifyFunc;
    win.windowIconifyFunc = cbfun;
    return prevcbfun;
  },

  glfwSetWindowMaximizeCallback: (winid, cbfun) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return null;
    var prevcbfun = win.windowMaximizeFunc;
    win.windowMaximizeFunc = cbfun;
    return prevcbfun;
  },

  glfwSetWindowIcon: (winid, count, images) => {},

  glfwSetWindowSizeLimits: (winid, minwidth, minheight, maxwidth, maxheight) => {},

  glfwSetWindowAspectRatio: (winid, numer, denom) => {},

  glfwGetWindowFrameSize: (winid, left, top, right, bottom) => { throw "glfwGetWindowFrameSize not implemented."; },

  glfwMaximizeWindow: (winid) => {},

  glfwFocusWindow: (winid) => {},

  glfwRequestWindowAttention: (winid) => {}, // maybe do window.focus()?

  glfwSetWindowMonitor: (winid, monitor, xpos, ypos, width, height, refreshRate) => { throw "glfwSetWindowMonitor not implemented."; },

  glfwCreateCursor: (image, xhot, yhot) => {},

  glfwCreateStandardCursor: (shape) => {},

  glfwDestroyCursor: (cursor) => {},

  glfwSetCursor: (winid, cursor) => {},

  glfwSetFramebufferSizeCallback: (winid, cbfun) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return null;
    var prevcbfun = win.framebufferSizeFunc;
    win.framebufferSizeFunc = cbfun;
    return prevcbfun;
  },

  glfwSetWindowContentScaleCallback: (winid, cbfun) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return null;
    var prevcbfun = win.windowContentScaleFunc;
    win.windowContentScaleFunc = cbfun;
    return prevcbfun;
  },

  glfwGetInputMode: (winid, mode) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;

    switch (mode) {
      case 0x00033001: { // GLFW_CURSOR
        if (Browser.pointerLock) {
          win.inputModes[mode] = 0x00034003; // GLFW_CURSOR_DISABLED
        } else {
          win.inputModes[mode] = 0x00034001; // GLFW_CURSOR_NORMAL
        }
      }
    }

    return win.inputModes[mode];
  },

  glfwSetInputMode: (winid, mode, value) => {
    GLFW.setInputMode(winid, mode, value);
  },

  glfwRawMouseMotionSupported: () => 0,

  glfwGetKey: (winid, key) => GLFW.getKey(winid, key),

  glfwGetKeyName: (key, scancode) => { throw "glfwGetKeyName not implemented."; },

  glfwGetKeyScancode: (key) => { throw "glfwGetKeyScancode not implemented."; },

  glfwGetMouseButton: (winid, button) => GLFW.getMouseButton(winid, button),

  glfwGetCursorPos: (winid, x, y) => GLFW.getCursorPos(winid, x, y),

  // I believe it is not possible to move the mouse with javascript
  glfwSetCursorPos: (winid, x, y) => GLFW.setCursorPos(winid, x, y),

  glfwSetKeyCallback: (winid, cbfun) => GLFW.setKeyCallback(winid, cbfun),

  glfwSetCharCallback: (winid, cbfun) => GLFW.setCharCallback(winid, cbfun),

  glfwSetCharModsCallback: (winid, cbfun) => { throw "glfwSetCharModsCallback not implemented."; },

  glfwSetMouseButtonCallback: (winid, cbfun) => GLFW.setMouseButtonCallback(winid, cbfun),

  glfwSetCursorPosCallback: (winid, cbfun) => GLFW.setCursorPosCallback(winid, cbfun),

  glfwSetCursorEnterCallback: (winid, cbfun) => {
    var win = GLFW.WindowFromId(winid);
    if (!win) return null;
    var prevcbfun = win.cursorEnterFunc;
    win.cursorEnterFunc = cbfun;
    return prevcbfun;
  },

  glfwSetScrollCallback: (winid, cbfun) => GLFW.setScrollCallback(winid, cbfun),

  glfwVulkanSupported: () => 0,

  glfwSetDropCallback: (winid, cbfun) => GLFW.setDropCallback(winid, cbfun),

  glfwGetTimerValue: () => { throw "glfwGetTimerValue is not implemented."; },

  glfwGetTimerFrequency: () => { throw "glfwGetTimerFrequency is not implemented."; },

  glfwGetRequiredInstanceExtensions: (count) => { throw "glfwGetRequiredInstanceExtensions is not implemented."; },

  glfwJoystickPresent: (joy) => {
    GLFW.refreshJoysticks();

    return GLFW.joys[joy] !== undefined;
  },

  glfwGetJoystickAxes: (joy, count) => {
    GLFW.refreshJoysticks();

    var state = GLFW.joys[joy];
    if (!state || !state.axes) {
      {{{ makeSetValue('count', '0', '0', 'i32') }}};
      return;
    }

    {{{ makeSetValue('count', '0', 'state.axesCount', 'i32') }}};
    return state.axes;
  },

  glfwGetJoystickButtons: (joy, count) => {
    GLFW.refreshJoysticks();

    var state = GLFW.joys[joy];
    if (!state || !state.buttons) {
      {{{ makeSetValue('count', '0', '0', 'i32') }}};
      return;
    }

    {{{ makeSetValue('count', '0', 'state.buttonsCount', 'i32') }}};
    return state.buttons;
  },

  glfwGetJoystickHats: (joy, count) => { throw "glfwGetJoystickHats is not implemented"; },

  glfwGetJoystickName: (joy) => {
    if (GLFW.joys[joy]) {
      return GLFW.joys[joy].id;
    }
    return 0;
  },

  glfwGetJoystickGUID: (jid) => { throw "glfwGetJoystickGUID not implemented"; },

  glfwSetJoystickUserPointer: (jid, ptr) => { throw "glfwSetJoystickUserPointer not implemented"; },

  glfwGetJoystickUserPointer: (jid) => { throw "glfwGetJoystickUserPointer not implemented"; },

  glfwJoystickIsGamepad: (jid) => { throw "glfwJoystickIsGamepad not implemented"; },

  glfwSetJoystickCallback: (cbfun) => GLFW.setJoystickCallback(cbfun),

  glfwSetClipboardString: (win, string) => {},

  glfwGetClipboardString: (win) => {},

  glfwMakeContextCurrent: (winid) => {},

  glfwGetCurrentContext: () => GLFW.active ? GLFW.active.id : 0,

  glfwSwapBuffers: (winid) => GLFW.swapBuffers(winid),

#elif USE_GLFW == 2
  glfwOpenWindow: (width, height, redbits, greenbits, bluebits, alphabits, depthbits, stencilbits, mode) => {
    GLFW.hints[0x00021001] = redbits;     // GLFW_RED_BITS
    GLFW.hints[0x00021002] = greenbits;   // GLFW_GREEN_BITS
    GLFW.hints[0x00021003] = bluebits;    // GLFW_BLUE_BITS
    GLFW.hints[0x00021004] = alphabits;   // GLFW_ALPHA_BITS
    GLFW.hints[0x00021005] = depthbits;   // GLFW_DEPTH_BITS
    GLFW.hints[0x00021006] = stencilbits; // GLFW_STENCIL_BITS
    GLFW.createWindow(width, height, "GLFW2 Window", 0, 0);
    return 1; // GL_TRUE
  },

  glfwCloseWindow: () => GLFW.destroyWindow(GLFW.active.id),

  glfwOpenWindowHint: (target, hint) => {
    target = GLFW.GLFW2ParamToGLFW3Param(target);
    GLFW.hints[target] = hint;
  },

  glfwGetWindowSize_v2: (width, height) => GLFW.getWindowSize(GLFW.active.id, width, height),

  glfwSetWindowSize_v2: (width, height) => GLFW.setWindowSize(GLFW.active.id, width, height),

  glfwSetWindowPos_v2: (x, y) => GLFW.setWindowPos(GLFW.active.id, x, y),

  glfwSetWindowTitle_v2: (title) => GLFW.setWindowTitle(GLFW.active.id, title),

  glfwIconifyWindow_v2: () => {
#if ASSERTIONS
    warnOnce('glfwIconifyWindow is not implemented');
#endif
  },

  glfwRestoreWindow_v2: () => {
#if ASSERTIONS
    warnOnce('glfwRestoreWindow is not implemented');
#endif
  },

  glfwSwapBuffers_v2: () => GLFW.swapBuffers(GLFW.active.id),

  glfwGetWindowParam: (param) => {
    param = GLFW.GLFW2ParamToGLFW3Param(param);
    return GLFW.hints[param];
  },

  glfwSetWindowSizeCallback_v2: (cbfun) => {
    GLFW.setWindowSizeCallback(GLFW.active.id, cbfun);
  },

  glfwSetWindowCloseCallback_v2: (cbfun) => {
    GLFW.setWindowCloseCallback(GLFW.active.id, cbfun);
  },

  glfwSetWindowRefreshCallback_v2: (cbfun) => GLFW.setWindowRefreshCallback(GLFW.active.id, cbfun),

  glfwGetKey_v2: (key) => GLFW.getKey(GLFW.active.id, key),

  glfwGetMouseButton_v2: (button) => GLFW.getMouseButton(GLFW.active.id, button),

  glfwGetMousePos: (x, y) => {
    GLFW.getMousePos(GLFW.active.id, x, y);
  },

  glfwSetMousePos: (x, y) => {
    GLFW.setCursorPos(GLFW.active.id, x, y);
  },

  glfwGetMouseWheel: () => 0,

  glfwSetMouseWheel: (pos) => {},

  glfwSetKeyCallback_v2: (cbfun) => {
    GLFW.setKeyCallback(GLFW.active.id, cbfun);
  },

  glfwSetCharCallback_v2: (cbfun) => {
    GLFW.setCharCallback(GLFW.active.id, cbfun);
  },

  glfwSetMouseButtonCallback_v2: (cbfun) => {
    GLFW.setMouseButtonCallback(GLFW.active.id, cbfun);
  },

  glfwSetMousePosCallback: (cbfun) => {
    GLFW.setCursorPosCallback(GLFW.active.id, cbfun);
  },

  glfwSetMouseWheelCallback: (cbfun) => {
    GLFW.setScrollCallback(GLFW.active.id, cbfun);
  },

  glfwGetDesktopMode: (mode) => { throw "glfwGetDesktopMode is not implemented."; },

  glfwSleep__deps: ['sleep'],
  glfwSleep: (time) => _sleep(time),

  glfwEnable: (target) => {
    target = GLFW.GLFW2ParamToGLFW3Param(target);
    GLFW.hints[target] = false;
  },

  glfwDisable: (target) => {
    target = GLFW.GLFW2ParamToGLFW3Param(target);
    GLFW.hints[target] = true;
  },

  glfwGetGLVersion: (major, minor, rev) => {
    {{{ makeSetValue('major', '0', '0', 'i32') }}};
    {{{ makeSetValue('minor', '0', '0', 'i32') }}};
    {{{ makeSetValue('rev', '0', '1', 'i32') }}};
  },

  glfwCreateThread: (fun, arg) => {
    {{{ makeDynCall('vp', 'fun') }}}(arg);
    // One single thread
    return 0;
  },

  glfwDestroyThread: (ID) => {},

  glfwWaitThread: (ID, waitmode) => {},

  // One single thread
  glfwGetThreadID: () => 0,

  glfwCreateMutex: () => { throw "glfwCreateMutex is not implemented."; },

  glfwDestroyMutex: (mutex) => { throw "glfwDestroyMutex is not implemented."; },

  glfwLockMutex: (mutex) => { throw "glfwLockMutex is not implemented."; },

  glfwUnlockMutex: (mutex) => { throw "glfwUnlockMutex is not implemented."; },

  glfwCreateCond: () => { throw "glfwCreateCond is not implemented."; },

  glfwDestroyCond: (cond) => { throw "glfwDestroyCond is not implemented."; },

  glfwWaitCond: (cond, mutex, timeout) => { throw "glfwWaitCond is not implemented."; },

  glfwSignalCond: (cond) => { throw "glfwSignalCond is not implemented."; },

  glfwBroadcastCond: (cond) => { throw "glfwBroadcastCond is not implemented."; },

  glfwGetNumberOfProcessors: () => 1, // Threads are disabled anyway…

  glfwReadImage: (name, img, flags) => { throw "glfwReadImage is not implemented."; },

  glfwReadMemoryImage: (data, size, img, flags) => { throw "glfwReadMemoryImage is not implemented."; },

  glfwFreeImage: (img) => { throw "glfwFreeImage is not implemented."; },

  glfwLoadTexture2D: (name, flags) => { throw "glfwLoadTexture2D is not implemented."; },

  glfwLoadMemoryTexture2D: (data, size, flags) => { throw "glfwLoadMemoryTexture2D is not implemented."; },

  glfwLoadTextureImage2D: (img, flags) => { throw "glfwLoadTextureImage2D is not implemented."; },
#endif // GLFW2
};

autoAddDeps(LibraryGLFW, '$GLFW');
addToLibrary(LibraryGLFW);
