
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
    lastX: 0,
    lastY: 0,
    buttons: 0,
    modifiers: 0,
    initWindowWidth: 256,
    initWindowHeight: 256,

    savePosition: function(event) {
      /* TODO maybe loop here ala http://www.quirksmode.org/js/findpos.html */
      GLUT.lastX = event['clientX'] - Module['canvas'].offsetLeft;
      GLUT.lastY = event['clientY'] - Module['canvas'].offsetTop;
    },

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
      GLUT.savePosition(event);
      if (GLUT.buttons == 0 && event.target == Module["canvas"] && GLUT.passiveMotionFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        FUNCTION_TABLE[GLUT.passiveMotionFunc](GLUT.lastX, GLUT.lastY);
      } else if (GLUT.buttons != 0 && GLUT.motionFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        FUNCTION_TABLE[GLUT.motionFunc](GLUT.lastX, GLUT.lastY);
      }
    },

    getSpecialKey: function(keycode) {
        var key = null;
        switch (keycode) {
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
      if (106 <= keycode && keycode <= 111)
	return keycode - 106 + 42; // *,+-./  TODO handle shift?

      switch (keycode) {
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
            FUNCTION_TABLE[GLUT.specialFunc](key, GLUT.lastX, GLUT.lastY);
          }
        }
        else
        {
          key = GLUT.getASCIIKey(event);
          if( key !== null && GLUT.keyboardFunc ) {
            event.preventDefault();
            GLUT.saveModifiers(event);
            FUNCTION_TABLE[GLUT.keyboardFunc](key, GLUT.lastX, GLUT.lastY);
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
            FUNCTION_TABLE[GLUT.specialUpFunc](key, GLUT.lastX, GLUT.lastY);
          }
        }
        else
        {
          key = GLUT.getASCIIKey(event);
          if( key !== null && GLUT.keyboardUpFunc ) {
            event.preventDefault ();
            GLUT.saveModifiers(event);
            FUNCTION_TABLE[GLUT.keyboardUpFunc](key, GLUT.lastX, GLUT.lastY);
          }
        }
      }
    },

    onMouseButtonDown: function(event){
      GLUT.savePosition(event);
      GLUT.buttons |= (1 << event['button']);

      if(event.target == Module["canvas"] && GLUT.mouseFunc){
        try {
          event.target.setCapture();
        } catch (e) {}
        event.preventDefault();
        GLUT.saveModifiers(event);
        FUNCTION_TABLE[GLUT.mouseFunc](event['button'], 0/*GLUT_DOWN*/, GLUT.lastX, GLUT.lastY);
      }
    },

    onMouseButtonUp: function(event){
      GLUT.savePosition(event);
      GLUT.buttons &= ~(1 << event['button']);

      if(GLUT.mouseFunc) {
        event.preventDefault();
        GLUT.saveModifiers(event);
        FUNCTION_TABLE[GLUT.mouseFunc](event['button'], 1/*GLUT_UP*/, GLUT.lastX, GLUT.lastY);
      }
    },

    requestFullScreen: function() {
      var RFS = function() {};
      if (Module["canvas"]['requestFullscreen']) {
        RFS = Module["canvas"]['requestFullscreen'];
      } else if (Module["canvas"]['requestFullScreen']) {
        RFS = Module["canvas"]['requestFullScreen'];
      } else if (Module["canvas"]['mozRequestFullScreen']) {
        RFS = Module["canvas"]['mozRequestFullScreen'];
      } else if (Module["canvas"]['webkitRequestFullScreen']) {
        RFS = Module["canvas"]['webkitRequestFullScreen'];
      }
      RFS.apply(Module["canvas"], []);
    },

    cancelFullScreen: function() {
      var CFS = function() {};
      if (document['exitFullscreen']) {
        CFS = document['exitFullscreen'];
      } else if (document['cancelFullScreen']) {
        CFS = document['cancelFullScreen'];
      } else if (document['mozCancelFullScreen']) {
        CFS = document['mozCancelFullScreen'];
      } else if (document['webkitCancelFullScreen']) {
        CFS = document['webkitCancelFullScreen'];
      }
      CFS.apply(document, []);
    },

    requestAnimationFrame: function(func) {
      var RAF = window['setTimeout'];
      if (window['requestAnimationFrame']) {
        RAF = window['requestAnimationFrame'];
      } else if (window['mozRequestAnimationFrame']) {
        RAF = window['mozRequestAnimationFrame'];
      } else if (window['webkitRequestAnimationFrame']) {
        RAF = window['webkitRequestAnimationFrame'];
      } else if (window['msRequestAnimationFrame']) {
        RAF = window['msRequestAnimationFrame'];
      }
      RAF.apply(window, [func]);
    },
  },

  glutGetModifiers: function() { return GLUT.modifiers; },

  glutInit: function(argcp, argv) {
    // Ignore arguments
    GLUT.initTime = Date.now();
  },

  glutInitWindowSize: function(width, height) {
    Module['canvas'].width  = GLUT.initWindowWidth  = width;
    Module['canvas'].height = GLUT.initWindowHeight = height;
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
    var callback = function() {
      if (GLUT.idleFunc) {
        FUNCTION_TABLE[GLUT.idleFunc]();
        window.setTimeout(callback, 0);
      }
    }
    if (!GLUT.idleFunc)
      window.setTimeout(callback, 0);
    GLUT.idleFunc = func;
  },

  glutTimerFunc: function(msec, func, value) {
    window.setTimeout(function() { FUNCTION_TABLE[func](value); }, msec);
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

  glutCreateWindow: function(name) {
#if USE_TYPED_ARRAYS
    try {
      var ctx = Module["canvas"].getContext('experimental-webgl');
      if (!ctx) throw 'Could not create canvas :(';
#if GL_DEBUG
      var wrapper = {};
      wrapper.objectMap = new WeakMap();
      wrapper.objectCounter = 1;
      for (var prop in ctx) {
        (function(prop) {
          switch (typeof ctx[prop]) {
            case 'function': {
              wrapper[prop] = function() {
                var printArgs = Array.prototype.slice.call(arguments).map(function(arg) {
                  if (wrapper.objectMap[arg]) return '<' + arg + '|' + wrapper.objectMap[arg] + '>';
                  if (arg.subarray) return '{' + arg + '|' + arg.length /*+ '|' + Array.prototype.slice.call(arg).toString().replace(/,/g, ', ')*/ + '}';
                  return arg;
                });
                Module.printErr('[gl_f:' + prop + ':' + printArgs + ']');
                var ret = ctx[prop].apply(ctx, arguments);
                var printRet = ret;
                if (typeof ret == 'object') {
                  wrapper.objectMap[ret] = wrapper.objectCounter++;
                  printRet = '<' + ret + '|' + wrapper.objectMap[ret] + '>';
                }
                Module.printErr('[     gl:' + prop + ':return:' + printRet + ']');
                return ret;
              }
              break;
            }
            case 'number': case 'string': {
              wrapper.__defineGetter__(prop, function() {
                //Module.printErr('[gl_g:' + prop + ':' + ctx[prop] + ']');
                return ctx[prop];
              });
              wrapper.__defineSetter__(prop, function(value) {
                Module.printErr('[gl_s:' + prop + ':' + value + ']');
                ctx[prop] = value;
              });
              break;
            }
          }
        })(prop);
      }
      Module.ctx = wrapper;
#else
      Module.ctx = ctx;
#endif
      // Set the background of the canvas to black, because glut gives us a
      // window which has a black background by default.
      Module["canvas"].style.backgroundColor = "black";
    } catch (e) {
      Module.print('(canvas not available)');
    }
#else
    Module.print('(USE_TYPED_ARRAYS needs to be enabled for WebGL)');
#endif
    return 1;
  },

  glutReshapeWindow__deps: ['$GLUT', 'glutPostRedisplay'],
  glutReshapeWindow: function(width, height) {
    GLUT.cancelFullScreen();
    Module['canvas'].width  = width;
    Module['canvas'].height = height;
    if (GLUT.reshapeFunc) {
      FUNCTION_TABLE[GLUT.reshapeFunc](width, height);
    }
    _glutPostRedisplay();
  },

  glutPositionWindow__deps: ['$GLUT', 'glutPostRedisplay'],
  glutPositionWindow: function(x, y) {
    GLUT.cancelFullScreen();
    /* TODO */
    _glutPostRedisplay();
  },

  glutFullScreen__deps: ['$GLUT', 'glutPostRedisplay'],
  glutFullScreen: function() {
    var width = screen["width"];
    var height = screen["height"];
    /* Can't call _glutReshapeWindow as that requests cancelling fullscreen. */
    Module['canvas'].width  = width;
    Module['canvas'].height = height;
    if (GLUT.reshapeFunc) {
      FUNCTION_TABLE[GLUT.reshapeFunc](width, height);
    }
    GLUT.requestFullScreen();
    window.setTimeout(function() {
      _glutPostRedisplay();
    }, 0);
  },

  glutInitDisplayMode: function(mode) {},
  glutSwapBuffers: function() {},

  glutPostRedisplay: function() {
    if (GLUT.displayFunc) {
      GLUT.requestAnimationFrame(FUNCTION_TABLE[GLUT.displayFunc]);
    }
  },

  glutMainLoop__deps: ['$GLUT', 'glutReshapeWindow', 'glutPostRedisplay'],
  glutMainLoop: function() {

    window.addEventListener("keydown", GLUT.onKeydown, true);
    window.addEventListener("keyup", GLUT.onKeyup, true);
    window.addEventListener("mousemove", GLUT.onMousemove, true);
    window.addEventListener("mousedown", GLUT.onMouseButtonDown, true);
    window.addEventListener("mouseup", GLUT.onMouseButtonUp, true);

    __ATEXIT__.push({ func: function() {
      window.removeEventListener("keydown", GLUT.onKeydown, true);
      window.removeEventListener("keyup", GLUT.onKeyup, true);
      window.removeEventListener("mousemove", GLUT.onMousemove, true);
      window.removeEventListener("mousedown", GLUT.onMouseButtonDown, true);
      window.removeEventListener("mouseup", GLUT.onMouseButtonUp, true);
      Module["canvas"].width = Module["canvas"].height = 1;
    } });

    _glutReshapeWindow(Module['canvas'].width, Module['canvas'].height);
    _glutPostRedisplay();
    throw 'GLUT mainloop should never return';
  },

};

autoAddDeps(LibraryGLUT, '$GLUT');
mergeInto(LibraryManager.library, LibraryGLUT);

