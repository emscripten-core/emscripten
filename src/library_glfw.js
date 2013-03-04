
var LibraryGLFW = {
  $GLFW: {
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
    // Set when going fullscreen
    windowX: 0,
    windowY: 0,
    windowWidth: 0,
    windowHeight: 0,

    savePosition: function(event) {
      /* TODO maybe loop here ala http://www.quirksmode.org/js/findpos.html */
      GLFW.lastX = event['clientX'] - Module['canvas'].offsetLeft;
      GLFW.lastY = event['clientY'] - Module['canvas'].offsetTop;
    },

    saveModifiers: function(event) {
/*
      GLFW.modifiers = 0;
      if (event['shiftKey'])
        GLFW.modifiers += 1; // GLFW_ACTIVE_SHIFT
      if (event['ctrlKey'])
        GLFW.modifiers += 2; // GLFW_ACTIVE_CTRL 
      if (event['altKey'])
        GLFW.modifiers += 4; // GLFW_ACTIVE_ALT
*/
    },

    onMousemove: function(event) {
      /* Send motion event only if the motion changed, prevents
       * spamming our app with uncessary callback call. It does happen in
       * Chrome on Windows.
       */
      var newX = event['clientX'] - Module['canvas'].offsetLeft;
      var newY = event['clientY'] - Module['canvas'].offsetTop;
      if (newX == GLFW.lastX && newY == GLFW.lastY)
        return;

      GLFW.savePosition(event);
/*
      if (GLFW.buttons == 0 && event.target == Module["canvas"] && GLFW.passiveMotionFunc) {
        event.preventDefault();
        GLFW.saveModifiers(event);
        Runtime.dynCall('vii', GLFW.passiveMotionFunc, [GLFW.lastX, GLFW.lastY]);
      } else if (GLFW.buttons != 0 && GLFW.motionFunc) {
        event.preventDefault();
        GLFW.saveModifiers(event);
        Runtime.dynCall('vii', GLFW.motionFunc, [GLFW.lastX, GLFW.lastY]);
      }
*/
    },

    getSpecialKey: function(keycode) {
        var key = null;
/*
        switch (keycode) {
          case 0x70 : //DOM_VK_F1
             key = 1 ;//GLUT_KEY_F1
             break;
          case 0x71 : //DOM_VK_F2
             key = 2 ;//GLUT_KEY_F2
             break;
          case 0x72 : //DOM_VK_F3
             key = 3 ;//GLUT_KEY_F3
             break;
          case 0x73 : //DOM_VK_F4
             key = 4 ;//GLUT_KEY_F4
             break;
          case 0x74 : //DOM_VK_F5
             key = 5 ;//GLUT_KEY_F5
             break;
          case 0x75 : //DOM_VK_F6
             key = 6 ;//GLUT_KEY_F6
             break;
          case 0x76 : //DOM_VK_F7
             key = 7 ;//GLUT_KEY_F7
             break;
          case 0x77 : //DOM_VK_F8
             key = 8 ;//GLUT_KEY_F8
             break;
          case 0x78 : //DOM_VK_F9
             key = 9 ;//GLUT_KEY_F9
             break;
          case 0x79 : //DOM_VK_F10
             key = 10 ;//GLUT_KEY_F10
             break;
          case 0x7a : //DOM_VK_F11
             key = 11 ;//GLUT_KEY_F11
             break;
          case 0x7b : //DOM_VK_F12
             key = 12 ;//GLUT_KEY_F12
             break;
          case 0x25 : //DOM_VK_LEFT
             key = 100 ;//GLUT_KEY_LEFT
             break;
          case 0x26 : //DOM_VK_UP
             key = 101 ;//GLUT_KEY_UP
             break;
          case 0x27 : //DOM_VK_RIGHT
             key = 102 ;//GLUT_KEY_RIGHT
             break;
          case 0x28 : //DOM_VK_DOWN
             key = 103 ;//GLUT_KEY_DOWN
             break;
          case 0x21 : //DOM_VK_PAGE_UP
             key = 104 ;//GLUT_KEY_PAGE_UP
             break;
          case 0x22 : //DOM_VK_PAGE_DOWN
             key = 105 ;//GLUT_KEY_PAGE_DOWN
             break;
          case 0x24 : //DOM_VK_HOME
             key = 106 ;//GLUT_KEY_HOME
             break;
          case 0x23 : //DOM_VK_END
             key = 107 ;//GLUT_KEY_END
             break;
          case 0x2d : //DOM_VK_INSERT
             key = 108 ;//GLUT_KEY_INSERT
             break;
          case 16   : //DOM_VK_SHIFT
          case 0x05 : //DOM_VK_LEFT_SHIFT
            key = 112 ;//GLUT_KEY_SHIFT_L
             break;
          case 0x06 : //DOM_VK_RIGHT_SHIFT
            key = 113 ;//GLUT_KEY_SHIFT_R
             break;
          case 17   : //DOM_VK_CONTROL
          case 0x03 : //DOM_VK_LEFT_CONTROL            
            key = 114 ;//GLUT_KEY_CONTROL_L
             break;
          case 0x04 : //DOM_VK_RIGHT_CONTROL
            key = 115 ;//GLUT_KEY_CONTROL_R
             break;
          case 18   : //DOM_VK_ALT
          case 0x02 : //DOM_VK_LEFT_ALT
            key = 116 ;//GLUT_KEY_ALT_L
             break;
          case 0x01 : //DOM_VK_RIGHT_ALT
            key = 117 ;//GLUT_KEY_ALT_R
             break;
        };
*/
        return key;
    },

    getASCIIKey: function(event) {
      if (event['ctrlKey'] || event['altKey'] || event['metaKey']) return null;
/*
      var keycode = event['keyCode'];
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
*/
      return null;
    },

    onKeydown: function(event) {
/*
      if (GLFW.specialFunc || GLFW.keyboardFunc) {
        var key = GLFW.getSpecialKey(event['keyCode']);
        if (key !== null) {
          if( GLFW.specialFunc ) {
            event.preventDefault();
            GLFW.saveModifiers(event);
            Runtime.dynCall('viii', GLFW.specialFunc, [key, GLFW.lastX, GLFW.lastY]);
          }
        }
        else
        {
          key = GLFW.getASCIIKey(event);
          if( key !== null && GLFW.keyboardFunc ) {
            event.preventDefault();
            GLFW.saveModifiers(event);
            Runtime.dynCall('viii', GLFW.keyboardFunc, [key, GLFW.lastX, GLFW.lastY]);
          }
        }
      }
*/
    },

    onKeyup: function(event) {
/*
      if (GLFW.specialUpFunc || GLFW.keyboardUpFunc) {
        var key = GLFW.getSpecialKey(event['keyCode']);
        if (key !== null) {
          if(GLFW.specialUpFunc) {
            event.preventDefault ();
            GLFW.saveModifiers(event);
            Runtime.dynCall('viii', GLFW.specialUpFunc, [key, GLFW.lastX, GLFW.lastY]);
          }
        }
        else
        {
          key = GLFW.getASCIIKey(event);
          if( key !== null && GLFW.keyboardUpFunc ) {
            event.preventDefault ();
            GLFW.saveModifiers(event);
            Runtime.dynCall('viii', GLFW.keyboardUpFunc, [key, GLFW.lastX, GLFW.lastY]);
          }
        }
      }
*/
    },

    onMouseButtonDown: function(event){
/*
      GLFW.savePosition(event);
      GLFW.buttons |= (1 << event['button']);

      if(event.target == Module["canvas"] && GLFW.mouseFunc){
        try {
          event.target.setCapture();
        } catch (e) {}
        event.preventDefault();
        GLFW.saveModifiers(event);
        Runtime.dynCall('viiii', GLFW.mouseFunc, [event['button'], 0, GLFW.lastX, GLFW.lastY]);
      }
    },

    onMouseButtonUp: function(event){
      GLFW.savePosition(event);
      GLFW.buttons &= ~(1 << event['button']);

      if(GLFW.mouseFunc) {
        event.preventDefault();
        GLFW.saveModifiers(event);
        Runtime.dynCall('viiii', GLFW.mouseFunc, [event['button'], 1, GLFW.lastX, GLFW.lastY]);
      }
*/
    },

    // TODO add fullscreen API ala:
    // http://johndyer.name/native-fullscreen-javascript-api-plus-jquery-plugin/
    onFullScreenEventChange: function(event){
/*
      var width;
      var height;
      if (document["fullScreen"] || document["mozFullScreen"] || document["webkitIsFullScreen"]) {
        width = screen["width"];
        height = screen["height"];
      } else {
        width = GLFW.windowWidth;
        height = GLFW.windowHeight;
	    // TODO set position
        document.removeEventListener('fullscreenchange', GLFW.onFullScreenEventChange, true);
        document.removeEventListener('mozfullscreenchange', GLFW.onFullScreenEventChange, true);
        document.removeEventListener('webkitfullscreenchange', GLFW.onFullScreenEventChange, true);
      }
      Browser.setCanvasSize(width, height);
      // Can't call _glfwReshapeWindow as that requests cancelling fullscreen.
      if (GLFW.reshapeFunc) {
        // console.log("GLFW.reshapeFunc (from FS): " + width + ", " + height);
        Runtime.dynCall('vii', GLFW.reshapeFunc, [width, height]);
      }
      _glfwPostRedisplay();
*/
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

	/* GLFW initialization, termination and version querying */
	glfwInit : function() { throw "glfwInit is not implemented yet."; },
	glfwTerminate : function() { throw "glfwTerminate is not implemented yet."; },
	glfwGetVersion : function( major, minor, rev ) { throw "glfwGetVersion is not implemented yet."; },

	/* Window handling */
	glfwOpenWindow : function( width, height, redbits, greenbits, bluebits, alphabits, depthbits, stencilbits, mode ) { throw "glfwOpenWindow is not implemented yet."; },
	glfwOpenWindowHint : function( target, hint ) { throw "glfwOpenWindowHint is not implemented yet."; },
	glfwCloseWindow : function() { throw "glfwCloseWindow is not implemented yet."; },
	glfwSetWindowTitle : function( title ) { throw "glfwSetWindowTitle is not implemented yet."; },
	glfwGetWindowSize : function( width, height ) { throw "glfwGetWindowSize is not implemented yet."; },
	glfwSetWindowSize : function( width, height ) { throw "glfwSetWindowSize is not implemented yet."; },
	glfwSetWindowPos : function( x, y ) { throw "glfwSetWindowPos is not implemented yet."; },
	glfwIconifyWindow : function() { throw "glfwIconifyWindow is not implemented yet."; },
	glfwRestoreWindow : function() { throw "glfwRestoreWindow is not implemented yet."; },
	glfwSwapBuffers : function() { throw "glfwSwapBuffers is not implemented yet."; },
	glfwSwapInterval : function( interval ) { throw "glfwSwapInterval is not implemented yet."; },
	glfwGetWindowParam : function( param ) { throw "glfwGetWindowParam is not implemented yet."; },
	glfwSetWindowSizeCallback : function( cbfun ) { throw "glfwSetWindowSizeCallback is not implemented yet."; },
	glfwSetWindowCloseCallback : function( cbfun ) { throw "glfwSetWindowCloseCallback is not implemented yet."; },
	glfwSetWindowRefreshCallback : function( cbfun ) { throw "glfwSetWindowRefreshCallback is not implemented yet."; },

	/* Video mode functions */
	glfwGetVideoModes : function( list, maxcount ) { throw "glfwGetVideoModes is not implemented yet."; },
	glfwGetDesktopMode : function( mode ) { throw "glfwGetDesktopMode is not implemented yet."; },

	/* Input handling */
	glfwPollEvents : function() { throw "glfwPollEvents is not implemented yet."; },
	glfwWaitEvents : function() { throw "glfwWaitEvents is not implemented yet."; },
	glfwGetKey : function( key ) { throw "glfwGetKey is not implemented yet."; },
	glfwGetMouseButton : function( button ) { throw "glfwGetMouseButton is not implemented yet."; },
	glfwGetMousePos : function( xpos, ypos ) { throw "glfwGetMousePos is not implemented yet."; },
	glfwSetMousePos : function( xpos, ypos ) { throw "glfwSetMousePos is not implemented yet."; },
	glfwGetMouseWheel : function() { throw "glfwGetMouseWheel is not implemented yet."; },
	glfwSetMouseWheel : function( pos ) { throw "glfwSetMouseWheel is not implemented yet."; },
	glfwSetKeyCallback : function( cbfun ) { throw "glfwSetKeyCallback is not implemented yet."; },
	glfwSetCharCallback : function( cbfun ) { throw "glfwSetCharCallback is not implemented yet."; },
	glfwSetMouseButtonCallback : function( cbfun ) { throw "glfwSetMouseButtonCallback is not implemented yet."; },
	glfwSetMousePosCallback : function( cbfun ) { throw "glfwSetMousePosCallback is not implemented yet."; },
	glfwSetMouseWheelCallback : function( cbfun ) { throw "glfwSetMouseWheelCallback is not implemented yet."; },

	/* Joystick input */
	glfwGetJoystickParam : function( joy, param ) { throw "glfwGetJoystickParam is not implemented yet."; },
	glfwGetJoystickPos : function( joy, pos, numaxes ) { throw "glfwGetJoystickPos is not implemented yet."; },
	glfwGetJoystickButtons : function( joy, buttons, numbuttons ) { throw "glfwGetJoystickButtons is not implemented yet."; },

	/* Time */
	glfwGetTime : function() { throw "glfwGetTime is not implemented yet."; },
	glfwSetTime : function( time ) { throw "glfwSetTime is not implemented yet."; },
	glfwSleep : function( time ) { throw "glfwSleep is not implemented yet."; },

	/* Extension support */
	glfwExtensionSupported : function( extension ) { throw " is not implemented yet."; },
	glfwGetProcAddress : function( procname ) { throw " is not implemented yet."; },
	glfwGetGLVersion : function( major, minor, rev ) { throw " is not implemented yet."; },

	/* Threading support */
	glfwCreateThread : function( fun, arg ) { throw "glfwCreateThread is not implemented yet."; },
	glfwDestroyThread : function( ID ) { throw "glfwDestroyThread is not implemented yet."; },
	glfwWaitThread : function( ID, waitmode ) { throw "glfwWaitThread is not implemented yet."; },
	glfwGetThreadID : function() { throw "glfwGetThreadID is not implemented yet."; },
	glfwCreateMutex : function() { throw "glfwCreateMutex is not implemented yet."; },
	glfwDestroyMutex : function( mutex ) { throw "glfwDestroyMutex is not implemented yet."; },
	glfwLockMutex : function( mutex ) { throw "glfwLockMutex is not implemented yet."; },
	glfwUnlockMutex : function( mutex ) { throw "glfwUnlockMutex is not implemented yet."; },
	glfwCreateCond : function() { throw "glfwCreateCond is not implemented yet."; },
	glfwDestroyCond : function( cond ) { throw "glfwDestroyCond is not implemented yet."; },
	glfwWaitCond : function( cond, mutex, timeout ) { throw "glfwWaitCond is not implemented yet."; },
	glfwSignalCond : function( cond ) { throw "glfwSignalCond is not implemented yet."; },
	glfwBroadcastCond : function( cond ) { throw "glfwBroadcastCond is not implemented yet."; },
	glfwGetNumberOfProcessors : function() { throw "glfwGetNumberOfProcessors is not implemented yet."; },

	/* Enable/disable functions */
	glfwEnable : function( token ) { throw "glfwEnable is not implemented yet."; },
	glfwDisable : function( token ) { throw "glfwDisable is not implemented yet."; },

	/* Image/texture I/O support */
	glfwReadImage : function( name, img, flags ) { throw "glfwReadImage is not implemented yet."; },
	glfwReadMemoryImage : function( data, size, img, flags ) { throw "glfwReadMemoryImage is not implemented yet."; },
	glfwFreeImage : function( img ) { throw "glfwFreeImage is not implemented yet."; },
	glfwLoadTexture2D : function( name, flags ) { throw "glfwLoadTexture2D is not implemented yet."; },
	glfwLoadMemoryTexture2D : function( data, size, flags ) { throw "glfwLoadMemoryTexture2D is not implemented yet."; },
	glfwLoadTextureImage2D : function( img, flags ) { throw "glfwLoadTextureImage2D is not implemented yet."; },

};

autoAddDeps(LibraryGLFW, '$GLFW');
mergeInto(LibraryManager.library, LibraryGLFW);

