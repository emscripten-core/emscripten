/*******************************************************************************
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
 * - Joystick support.
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
 *
 ******************************************************************************/

var LibraryGLFW = {
  $GLFW__deps: ['emscripten_get_now', '$GL', '$Browser'],
  $GLFW: {

    Window: function(id, width, height, title, monitor, share) {
      this.id = id;
      this.x = 0;
      this.y = 0;
      this.fullscreen = false; // Used to determine if app in fullscreen mode
      this.storedX = 0; // Used to store X before fullscreen
      this.storedY = 0; // Used to store Y before fullscreen
      this.width = width;
      this.height = height;
      this.storedWidth = width; // Used to store width before fullscreen
      this.storedHeight = height; // Used to store height before fullscreen
      this.title = title;
      this.monitor = monitor;
      this.share = share;
      this.attributes = GLFW.hints;
      this.inputModes = {
        0x00033001:0x00034001, // GLFW_CURSOR (GLFW_CURSOR_NORMAL)
        0x00033002:0, // GLFW_STICKY_KEYS
        0x00033003:0, // GLFW_STICKY_MOUSE_BUTTONS
      };
      this.buttons = 0;
      this.keys = new Array();
      this.shouldClose = 0;
      this.title = null;
      this.windowPosFunc = null; // GLFWwindowposfun
      this.windowSizeFunc = null; // GLFWwindowsizefun
      this.windowCloseFunc = null; // GLFWwindowclosefun
      this.windowRefreshFunc = null; // GLFWwindowrefreshfun
      this.windowFocusFunc = null; // GLFWwindowfocusfun
      this.windowIconifyFunc = null; // GLFWwindowiconifyfun
      this.framebufferSizeFunc = null; // GLFWframebuffersizefun
      this.mouseButtonFunc = null; // GLFWmousebuttonfun
      this.cursorPosFunc = null; // GLFWcursorposfun
      this.cursorEnterFunc = null; // GLFWcursorenterfun
      this.scrollFunc = null; // GLFWscrollfun
      this.keyFunc = null; // GLFWkeyfun
      this.charFunc = null; // GLFWcharfun
      this.userptr = null;
    },

    WindowFromId: function(id) {
      if (id <= 0 || !GLFW.windows) return null;
      return GLFW.windows[id - 1];
    },

    errorFunc: null, // GLFWerrorfun
    monitorFunc: null, // GLFWmonitorfun
    active: null, // active window
    windows: null,
    monitors: null,
    monitorString: null,
    versionString: null,
    initialTime: null,
    extensions: null,
    hints: null,
    defaultHints: {
      0x00020001:0, // GLFW_FOCUSED
      0x00020002:0, // GLFW_ICONIFIED
      0x00020003:1, // GLFW_RESIZABLE
      0x00020004:1, // GLFW_VISIBLE
      0x00020005:1, // GLFW_DECORATED

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
    DOMToGLFWKeyCode: function(keycode) {
      switch (keycode) {
        // these keycodes are only defined for GLFW3, assume they are the same for GLFW2
        case 0x20:return 32; // DOM_VK_SPACE -> GLFW_KEY_SPACE
        case 0xDE:return 39; // DOM_VK_QUOTE -> GLFW_KEY_APOSTROPHE
        case 0xBC:return 44; // DOM_VK_COMMA -> GLFW_KEY_COMMA
        case 0xAD:return 45; // DOM_VK_HYPHEN_MINUS -> GLFW_KEY_MINUS
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
        case 0x61:return 61; // DOM_VK_EQUALS -> GLFW_KEY_EQUAL
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
        case 0xC0:return 94; // DOM_VK_BACK_QUOTE -> GLFW_KEY_GRAVE_ACCENT
        
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
        case 0x27:return (256+27); // DOM_VK_RIGHT -> GLFW_KEY_RIGHT
        case 0x25:return (256+28); // DOM_VK_LEFT -> GLFW_KEY_LEFT
        case 0x28:return (256+29); // DOM_VK_DOWN -> GLFW_KEY_DOWN
        case 0x26:return (256+30); // DOM_VK_UP -> GLFW_KEY_UP
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

    getModBits: function(win) {
      var mod = 0;
      if (win.keys[340]) mod |= 0x0001; // GLFW_MOD_SHIFT
      if (win.keys[341]) mod |= 0x0002; // GLFW_MOD_CONTROL
      if (win.keys[342]) mod |= 0x0004; // GLFW_MOD_ALT
      if (win.keys[343]) mod |= 0x0008; // GLFW_MOD_SUPER
      return mod;
    },

    onKeyPress: function(event) {
      if (!GLFW.active || !GLFW.active.charFunc) return;

      // correct unicode charCode is only available with onKeyPress event
      var charCode = event.charCode;
      if (charCode == 0 || (charCode >= 0x00 && charCode <= 0x1F)) return;

#if USE_GLFW == 2
      Runtime.dynCall('vii', GLFW.active.charFunc, [charCode, 1]);
#endif

#if USE_GLFW == 3
      Runtime.dynCall('vii', GLFW.active.charFunc, [GLFW.active.id, charCode]);
#endif
    },

    onKeyChanged: function(event, status) {
      if (!GLFW.active) return;

      var key = GLFW.DOMToGLFWKeyCode(event.keyCode);
      if (key == -1) return;

#if USE_GLFW == 3
      var repeat = status && GLFW.active.keys[key];
#endif
      GLFW.active.keys[key] = status;
      if (!GLFW.active.keyFunc) return;

#if USE_GLFW == 2
      Runtime.dynCall('vii', GLFW.active.keyFunc, [key, status]);
#endif

#if USE_GLFW == 3
      if (repeat) status = 2; // GLFW_REPEAT
      Runtime.dynCall('viiiii', GLFW.active.keyFunc, [GLFW.active.id, key, event.keyCode, status, GLFW.getModBits(GLFW.active)]);
#endif
    },

    onKeydown: function(event) {
      GLFW.onKeyChanged(event, 1); // GLFW_PRESS or GLFW_REPEAT

      // This logic comes directly from the sdl implementation. We cannot
      // call preventDefault on all keydown events otherwise onKeyPress will
      // not get called
      if (event.keyCode === 8 /* backspace */ || event.keyCode === 9 /* tab */) {
        event.preventDefault();
      }
    },

    onKeyup: function(event) {
      GLFW.onKeyChanged(event, 0); // GLFW_RELEASE
    },

    onMousemove: function(event) {
      if (!GLFW.active) return;

      Browser.calculateMouseEvent(event);

      if (event.target != Module["canvas"] || !GLFW.active.cursorPosFunc) return;

#if USE_GLFW == 2
      Runtime.dynCall('vii', GLFW.active.cursorPosFunc, [Browser.mouseX, Browser.mouseY]);
#endif

#if USE_GLFW == 3
      Runtime.dynCall('vidd', GLFW.active.cursorPosFunc, [GLFW.active.id, Browser.mouseX, Browser.mouseY]);
#endif
    },

    onMouseenter: function(event) {
      if (!GLFW.active) return;

      if (event.target != Module["canvas"] || !GLFW.active.cursorEnterFunc) return;

#if USE_GLFW == 3
      Runtime.dynCall('vii', GLFW.active.cursorEnterFunc, [GLFW.active.id, 1]);
#endif
    },

    onMouseleave: function(event) {
      if (!GLFW.active) return;

      if (event.target != Module["canvas"] || !GLFW.active.cursorEnterFunc) return;

#if USE_GLFW == 3
      Runtime.dynCall('vii', GLFW.active.cursorEnterFunc, [GLFW.active.id, 0]);
#endif
    },

    onMouseButtonChanged: function(event, status) {
      if (!GLFW.active || !GLFW.active.mouseButtonFunc) return;

      Browser.calculateMouseEvent(event);

      if (event.target != Module["canvas"]) return;

      if (status == 1) { // GLFW_PRESS
        try {
          event.target.setCapture();
        } catch (e) {}
      }

      // DOM and glfw have different button codes
      var eventButton = event['button'];
      if (eventButton > 0) {
        if (eventButton == 1) {
          eventButton = 2;
        } else {
          eventButton = 1;
        }
      }

#if USE_GLFW == 2
      Runtime.dynCall('vii', GLFW.active.mouseButtonFunc, [eventButton, status]);
#endif

#if USE_GLFW == 3
      Runtime.dynCall('viiii', GLFW.active.mouseButtonFunc, [GLFW.active.id, eventButton, status, GLFW.getModBits(GLFW.active)]);
#endif
    },

    onMouseButtonDown: function(event) {
      if (!GLFW.active) return;
      GLFW.active.buttons |= (1 << event['button']);
      GLFW.onMouseButtonChanged(event, 1); // GLFW_PRESS
    },

    onMouseButtonUp: function(event) {
      if (!GLFW.active) return;
      GLFW.active.buttons &= ~(1 << event['button']);
      GLFW.onMouseButtonChanged(event, 0); // GLFW_RELEASE
    },

    onMouseWheel: function(event) {
      // Note the minus sign that flips browser wheel direction (positive direction scrolls page down) to native wheel direction (positive direction is mouse wheel up)
      var delta = -Browser.getMouseWheelDelta(event);
      delta = (delta == 0) ? 0 : (delta > 0 ? Math.max(delta, 1) : Math.min(delta, -1)); // Quantize to integer so that minimum scroll is at least +/- 1.
      GLFW.wheelPos += delta;

      if (!GLFW.active || !GLFW.active.scrollFunc || event.target != Module['canvas']) return;

#if USE_GLFW == 2
      Runtime.dynCall('vi', GLFW.active.scrollFunc, [GLFW.wheelPos]);
#endif

#if USE_GLFW == 3
      var sx = 0;
      var sy = 0;
      if (event.type == 'mousewheel') {
        sx = event.wheelDeltaX;
        sy = event.wheelDeltaY;
      } else {
        sx = event.deltaX;
        sy = event.deltaY;
      }

      Runtime.dynCall('vidd', GLFW.active.scrollFunc, [GLFW.active.id, sx, sy]);
#endif

      event.preventDefault();
    },

    onCanvasResize: function(width, height) {
      if (!GLFW.active) return;

      var resizeNeeded = true;

      // If the client is requestiong fullscreen mode
      if (document["fullscreen"] || document["fullScreen"] || document["mozFullScreen"] || document["webkitIsFullScreen"]) {
        GLFW.active.storedX = GLFW.active.x;
        GLFW.active.storedY = GLFW.active.y;
        GLFW.active.storedWidth = GLFW.active.width;
        GLFW.active.storedHeight = GLFW.active.height;
        GLFW.active.x = GLFW.active.y = 0;
        GLFW.active.width = screen.width;
        GLFW.active.height = screen.height;
        GLFW.active.fullscreen = true;

      // If the client is reverting from fullscreen mode
      } else if (GLFW.active.fullscreen == true) {
        GLFW.active.x = GLFW.active.storedX;
        GLFW.active.y = GLFW.active.storedY;
        GLFW.active.width = GLFW.active.storedWidth;
        GLFW.active.height = GLFW.active.storedHeight;
        GLFW.active.fullscreen = false;

      // If the width/height values do not match current active window sizes
      } else if (GLFW.active.width != width || GLFW.active.height != height) {
          GLFW.active.width = width;
          GLFW.active.height = height;
      } else {
        resizeNeeded = false;
      }

      // If any of the above conditions were true, we need to resize the canvas
      if (resizeNeeded) {
        // resets the canvas size to counter the aspect preservation of Browser.updateCanvasDimensions
        Browser.setCanvasSize(GLFW.active.width, GLFW.active.height);
        // TODO: Client dimensions (clientWidth/clientHeight) vs pixel dimensions (width/height) of
        // the canvas should drive window and framebuffer size respectfully.
        GLFW.onWindowSizeChanged();
        GLFW.onFramebufferSizeChanged();
      }
    },

    onWindowSizeChanged: function() {
      if (!GLFW.active) return;

      if (!GLFW.active.windowSizeFunc) return;

#if USE_GLFW == 2
      Runtime.dynCall('vii', GLFW.active.windowSizeFunc, [GLFW.active.width, GLFW.active.height]);
#endif

#if USE_GLFW == 3
      Runtime.dynCall('viii', GLFW.active.windowSizeFunc, [GLFW.active.id, GLFW.active.width, GLFW.active.height]);
#endif
    },

    onFramebufferSizeChanged: function() {
      if (!GLFW.active) return;

      if (!GLFW.active.framebufferSizeFunc) return;

#if USE_GLFW == 3
      Runtime.dynCall('viii', GLFW.active.framebufferSizeFunc, [GLFW.active.id, GLFW.active.width, GLFW.active.height]);
#endif
    },

    requestFullscreen: function() {
      var RFS = Module["canvas"]['requestFullscreen'] ||
                Module["canvas"]['mozRequestFullScreen'] ||
                Module["canvas"]['webkitRequestFullScreen'] ||
                (function() {});
      RFS.apply(Module["canvas"], []);
    },

    requestFullScreen: function() {
      Module.printErr('GLFW.requestFullScreen() is deprecated. Please call GLFW.requestFullscreen instead.');
      GLFW.requestFullScreen = function() {
        return GLFW.requestFullscreen();
      }
      return GLFW.requestFullscreen();
    },

    exitFullscreen: function() {
      var CFS = document['exitFullscreen'] ||
                document['cancelFullScreen'] ||
                document['mozCancelFullScreen'] ||
                document['webkitCancelFullScreen'] ||
          (function() {});
      CFS.apply(document, []);
    },

    cancelFullScreen: function() {
      Module.printErr('GLFW.cancelFullScreen() is deprecated. Please call GLFW.exitFullscreen instead.');
      GLFW.cancelFullScreen = function() {
        return GLFW.exitFullscreen();
      }
      return GLFW.exitFullscreen();
    },

    getTime: function() {
      return _emscripten_get_now() / 1000;
    },

    /* GLFW2 wrapping */

    setWindowTitle: function(winid, title) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;

      win.title = Pointer_stringify(title);
      if (GLFW.active.id == win.id) {
        document.title = win.title;
      }
    },

    setKeyCallback: function(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.keyFunc = cbfun;
    },

    setCharCallback: function(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.charFunc = cbfun;
    },

    setMouseButtonCallback: function(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.mouseButtonFunc = cbfun;
    },

    setCursorPosCallback: function(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.cursorPosFunc = cbfun;
    },

    setScrollCallback: function(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.scrollFunc = cbfun;
    },

    setWindowSizeCallback: function(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.windowSizeFunc = cbfun;
     
#if USE_GLFW == 2
      // As documented in GLFW2 API (http://www.glfw.org/GLFWReference27.pdf#page=22), when size
      // callback function is set, it will be called with the current window size before this
      // function returns.
      // GLFW3 on the over hand doesn't have this behavior (https://github.com/glfw/glfw/issues/62).
      if (!win.windowSizeFunc) return;
      Runtime.dynCall('vii', win.windowSizeFunc, [win.width, win.height]);
#endif
    },

    setWindowCloseCallback: function(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.windowCloseFunc = cbfun;
    },

    setWindowRefreshCallback: function(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.windowRefreshFunc = cbfun;
    },

    onClickRequestPointerLock: function(e) {
      if (!Browser.pointerLock && Module['canvas'].requestPointerLock) {
        Module['canvas'].requestPointerLock();
        e.preventDefault();
      }
    },

    setInputMode: function(winid, mode, value) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;

      switch(mode) {
        case 0x00033001: { // GLFW_CURSOR
          switch(value) {
            case 0x00034001: { // GLFW_CURSOR_NORMAL
              win.inputModes[mode] = value;
              Module['canvas'].removeEventListener('click', GLFW.onClickRequestPointerLock, true);
              Module['canvas'].exitPointerLock();
              break;
            }
            case 0x00034002: { // GLFW_CURSOR_HIDDEN
              console.log("glfwSetInputMode called with GLFW_CURSOR_HIDDEN value not implemented.");
              break;
            }
            case 0x00034003: { // GLFW_CURSOR_DISABLED
              win.inputModes[mode] = value;
              Module['canvas'].addEventListener('click', GLFW.onClickRequestPointerLock, true);
              Module['canvas'].requestPointerLock();
              break;
            }
            default: {
              console.log("glfwSetInputMode called with unknown value parameter value: " + value + ".");
              break;
            }
          }
          break;
        }
        case 0x00033002: { // GLFW_STICKY_KEYS
          console.log("glfwSetInputMode called with GLFW_STICKY_KEYS mode not implemented.");
          break;
        }
        case 0x00033003: { // GLFW_STICKY_MOUSE_BUTTONS
          console.log("glfwSetInputMode called with GLFW_STICKY_MOUSE_BUTTONS mode not implemented.");
          break;
        }
        default: {
          console.log("glfwSetInputMode called with unknown mode parameter value: " + mode + ".");
          break;
        }
      }
    },

    getKey: function(winid, key) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return 0;
      return win.keys[key];
    },

    getMouseButton: function(winid, button) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return 0;
      return (win.buttons & (1 << button)) > 0;
    },

    getCursorPos: function(winid, x, y) {
      setValue(x, Browser.mouseX, 'double');
      setValue(y, Browser.mouseY, 'double');
    },

    getMousePos: function(winid, x, y) {
      setValue(x, Browser.mouseX, 'i32');
      setValue(y, Browser.mouseY, 'i32');
    },

    setCursorPos: function(winid, x, y) {
    },

    getWindowPos: function(winid, x, y) {
      var wx = 0;
      var wy = 0;

      var win = GLFW.WindowFromId(winid);
      if (win) {
        wx = win.x;
        wy = win.y;
      }

      setValue(x, wx, 'i32');
      setValue(y, wy, 'i32');
    },

    setWindowPos: function(winid, x, y) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.x = x;
      win.y = y;
    },

    getWindowSize: function(winid, width, height) {
      var ww = 0;
      var wh = 0;

      var win = GLFW.WindowFromId(winid);
      if (win) {
        ww = win.width;
        wh = win.height;
      }

      setValue(width, ww, 'i32');
      setValue(height, wh, 'i32');
    },

    setWindowSize: function(winid, width, height) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;

      if (GLFW.active.id == win.id) {
        if (width == screen.width && height == screen.height) {
          GLFW.requestFullscreen();
        } else {
          GLFW.exitFullscreen();
          Browser.setCanvasSize(width, height);
          win.width = width;
          win.height = height;
        }
      }

      if (!win.windowSizeFunc) return;

