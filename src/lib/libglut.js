/**
 * @license
 * Copyright 2012 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryGLUT = {
  $GLUT: {
    initTime: null,
    idleFunc: null,
    displayFunc: null,
    keyboardFunc: null,
    keyboardUpFunc: null,
    specialFunc: null,
    specialUpFunc: null,
    reshapeFunc: null,
    motionFunc: null,
    passiveMotionFunc: null,
    mouseFunc: null,
    buttons: 0,
    modifiers: 0,
    initWindowWidth: 256,
    initWindowHeight: 256,
    initDisplayMode: cDefs.GLUT_RGBA | cDefs.GLUT_DOUBLE | cDefs.GLUT_DEPTH,
    // Set when going fullscreen
    windowWidth: 0,
    windowHeight: 0,
    requestedAnimationFrame: false,

    saveModifiers: (event) => {
      GLUT.modifiers = 0;
      if (event['shiftKey'])
        GLUT.modifiers |= GLUT_ACTIVE_SHIFT;
      if (event['ctrlKey'])
        GLUT.modifiers |= GLUT_ACTIVE_CTRL;
      if (event['altKey'])
        GLUT.modifiers |= GLUT_ACTIVE_ALT;
    },

    onMousemove: (event) => {
      /* Send motion event only if the motion changed, prevents
       * spamming our app with unnecessary callbacks. It does happen in
       * Chrome on Windows.
       */
      var lastX = Browser.mouseX;
      var lastY = Browser.mouseY;
      Browser.calculateMouseEvent(event);
      var newX = Browser.mouseX;
      var newY = Browser.mouseY;
      if (newX == lastX && newY == lastY) return;

      if (GLUT.buttons == 0 && event.target == Browser.getCanvas() && GLUT.passiveMotionFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        {{{ makeDynCall('vii', 'GLUT.passiveMotionFunc') }}}(lastX, lastY);
      } else if (GLUT.buttons != 0 && GLUT.motionFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        {{{ makeDynCall('vii', 'GLUT.motionFunc') }}}(lastX, lastY);
      }
    },

    specialKeys: {
      8: 120, // backspace
      46: 111, // delete

      0x70 /*DOM_VK_F1*/: cDefs.GLUT_KEY_F1,
      0x71 /*DOM_VK_F2*/: cDefs.GLUT_KEY_F2,
      0x72 /*DOM_VK_F3*/: cDefs.GLUT_KEY_F3,
      0x73 /*DOM_VK_F4*/: cDefs.GLUT_KEY_F4,
      0x74 /*DOM_VK_F5*/: cDefs.GLUT_KEY_F5,
      0x75 /*DOM_VK_F6*/: cDefs.GLUT_KEY_F6,
      0x76 /*DOM_VK_F7*/: cDefs.GLUT_KEY_F7,
      0x77 /*DOM_VK_F8*/: cDefs.GLUT_KEY_F8,
      0x78 /*DOM_VK_F9*/: cDefs.GLUT_KEY_F9,
      0x79 /*DOM_VK_F10*/: cDefs.GLUT_KEY_F10,
      0x7a /*DOM_VK_F11*/: cDefs.GLUT_KEY_F11,
      0x7b /*DOM_VK_F12*/: cDefs.GLUT_KEY_F12,
      0x25 /*DOM_VK_LEFT*/: cDefs.GLUT_KEY_LEFT,
      0x26 /*DOM_VK_UP*/: cDefs.GLUT_KEY_UP,
      0x27 /*DOM_VK_RIGHT*/: cDefs.GLUT_KEY_RIGHT,
      0x28 /*DOM_VK_DOWN*/: cDefs.GLUT_KEY_DOWN,
      0x21 /*DOM_VK_PAGE_UP*/: cDefs.GLUT_KEY_PAGE_UP,
      0x22 /*DOM_VK_PAGE_DOWN*/: cDefs.GLUT_KEY_PAGE_DOWN,
      0x24 /*DOM_VK_HOME*/: cDefs.GLUT_KEY_HOME,
      0x23 /*DOM_VK_END*/: cDefs.GLUT_KEY_END,
      0x2d /*DOM_VK_INSERT*/: cDefs.GLUT_KEY_INSERT,

      16   /*DOM_VK_SHIFT*/: 112, // GLUT_KEY_SHIFT_L
      0x05 /*DOM_VK_LEFT_SHIFT*/: 112, // GLUT_KEY_SHIFT_L
      0x06 /*DOM_VK_RIGHT_SHIFT*/: 113, // GLUT_KEY_SHIFT_R

      17   /*DOM_VK_CONTROL*/: 114, // GLUT_KEY_CONTROL_L
      0x03 /*DOM_VK_LEFT_CONTROL*/: 114, // GLUT_KEY_CONTROL_L
      0x04 /*DOM_VK_RIGHT_CONTROL*/: 115, // GLUT_KEY_CONTROL_R

      18   /*DOM_VK_ALT*/: 116, // GLUT_KEY_ALT_L
      0x02 /*DOM_VK_LEFT_ALT*/: 116, // GLUT_KEY_ALT_L
      0x01 /*DOM_VK_RIGHT_ALT*/: 117, // GLUT_KEY_ALT_R
    },

    getSpecialKey: (keycode) => GLUT.specialKeys[keycode] ?? null,

    getASCIIKey: (event) => {
      if (event['ctrlKey'] || event['altKey'] || event['metaKey']) return null;

      var keycode = event['keyCode'];

      /* The exact list is soooo hard to find in a canonical place! */

      if (48 <= keycode && keycode <= 57)
        return keycode; // numeric  TODO handle shift?
      if (65 <= keycode && keycode <= 90)
        return event['shiftKey'] ? keycode : keycode + 32;
      if (96 <= keycode && keycode <= 105)
        return keycode - 48; // numpad numbers
      if (106 <= keycode && keycode <= 111)
        return keycode - 106 + 42; // *,+-./  TODO handle shift?

      switch (keycode) {
        case 9:  // tab key
        case 13: // return key
        case 27: // escape
        case 32: // space
        case 61: // equal
          return keycode;
      }

      var s = event['shiftKey'];
      switch (keycode) {
        case 186: return s ? 58 : 59; // colon / semi-colon
        case 187: return s ? 43 : 61; // add / equal (these two may be wrong)
        case 188: return s ? 60 : 44; // less-than / comma
        case 189: return s ? 95 : 45; // dash
        case 190: return s ? 62 : 46; // greater-than / period
        case 191: return s ? 63 : 47; // forward slash
        case 219: return s ? 123 : 91; // open bracket
        case 220: return s ? 124 : 47; // back slash
        case 221: return s ? 125 : 93; // close bracket
        case 222: return s ? 34 : 39; // single quote
      }

      return null;
    },

    onKeydown: (event) => {
      if (GLUT.specialFunc || GLUT.keyboardFunc) {
        var key = GLUT.getSpecialKey(event['keyCode']);
        if (key !== null) {
          if (GLUT.specialFunc) {
            event.preventDefault();
            GLUT.saveModifiers(event);
            {{{ makeDynCall('viii', 'GLUT.specialFunc') }}}(key, Browser.mouseX, Browser.mouseY);
          }
        } else {
          key = GLUT.getASCIIKey(event);
          if (key !== null && GLUT.keyboardFunc) {
            event.preventDefault();
            GLUT.saveModifiers(event);
            {{{ makeDynCall('viii', 'GLUT.keyboardFunc') }}}(key, Browser.mouseX, Browser.mouseY);
          }
        }
      }
    },

    onKeyup: (event) => {
      if (GLUT.specialUpFunc || GLUT.keyboardUpFunc) {
        var key = GLUT.getSpecialKey(event['keyCode']);
        if (key !== null) {
          if (GLUT.specialUpFunc) {
            event.preventDefault();
            GLUT.saveModifiers(event);
            {{{ makeDynCall('viii', 'GLUT.specialUpFunc') }}}(key, Browser.mouseX, Browser.mouseY);
          }
        } else {
          key = GLUT.getASCIIKey(event);
          if (key !== null && GLUT.keyboardUpFunc) {
            event.preventDefault();
            GLUT.saveModifiers(event);
            {{{ makeDynCall('viii', 'GLUT.keyboardUpFunc') }}}(key, Browser.mouseX, Browser.mouseY);
          }
        }
      }
    },

    touchHandler: (event) => {
      if (event.target != Browser.getCanvas()) {
        return;
      }

      var touches = event.changedTouches,
          main = touches[0],
          type = '';

      switch (event.type) {
        case 'touchstart': type = 'mousedown'; break;
        case 'touchmove': type = 'mousemove'; break;
        case 'touchend': type = 'mouseup'; break;
        default: return;
      }

      var simulatedEvent = document.createEvent('MouseEvent');
      simulatedEvent.initMouseEvent(type, true, true, window, 1,
                                    main.screenX, main.screenY,
                                    main.clientX, main.clientY, false,
                                    false, false, false, 0/*main*/, null);

      main.target.dispatchEvent(simulatedEvent);
      event.preventDefault();
    },

    onMouseButtonDown: (event) => {
      Browser.calculateMouseEvent(event);

      GLUT.buttons |= (1 << event['button']);

      if (event.target == Browser.getCanvas() && GLUT.mouseFunc) {
        try {
          event.target.setCapture();
        } catch (e) {}
        event.preventDefault();
        GLUT.saveModifiers(event);
        {{{ makeDynCall('viiii', 'GLUT.mouseFunc') }}}(event['button'], GLUT_DOWN, Browser.mouseX, Browser.mouseY);
      }
    },

    onMouseButtonUp: (event) => {
      Browser.calculateMouseEvent(event);

      GLUT.buttons &= ~(1 << event['button']);

      if (GLUT.mouseFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        {{{ makeDynCall('viiii', 'GLUT.mouseFunc') }}}(event['button'], GLUT_UP, Browser.mouseX, Browser.mouseY);
      }
    },

    onMouseWheel: (event) => {
      Browser.calculateMouseEvent(event);

      // cross-browser wheel delta
      // Note the minus sign that flips browser wheel direction (positive direction scrolls page down) to native wheel direction (positive direction is mouse wheel up)
      var delta = -Browser.getMouseWheelDelta(event);
      delta = (delta == 0) ? 0 : (delta > 0 ? Math.max(delta, 1) : Math.min(delta, -1)); // Quantize to integer so that minimum scroll is at least +/- 1.

      var button = 3; // wheel up
      if (delta < 0) {
        button = 4; // wheel down
      }

      if (GLUT.mouseFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        {{{ makeDynCall('viiii', 'GLUT.mouseFunc') }}}(button, GLUT_DOWN, Browser.mouseX, Browser.mouseY);
      }
    },

    // TODO add fullscreen API ala:
    // http://johndyer.name/native-fullscreen-javascript-api-plus-jquery-plugin/
    onFullscreenEventChange: (event) => {
      var width;
      var height;
      if (getFullscreenElement()) {
        width = screen['width'];
        height = screen['height'];
      } else {
        width = GLUT.windowWidth;
        height = GLUT.windowHeight;
        // TODO set position
        document.removeEventListener('fullscreenchange', GLUT.onFullscreenEventChange, true);
#if MIN_SAFARI_VERSION < 160400
        document.removeEventListener('webkitfullscreenchange', GLUT.onFullscreenEventChange, true);
#endif
      }
      Browser.setCanvasSize(width, height, true); // N.B. GLUT.reshapeFunc is also registered as a canvas resize callback.
                                                  // Just call it once here.
      /* Can't call _glutReshapeWindow as that requests cancelling fullscreen. */
      if (GLUT.reshapeFunc) {
        // out('GLUT.reshapeFunc (from FS): ' + width + ', ' + height);
        {{{ makeDynCall('vii', 'GLUT.reshapeFunc') }}}(width, height);
      }
      _glutPostRedisplay();
    },

    // Resize callback stage 1: update canvas by setCanvasSize, which notifies resizeListeners including GLUT.reshapeFunc
    onResize: () => {
      // Update canvas size to clientWidth and clientHeight, which include CSS scaling
      var canvas = Browser.getCanvas();
      Browser.setCanvasSize(canvas.clientWidth, canvas.clientHeight, /*noUpdates*/false);
    }
  },

  glutGetModifiers__proxy: 'sync',
  glutGetModifiers: () => GLUT.modifiers,

  glutInit__proxy: 'sync',
  glutInit: (argcp, argv) => {
    // Ignore arguments
    GLUT.initTime = Date.now();

    var isTouchDevice = 'ontouchstart' in document.documentElement;
    if (isTouchDevice) {
      // onMouseButtonDown, onMouseButtonUp and onMousemove handlers
      // depend on Browser.mouseX / Browser.mouseY fields. Those fields
      // don't get updated by touch events. So register a touchHandler
      // function that translates the touch events to mouse events.

      // GLUT doesn't support touch, mouse only, so from touch events we
      // are only looking at single finger touches to emulate left click,
      // so we can use workaround and convert all touch events in mouse
      // events. See touchHandler.
      window.addEventListener('touchmove', GLUT.touchHandler, true);
      window.addEventListener('touchstart', GLUT.touchHandler, true);
      window.addEventListener('touchend', GLUT.touchHandler, true);
    }

    window.addEventListener('keydown', GLUT.onKeydown, true);
    window.addEventListener('keyup', GLUT.onKeyup, true);
    window.addEventListener('mousemove', GLUT.onMousemove, true);
    window.addEventListener('mousedown', GLUT.onMouseButtonDown, true);
    window.addEventListener('mouseup', GLUT.onMouseButtonUp, true);
    // IE9, Chrome, Safari, Opera
    window.addEventListener('mousewheel', GLUT.onMouseWheel, true);
    // Firefox
    window.addEventListener('DOMMouseScroll', GLUT.onMouseWheel, true);

    // Resize callback stage 1: update canvas which notifies resizeListeners
    window.addEventListener('resize', GLUT.onResize, true);

    // Resize callback stage 2: updateResizeListeners notifies reshapeFunc
    Browser.resizeListeners.push((width, height) => {
      if (GLUT.reshapeFunc) {
        {{{ makeDynCall('vii', 'GLUT.reshapeFunc') }}}(width, height);
      }
    });

    addOnExit(() => {
      if (isTouchDevice) {
        window.removeEventListener('touchmove', GLUT.touchHandler, true);
        window.removeEventListener('touchstart', GLUT.touchHandler, true);
        window.removeEventListener('touchend', GLUT.touchHandler, true);
      }

      window.removeEventListener('keydown', GLUT.onKeydown, true);
      window.removeEventListener('keyup', GLUT.onKeyup, true);
      window.removeEventListener('mousemove', GLUT.onMousemove, true);
      window.removeEventListener('mousedown', GLUT.onMouseButtonDown, true);
      window.removeEventListener('mouseup', GLUT.onMouseButtonUp, true);
      // IE9, Chrome, Safari, Opera
      window.removeEventListener('mousewheel', GLUT.onMouseWheel, true);
      // Firefox
      window.removeEventListener('DOMMouseScroll', GLUT.onMouseWheel, true);

      window.removeEventListener('resize', GLUT.onResize, true);
    });
  },

  glutInitWindowSize__proxy: 'sync',
  glutInitWindowSize: (width, height) => {
    Browser.setCanvasSize( GLUT.initWindowWidth = width,
                           GLUT.initWindowHeight = height );
  },

  glutInitWindowPosition__proxy: 'sync',
  // Ignore for now
  glutInitWindowPosition: (x, y) => {},

  glutGet: (type) => {
    switch (type) {
      case GLUT_WINDOW_X:
        return 0; /* TODO */
      case GLUT_WINDOW_Y:
        return 0; /* TODO */
      case GLUT_WINDOW_WIDTH:
        return Browser.getCanvas().width;
      case GLUT_WINDOW_HEIGHT:
        return Browser.getCanvas().height;
      case GLUT_SCREEN_WIDTH:
        return Browser.getCanvas().width;
      case GLUT_SCREEN_HEIGHT:
        return Browser.getCanvas().height;
      case GLUT_INIT_WINDOW_X:
        return 0; /* TODO */
      case GLUT_INIT_WINDOW_Y:
        return 0; /* TODO */
      case GLUT_INIT_WINDOW_WIDTH:
        return GLUT.initWindowWidth;
      case GLUT_INIT_WINDOW_HEIGHT:
        return GLUT.initWindowHeight;
      case GLUT_ELAPSED_TIME:
        var now = Date.now();
        return now - GLUT.initTime;
      case GLUT_WINDOW_STENCIL_SIZE:
        return GLctx.getContextAttributes().stencil ? 8 : 0;
      case GLUT_WINDOW_DEPTH_SIZE:
        return GLctx.getContextAttributes().depth ? 8 : 0;
      case GLUT_WINDOW_ALPHA_SIZE:
        return GLctx.getContextAttributes().alpha ? 8 : 0;
      case GLUT_WINDOW_NUM_SAMPLES:
        return GLctx.getContextAttributes().antialias ? 1 : 0;

      default:
        abort(`glutGet(${type}) not implemented yet`);
    }
  },

  glutIdleFunc__proxy: 'sync',
  glutIdleFunc: (func) => {
    function callback() {
      if (GLUT.idleFunc) {
        {{{ makeDynCall('v', 'GLUT.idleFunc') }}}();
        safeSetTimeout(callback, 4); // HTML spec specifies a 4ms minimum delay on the main thread; workers might get more, but we standardize here
      }
    }
    if (!GLUT.idleFunc) {
      safeSetTimeout(callback, 0);
    }
    GLUT.idleFunc = func;
  },

  glutTimerFunc__proxy: 'sync',
  glutTimerFunc: (msec, func, value) =>
    safeSetTimeout(() => {{{ makeDynCall('vi', 'func') }}}(value), msec),

  glutDisplayFunc__proxy: 'sync',
  glutDisplayFunc: (func) => {
    GLUT.displayFunc = func;
  },

  glutKeyboardFunc__proxy: 'sync',
  glutKeyboardFunc: (func) => {
    GLUT.keyboardFunc = func;
  },

  glutKeyboardUpFunc__proxy: 'sync',
  glutKeyboardUpFunc: (func) => {
    GLUT.keyboardUpFunc = func;
  },

  glutSpecialFunc__proxy: 'sync',
  glutSpecialFunc: (func) => {
    GLUT.specialFunc = func;
  },

  glutSpecialUpFunc__proxy: 'sync',
  glutSpecialUpFunc: (func) => {
    GLUT.specialUpFunc = func;
  },

  glutReshapeFunc__proxy: 'sync',
  glutReshapeFunc: (func) => {
    GLUT.reshapeFunc = func;
  },

  glutMotionFunc__proxy: 'sync',
  glutMotionFunc: (func) => {
    GLUT.motionFunc = func;
  },

  glutPassiveMotionFunc__proxy: 'sync',
  glutPassiveMotionFunc: (func) => {
    GLUT.passiveMotionFunc = func;
  },

  glutMouseFunc__proxy: 'sync',
  glutMouseFunc: (func) => {
    GLUT.mouseFunc = func;
  },

  $glutCursorStyles__internal: true,
  $glutCursorStyles: {
    [cDefs.GLUT_CURSOR_RIGHT_ARROW]: 'auto', // No equivalent css cursor style, fallback to 'auto'
    [cDefs.GLUT_CURSOR_LEFT_ARROW]: 'auto',
    [cDefs.GLUT_CURSOR_INFO]: 'pointer',
    [cDefs.GLUT_CURSOR_DESTROY]: 'auto',
    [cDefs.GLUT_CURSOR_HELP]: 'help',
    [cDefs.GLUT_CURSOR_CYCLE]: 'auto',
    [cDefs.GLUT_CURSOR_SPRAY]: 'auto',
    [cDefs.GLUT_CURSOR_WAIT]: 'wait',
    [cDefs.GLUT_CURSOR_TEXT]: 'text',
    [cDefs.GLUT_CURSOR_CROSSHAIR]: 'crosshair',
    [cDefs.GLUT_CURSOR_FULL_CROSSHAIR]: 'crosshair',
    [cDefs.GLUT_CURSOR_UP_DOWN]: 'ns-resize',
    [cDefs.GLUT_CURSOR_LEFT_RIGHT]: 'ew-resize',
    [cDefs.GLUT_CURSOR_TOP_SIDE]: 'n-resize',
    [cDefs.GLUT_CURSOR_BOTTOM_SIDE]: 's-resize',
    [cDefs.GLUT_CURSOR_LEFT_SIDE]: 'w-resize',
    [cDefs.GLUT_CURSOR_RIGHT_SIDE]: 'e-resize',
    [cDefs.GLUT_CURSOR_TOP_LEFT_CORNER]: 'nw-resize',
    [cDefs.GLUT_CURSOR_TOP_RIGHT_CORNER]: 'ne-resize',
    [cDefs.GLUT_CURSOR_BOTTOM_RIGHT_CORNER]: 'se-resize',
    [cDefs.GLUT_CURSOR_BOTTOM_LEFT_CORNER]: 'sw-resize',
    [cDefs.GLUT_CURSOR_INHERIT]: 'auto',
    [cDefs.GLUT_CURSOR_NONE]: 'none',
  },

  glutSetCursor__proxy: 'sync',
  glutSetCursor: (cursor) => {
    var cursorStyle = glutCursorStyles[cursor];
    if (!cursorStyle) {
      abort('glutSetCursor: Unknown cursor type: ' + cursor);
    }
    Browser.getCanvas().style.cursor = cursorStyle;
  },

  glutCreateWindow__proxy: 'sync',
  glutCreateWindow: (name) => {
    var contextAttributes = {
      antialias: (GLUT.initDisplayMode & GLUT_MULTISAMPLE) != 0,
      depth: (GLUT.initDisplayMode & GLUT_DEPTH) != 0,
      stencil: (GLUT.initDisplayMode & GLUT_STENCIL) != 0,
      alpha: (GLUT.initDisplayMode & GLUT_ALPHA) != 0
    };
#if OFFSCREEN_FRAMEBUFFER
    // TODO: Make glutCreateWindow explicitly aware of whether it is being proxied or not, and set these to true only when proxying is being performed.
    GL.enableOffscreenFramebufferAttributes(contextAttributes);
#endif
    if (!Browser.createContext(Browser.getCanvas(), /*useWebGL=*/true, /*setInModule=*/true, contextAttributes)) {
      return 0; // failure
    }
    return 1; // a new GLUT window ID for the created context
  },

  glutDestroyWindow__proxy: 'sync',
  glutDestroyWindow: (name) => {
    delete Module['ctx'];
    return 1;
  },

  glutReshapeWindow__proxy: 'sync',
  glutReshapeWindow: (width, height) => {
    Browser.exitFullscreen();
    Browser.setCanvasSize(width, height, true); // N.B. GLUT.reshapeFunc is also registered as a canvas resize callback.
                                                // Just call it once here.
    if (GLUT.reshapeFunc) {
      {{{ makeDynCall('vii', 'GLUT.reshapeFunc') }}}(width, height);
    }
    _glutPostRedisplay();
  },

  glutPositionWindow__proxy: 'sync',
  glutPositionWindow: (x, y) => {
    Browser.exitFullscreen();
    /* TODO */
    _glutPostRedisplay();
  },

  glutFullScreen__proxy: 'sync',
  glutFullScreen: () => {
    var canvas = Browser.getCanvas();
    GLUT.windowWidth = canvas.width;
    GLUT.windowHeight = canvas.height;
    document.addEventListener('fullscreenchange', GLUT.onFullscreenEventChange, true);
#if MIN_SAFARI_VERSION < 160400
    document.addEventListener('webkitfullscreenchange', GLUT.onFullscreenEventChange, true);
#endif
    Browser.requestFullscreen(/*lockPointer=*/false, /*resizeCanvas=*/false);
  },

  glutInitDisplayMode__proxy: 'sync',
  glutInitDisplayMode: (mode) => GLUT.initDisplayMode = mode,

  glutSwapBuffers__proxy: 'sync',
  glutSwapBuffers: () => {},

  glutPostRedisplay__proxy: 'sync',
  glutPostRedisplay: () => {
    if (GLUT.displayFunc && !GLUT.requestedAnimationFrame) {
      GLUT.requestedAnimationFrame = true;
      MainLoop.requestAnimationFrame(() => {
        GLUT.requestedAnimationFrame = false;
        MainLoop.runIter(() => {{{ makeDynCall('v', 'GLUT.displayFunc') }}}());
      });
    }
  },

  glutMainLoop__proxy: 'sync',
  glutMainLoop: () => {
    // Do an initial resize, since there's no window resize event on startup
    GLUT.onResize();
    _glutPostRedisplay();
    throw 'unwind';
  },

};

addToLibrary(LibraryGLUT);
