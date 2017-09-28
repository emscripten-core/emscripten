
var LibraryGLUT = {
  $GLUT__deps: ['$Browser'],
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
    initDisplayMode: 0x0000 /*GLUT_RGBA*/ | 0x0002 /*GLUT_DOUBLE*/ | 0x0010 /*GLUT_DEPTH*/,
    // Set when going fullscreen
    windowX: 0,
    windowY: 0,
    windowWidth: 0,
    windowHeight: 0,
    requestedAnimationFrame: false,

    saveModifiers: function(event) {
      GLUT.modifiers = 0;
      if (event['shiftKey'])
        GLUT.modifiers += 1; /* GLUT_ACTIVE_SHIFT */
      if (event['ctrlKey'])
        GLUT.modifiers += 2; /* GLUT_ACTIVE_CTRL */
      if (event['altKey'])
        GLUT.modifiers += 4; /* GLUT_ACTIVE_ALT */
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
      if (newX == lastX && newY == lastY) return;

      if (GLUT.buttons == 0 && event.target == Module["canvas"] && GLUT.passiveMotionFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        Module['dynCall_vii'](GLUT.passiveMotionFunc, lastX, lastY);
      } else if (GLUT.buttons != 0 && GLUT.motionFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        Module['dynCall_vii'](GLUT.motionFunc, lastX, lastY);
      }
    },

    getSpecialKey: function(keycode) {
        var key = null;
        switch (keycode) {
          case 8:  key = 120 /* backspace */; break;
          case 46: key = 111 /* delete */; break;

          case 0x70 /*DOM_VK_F1*/: key = 1 /* GLUT_KEY_F1 */; break;
          case 0x71 /*DOM_VK_F2*/: key = 2 /* GLUT_KEY_F2 */; break;
          case 0x72 /*DOM_VK_F3*/: key = 3 /* GLUT_KEY_F3 */; break;
          case 0x73 /*DOM_VK_F4*/: key = 4 /* GLUT_KEY_F4 */; break;
          case 0x74 /*DOM_VK_F5*/: key = 5 /* GLUT_KEY_F5 */; break;
          case 0x75 /*DOM_VK_F6*/: key = 6 /* GLUT_KEY_F6 */; break;
          case 0x76 /*DOM_VK_F7*/: key = 7 /* GLUT_KEY_F7 */; break;
          case 0x77 /*DOM_VK_F8*/: key = 8 /* GLUT_KEY_F8 */; break;
          case 0x78 /*DOM_VK_F9*/: key = 9 /* GLUT_KEY_F9 */; break;
          case 0x79 /*DOM_VK_F10*/: key = 10 /* GLUT_KEY_F10 */; break;
          case 0x7a /*DOM_VK_F11*/: key = 11 /* GLUT_KEY_F11 */; break;
          case 0x7b /*DOM_VK_F12*/: key = 12 /* GLUT_KEY_F12 */; break;
          case 0x25 /*DOM_VK_LEFT*/: key = 100 /* GLUT_KEY_LEFT */; break;
          case 0x26 /*DOM_VK_UP*/: key = 101 /* GLUT_KEY_UP */; break;
          case 0x27 /*DOM_VK_RIGHT*/: key = 102 /* GLUT_KEY_RIGHT */; break;
          case 0x28 /*DOM_VK_DOWN*/: key = 103 /* GLUT_KEY_DOWN */; break;
          case 0x21 /*DOM_VK_PAGE_UP*/: key = 104 /* GLUT_KEY_PAGE_UP */; break;
          case 0x22 /*DOM_VK_PAGE_DOWN*/: key = 105 /* GLUT_KEY_PAGE_DOWN */; break;
          case 0x24 /*DOM_VK_HOME*/: key = 106 /* GLUT_KEY_HOME */; break;
          case 0x23 /*DOM_VK_END*/: key = 107 /* GLUT_KEY_END */; break;
          case 0x2d /*DOM_VK_INSERT*/: key = 108 /* GLUT_KEY_INSERT */; break;

          case 16   /*DOM_VK_SHIFT*/:
          case 0x05 /*DOM_VK_LEFT_SHIFT*/:
            key = 112 /* GLUT_KEY_SHIFT_L */;
            break;
          case 0x06 /*DOM_VK_RIGHT_SHIFT*/:
            key = 113 /* GLUT_KEY_SHIFT_R */;
            break;

          case 17   /*DOM_VK_CONTROL*/:
          case 0x03 /*DOM_VK_LEFT_CONTROL*/:
            key = 114 /* GLUT_KEY_CONTROL_L */;
            break;
          case 0x04 /*DOM_VK_RIGHT_CONTROL*/:
            key = 115 /* GLUT_KEY_CONTROL_R */;
            break;

          case 18   /*DOM_VK_ALT*/:
          case 0x02 /*DOM_VK_LEFT_ALT*/:
            key = 116 /* GLUT_KEY_ALT_L */;
            break;
          case 0x01 /*DOM_VK_RIGHT_ALT*/:
            key = 117 /* GLUT_KEY_ALT_R */;
            break;
        };
        return key;
    },

    getASCIIKey: function(event) {
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
        case 221: return s ? 125 : 93; // close braket
        case 222: return s ? 34 : 39; // single quote
      }

      return null;
    },

    onKeydown: function(event) {
      if (GLUT.specialFunc || GLUT.keyboardFunc) {
        var key = GLUT.getSpecialKey(event['keyCode']);
        if (key !== null) {
          if( GLUT.specialFunc ) {
            event.preventDefault();
            GLUT.saveModifiers(event);
            Module['dynCall_viii'](GLUT.specialFunc, key, Browser.mouseX, Browser.mouseY);
          }
        }
        else
        {
          key = GLUT.getASCIIKey(event);
          if( key !== null && GLUT.keyboardFunc ) {
            event.preventDefault();
            GLUT.saveModifiers(event);
            Module['dynCall_viii'](GLUT.keyboardFunc, key, Browser.mouseX, Browser.mouseY);
          }
        }
      }
    },

    onKeyup: function(event) {
      if (GLUT.specialUpFunc || GLUT.keyboardUpFunc) {
        var key = GLUT.getSpecialKey(event['keyCode']);
        if (key !== null) {
          if(GLUT.specialUpFunc) {
            event.preventDefault ();
            GLUT.saveModifiers(event);
            Module['dynCall_viii'](GLUT.specialUpFunc, key, Browser.mouseX, Browser.mouseY);
          }
        }
        else
        {
          key = GLUT.getASCIIKey(event);
          if( key !== null && GLUT.keyboardUpFunc ) {
            event.preventDefault ();
            GLUT.saveModifiers(event);
            Module['dynCall_viii'](GLUT.keyboardUpFunc, key, Browser.mouseX, Browser.mouseY);
          }
        }
      }
    },

    touchHandler: function(event) {
      if (event.target != Module['canvas']) {
        return;
      }

      var touches = event.changedTouches,
          main = touches[0],
          type = "";

      switch(event.type) {
        case "touchstart": type = "mousedown"; break;
        case "touchmove": type = "mousemove"; break;
        case "touchend": type = "mouseup"; break;
        default: return;
      }

      var simulatedEvent = document.createEvent("MouseEvent");
      simulatedEvent.initMouseEvent(type, true, true, window, 1, 
                                    main.screenX, main.screenY, 
                                    main.clientX, main.clientY, false, 
                                    false, false, false, 0/*main*/, null);

      main.target.dispatchEvent(simulatedEvent);
      event.preventDefault();
    },

    onMouseButtonDown: function(event) {
      Browser.calculateMouseEvent(event);

      GLUT.buttons |= (1 << event['button']);

      if (event.target == Module["canvas"] && GLUT.mouseFunc) {
        try {
          event.target.setCapture();
        } catch (e) {}
        event.preventDefault();
        GLUT.saveModifiers(event);
        Module['dynCall_viiii'](GLUT.mouseFunc, event['button'], 0/*GLUT_DOWN*/, Browser.mouseX, Browser.mouseY);
      }
    },

    onMouseButtonUp: function(event) {
      Browser.calculateMouseEvent(event);

      GLUT.buttons &= ~(1 << event['button']);

      if (GLUT.mouseFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        Module['dynCall_viiii'](GLUT.mouseFunc, event['button'], 1/*GLUT_UP*/, Browser.mouseX, Browser.mouseY);
      }
    },

    onMouseWheel: function(event) {
      Browser.calculateMouseEvent(event);

      // cross-browser wheel delta
      var e = window.event || event; // old IE support
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
        Module['dynCall_viiii'](GLUT.mouseFunc, button, 0/*GLUT_DOWN*/, Browser.mouseX, Browser.mouseY);
      }
    },

    // TODO add fullscreen API ala:
    // http://johndyer.name/native-fullscreen-javascript-api-plus-jquery-plugin/
    onFullscreenEventChange: function(event) {
      var width;
      var height;
      if (document["fullscreen"] || document["fullScreen"] || document["mozFullScreen"] || document["webkitIsFullScreen"]) {
        width = screen["width"];
        height = screen["height"];
      } else {
        width = GLUT.windowWidth;
        height = GLUT.windowHeight;
        // TODO set position
        document.removeEventListener('fullscreenchange', GLUT.onFullscreenEventChange, true);
        document.removeEventListener('mozfullscreenchange', GLUT.onFullscreenEventChange, true);
        document.removeEventListener('webkitfullscreenchange', GLUT.onFullscreenEventChange, true);
      }
      Browser.setCanvasSize(width, height);
      /* Can't call _glutReshapeWindow as that requests cancelling fullscreen. */
      if (GLUT.reshapeFunc) {
        // console.log("GLUT.reshapeFunc (from FS): " + width + ", " + height);
        Module['dynCall_vii'](GLUT.reshapeFunc, width, height);
      }
      _glutPostRedisplay();
    },

    requestFullscreen: function() {
      Browser.requestFullscreen(/*lockPointer=*/false, /*resieCanvas=*/false);
    },

    requestFullScreen: function() {
      Module.printErr('GLUT.requestFullScreen() is deprecated. Please call GLUT.requestFullscreen instead.');
      GLUT.requestFullScreen = function() {
        return GLUT.requestFullscreen();
      }
      return GLUT.requestFullscreen();
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
      Module.printErr('GLUT.cancelFullScreen() is deprecated. Please call GLUT.exitFullscreen instead.');
      GLUT.cancelFullScreen = function() {
        return GLUT.exitFullscreen();
      }
      return GLUT.exitFullscreen();
    }
  },

  glutGetModifiers: function() { return GLUT.modifiers; },

  glutInit__deps: ['$Browser'],
  glutInit: function(argcp, argv) {
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
      window.addEventListener("touchmove", GLUT.touchHandler, true);
      window.addEventListener("touchstart", GLUT.touchHandler, true);
      window.addEventListener("touchend", GLUT.touchHandler, true);
    }

    window.addEventListener("keydown", GLUT.onKeydown, true);
    window.addEventListener("keyup", GLUT.onKeyup, true);
    window.addEventListener("mousemove", GLUT.onMousemove, true);
    window.addEventListener("mousedown", GLUT.onMouseButtonDown, true);
    window.addEventListener("mouseup", GLUT.onMouseButtonUp, true);
    // IE9, Chrome, Safari, Opera
    window.addEventListener("mousewheel", GLUT.onMouseWheel, true);
    // Firefox
    window.addEventListener("DOMMouseScroll", GLUT.onMouseWheel, true);

    Browser.resizeListeners.push(function(width, height) {
      if (GLUT.reshapeFunc) {
        Module['dynCall_vii'](GLUT.reshapeFunc, width, height);
      }
    });

    __ATEXIT__.push(function() {
      if (isTouchDevice) {
        window.removeEventListener("touchmove", GLUT.touchHandler, true);
        window.removeEventListener("touchstart", GLUT.touchHandler, true);
        window.removeEventListener("touchend", GLUT.touchHandler, true);
      }

      window.removeEventListener("keydown", GLUT.onKeydown, true);
      window.removeEventListener("keyup", GLUT.onKeyup, true);
      window.removeEventListener("mousemove", GLUT.onMousemove, true);
      window.removeEventListener("mousedown", GLUT.onMouseButtonDown, true);
      window.removeEventListener("mouseup", GLUT.onMouseButtonUp, true);
      // IE9, Chrome, Safari, Opera
      window.removeEventListener("mousewheel", GLUT.onMouseWheel, true);
      // Firefox
      window.removeEventListener("DOMMouseScroll", GLUT.onMouseWheel, true);

      Module["canvas"].width = Module["canvas"].height = 1;
    });
  },

  glutInitWindowSize: function(width, height) {
    Browser.setCanvasSize( GLUT.initWindowWidth = width,
                           GLUT.initWindowHeight = height );
  },

  glutInitWindowPosition: function(x, y) {
    // Ignore for now
  },

  glutGet: function(type) {
    switch (type) {
      case 100: /* GLUT_WINDOW_X */
        return 0; /* TODO */
      case 101: /* GLUT_WINDOW_Y */
        return 0; /* TODO */
      case 102: /* GLUT_WINDOW_WIDTH */
        return Module['canvas'].width;
      case 103: /* GLUT_WINDOW_HEIGHT */
        return Module['canvas'].height;
      case 200: /* GLUT_SCREEN_WIDTH */
        return Module['canvas'].width;
      case 201: /* GLUT_SCREEN_HEIGHT */
        return Module['canvas'].height;
      case 500: /* GLUT_INIT_WINDOW_X */
        return 0; /* TODO */
      case 501: /* GLUT_INIT_WINDOW_Y */
        return 0; /* TODO */
      case 502: /* GLUT_INIT_WINDOW_WIDTH */
        return GLUT.initWindowWidth;
      case 503: /* GLUT_INIT_WINDOW_HEIGHT */
        return GLUT.initWindowHeight;
      case 700: /* GLUT_ELAPSED_TIME */
        var now = Date.now();
        return now - GLUT.initTime;

      default:
        throw "glutGet(" + type + ") not implemented yet";
    }
  },

  glutIdleFunc: function(func) {
    function callback() {
      if (GLUT.idleFunc) {
        Module['dynCall_v'](GLUT.idleFunc);
        Browser.safeSetTimeout(callback, 4); // HTML spec specifies a 4ms minimum delay on the main thread; workers might get more, but we standardize here
      }
    }
    if (!GLUT.idleFunc) {
      Browser.safeSetTimeout(callback, 0);
    }
    GLUT.idleFunc = func;
  },

  glutTimerFunc: function(msec, func, value) {
    Browser.safeSetTimeout(function() { Module['dynCall_vi'](func, value); }, msec);
  },

  glutDisplayFunc: function(func) {
    GLUT.displayFunc = func;
  },

  glutKeyboardFunc: function(func) {
    GLUT.keyboardFunc = func;
  },

  glutKeyboardUpFunc: function(func) {
    GLUT.keyboardUpFunc = func;
  },

  glutSpecialFunc: function(func) {
    GLUT.specialFunc = func;
  },

  glutSpecialUpFunc: function(func) {
    GLUT.specialUpFunc = func;
  },

  glutReshapeFunc: function(func) {
    GLUT.reshapeFunc = func;
  },

  glutMotionFunc: function(func) {
    GLUT.motionFunc = func;
  },

  glutPassiveMotionFunc: function(func) {
    GLUT.passiveMotionFunc = func;
  },

  glutMouseFunc: function(func) {
    GLUT.mouseFunc = func;
  },

  glutSetCursor: function(cursor) {
    var cursorStyle = 'auto';
    switch(cursor) {
      case 0x0000: /* GLUT_CURSOR_RIGHT_ARROW */
        // No equivalent css cursor style, fallback to 'auto'
        break;
      case 0x0001: /* GLUT_CURSOR_LEFT_ARROW */
        // No equivalent css cursor style, fallback to 'auto'
        break;
      case 0x0002: /* GLUT_CURSOR_INFO */
        cursorStyle = 'pointer';
        break;
      case 0x0003: /* GLUT_CURSOR_DESTROY */
        // No equivalent css cursor style, fallback to 'auto'
        break;
      case 0x0004: /* GLUT_CURSOR_HELP */
        cursorStyle = 'help';
        break;
      case 0x0005: /* GLUT_CURSOR_CYCLE */
        // No equivalent css cursor style, fallback to 'auto'
        break;
      case 0x0006: /* GLUT_CURSOR_SPRAY */
        // No equivalent css cursor style, fallback to 'auto'
        break;
      case 0x0007: /* GLUT_CURSOR_WAIT */
        cursorStyle = 'wait';
        break;
      case 0x0008: /* GLUT_CURSOR_TEXT */
        cursorStyle = 'text';
        break;
      case 0x0009: /* GLUT_CURSOR_CROSSHAIR */
      case 0x0066: /* GLUT_CURSOR_FULL_CROSSHAIR */
        cursorStyle = 'crosshair';
        break;
      case 0x000A: /* GLUT_CURSOR_UP_DOWN */
        cursorStyle = 'ns-resize';
        break;
      case 0x000B: /* GLUT_CURSOR_LEFT_RIGHT */
        cursorStyle = 'ew-resize';
        break;
      case 0x000C: /* GLUT_CURSOR_TOP_SIDE */
        cursorStyle = 'n-resize';
        break;
      case 0x000D: /* GLUT_CURSOR_BOTTOM_SIDE */
        cursorStyle = 's-resize';
        break;
      case 0x000E: /* GLUT_CURSOR_LEFT_SIDE */
        cursorStyle = 'w-resize';
        break;
      case 0x000F: /* GLUT_CURSOR_RIGHT_SIDE */
        cursorStyle = 'e-resize';
        break;
      case 0x0010: /* GLUT_CURSOR_TOP_LEFT_CORNER */
        cursorStyle = 'nw-resize';
        break;
      case 0x0011: /* GLUT_CURSOR_TOP_RIGHT_CORNER */
        cursorStyle = 'ne-resize';
        break;
      case 0x0012: /* GLUT_CURSOR_BOTTOM_RIGHT_CORNER */
        cursorStyle = 'se-resize';
        break;
      case 0x0013: /* GLUT_CURSOR_BOTTOM_LEFT_CORNER */
        cursorStyle = 'sw-resize';
        break;
      case 0x0064: /* GLUT_CURSOR_INHERIT */
        break;
      case 0x0065: /* GLUT_CURSOR_NONE */
        cursorStyle = 'none';
        break;
      default:
        throw "glutSetCursor: Unknown cursor type: " + cursor;
    }
    Module['canvas'].style.cursor = cursorStyle;
  },
  
  glutCreateWindow__deps: ['$Browser'],
  glutCreateWindow: function(name) {
    var contextAttributes = {
      antialias: ((GLUT.initDisplayMode & 0x0080 /*GLUT_MULTISAMPLE*/) != 0),
      depth: ((GLUT.initDisplayMode & 0x0010 /*GLUT_DEPTH*/) != 0),
      stencil: ((GLUT.initDisplayMode & 0x0020 /*GLUT_STENCIL*/) != 0),
      alpha: ((GLUT.initDisplayMode & 0x0008 /*GLUT_ALPHA*/) != 0)
    };
    Module.ctx = Browser.createContext(Module['canvas'], true, true, contextAttributes);
    return Module.ctx ? 1 /* a new GLUT window ID for the created context */ : 0 /* failure */;
  },

  glutDestroyWindow__deps: ['$Browser'],
  glutDestroyWindow: function(name) {
    Module.ctx = Browser.destroyContext(Module['canvas'], true, true);
    return 1;
  },

  glutReshapeWindow__deps: ['$GLUT', 'glutPostRedisplay'],
  glutReshapeWindow: function(width, height) {
    GLUT.exitFullscreen();
    Browser.setCanvasSize(width, height);
    if (GLUT.reshapeFunc) {
      Module['dynCall_vii'](GLUT.reshapeFunc, width, height);
    }
    _glutPostRedisplay();
  },

  glutPositionWindow__deps: ['$GLUT', 'glutPostRedisplay'],
  glutPositionWindow: function(x, y) {
    GLUT.exitFullscreen();
    /* TODO */
    _glutPostRedisplay();
  },

  glutFullScreen__deps: ['$GLUT', 'glutPostRedisplay'],
  glutFullScreen: function() {
    GLUT.windowX = 0; // TODO
    GLUT.windowY = 0; // TODO
    GLUT.windowWidth  = Module['canvas'].width;
    GLUT.windowHeight = Module['canvas'].height;
    document.addEventListener('fullscreenchange', GLUT.onFullscreenEventChange, true);
    document.addEventListener('mozfullscreenchange', GLUT.onFullscreenEventChange, true);
    document.addEventListener('webkitfullscreenchange', GLUT.onFullscreenEventChange, true);
    GLUT.requestFullscreen();
  },

  glutInitDisplayMode: function(mode) {
    GLUT.initDisplayMode = mode;
  },

  glutSwapBuffers: function() {},

  glutPostRedisplay: function() {
    if (GLUT.displayFunc && !GLUT.requestedAnimationFrame) {
      GLUT.requestedAnimationFrame = true;
      Browser.requestAnimationFrame(function() {
        GLUT.requestedAnimationFrame = false;
        Browser.mainLoop.runIter(function() {
          Module['dynCall_v'](GLUT.displayFunc);
        });
      });
    }
  },

  glutMainLoop__deps: ['$GLUT', 'glutReshapeWindow', 'glutPostRedisplay'],
  glutMainLoop: function() {
    _glutReshapeWindow(Module['canvas'].width, Module['canvas'].height);
    _glutPostRedisplay();
    throw 'SimulateInfiniteLoop';
  },

};

autoAddDeps(LibraryGLUT, '$GLUT');
mergeInto(LibraryManager.library, LibraryGLUT);