#if USE_GLFW == 2
      Runtime.dynCall('vii', win.windowSizeFunc, [width, height]);
#endif

#if USE_GLFW == 3
      Runtime.dynCall('viii', win.windowSizeFunc, [win.id, width, height]);
#endif
    },

    createWindow: function(width, height, title, monitor, share) {
      var i, id;
      for (i = 0; i < GLFW.windows.length && GLFW.windows[i] !== null; i++);
      if (i > 0) throw "glfwCreateWindow only supports one window at time currently";

      // id for window
      id = i + 1;

      // not valid
      if (width <= 0 || height <= 0) return 0;

      if (monitor) {
        GLFW.requestFullscreen();
      } else {
        Browser.setCanvasSize(width, height);
      }

      // Create context when there are no existing alive windows
      for (i = 0; i < GLFW.windows.length && GLFW.windows[i] == null; i++);
      if (i == GLFW.windows.length) {
        var contextAttributes = {
          antialias: (GLFW.hints[0x0002100D] > 1), // GLFW_SAMPLES
          depth: (GLFW.hints[0x00021005] > 0),     // GLFW_DEPTH_BITS
          stencil: (GLFW.hints[0x00021006] > 0),   // GLFW_STENCIL_BITS
          alpha: (GLFW.hints[0x00021004] > 0)      // GLFW_ALPHA_BITS 
        }
        Module.ctx = Browser.createContext(Module['canvas'], true, true, contextAttributes);
      }

      // If context creation failed, do not return a valid window
      if (!Module.ctx) return 0;

      // Get non alive id
      var win = new GLFW.Window(id, width, height, title, monitor, share);

      // Set window to array
      if (id - 1 == GLFW.windows.length) {
        GLFW.windows.push(win);
      } else {
        GLFW.windows[id - 1] = win;
      }

      GLFW.active = win;
      return win.id;
    },

    destroyWindow: function(winid) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;

