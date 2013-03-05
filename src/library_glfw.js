
var LibraryGLFW = {
  $GLFW: {
    keyboardFunc: null,
	charFunc: null,
	mouseButtonFunc: null,
	mousePosFunc: null,
	mouseWheelFunc: null,
    resizeFunc: null,
	closeFunc: null,
	refreshFunc: null,
    mouseFunc: null,
	params: null,
    initTime: null,
    lastX: 0,
    lastY: 0,
    buttons: 0,
    modifiers: 0,
    initWindowWidth: 640,
    initWindowHeight: 480,

/*******************************************************************************
 * DOM EVENT CALLBACKS
 ******************************************************************************/

/*
    // Set when going fullscreen
    windowX: 0,
    windowY: 0,
    windowWidth: 0,
    windowHeight: 0,
*/
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

	onKeyChanged: function(event, status){
      if (GLFW.charFunc || GLFW.keyboardFunc) {
        var key = GLFW.getSpecialKey(event['keyCode']);
        if (key !== null) {
          if( GLFW.keyboardFunc ) {
            event.preventDefault();
            GLFW.saveModifiers(event);
            Runtime.dynCall('vii', GLFW.keyboardFunc, [key, status]);
          }
        }
 
        key = GLFW.getASCIIKey(event);
        if( key !== null && GLFW.charFunc ) {
          event.preventDefault();
          GLFW.saveModifiers(event);
          Runtime.dynCall('vii', GLFW.charFunc, [key, status]);
        }
      }
	},

    onKeydown: function(event) {
		GLFW.onKeyChanged(event, 1);//GLFW_PRESS
    },

    onKeyup: function(event) {
		GLFW.onKeyChanged(event, 0);//GLFW_RELEASE
    },

	onMouseButtonChanged: function(event, status){
	  if(GLFW.mouseButtonFunc == null)
		return;

      GLFW.savePosition(event);
      if(event.target == Module["canvas"] || status == 0){//GLFW_RELEASE
		if(status == 1){//GLFW_PRESS
          try {
            event.target.setCapture();
          } catch (e) {}
	    }
        event.preventDefault();
        GLFW.saveModifiers(event);
		//DOM and glfw have the same button codes
        Runtime.dynCall('vii', GLFW.mouseFunc, [event['button'], status]);
	  }	
	},

    onMouseButtonDown: function(event){
      GLFW.buttons |= (1 << event['button']);
	  GLFW.onMouseButtonChanged(event, 1);//GLFW_PRESS
	},

	onMouseButtonUp: function(event){
	  GLFW.buttons &= ~(1 << event['button']);
	  GLFW.onMouseButtonChanged(event, 0);//GLFW_RELEASE    
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
      if (GLFW.resizeFunc) {
        // console.log("GLFW.resizeFunc (from FS): " + width + ", " + height);
        Runtime.dynCall('vii', GLFW.resizeFunc, [width, height]);
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

/*******************************************************************************
 * GLFW FUNCTIONS
 ******************************************************************************/

	/* GLFW initialization, termination and version querying */
	glfwInit : function() { 
		GLFW.initTime = Date.now() / 1000;

		window.addEventListener("keydown", GLFW.onKeydown, true);
		window.addEventListener("keyup", GLFW.onKeyup, true);
		window.addEventListener("mousemove", GLFW.onMousemove, true);
		window.addEventListener("mousedown", GLFW.onMouseButtonDown, true);
		window.addEventListener("mouseup", GLFW.onMouseButtonUp, true);

		__ATEXIT__.push({ func: function() {
		  window.removeEventListener("keydown", GLFW.onKeydown, true);
		  window.removeEventListener("keyup", GLFW.onKeyup, true);
		  window.removeEventListener("mousemove", GLFW.onMousemove, true);
		  window.removeEventListener("mousedown", GLFW.onMouseButtonDown, true);
		  window.removeEventListener("mouseup", GLFW.onMouseButtonUp, true);
		  Module["canvas"].width = Module["canvas"].height = 1;
		} });

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

		return 1; //GL_TRUE
	},

	glfwTerminate : function() {},

	glfwGetVersion : function( major, minor, rev ) {	
		setValue(major, 2, 'i32');
		setValue(minor, 7, 'i32');
		setValue(rev, 7, 'i32');
	},

	/* Window handling */
	glfwOpenWindow__deps: ['$Browser'],
	glfwOpenWindow : function( width, height, redbits, greenbits, bluebits, alphabits, depthbits, stencilbits, mode ) { 
		if(width == 0 && height > 0)
			width = 4 * height / 3;
		if(width > 0 && height == 0)
			height = 3 * width / 4;

        GLFW.params[0x00020005] = redbits; //GLFW_RED_BITS
        GLFW.params[0x00020006] = greenbits; //GLFW_GREEN_BITS 
        GLFW.params[0x00020007] = bluebits; //GLFW_BLUE_BITS
        GLFW.params[0x00020008] = alphabits; //GLFW_ALPHA_BITS
        GLFW.params[0x00020009] = depthbits; //GLFW_DEPTH_BITS
        GLFW.params[0x0002000A] = stencilbits; //GLFW_STENCIL_BITS

		if(mode == 0x00010001){//GLFW_WINDOW
			Browser.setCanvasSize( GLFW.initWindowWidth = width,
                           GLFW.initWindowHeight = height );
			GLFW.params[0x00030003] = true; //GLFW_STICKY_MOUSE_BUTTONS
		}
		else if(mode == 0x00010002){//GLFW_FULLSCREEN
			GLFW.params[0x00030003] = false; //GLFW_STICKY_MOUSE_BUTTONS
		}
		else{
			throw "Invalid glfwOpenWindow mode.";
		}
		
		Module.ctx = Browser.createContext(Module['canvas'], true, true);
		return 1; //GL_TRUE
	},
	
	glfwOpenWindowHint : function( target, hint ) {
		GLFW.params[target] = hint;
	},

	glfwCloseWindow__deps: ['$Browser'],
	glfwCloseWindow : function() {
		if (GLFW.closeFunc) {
    		Runtime.dynCall('v', GLUT.closeFunc, []);
    	}
		Module.ctx = Browser.destroyContext(Module['canvas'], true, true);
	},

	glfwSetWindowTitle : function( title ) {
		document.title = Pointer_stringify(title);
	},

	glfwGetWindowSize : function( width, height ) {
		setValue(width, Module['canvas'].width, 'i32');
		setValue(height, Module['canvas'].height, 'i32');
	},

	glfwSetWindowSize : function( width, height ) {
		GLFW.cancelFullScreen();
    	Browser.setCanvasSize(width, height);
    	if (GLFW.resizeFunc) {
    		Runtime.dynCall('vii', GLUT.resizeFunc, [width, height]);
    	}
	},

	glfwSetWindowPos : function( x, y ) { throw "glfwSetWindowPos is not implemented yet."; },
	glfwIconifyWindow : function() { throw "glfwIconifyWindow is not implemented yet."; },
	glfwRestoreWindow : function() { throw "glfwRestoreWindow is not implemented yet."; },

	glfwSwapBuffers : function() {},

	glfwSwapInterval : function( interval ) {},

	glfwGetWindowParam : function( param ) {
		return GLFW.params[param];
	},

	glfwSetWindowSizeCallback : function( cbfun ) {
		GLFW.resizeFunc = cbfun; 
	},

	glfwSetWindowCloseCallback : function( cbfun ) {
		GLFW.closeFunc = cbfun;	
	},

	glfwSetWindowRefreshCallback : function( cbfun ) {
		GLFW.refreshFunc = cbfun;
	},

	/* Video mode functions */
	glfwGetVideoModes : function( list, maxcount ) { throw "glfwGetVideoModes is not implemented yet."; },
	glfwGetDesktopMode : function( mode ) { throw "glfwGetDesktopMode is not implemented yet."; },

	/* Input handling */
	glfwPollEvents : function() { throw "glfwPollEvents is not implemented yet."; },
	glfwWaitEvents : function() { throw "glfwWaitEvents is not implemented yet."; },

	glfwGetKey : function( key ) {
		//TODO, actually something
		return 0;//GLFW_RELEASE	
	},

	glfwGetMouseButton : function( button ) {
		return GLUT.buttons & button;
	},

	glfwGetMousePos : function( xpos, ypos ) { 
		setValue(xpos, GLFW.lastX, 'i32');
		setValue(ypos, GLFW.lastY, 'i32');
	},

	glfwSetMousePos : function( xpos, ypos ) { throw "glfwSetMousePos is not implemented yet."; },
	glfwGetMouseWheel : function() { throw "glfwGetMouseWheel is not implemented yet."; },
	glfwSetMouseWheel : function( pos ) { throw "glfwSetMouseWheel is not implemented yet."; },

	glfwSetKeyCallback : function( cbfun ) {
		GLFW.keyFunc = cbfun;
	},

	glfwSetCharCallback : function( cbfun ) {
		GLFW.charFunc = cbfun; 
	},

	glfwSetMouseButtonCallback : function( cbfun ) {
		GLFW.mouseButtonFunc = cbfun;
	},

	glfwSetMousePosCallback : function( cbfun ) {
		GLFW.mousePosFunc = cbfun;
	},

	glfwSetMouseWheelCallback : function( cbfun ) {
		GLFW.mouseWheelFunc = cbfun;
	},

	/* Joystick input */
	glfwGetJoystickParam : function( joy, param ) { throw "glfwGetJoystickParam is not implemented yet."; },
	glfwGetJoystickPos : function( joy, pos, numaxes ) { throw "glfwGetJoystickPos is not implemented yet."; },
	glfwGetJoystickButtons : function( joy, buttons, numbuttons ) { throw "glfwGetJoystickButtons is not implemented yet."; },

	/* Time */
	glfwGetTime : function() {
		return (Date.now()/1000) - GLFW.initTime;
	},

	glfwSetTime : function( time ) {
		GLFW.initTime = Date.now()/1000 + time;
	},

	glfwSleep : function( time ) { throw "glfwSleep is not implemented yet."; },

	/* Extension support */
	glfwExtensionSupported : function( extension ) { throw "glfwExtensionSupported is not implemented yet."; },
	glfwGetProcAddress : function( procname ) {	throw "glfwGetProcAddress is not implemented yet."; },
	glfwGetGLVersion : function( major, minor, rev ) { throw "glfwGetGLVersion is not implemented yet."; },

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
	glfwEnable : function( token ) {
		GLFW.params[token] = false;
	},

	glfwDisable : function( token ) {
		GLFW.params[token] = true;
	},

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

