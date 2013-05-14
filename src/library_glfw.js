/*******************************************************************************
 * EMSCRIPTEN GLFW 2.7.7 emulation.
 * It tries to emulate the behavior described in
 * http://www.glfw.org/GLFWReference277.pdf
 *
 * What it does:
 * - Creates a GL context.
 * - Manage keyboard and mouse events.
 * - GL Extensions support.
 *
 * What it does not but should probably do:
 * - Transmit events when glfwPollEvents, glfwWaitEvents or glfwSwapBuffers is
 *    called. Events callbacks are called as soon as event are received.
 * - Thread emulation.
 * - Joystick support.
 * - Image/Texture I/O support (that is deleted in GLFW 3).
 * - Video modes detection.
 *
 * Authors:
 * - Éloi Rivard <eloi.rivard@gmail.com>
 *
 ******************************************************************************/

var LibraryGLFW = {
  $GLFW: {

    keyFunc: null,
    charFunc: null,
    mouseButtonFunc: null,
    mousePosFunc: null,
    mouseWheelFunc: null,
    resizeFunc: null,
    closeFunc: null,
    refreshFunc: null,
    params: null,
    initTime: null,
    wheelPos: 0,
    buttons: 0,
    keys: 0,
    initWindowWidth: 640,
    initWindowHeight: 480,
    windowX: 0,
    windowY: 0,
    windowWidth: 0,
    windowHeight: 0,

/*******************************************************************************
 * DOM EVENT CALLBACKS
 ******************************************************************************/

    DOMToGLFWKeyCode: function(keycode) {
      switch (keycode) {
        case 0x09: return 295 ; //DOM_VK_TAB -> GLFW_KEY_TAB
        case 0x1B: return 255 ; //DOM_VK_ESCAPE -> GLFW_KEY_ESC
        case 0x6A: return 313 ; //DOM_VK_MULTIPLY -> GLFW_KEY_KP_MULTIPLY
        case 0x6B: return 315 ; //DOM_VK_ADD -> GLFW_KEY_KP_ADD
        case 0x6D: return 314 ; //DOM_VK_SUBTRACT -> GLFW_KEY_KP_SUBTRACT
        case 0x6E: return 316 ; //DOM_VK_DECIMAL -> GLFW_KEY_KP_DECIMAL
        case 0x6F: return 312 ; //DOM_VK_DIVIDE -> GLFW_KEY_KP_DIVIDE
        case 0x70: return 258 ; //DOM_VK_F1 -> GLFW_KEY_F1
        case 0x71: return 259 ; //DOM_VK_F2 -> GLFW_KEY_F2
        case 0x72: return 260 ; //DOM_VK_F3 -> GLFW_KEY_F3
        case 0x73: return 261 ; //DOM_VK_F4 -> GLFW_KEY_F4
        case 0x74: return 262 ; //DOM_VK_F5 -> GLFW_KEY_F5
        case 0x75: return 263 ; //DOM_VK_F6 -> GLFW_KEY_F6
        case 0x76: return 264 ; //DOM_VK_F7 -> GLFW_KEY_F7
        case 0x77: return 265 ; //DOM_VK_F8 -> GLFW_KEY_F8
        case 0x78: return 266 ; //DOM_VK_F9 -> GLFW_KEY_F9
        case 0x79: return 267 ; //DOM_VK_F10 -> GLFW_KEY_F10
        case 0x7a: return 268 ; //DOM_VK_F11 -> GLFW_KEY_F11
        case 0x7b: return 269 ; //DOM_VK_F12 -> GLFW_KEY_F12
        case 0x25: return 285 ; //DOM_VK_LEFT -> GLFW_KEY_LEFT
        case 0x26: return 283 ; //DOM_VK_UP -> GLFW_KEY_UP
        case 0x27: return 286 ; //DOM_VK_RIGHT -> GLFW_KEY_RIGHT
        case 0x28: return 284 ; //DOM_VK_DOWN -> GLFW_KEY_DOWN
        case 0x21: return 298 ; //DOM_VK_PAGE_UP -> GLFW_KEY_PAGEUP
        case 0x22: return 299 ; //DOM_VK_PAGE_DOWN -> GLFW_KEY_PAGEDOWN
        case 0x24: return 300 ; //DOM_VK_HOME -> GLFW_KEY_HOME
        case 0x23: return 301 ; //DOM_VK_END -> GLFW_KEY_END
        case 0x2d: return 296 ; //DOM_VK_INSERT -> GLFW_KEY_INSERT
        case 16  : return 287 ; //DOM_VK_SHIFT -> GLFW_KEY_LSHIFT
        case 0x05: return 287 ; //DOM_VK_LEFT_SHIFT -> GLFW_KEY_LSHIFT
        case 0x06: return 288 ; //DOM_VK_RIGHT_SHIFT -> GLFW_KEY_RSHIFT
        case 17  : return 289 ; //DOM_VK_CONTROL -> GLFW_KEY_LCTRL
        case 0x03: return 289 ; //DOM_VK_LEFT_CONTROL -> GLFW_KEY_LCTRL
        case 0x04: return 290 ; //DOM_VK_RIGHT_CONTROL -> GLFW_KEY_RCTRL
        case 18  : return 291 ; //DOM_VK_ALT -> GLFW_KEY_LALT
        case 0x02: return 291 ; //DOM_VK_LEFT_ALT -> GLFW_KEY_LALT
        case 0x01: return 292 ; //DOM_VK_RIGHT_ALT -> GLFW_KEY_RALT
        case 96  : return 302 ; //GLFW_KEY_KP_0
        case 97  : return 303 ; //GLFW_KEY_KP_1
        case 98  : return 304 ; //GLFW_KEY_KP_2
        case 99  : return 305 ; //GLFW_KEY_KP_3
        case 100 : return 306 ; //GLFW_KEY_KP_4
        case 101 : return 307 ; //GLFW_KEY_KP_5
        case 102 : return 308 ; //GLFW_KEY_KP_6
        case 103 : return 309 ; //GLFW_KEY_KP_7
        case 104 : return 310 ; //GLFW_KEY_KP_8
        case 105 : return 311 ; //GLFW_KEY_KP_9
        default  : return keycode;
      };
    },

    //UCS-2 to UTF16 (ISO 10646)
    getUnicodeChar: function(value) {
      var output = '';
      if (value > 0xFFFF) {
        value -= 0x10000;
        output += String.fromCharCode(value >>> 10 & 0x3FF | 0xD800);
        value = 0xDC00 | value & 0x3FF;
      }
      output += String.fromCharCode(value);
      return output;
    },

    onKeyPress: function(event) {
      //charCode is only available whith onKeyPress event
      var char = GLFW.getUnicodeChar(event.charCode);

      if (event.charCode) {
        var char = GLFW.getUnicodeChar(event.charCode);
        if (char !== null && GLFW.charFunc) {
          event.preventDefault();
          Runtime.dynCall('vii', GLFW.charFunc, [event.charCode, 1]);
        }
      }
    },

    onKeyChanged: function(event, status) {
      var key = GLFW.DOMToGLFWKeyCode(event.keyCode);
      if (key && GLFW.keyFunc) {
        GLFW.keys[key] = status;
        event.preventDefault();
        Runtime.dynCall('vii', GLFW.keyFunc, [key, status]);
      }
    },

    onKeydown: function(event) {
      GLFW.onKeyChanged(event, 1);//GLFW_PRESS
    },

    onKeyup: function(event) {
      GLFW.onKeyChanged(event, 0);//GLFW_RELEASE
    },

    onMousemove: function(event) {
      /* Send motion event only if the motion changed, prevents
       * spamming our app with uncessary callback call. It does happen in
       * Chrome on Windows.
       */
      var lastX = Browser.mouseX;
      var lastY = Browser.mouseY;
      Browser.calculateMouseEvent(event);
      var newX = Browser.mouseX;
      var newY = Browser.mouseY;

      if (event.target == Module["canvas"] && GLFW.mousePosFunc) {
        event.preventDefault();
        Runtime.dynCall('vii', GLFW.mousePosFunc, [lastX, lastY]);
      }
    },

    onMouseButtonChanged: function(event, status) {
      if (GLFW.mouseButtonFunc == null) {
        return;
      }

      Browser.calculateMouseEvent(event);

      if (event.target != Module["canvas"]) {
        return;
      }

      if (status == 1) {//GLFW_PRESS
        try {
          event.target.setCapture();
        } catch (e) {}
      }

      event.preventDefault();
      //DOM and glfw have the same button codes
      Runtime.dynCall('vii', GLFW.mouseButtonFunc, [event['button'], status]);
    },

    onMouseButtonDown: function(event) {
      GLFW.buttons |= (1 << event['button']);
      GLFW.onMouseButtonChanged(event, 1);//GLFW_PRESS
    },

    onMouseButtonUp: function(event) {
      GLFW.buttons &= ~(1 << event['button']);
      GLFW.onMouseButtonChanged(event, 0);//GLFW_RELEASE
    },

    onMouseWheel: function(event) {
      if (event.detail > 0) {
        GLFW.wheelPos++;
      }

      if (event.detail < 0) {
        GLFW.wheelPos--;
      }

      if (GLFW.mouseWheelFunc && event.target == Module["canvas"]) {
        Runtime.dynCall('vi', GLFW.mouseWheelFunc, [GLFW.wheelPos]);
        event.preventDefault();
      }
    },

    // TODO add fullscreen API ala:
    // http://johndyer.name/native-fullscreen-javascript-api-plus-jquery-plugin/
    onFullScreenEventChange: function(event) {
      var width;
      var height;
      if (document["fullScreen"] || document["mozFullScreen"] || document["webkitIsFullScreen"]) {
        width = screen["width"];
        height = screen["height"];
      }
      else {
        width = GLFW.windowWidth;
        height = GLFW.windowHeight;
        // TODO set position
        document.removeEventListener('fullscreenchange', GLFW.onFullScreenEventChange, true);
        document.removeEventListener('mozfullscreenchange', GLFW.onFullScreenEventChange, true);
        document.removeEventListener('webkitfullscreenchange', GLFW.onFullScreenEventChange, true);
      }
      Browser.setCanvasSize(width, height);

      if (GLFW.resizeFunc) {
        Runtime.dynCall('vii', GLFW.resizeFunc, [width, height]);
      }
    },

    requestFullScreen: function() {
      var RFS = Module["canvas"]['requestFullscreen'] ||
                Module["canvas"]['requestFullScreen'] ||
                Module["canvas"]['mozRequestFullScreen'] ||
                Module["canvas"]['webkitRequestFullScreen'] ||
                (function() {});
      RFS.apply(Module["canvas"], []);
    },

    cancelFullScreen: function() {
      var CFS = document['exitFullscreen'] ||
                document['cancelFullScreen'] ||
                document['mozCancelFullScreen'] ||
                document['webkitCancelFullScreen'] ||
          (function() {});
      CFS.apply(document, []);
    }
  },

/*******************************************************************************
 * GLFW FUNCTIONS
 ******************************************************************************/

  /* GLFW initialization, termination and version querying */
  glfwInit: function() {
    GLFW.initTime = Date.now() / 1000;

    window.addEventListener("keydown", GLFW.onKeydown, true);
    window.addEventListener("keypress", GLFW.onKeyPress, true);
    window.addEventListener("keyup", GLFW.onKeyup, true);
    window.addEventListener("mousemove", GLFW.onMousemove, true);
    window.addEventListener("mousedown", GLFW.onMouseButtonDown, true);
    window.addEventListener("mouseup", GLFW.onMouseButtonUp, true);
    window.addEventListener('DOMMouseScroll', GLFW.onMouseWheel, true);
    window.addEventListener('mousewheel', GLFW.onMouseWheel, true);

    __ATEXIT__.push({ func: function() {
      window.removeEventListener("keydown", GLFW.onKeydown, true);
      window.removeEventListener("keypress", GLFW.onKeyPress, true);
      window.removeEventListener("keyup", GLFW.onKeyup, true);
      window.removeEventListener("mousemove", GLFW.onMousemove, true);
      window.removeEventListener("mousedown", GLFW.onMouseButtonDown, true);
      window.removeEventListener("mouseup", GLFW.onMouseButtonUp, true);
      window.removeEventListener('DOMMouseScroll', GLFW.onMouseWheel, true);
      window.removeEventListener('mousewheel', GLFW.onMouseWheel, true);
      Module["canvas"].width = Module["canvas"].height = 1;
    }});

    //TODO: Init with correct values
    GLFW.params = new Array();
    GLFW.params[0x00030001] = true; //GLFW_MOUSE_CURSOR
    GLFW.params[0x00030002] = false; //GLFW_STICKY_KEYS
    GLFW.params[0x00030003] = true; //GLFW_STICKY_MOUSE_BUTTONS
    GLFW.params[0x00030004] = false; //GLFW_SYSTEM_KEYS
    GLFW.params[0x00030005] = false; //GLFW_KEY_REPEAT
    GLFW.params[0x00030006] = true; //GLFW_AUTO_POLL_EVENTS
    GLFW.params[0x00020001] = true; //GLFW_OPENED
    GLFW.params[0x00020002] = true; //GLFW_ACTIVE
    GLFW.params[0x00020003] = false; //GLFW_ICONIFIED
    GLFW.params[0x00020004] = true; //GLFW_ACCELERATED
    GLFW.params[0x00020005] = 0; //GLFW_RED_BITS
    GLFW.params[0x00020006] = 0; //GLFW_GREEN_BITS
    GLFW.params[0x00020007] = 0; //GLFW_BLUE_BITS
    GLFW.params[0x00020008] = 0; //GLFW_ALPHA_BITS
    GLFW.params[0x00020009] = 0; //GLFW_DEPTH_BITS
    GLFW.params[0x0002000A] = 0; //GLFW_STENCIL_BITS
    GLFW.params[0x0002000B] = 0; //GLFW_REFRESH_RATE
    GLFW.params[0x0002000C] = 0; //GLFW_ACCUM_RED_BITS
    GLFW.params[0x0002000D] = 0; //GLFW_ACCUM_GREEN_BITS
    GLFW.params[0x0002000E] = 0; //GLFW_ACCUM_BLUE_BITS
    GLFW.params[0x0002000F] = 0; //GLFW_ACCUM_ALPHA_BITS
    GLFW.params[0x00020010] = 0; //GLFW_AUX_BUFFERS
    GLFW.params[0x00020011] = 0; //GLFW_STEREO
    GLFW.params[0x00020012] = 0; //GLFW_WINDOW_NO_RESIZE
    GLFW.params[0x00020013] = 0; //GLFW_FSAA_SAMPLES
    GLFW.params[0x00020014] = 0; //GLFW_OPENGL_VERSION_MAJOR
    GLFW.params[0x00020015] = 0; //GLFW_OPENGL_VERSION_MINOR
    GLFW.params[0x00020016] = 0; //GLFW_OPENGL_FORWARD_COMPAT
    GLFW.params[0x00020017] = 0; //GLFW_OPENGL_DEBUG_CONTEXT
    GLFW.params[0x00020018] = 0; //GLFW_OPENGL_PROFILE

    GLFW.keys = new Array();

    return 1; //GL_TRUE
  },

  glfwTerminate: function() {},

  glfwGetVersion: function(major, minor, rev) {
    setValue(major, 2, 'i32');
    setValue(minor, 7, 'i32');
    setValue(rev, 7, 'i32');
  },

  /* Window handling */
  glfwOpenWindow__deps: ['$Browser'],
  glfwOpenWindow: function(width, height, redbits, greenbits, bluebits, alphabits, depthbits, stencilbits, mode) {
    if (width == 0 && height > 0) {
      width = 4 * height / 3;
    }
    if (width > 0 && height == 0) {
      height = 3 * width / 4;
    }
    GLFW.params[0x00020005] = redbits; //GLFW_RED_BITS
    GLFW.params[0x00020006] = greenbits; //GLFW_GREEN_BITS
    GLFW.params[0x00020007] = bluebits; //GLFW_BLUE_BITS
    GLFW.params[0x00020008] = alphabits; //GLFW_ALPHA_BITS
    GLFW.params[0x00020009] = depthbits; //GLFW_DEPTH_BITS
    GLFW.params[0x0002000A] = stencilbits; //GLFW_STENCIL_BITS

    if (mode == 0x00010001) {//GLFW_WINDOW
      Browser.setCanvasSize(GLFW.initWindowWidth = width,
                            GLFW.initWindowHeight = height);
      GLFW.params[0x00030003] = true; //GLFW_STICKY_MOUSE_BUTTONS
    }
    else if (mode == 0x00010002) {//GLFW_FULLSCREEN
      GLFW.requestFullScreen();
      GLFW.params[0x00030003] = false; //GLFW_STICKY_MOUSE_BUTTONS
    }
    else{
      throw "Invalid glfwOpenWindow mode.";
    }

    Module.ctx = Browser.createContext(Module['canvas'], true, true);
    return 1; //GL_TRUE
  },

  glfwOpenWindowHint: function(target, hint) {
    GLFW.params[target] = hint;
  },

  glfwCloseWindow__deps: ['$Browser'],
  glfwCloseWindow: function() {
    if (GLFW.closeFunc) {
      Runtime.dynCall('v', GLFW.closeFunc, []);
    }
    Module.ctx = Browser.destroyContext(Module['canvas'], true, true);
  },

  glfwSetWindowTitle: function(title) {
    document.title = Pointer_stringify(title);
  },

  glfwGetWindowSize: function(width, height) {
    setValue(width, Module['canvas'].width, 'i32');
    setValue(height, Module['canvas'].height, 'i32');
  },

  glfwSetWindowSize: function(width, height) {
    GLFW.cancelFullScreen();
      Browser.setCanvasSize(width, height);
      if (GLFW.resizeFunc) {
        Runtime.dynCall('vii', GLFW.resizeFunc, [width, height]);
      }
  },

  glfwSetWindowPos: function(x, y) {},

  glfwIconifyWindow: function() {},

  glfwRestoreWindow: function() {},

  glfwSwapBuffers: function() {},

  glfwSwapInterval: function(interval) {},

  glfwGetWindowParam: function(param) {
    return GLFW.params[param];
  },

  glfwSetWindowSizeCallback: function(cbfun) {
    GLFW.resizeFunc = cbfun;
  },

  glfwSetWindowCloseCallback: function(cbfun) {
    GLFW.closeFunc = cbfun;
  },

  glfwSetWindowRefreshCallback: function(cbfun) {
    GLFW.refreshFunc = cbfun;
  },

  /* Video mode functions */
  glfwGetVideoModes: function(list, maxcount) { throw "glfwGetVideoModes is not implemented."; },

  glfwGetDesktopMode: function(mode) { throw "glfwGetDesktopMode is not implemented."; },

  /* Input handling */
  glfwPollEvents: function() {},

  glfwWaitEvents: function() {},

  glfwGetKey: function(key) {
    return GLFW.keys[key];
  },

  glfwGetMouseButton: function(button) {
    return (GLFW.buttons & (1 << button)) > 0;
  },

  glfwGetMousePos: function(xpos, ypos) {
    setValue(xpos, Browser.mouseX, 'i32');
    setValue(ypos, Browser.mouseY, 'i32');
  },

  //I believe it is not possible to move the mouse with javascript
  glfwSetMousePos: function(xpos, ypos) {},

  glfwGetMouseWheel: function() {
    return GLFW.wheelPos;
  },

  glfwSetMouseWheel: function(pos) {
    GLFW.wheelPos = pos;
  },

  glfwSetKeyCallback: function(cbfun) {
    GLFW.keyFunc = cbfun;
  },

  glfwSetCharCallback: function(cbfun) {
    GLFW.charFunc = cbfun;
  },

  glfwSetMouseButtonCallback: function(cbfun) {
    GLFW.mouseButtonFunc = cbfun;
  },

  glfwSetMousePosCallback: function(cbfun) {
    GLFW.mousePosFunc = cbfun;
  },

  glfwSetMouseWheelCallback: function(cbfun) {
    GLFW.mouseWheelFunc = cbfun;
  },

  /* Joystick input */
  glfwGetJoystickParam: function(joy, param) { throw "glfwGetJoystickParam is not implemented."; },

  glfwGetJoystickPos: function(joy, pos, numaxes) { throw "glfwGetJoystickPos is not implemented."; },

  glfwGetJoystickButtons: function(joy, buttons, numbuttons) { throw "glfwGetJoystickButtons is not implemented."; },

  /* Time */
  glfwGetTime: function() {
    return (Date.now()/1000) - GLFW.initTime;
  },

  glfwSetTime: function(time) {
    GLFW.initTime = Date.now()/1000 + time;
  },

  glfwSleep__deps: ['sleep'],
  glfwSleep: function(time) {
    _sleep(time);
  },

  /* Extension support */
  glfwExtensionSupported: function(extension) {
    return Module.ctx.getSupportedExtensions().indexOf(Pointer_stringify(extension)) > -1;
  },

  glfwGetProcAddress__deps: ['glfwGetProcAddress'],
  glfwGetProcAddress: function(procname) {
    return _getProcAddress(procname);
  },

  glfwGetGLVersion: function(major, minor, rev) {
    setValue(major, 0, 'i32');
    setValue(minor, 0, 'i32');
    setValue(rev, 1, 'i32');
  },

  /* Threading support */
  glfwCreateThread: function(fun, arg) {
    var str = 'v';
    for (var i in arg) {
      str += 'i';
    }
    Runtime.dynCall(str, fun, arg);
    //One single thread
    return 0;
  },

  glfwDestroyThread: function(ID) {},

  glfwWaitThread: function(ID, waitmode) {},

  glfwGetThreadID: function() {
    //One single thread
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
    //Threads are disabled anyway…
    return 1;
  },

  /* Enable/disable functions */
  glfwEnable: function(token) {
    GLFW.params[token] = false;
  },

  glfwDisable: function(token) {
    GLFW.params[token] = true;
  },

  /* Image/texture I/O support */
  glfwReadImage: function(name, img, flags) { throw "glfwReadImage is not implemented."; },

  glfwReadMemoryImage: function(data, size, img, flags) { throw "glfwReadMemoryImage is not implemented."; },

  glfwFreeImage: function(img) { throw "glfwFreeImage is not implemented."; },

  glfwLoadTexture2D: function(name, flags) { throw "glfwLoadTexture2D is not implemented."; },

  glfwLoadMemoryTexture2D: function(data, size, flags) { throw "glfwLoadMemoryTexture2D is not implemented."; },

  glfwLoadTextureImage2D: function(img, flags) { throw "glfwLoadTextureImage2D is not implemented."; },
};

autoAddDeps(LibraryGLFW, '$GLFW');
mergeInto(LibraryManager.library, LibraryGLFW);