#if USE_GLFW == 3
      if (win.windowCloseFunc)
        Runtime.dynCall('vi', win.windowCloseFunc, [win.id]);
#endif

      GLFW.windows[win.id - 1] = null;
      if (GLFW.active.id == win.id)
        GLFW.active = null;

      // Destroy context when no alive windows
      for (var i = 0; i < GLFW.windows.length; i++)
        if (GLFW.windows[i] !== null) return;

      Module.ctx = Browser.destroyContext(Module['canvas'], true, true);
    },

    swapBuffers: function(winid) {
    },

    GLFW2ParamToGLFW3Param: function(param) {
      table = {
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

  glfwInit: function() {
    if (GLFW.windows) return 1; // GL_TRUE

    GLFW.initialTime = GLFW.getTime();
    GLFW.hints = GLFW.defaultHints;
    GLFW.windows = new Array()
    GLFW.active = null;

    window.addEventListener("keydown", GLFW.onKeydown, true);
    window.addEventListener("keypress", GLFW.onKeyPress, true);
    window.addEventListener("keyup", GLFW.onKeyup, true);
    Module["canvas"].addEventListener("mousemove", GLFW.onMousemove, true);
    Module["canvas"].addEventListener("mousedown", GLFW.onMouseButtonDown, true);
    Module["canvas"].addEventListener("mouseup", GLFW.onMouseButtonUp, true);
    Module["canvas"].addEventListener('wheel', GLFW.onMouseWheel, true);
    Module["canvas"].addEventListener('mousewheel', GLFW.onMouseWheel, true);
    Module["canvas"].addEventListener('mouseenter', GLFW.onMouseenter, true);
    Module["canvas"].addEventListener('mouseleave', GLFW.onMouseleave, true);

    Browser.resizeListeners.push(function(width, height) {
       GLFW.onCanvasResize(width, height);
    });
    return 1; // GL_TRUE
  },

  glfwTerminate: function() {
    window.removeEventListener("keydown", GLFW.onKeydown, true);
    window.removeEventListener("keypress", GLFW.onKeyPress, true);
    window.removeEventListener("keyup", GLFW.onKeyup, true);
    Module["canvas"].removeEventListener("mousemove", GLFW.onMousemove, true);
    Module["canvas"].removeEventListener("mousedown", GLFW.onMouseButtonDown, true);
    Module["canvas"].removeEventListener("mouseup", GLFW.onMouseButtonUp, true);
    Module["canvas"].removeEventListener('wheel', GLFW.onMouseWheel, true);
    Module["canvas"].removeEventListener('mousewheel', GLFW.onMouseWheel, true);
    Module["canvas"].removeEventListener('mouseenter', GLFW.onMouseenter, true);
    Module["canvas"].removeEventListener('mouseleave', GLFW.onMouseleave, true);
    Module["canvas"].width = Module["canvas"].height = 1;
    GLFW.windows = null;
    GLFW.active = null;
  },

  glfwGetVersion: function(major, minor, rev) {
#if USE_GLFW == 2
    setValue(major, 2, 'i32');
    setValue(minor, 7, 'i32');
    setValue(rev, 7, 'i32');
#endif

#if USE_GLFW == 3
    setValue(major, 3, 'i32');
    setValue(minor, 0, 'i32');
    setValue(rev, 0, 'i32');
#endif
  },

  glfwPollEvents: function() {},

  glfwWaitEvents: function() {},

  glfwGetTime: function() {
    return GLFW.getTime() - GLFW.initialTime;
  },

  glfwSetTime: function(time) {
    GLFW.initialTime = GLFW.getTime() + time;
  },

  glfwExtensionSupported: function(extension) {
    if (!GLFW.extensions) {
      GLFW.extensions = Pointer_stringify(_glGetString(0x1F03)).split(' ');
    }

    if (GLFW.extensions.indexOf(extension) != -1) return 1;

    // extensions from GLEmulations do not come unprefixed
    // so, try with prefix
    return (GLFW.extensions.indexOf("GL_" + extension) != -1);
  },

  glfwGetProcAddress__deps: ['emscripten_GetProcAddress'],
  glfwGetProcAddress: function(procname) {
    return _emscripten_GetProcAddress(procname);
  },

  glfwSwapInterval__deps: ['emscripten_set_main_loop_timing'],
  glfwSwapInterval: function(interval) {
    interval = Math.abs(interval); // GLFW uses negative values to enable GLX_EXT_swap_control_tear, which we don't have, so just treat negative and positive the same.
    if (interval == 0) _emscripten_set_main_loop_timing(0/*EM_TIMING_SETTIMEOUT*/, 0);
    else _emscripten_set_main_loop_timing(1/*EM_TIMING_RAF*/, interval);
  },

#if USE_GLFW == 3
  glfwGetVersionString: function() {
    if (!GLFW.versionString) {
      GLFW.versionString = allocate(intArrayFromString("3.0.0 JS WebGL Emscripten"), 'i8', ALLOC_NORMAL);
    }
    return GLFW.versionString;
  },

  glfwSetErrorCallback: function(cbfun) {
    GLFW.errorFunc = cbfun;
  },

  glfwGetMonitors: function(count) {
    setValue(count, 1, 'i32');
    if (!GLFW.monitors) {
      GLFW.monitors = allocate([1, 0, 0, 0], 'i32', ALLOC_NORMAL);
    }
    return GLFW.monitors;
  },

  glfwGetPrimaryMonitor: function() {
    return 1;
  },

  glfwGetMonitorPos: function(monitor, x, y) {
    setValue(x, 0, 'i32');
    setValue(y, 0, 'i32');
  },

  glfwGetMonitorPhysicalSize: function(monitor, width, height) {
    // AFAIK there is no way to do this in javascript
    // Maybe with platform specific ccalls?
    //
    // Lets report 0 now which is wrong as it can get for end user.
    setValue(width, 0, 'i32');
    setValue(height, 0, 'i32');
  },

  glfwGetMonitorName: function(mon) {
    if (!GLFW.monitorString) {
      GLFW.monitorString = allocate(intArrayFromString("HTML5 WebGL Canvas"), 'i8', ALLOC_NORMAL);
    }
    return GLFW.monitorString;
  },

  glfwSetMonitorCallback: function(cbfun) {
    GLFW.monitorFunc = cbfun;
  },

  // TODO: implement
  glfwGetVideoModes: function(monitor, count) {
    setValue(count, 0, 'i32');
    return 0;
  },

  // TODO: implement
  glfwGetVideoMode: function(monitor) { return 0; },

  // TODO: implement
  glfwSetGamma: function(monitor, gamma) { },

  glfwGetGammaRamp: function(monitor) {
    throw "glfwGetGammaRamp not implemented.";
  },

  glfwSetGammaRamp: function(monitor, ramp) {
    throw "glfwSetGammaRamp not implemented.";
  },

  glfwDefaultWindowHints: function() {
    GLFW.hints = GLFW.defaultHints;
  },

  glfwWindowHint: function(target, hint) {
    GLFW.hints[target] = hint;
  },

  glfwCreateWindow: function(width, height, title, monitor, share) {
    return GLFW.createWindow(width, height, title, monitor, share);
  },

  glfwDestroyWindow: function(winid) {
    return GLFW.destroyWindow(winid);
  },

  glfwWindowShouldClose: function(winid) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return 0;
    return win.shouldClose;
  },

  glfwSetWindowShouldClose: function(winid, value) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    win.shouldClose = value;
  },

  glfwSetWindowTitle: function(winid, title) {
    GLFW.setWindowTitle(winid, title);
  },

  glfwGetWindowPos: function(winid, x, y) {
    GLFW.getWindowPos(winid, x, y);
  },

  glfwSetWindowPos: function(winid, x, y) {
    GLFW.setWindowPos(winid, x, y);
  },

  glfwGetWindowSize: function(winid, width, height) {
    GLFW.getWindowSize(winid, width, height);
  },

  glfwSetWindowSize: function(winid, width, height) {
    GLFW.setWindowSize(winid, width, height);
  },

  glfwGetFramebufferSize: function(winid, width, height) {
    var ww = 0;
    var wh = 0;

    var win = GLFW.WindowFromId(winid);
    if (win) {
      ww = win.width;
      wh = win.height;
    }

    setValue(width, ww, 'i32');
    setValue(height, wh, 'i32');
  },

  glfwIconifyWindow: function(winid) {
    GLFW.iconifyWindow(winid);
  },

  glfwRestoreWindow: function(winid) {
    GLFW.restoreWindow(winid);
  },

  glfwShowWindow: function(winid) {},

  glfwHideWindow: function(winid) {},

  glfwGetWindowMonitor: function(winid) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return 0;
    return win.monitor;
  },

  glfwGetWindowAttrib: function(winid, attrib) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return 0;
    return win.attributes[attrib];
  },

  glfwSetWindowUserPointer: function(winid, ptr) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    win.userptr = ptr;
  },

  glfwGetWindowUserPointer: function(winid) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return 0;
    return win.userptr;
  },

  glfwSetWindowPosCallback: function(winid, cbfun) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    win.windowPosFunc = cbfun;
  },

  glfwSetWindowSizeCallback: function(winid, cbfun) {
    GLFW.setWindowSizeCallback(winid, cbfun);
  },

  glfwSetWindowCloseCallback: function(winid, cbfun) {
    GLFW.setWindowCloseCallback(winid, cbfun);
  },

  glfwSetWindowRefreshCallback: function(winid, cbfun) {
    GLFW.setWindowRefreshCallback(winid, cbfun);
  },

  glfwSetWindowFocusCallback: function(winid, cbfun) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    win.windowFocusFunc = cbfun;
  },

  glfwSetWindowIconifyCallback: function(winid, cbfun) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    win.windowIconifyFunc = cbfun;
  },

  glfwSetFramebufferSizeCallback: function(winid, cbfun) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    win.framebufferSizeFunc = cbfun;
  },

  glfwGetInputMode: function(winid, mode) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    return win.inputModes[mode];
  },

  glfwSetInputMode: function(winid, mode, value) {
    GLFW.setInputMode(winid, mode, value);
  },

  glfwGetKey: function(winid, key) {
    return GLFW.getKey(winid, key);
  },

  glfwGetMouseButton: function(winid, button) {
    return GLFW.getMouseButton(winid, button);
  },

  glfwGetCursorPos: function(winid, x, y) {
    GLFW.getCursorPos(winid, x, y);
  },

  // I believe it is not possible to move the mouse with javascript
  glfwSetCursorPos: function(winid, x, y) {
    GLFW.setCursorPos(winid, x, y);
  },

  glfwSetKeyCallback: function(winid, cbfun) {
    GLFW.setKeyCallback(winid, cbfun);
  },

  glfwSetCharCallback: function(winid, cbfun) {
    GLFW.setCharCallback(winid, cbfun);
  },

  glfwSetMouseButtonCallback: function(winid, cbfun) {
    GLFW.setMouseButtonCallback(winid, cbfun);
  },

  glfwSetCursorPosCallback: function(winid, cbfun) {
    GLFW.setCursorPosCallback(winid, cbfun);
  },

  glfwSetCursorEnterCallback: function(winid, cbfun) {
    var win = GLFW.WindowFromId(winid);
    if (!win) return;
    win.cursorEnterFunc = cbfun;
  },

  glfwSetScrollCallback: function(winid, cbfun) {
    GLFW.setScrollCallback(winid, cbfun);
  },

  glfwJoystickPresent: function(joy) { throw "glfwJoystickPresent is not implemented."; },

  glfwGetJoystickAxes: function(joy, count) { throw "glfwGetJoystickAxes is not implemented."; },

  glfwGetJoystickButtons: function(joy, count) { throw "glfwGetJoystickButtons is not implemented."; },

  glfwGetJoystickName: function(joy) { throw "glfwGetJoystickName is not implemented."; },

  glfwSetClipboardString: function(win, string) {},

  glfwGetClipboardString: function(win) {},

  glfwMakeContextCurrent: function(winid) {},

  glfwGetCurrentContext: function() {
    return GLFW.active.id;
  },

  glfwSwapBuffers: function(winid) {
    GLFW.swapBuffers(winid);
  },

#endif // GLFW 3

#if USE_GLFW == 2
  glfwOpenWindow: function(width, height, redbits, greenbits, bluebits, alphabits, depthbits, stencilbits, mode) {
    GLFW.hints[0x00021001] = redbits;     // GLFW_RED_BITS
    GLFW.hints[0x00021002] = greenbits;   // GLFW_GREEN_BITS
    GLFW.hints[0x00021003] = bluebits;    // GLFW_BLUE_BITS
    GLFW.hints[0x00021004] = alphabits;   // GLFW_ALPHA_BITS
    GLFW.hints[0x00021005] = depthbits;   // GLFW_DEPTH_BITS
    GLFW.hints[0x00021006] = stencilbits; // GLFW_STENCIL_BITS
    GLFW.createWindow(width, height, "GLFW2 Window", 0, 0);
    return 1; // GL_TRUE
  },

  glfwCloseWindow: function() {
    GLFW.destroyWindow(GLFW.active.id);
  },

  glfwOpenWindowHint: function(target, hint) {
    target = GLFW.GLFW2ParamToGLFW3Param(target);
    GLFW.hints[target] = hint;
  },

  glfwGetWindowSize: function(width, height) {
    GLFW.getWindowSize(GLFW.active.id, width, height);
  },

  glfwSetWindowSize: function(width, height) {
    GLFW.setWindowSize(GLFW.active.id, width, height);
  },

  glfwGetWindowPos: function(x, y) {
    GLFW.getWindowPos(GLFW.active.id, x, y);
  },

  glfwSetWindowPos: function(x, y) {
    GLFW.setWindowPos(GLFW.active.id, x, y);
  },

  glfwSetWindowTitle: function(title) {
    GLFW.setWindowTitle(GLFW.active.id, title);
  },

  glfwIconifyWindow: function() {
    GLFW.iconifyWindow(GLFW.active.id);
  },

  glfwRestoreWindow: function() {
    GLFW.restoreWindow(GLFW.active.id);
  },

  glfwSwapBuffers: function() {
    GLFW.swapBuffers(GLFW.active.id);
  },

  glfwGetWindowParam: function(param) {
    param = GLFW.GLFW2ParamToGLFW3Param(param);
    return GLFW.hints[param];
  },

  glfwSetWindowSizeCallback: function(cbfun) {
    GLFW.setWindowSizeCallback(GLFW.active.id, cbfun);
  },

  glfwSetWindowCloseCallback: function(cbfun) {
    GLFW.setWindowCloseCallback(GLFW.active.id, cbfun);
  },

  glfwSetWindowRefreshCallback: function(cbfun) {
    GLFW.setWindowRefreshCallback(GLFW.active.id, cbfun);
  },

  glfwGetKey: function(key) {
    return GLFW.getKey(GLFW.active.id, key);
  },

  glfwGetMouseButton: function(button) {
    return GLFW.getMouseButton(GLFW.active.id, button);
  },

  glfwGetMousePos: function(x, y) {
    GLFW.getMousePos(GLFW.active.id, x, y);
  },

  glfwSetMousePos: function(x, y) {
    GLFW.setCursorPos(GLFW.active.id, x, y);
  },

  glfwGetMouseWheel: function() {
    return 0;
  },

  glfwSetMouseWheel: function(pos) {
  },

  glfwSetKeyCallback: function(cbfun) {
    GLFW.setKeyCallback(GLFW.active.id, cbfun);
  },

  glfwSetCharCallback: function(cbfun) {
    GLFW.setCharCallback(GLFW.active.id, cbfun);
  },

  glfwSetMouseButtonCallback: function(cbfun) {
    GLFW.setMouseButtonCallback(GLFW.active.id, cbfun);
  },

  glfwSetMousePosCallback: function(cbfun) {
    GLFW.setCursorPosCallback(GLFW.active.id, cbfun);
  },

  glfwSetMouseWheelCallback: function(cbfun) {
    GLFW.setScrollCallback(GLFW.active.id, cbfun);
  },

  glfwGetDesktopMode: function(mode) {
    throw "glfwGetDesktopMode is not implemented.";
  },

  glfwSleep__deps: ['sleep'],
  glfwSleep: function(time) {
    _sleep(time);
  },

  glfwEnable: function(target) {
    target = GLFW.GLFW2ParamToGLFW3Param(target);
    GLFW.hints[target] = false;
  },

  glfwDisable: function(target) {
    target = GLFW.GLFW2ParamToGLFW3Param(target);
    GLFW.hints[target] = true;
  },

  glfwGetGLVersion: function(major, minor, rev) {
    setValue(major, 0, 'i32');
    setValue(minor, 0, 'i32');
    setValue(rev, 1, 'i32');
  },

  glfwCreateThread: function(fun, arg) {
    var str = 'v';
    for (var i in arg) {
      str += 'i';
    }
    Runtime.dynCall(str, fun, arg);
    // One single thread
    return 0;
  },

  glfwDestroyThread: function(ID) {},

  glfwWaitThread: function(ID, waitmode) {},

  glfwGetThreadID: function() {
    // One single thread
    return 0;
  },

  glfwCreateMutex: function() { throw "glfwCreateMutex is not implemented."; },

  glfwDestroyMutex: function(mutex) { throw "glfwDestroyMutex is not implemented."; },

  glfwLockMutex: function(mutex) { throw "glfwLockMutex is not implemented."; },

  glfwUnlockMutex: function(mutex) { throw "glfwUnlockMutex is not implemented."; },

  glfwCreateCond: function() { throw "glfwCreateCond is not implemented."; },

  glfwDestroyCond: function(cond) { throw "glfwDestroyCond is not implemented."; },

  glfwWaitCond: function(cond, mutex, timeout) { throw "glfwWaitCond is not implemented."; },

  glfwSignalCond: function(cond) { throw "glfwSignalCond is not implemented."; },

  glfwBroadcastCond: function(cond) { throw "glfwBroadcastCond is not implemented."; },

  glfwGetNumberOfProcessors: function() {
    // Threads are disabled anyway…
    return 1;
  },

  glfwReadImage: function(name, img, flags) { throw "glfwReadImage is not implemented."; },

  glfwReadMemoryImage: function(data, size, img, flags) { throw "glfwReadMemoryImage is not implemented."; },

  glfwFreeImage: function(img) { throw "glfwFreeImage is not implemented."; },

  glfwLoadTexture2D: function(name, flags) { throw "glfwLoadTexture2D is not implemented."; },

  glfwLoadMemoryTexture2D: function(data, size, flags) { throw "glfwLoadMemoryTexture2D is not implemented."; },

  glfwLoadTextureImage2D: function(img, flags) { throw "glfwLoadTextureImage2D is not implemented."; },
#endif // GLFW2
};

autoAddDeps(LibraryGLFW, '$GLFW');
mergeInto(LibraryManager.library, LibraryGLFW);
