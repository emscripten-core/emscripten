var LibraryJSEvents = {
  $JSEvents: {
    // pointers to structs malloc()ed to Emscripten HEAP for JS->C interop.
    keyEvent: 0,
    mouseEvent: 0,
    wheelEvent: 0,
    uiEvent: 0,
    focusEvent: 0,
    deviceOrientationEvent: 0,
    deviceMotionEvent: 0,
    fullscreenChangeEvent: 0,
    pointerlockChangeEvent: 0,
    visibilityChangeEvent: 0,
    touchEvent: 0,

    // When the C runtime exits via exit(), we unregister all event handlers added by this library to be nice and clean.
    // Track in this field whether we have yet registered that __ATEXIT__ handler.
    removeEventListenersRegistered: false, 

    registerRemoveEventListeners: function() {
      if (!JSEvents.removeEventListenersRegistered) {
      __ATEXIT__.push({ func: function() {
          for(var i = JSEvents.eventHandlers.length-1; i >= 0; --i) {
            JSEvents._removeHandler(i);
          }
         } });
        JSEvents.removeEventListenersRegistered = true;
      }
    },

    findEventTarget: function(target) {
      if (target) {
        if (typeof target == "number") {
          target = Pointer_stringify(target);
        }
        if (target == '#window') return window;
        else if (target == '#document') return document;
        else if (target == '#screen') return window.screen;
        else if (target == '#canvas') return Module['canvas'];

        if (typeof target == 'string') return document.getElementById(target);
        else return target;
      } else {
        // The sensible target varies between events, but use window as the default
        // since DOM events mostly can default to that. Specific callback registrations
        // override their own defaults.
        return window;
      }
    },

    deferredCalls: [],

    // Queues the given function call to occur the next time we enter an event handler.
    // Existing implementations of pointerlock apis have required that 
    // the target element is active in fullscreen mode first. Thefefore give
    // fullscreen mode request a precedence of 1 and pointer lock a precedence of 2
    // and sort by that to always request fullscreen before pointer lock.
    deferCall: function(targetFunction, precedence, argsList) {
      function arraysHaveEqualContent(arrA, arrB) {
        if (arrA.length != arrB.length) return false;

        for(var i in arrA) {
          if (arrA[i] != arrB[i]) return false;
        }
        return true;
      }
      // Test if the given call was already queued, and if so, don't add it again.
      for(var i in JSEvents.deferredCalls) {
        var call = JSEvents.deferredCalls[i];
        if (call.targetFunction == targetFunction && arraysHaveEqualContent(call.argsList, argsList)) {
          return;
        }
      }
      JSEvents.deferredCalls.push({
        targetFunction: targetFunction,
        precedence: precedence,
        argsList: argsList
      });

      JSEvents.deferredCalls.sort(function(x,y) { return x.precedence < y.precedence; });
    },
    
    // Erases all deferred calls to the given target function from the queue list.
    removeDeferredCalls: function(targetFunction) {
      for(var i = 0; i < JSEvents.deferredCalls.length; ++i) {
        if (JSEvents.deferredCalls[i].targetFunction == targetFunction) {
          JSEvents.deferredCalls.splice(i, 1);
          --i;
        }
      }
    },
    
    canPerformEventHandlerRequests: function() {
      return JSEvents.inEventHandler && JSEvents.currentEventHandler.allowsDeferredCalls;
    },
    
    runDeferredCalls: function() {
      if (!JSEvents.canPerformEventHandlerRequests()) {
        return;
      }
      for(var i = 0; i < JSEvents.deferredCalls.length; ++i) {
        var call = JSEvents.deferredCalls[i];
        JSEvents.deferredCalls.splice(i, 1);
        --i;
        call.targetFunction.apply(this, call.argsList);
      }
    },

    // If positive, we are currently executing in a JS event handler.
    inEventHandler: 0,
    // If we are in an event handler, specifies the event handler object from the eventHandlers array that is currently running.
    currentEventHandler: null,

    // Stores objects representing each currently registered JS event handler.
    eventHandlers: [],

    _removeHandler: function(i) {
      JSEvents.eventHandlers[i].target.removeEventListener(JSEvents.eventHandlers[i].eventTypeString, JSEvents.eventHandlers[i].handlerFunc, true);
      JSEvents.eventHandlers.splice(i, 1);
    },
    
    registerOrRemoveHandler: function(eventHandler) {
      var jsEventHandler = function jsEventHandler(event) {
        // Increment nesting count for the event handler.
        ++JSEvents.inEventHandler;
        JSEvents.currentEventHandler = eventHandler;
        // Process any old deferred calls the user has placed.
        JSEvents.runDeferredCalls();
        // Process the actual event, calls back to user C code handler.
        eventHandler.handlerFunc(event);
        // Process any new deferred calls that were placed right now from this event handler.
        JSEvents.runDeferredCalls();
        // Out of event handler - restore nesting count.
        --JSEvents.inEventHandler;
      }
      
      if (eventHandler.callbackfunc) {
        eventHandler.target.addEventListener(eventHandler.eventTypeString, jsEventHandler, eventHandler.useCapture);
        JSEvents.eventHandlers.push(eventHandler);
        JSEvents.registerRemoveEventListeners();
      } else {
        for(var i = 0; i < JSEvents.eventHandlers.length; ++i) {
          if (JSEvents.eventHandlers[i].target == eventHandler.target
           && JSEvents.eventHandlers[i].eventTypeString == eventHandler.eventTypeString) {
             JSEvents._removeHandler(i--);
           }
        }
      }
    },

    registerKeyEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.keyEvent) {
        JSEvents.keyEvent = _malloc( {{{ C_STRUCTS.EmscriptenKeyboardEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;
        writeStringToMemory(e.key ? e.key : "", JSEvents.keyEvent + {{{ C_STRUCTS.EmscriptenKeyboardEvent.key }}} );
        writeStringToMemory(e.code ? e.code : "", JSEvents.keyEvent + {{{ C_STRUCTS.EmscriptenKeyboardEvent.code }}} );
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.location, 'e.location', 'i32') }}}
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.ctrlKey, 'e.ctrlKey', 'i32') }}}
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.shiftKey, 'e.shiftKey', 'i32') }}}
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.altKey, 'e.altKey', 'i32') }}}
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.metaKey, 'e.metaKey', 'i32') }}}
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.repeat, 'e.repeat', 'i32') }}}
        writeStringToMemory(e.locale ? e.locale : "", JSEvents.keyEvent + {{{ C_STRUCTS.EmscriptenKeyboardEvent.locale }}} );
        writeStringToMemory(e.char ? e.char : "", JSEvents.keyEvent + {{{ C_STRUCTS.EmscriptenKeyboardEvent.charValue }}} );
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.charCode, 'e.charCode', 'i32') }}}
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.keyCode, 'e.keyCode', 'i32') }}}
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.which, 'e.which', 'i32') }}}
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.keyEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var isInternetExplorer = (navigator.userAgent.indexOf('MSIE') !== -1 || navigator.appVersion.indexOf('Trident/') > 0);

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: isInternetExplorer ? false : true, // MSIE doesn't allow fullscreen and pointerlock requests from key handlers, others do.
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    fillMouseEventData: function(eventStruct, e) {
      var rect = Module['canvas'].getBoundingClientRect();
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.timestamp, 'JSEvents.tick()', 'double') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.screenX, 'e.screenX', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.screenY, 'e.screenY', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.clientX, 'e.clientX', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.clientY, 'e.clientY', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.ctrlKey, 'e.ctrlKey', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.shiftKey, 'e.shiftKey', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.altKey, 'e.altKey', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.metaKey, 'e.metaKey', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.button, 'e.button', 'i16') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.buttons, 'e.buttons', 'i16') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.movementX, 'e.movementX', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.movementY, 'e.movementY', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.canvasX, 'e.clientX - rect.left', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.canvasY, 'e.clientY - rect.top', 'i32') }}}
    },
    
    registerMouseEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.mouseEvent) {
        JSEvents.mouseEvent = _malloc( {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;
        JSEvents.fillMouseEventData(JSEvents.mouseEvent, e);
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.mouseEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: eventTypeString != 'mousemove', // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerWheelEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.wheelEvent) {
        JSEvents.wheelEvent = _malloc( {{{ C_STRUCTS.EmscriptenWheelEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;
        JSEvents.fillMouseEventData(JSEvents.wheelEvent, e);
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaX, 'e.deltaX', 'double') }}}
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaY, 'e.deltaY', 'double') }}}
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaZ, 'e.deltaZ', 'double') }}}
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaMode, 'e.deltaMode', 'i32') }}}
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.wheelEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: true,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    pageScrollPos: function() {
      if (window.pageXOffset > 0 || window.pageYOffset > 0) {
        return [window.pageXOffset, window.pageYOffset];
      }
      if (typeof document.documentElement.scrollLeft !== 'undefined' || typeof document.documentElement.scrollTop !== 'undefined') {
        return [document.documentElement.scrollLeft, document.documentElement.scrollTop];
      }
      return [document.body.scrollLeft|0, document.body.scrollTop|0];
    },

    registerUiEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.uiEvent) {
        JSEvents.uiEvent = _malloc( {{{ C_STRUCTS.EmscriptenUiEvent.__size__ }}} );
      }

      if (eventTypeString == "scroll" && !target) {
        target = document; // By default read scroll events on document rather than window.
      } else {
        target = JSEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;
        if (e.target != target) {
          // Never take ui events such as scroll via a 'bubbled' route, but always from the direct element that
          // was targeted. Otherwise e.g. if app logs a message in response to a page scroll, the Emscripten log
          // message box could cause to scroll, generating a new (bubbled) scroll message, causing a new log print,
          // causing a new scroll, etc..
          return;
        }
        var scrollPos = JSEvents.pageScrollPos();
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.detail, 'e.detail', 'i32') }}}
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientWidth, 'document.body.clientWidth', 'i32') }}}
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientHeight, 'document.body.clientHeight', 'i32') }}}
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerWidth, 'window.innerWidth', 'i32') }}}
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerHeight, 'window.innerHeight', 'i32') }}}
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterWidth, 'window.outerWidth', 'i32') }}}
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterHeight, 'window.outerHeight', 'i32') }}}
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollTop, 'scrollPos[0]', 'i32') }}}
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollLeft, 'scrollPos[1]', 'i32') }}}
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.uiEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false, // Neither scroll or resize events allow running requests inside them.
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    getNodeNameForTarget: function(target) {
      if (!target) return '';
      if (target == window) return '#window';
      if (target == window.screen) return '#screen';
      return (target && target.nodeName) ? target.nodeName : '';
    },

    registerFocusEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.focusEvent) {
        JSEvents.focusEvent = _malloc( {{{ C_STRUCTS.EmscriptenFocusEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;

        var nodeName = JSEvents.getNodeNameForTarget(e.target);
        var id = e.target.id ? e.target.id : '';
        writeStringToMemory(nodeName, JSEvents.focusEvent + {{{ C_STRUCTS.EmscriptenFocusEvent.nodeName }}} );
        writeStringToMemory(id, JSEvents.focusEvent + {{{ C_STRUCTS.EmscriptenFocusEvent.id }}} );
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.focusEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    tick: function() {
      if (window['performance'] && window['performance']['now']) return window['performance']['now']();
      else return Date.now();
    },

    registerDeviceOrientationEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.deviceOrientationEvent) {
        JSEvents.deviceOrientationEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceOrientationEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;

        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.timestamp, 'JSEvents.tick()', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.alpha, 'e.alpha', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.beta, 'e.beta', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.gamma, 'e.gamma', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.absolute, 'e.absolute', 'i32') }}}

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.deviceOrientationEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerDeviceMotionEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.deviceMotionEvent) {
        JSEvents.deviceMotionEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;

        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.timestamp, 'JSEvents.tick()', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationX, 'e.acceleration.x', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationY, 'e.acceleration.y', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationZ, 'e.acceleration.z', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityX, 'e.accelerationIncludingGravity.x', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityY, 'e.accelerationIncludingGravity.y', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityZ, 'e.accelerationIncludingGravity.z', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateAlpha, 'e.rotationRate.alpha', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateBeta, 'e.rotationRate.beta', 'double') }}}
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateGamma, 'e.rotationRate.gamma', 'double') }}}

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.deviceMotionEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    screenOrientation: function() {
      if (!window.screen) return undefined;
      return window.screen.orientation || window.screen.mozOrientation || window.screen.webkitOrientation || window.screen.msOrientation;
    },

    fillOrientationChangeEventData: function(eventStruct, e) {
      var orientations  = ["portrait-primary", "portrait-secondary", "landscape-primary", "landscape-secondary"];
      var orientations2 = ["portrait",         "portrait",           "landscape",         "landscape"];

      var orientationString = JSEvents.screenOrientation();
      var orientation = orientations.indexOf(orientationString);
      if (orientation == -1) {
        orientation = orientations2.indexOf(orientationString);
      }

      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenOrientationChangeEvent.orientationIndex, '1 << orientation', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenOrientationChangeEvent.orientationAngle, 'window.orientation', 'i32') }}}
    },

    registerOrientationChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.orientationChangeEvent) {
        JSEvents.orientationChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenOrientationChangeEvent.__size__ }}} );
      }

      if (!target) {
        target = window.screen; // Orientation events need to be captured from 'window.screen' instead of 'window'
      } else {
        target = JSEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JSEvents.fillOrientationChangeEventData(JSEvents.orientationChangeEvent, e);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.orientationChangeEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      if (eventTypeString == "orientationchange" && window.screen.mozOrientation !== undefined) {
        eventTypeString = "mozorientationchange";
      }

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    fullscreenEnabled: function() {
      return document.fullscreenEnabled || document.mozFullscreenEnabled || document.mozFullScreenEnabled || document.webkitFullscreenEnabled || document.msFullscreenEnabled;
    },
    
    fillFullscreenChangeEventData: function(eventStruct, e) {
      var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
      var isFullscreen = !!fullscreenElement;
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.isFullscreen, 'isFullscreen', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.fullscreenEnabled, 'JSEvents.fullscreenEnabled()', 'i32') }}}
      var nodeName = JSEvents.getNodeNameForTarget(fullscreenElement);
      var id = (fullscreenElement && fullscreenElement.id) ? fullscreenElement.id : '';
      writeStringToMemory(nodeName, eventStruct + {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.nodeName }}} );
      writeStringToMemory(id, eventStruct + {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.id }}} );
    },

    registerFullscreenChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.fullscreenChangeEvent) {
        JSEvents.fullscreenChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.__size__ }}} );
      }

      if (!target) {
        target = document; // Fullscreen change events need to be captured from 'document' by default instead of 'window'
      } else {
        target = JSEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JSEvents.fillFullscreenChangeEventData(JSEvents.fullscreenChangeEvent, e);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.fullscreenChangeEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    requestFullscreen: function(target) {
      if (target.requestFullscreen) {
        target.requestFullscreen();
      } else if (target.msRequestFullscreen) {
        target.msRequestFullscreen();
      } else if (target.mozRequestFullScreen) {
        target.mozRequestFullScreen();
      } else if (target.mozRequestFullscreen) {
        target.mozRequestFullscreen();
      } else if (target.webkitRequestFullscreen) {
        target.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
      } else {
        if (typeof JSEvents.fullscreenEnabled() === 'undefined') {
          return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
        } else {
          return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
        }
      }
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    },

    fillPointerlockChangeEventData: function(eventStruct, e) {
      var pointerLockElement = document.pointerLockElement || document.mozPointerLockElement || document.webkitPointerLockElement || document.msPointerLockElement;
      var isPointerlocked = !!pointerLockElement;
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenPointerlockChangeEvent.isActive, 'isPointerlocked', 'i32') }}}
      var nodeName = JSEvents.getNodeNameForTarget(pointerLockElement);
      var id = (pointerLockElement && pointerLockElement.id) ? pointerLockElement.id : '';
      writeStringToMemory(nodeName, eventStruct + {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.nodeName }}} );
      writeStringToMemory(id, eventStruct + {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.id }}});
    },

    registerPointerlockChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.pointerlockChangeEvent) {
        JSEvents.pointerlockChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.__size__ }}} );
      }

      if (!target) {
        target = document; // Pointer lock change events need to be captured from 'document' by default instead of 'window'
      } else {
        target = JSEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JSEvents.fillPointerlockChangeEventData(JSEvents.pointerlockChangeEvent, e);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.pointerlockChangeEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    requestPointerLock: function(target) {
      if (target.requestPointerLock) {
        target.requestPointerLock();
      } else if (target.mozRequestPointerLock) {
        target.mozRequestPointerLock();
      } else if (target.webkitRequestPointerLock) {
        target.webkitRequestPointerLock();
      } else if (target.msRequestPointerLock) {
        target.msRequestPointerLock();
      } else {
        // document.body is known to accept pointer lock, so use that to differentiate if the user passed a bad element,
        // or if the whole browser just doesn't support the feature.
        if (document.body.requestPointerLock || document.body.mozRequestPointerLock || document.body.webkitRequestPointerLock || document.body.msRequestPointerLock) {
          return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
        } else {
          return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
        }
      }
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    },

    fillVisibilityChangeEventData: function(eventStruct, e) {
      var visibilityStates = [ "hidden", "visible", "prerender", "unloaded" ];
      var visibilityState = visibilityStates.indexOf(document.visibilityState);

      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenVisibilityChangeEvent.hidden, 'document.hidden', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenVisibilityChangeEvent.visibilityState, 'visibilityState', 'i32') }}}
    },

    registerVisibilityChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.visibilityChangeEvent) {
        JSEvents.visibilityChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenVisibilityChangeEvent.__size__ }}} );
      }

      if (!target) {
        target = document; // Visibility change events need to be captured from 'document' by default instead of 'window'
      } else {
        target = JSEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JSEvents.fillVisibilityChangeEventData(JSEvents.visibilityChangeEvent, e);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.visibilityChangeEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerTouchEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.touchEvent) {
        JSEvents.touchEvent = _malloc( {{{ C_STRUCTS.EmscriptenTouchEvent.__size__ }}} );
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        var touches = {};
        for(var i = 0; i < e.touches.length; ++i) {
          var touch = e.touches[i];
          touches[touch.identifier] = touch;
        }
        for(var i = 0; i < e.changedTouches.length; ++i) {
          var touch = e.changedTouches[i];
          touches[touch.identifier] = touch;
          touch.changed = true;
        }
        for(var i = 0; i < e.targetTouches.length; ++i) {
          var touch = e.targetTouches[i];
          touches[touch.identifier].onTarget = true;
        }
        
        var ptr = JSEvents.touchEvent;
        {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchEvent.ctrlKey, 'e.ctrlKey', 'i32') }}}
        {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchEvent.shiftKey, 'e.shiftKey', 'i32') }}}
        {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchEvent.altKey, 'e.altKey', 'i32') }}}
        {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchEvent.metaKey, 'e.metaKey', 'i32') }}}
        ptr += {{{ C_STRUCTS.EmscriptenTouchEvent.touches }}}; // Advance to the start of the touch array.
        var rect = Module['canvas'].getBoundingClientRect();
        var numTouches = 0;
        for(var i in touches) {
          var t = touches[i];
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.identifier, 't.identifier', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.screenX, 't.screenX', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.screenY, 't.screenY', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.clientX, 't.clientX', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.clientY, 't.clientY', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.pageX, 't.pageX', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.pageY, 't.pageY', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.isChanged, 't.changed', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.onTarget, 't.onTarget', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.canvasX, 't.clientX - rect.left', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.canvasY, 't.clientY - rect.top', 'i32') }}}
          ptr += {{{ C_STRUCTS.EmscriptenTouchPoint.__size__ }}};

          if (++numTouches >= 32) {
            break;
          }
        }
        {{{ makeSetValue('JSEvents.touchEvent', C_STRUCTS.EmscriptenTouchEvent.numTouches, 'numTouches', 'i32') }}}

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.touchEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false, // XXX Currently disabled, see bug https://bugzilla.mozilla.org/show_bug.cgi?id=966493
        // Once the above bug is resolved, enable the following condition if possible:
        // allowsDeferredCalls: eventTypeString == 'touchstart',
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    fillGamepadEventData: function(eventStruct, e) {
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.timestamp, 'e.timestamp', 'double') }}}
      for(var i = 0; i < e.axes.length; ++i) {
        {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.EmscriptenGamepadEvent.axis, 'e.axes[i]', 'double') }}}
      }
      for(var i = 0; i < e.buttons.length; ++i) {
        {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.EmscriptenGamepadEvent.analogButton, 'e.buttons[i].value', 'double') }}}
      }
      for(var i = 0; i < e.buttons.length; ++i) {
        {{{ makeSetValue('eventStruct+i*4', C_STRUCTS.EmscriptenGamepadEvent.digitalButton, 'e.buttons[i].pressed', 'i32') }}}
      }
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.connected, 'e.connected', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.index, 'e.index', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.numAxes, 'e.axes.length', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.numButtons, 'e.buttons.length', 'i32') }}}
      writeStringToMemory(e.id, eventStruct + {{{ C_STRUCTS.EmscriptenGamepadEvent.id }}} );
      writeStringToMemory(e.mapping, eventStruct + {{{ C_STRUCTS.EmscriptenGamepadEvent.mapping }}} );
    },

    registerGamepadEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.gamepadEvent) {
        JSEvents.gamepadEvent = _malloc( {{{ C_STRUCTS.EmscriptenGamepadEvent.__size__ }}} );
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JSEvents.fillGamepadEventData(JSEvents.gamepadEvent, e.gamepad);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.gamepadEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: true,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerBeforeUnloadEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      var handlerFunc = function(event) {
        var e = event || window.event;

        var confirmationMessage = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, 0, userData]);
        
        confirmationMessage = Pointer_stringify(confirmationMessage);
        if (confirmationMessage) {
          e.preventDefault();
          e.returnValue = confirmationMessage;
          return confirmationMessage;
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    battery: function() { return navigator.battery || navigator.mozBattery || navigator.webkitBattery; },

    fillBatteryEventData: function(eventStruct, e) {
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.chargingTime, 'e.chargingTime', 'double') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.dischargingTime, 'e.dischargingTime', 'double') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.level, 'e.level', 'double') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.charging, 'e.charging', 'i32') }}}
    },
    
    registerBatteryEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.batteryEvent) {
        JSEvents.batteryEvent = _malloc( {{{ C_STRUCTS.EmscriptenBatteryEvent.__size__ }}} );
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JSEvents.fillBatteryEventData(JSEvents.batteryEvent, JSEvents.battery());

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.batteryEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerWebGlEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!target) {
        target = Module['canvas'];
      }
      var handlerFunc = function(event) {
        var e = event || window.event;

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, 0, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },
  },

  emscripten_set_keypress_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYPRESS') }}}, "keypress");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_keydown_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYDOWN') }}}, "keydown");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_keyup_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYUP') }}}, "keyup");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_click_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_CLICK') }}}, "click");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mousedown_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEDOWN') }}}, "mousedown");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseup_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEUP') }}}, "mouseup");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_dblclick_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DBLCLICK') }}}, "dblclick");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mousemove_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEMOVE') }}}, "mousemove");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_mouse_status: function(mouseState) {
    if (!JSEvents.mouseEvent) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    // HTML5 does not really have a polling API for mouse events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler to any of the mouse function.
    HEAP32.set(HEAP32.subarray(JSEvents.mouseEvent, {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}}), mouseState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_wheel_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerWheelEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WHEEL') }}}, "wheel");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_resize_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_RESIZE') }}}, "resize");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_scroll_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_SCROLL') }}}, "scroll");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_blur_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BLUR') }}}, "blur");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focus_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUS') }}}, "focus");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focusin_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUSIN') }}}, "focusin");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focusout_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUSOUT') }}}, "focusout");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_deviceorientation_callback: function(userData, useCapture, callbackfunc) {
    JSEvents.registerDeviceOrientationEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DEVICEORIENTATION') }}}, "deviceorientation");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_deviceorientation_status: function(orientationState) {
    if (!JSEvents.deviceOrientationEvent) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    // HTML5 does not really have a polling API for device orientation events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler.
    HEAP32.set(HEAP32.subarray(JSEvents.deviceOrientationEvent, {{{ C_STRUCTS.EmscriptenDeviceOrientationEvent.__size__ }}}), orientationState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_devicemotion_callback: function(userData, useCapture, callbackfunc) {
    JSEvents.registerDeviceMotionEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DEVICEMOTION') }}}, "devicemotion");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_devicemotion_status: function(motionState) {
    if (!JSEvents.deviceMotionEvent) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    // HTML5 does not really have a polling API for device motion events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler.
    HEAP32.set(HEAP32.subarray(JSEvents.deviceMotionEvent, {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}}), motionState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_orientationchange_callback: function(userData, useCapture, callbackfunc) {
    if (!window.screen || !window.screen.addEventListener) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.registerOrientationChangeEventCallback(window.screen, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_ORIENTATIONCHANGE') }}}, "orientationchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_get_orientation_status: function(orientationChangeEvent) {
    if (!JSEvents.screenOrientation() && typeof window.orientation === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.fillOrientationChangeEventData(orientationChangeEvent);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_lock_orientation: function(allowedOrientations) {
    var orientations = [];
    if (allowedOrientations & 1) orientations.push("portrait-primary");
    if (allowedOrientations & 2) orientations.push("portrait-secondary");
    if (allowedOrientations & 4) orientations.push("landscape-primary");
    if (allowedOrientations & 8) orientations.push("landscape-secondary");
    var succeeded;
    if (window.screen.lockOrientation) {
      succeeded = window.screen.lockOrientation(orientations);
    } else if (window.screen.mozLockOrientation) {
      succeeded = window.screen.mozLockOrientation(orientations);
    } else if (window.screen.webkitLockOrientation) {
      succeeded = window.screen.webkitLockOrientation(orientations);
    } else if (window.screen.msLockOrientation) {
      succeeded = window.screen.msLockOrientation(orientations);
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    if (succeeded) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_FAILED') }}};
    }
  },
  
  emscripten_unlock_orientation: function() {
    if (window.screen.unlockOrientation) {
      window.screen.unlockOrientation();
    } else if (window.screen.mozUnlockOrientation) {
      window.screen.mozUnlockOrientation();
    } else if (window.screen.webkitUnlockOrientation) {
      window.screen.webkitUnlockOrientation();
    } else if (window.screen.msUnlockOrientation) {
      window.screen.msUnlockOrientation();
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_fullscreenchange_callback: function(target, userData, useCapture, callbackfunc) {
    if (typeof JSEvents.fullscreenEnabled() === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    if (!target) target = document;
    else {
      target = JSEvents.findEventTarget(target);
      if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    }
    JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "fullscreenchange");
    JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "mozfullscreenchange");
    JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "webkitfullscreenchange");
    JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "msfullscreenchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_fullscreen_status: function(fullscreenStatus) {
    if (typeof JSEvents.fullscreenEnabled() === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.fillFullscreenChangeEventData(fullscreenStatus);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  // https://developer.mozilla.org/en-US/docs/Web/Guide/API/DOM/Using_full_screen_mode
  emscripten_request_fullscreen: function(target, deferUntilInEventHandler) {
    if (typeof JSEvents.fullscreenEnabled() === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    if (!JSEvents.fullscreenEnabled()) return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    if (!target) target = '#canvas';
    target = JSEvents.findEventTarget(target);
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    if (!target.requestFullscreen && !target.msRequestFullscreen && !target.mozRequestFullScreen && !target.mozRequestFullscreen && !target.webkitRequestFullscreen) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

    var canPerformRequests = JSEvents.canPerformEventHandlerRequests();

    // Queue this function call if we're not currently in an event handler and the user saw it appropriate to do so.
    if (!canPerformRequests) {
      if (deferUntilInEventHandler) {
        JSEvents.deferCall(JSEvents.requestFullscreen, 1 /* priority over pointer lock */, [target]);
        return {{{ cDefine('EMSCRIPTEN_RESULT_DEFERRED') }}};
      } else {
        return {{{ cDefine('EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED') }}};
      }
    }

    return JSEvents.requestFullscreen(target);
  },
  
  emscripten_exit_fullscreen: function() {
    if (typeof JSEvents.fullscreenEnabled() === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    // Make sure no queued up calls will fire after this.
    JSEvents.removeDeferredCalls(JSEvents.requestFullscreen);

    if (document.exitFullscreen) {
      document.exitFullscreen();
    } else if (document.msExitFullscreen) {
      document.msExitFullscreen();
    } else if (document.mozCancelFullScreen) {
      document.mozCancelFullScreen();
    } else if (document.webkitExitFullscreen) {
      document.webkitExitFullscreen();
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_pointerlockchange_callback: function(target, userData, useCapture, callbackfunc) {
    if (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    if (!target) target = document;
    else {
      target = JSEvents.findEventTarget(target);
      if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    }
    JSEvents.registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "pointerlockchange");
    JSEvents.registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "mozpointerlockchange");
    JSEvents.registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "webkitpointerlockchange");
    JSEvents.registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "mspointerlockchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_pointerlock_status: function(pointerlockStatus) {
    if (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    JSEvents.fillPointerlockChangeEventData(pointerlockStatus);  
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_request_pointerlock: function(target, deferUntilInEventHandler) {
    if (!target) target = '#canvas';
    target = JSEvents.findEventTarget(target);
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    if (!target.requestPointerLock && !target.mozRequestPointerLock && !target.webkitRequestPointerLock && !target.msRequestPointerLock) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }

    var canPerformRequests = JSEvents.canPerformEventHandlerRequests();

    // Queue this function call if we're not currently in an event handler and the user saw it appropriate to do so.
    if (!canPerformRequests) {
      if (deferUntilInEventHandler) {
        JSEvents.deferCall(JSEvents.requestPointerLock, 2 /* priority below fullscreen */, [target]);
        return {{{ cDefine('EMSCRIPTEN_RESULT_DEFERRED') }}};
      } else {
        return {{{ cDefine('EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED') }}};
      }
    }

    return JSEvents.requestPointerLock(target);
  },

  emscripten_exit_pointerlock: function() {
    // Make sure no queued up calls will fire after this.
    JSEvents.removeDeferredCalls(JSEvents.requestPointerLock);

    if (document.exitPointerLock) {
      document.exitPointerLock();
    } else if (document.msExitPointerLock) {
      document.msExitPointerLock();
    } else if (document.mozExitPointerLock) {
      document.mozExitPointerLock();
    } else if (document.webkitExitPointerLock) {
      document.webkitExitPointerLock();
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_vibrate: function(msecs) {
    if (!navigator.vibrate) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};    
    navigator.vibrate(msecs);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_vibrate_pattern: function(msecsArray, numEntries) {
    if (!navigator.vibrate) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};

    var vibrateList = [];
    for(var i = 0; i < numEntries; ++i) {
      var msecs = {{{ makeGetValue('msecsArray', 'i*4', 'i32') }}}
      vibrateList.push(msecs);
    }
    navigator.vibrate(vibrateList);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_visibilitychange_callback: function(userData, useCapture, callbackfunc) {
    JSEvents.registerVisibilityChangeEventCallback(document, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_VISIBILITYCHANGE') }}}, "visibilitychange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_visibility_status: function(visibilityStatus) {
    if (typeof document.visibilityState === 'undefined' && typeof document.hidden === 'undefined') {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    JSEvents.fillVisibilityChangeEventData(visibilityStatus);  
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchstart_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHSTART') }}}, "touchstart");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchend_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHEND') }}}, "touchend");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchmove_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHMOVE') }}}, "touchmove");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchcancel_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHCANCEL') }}}, "touchcancel");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_gamepadconnected_callback: function(userData, useCapture, callbackfunc) {
    if (!navigator.getGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADCONNECTED') }}}, "gamepadconnected");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_gamepaddisconnected_callback: function(userData, useCapture, callbackfunc) {
    if (!navigator.getGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED') }}}, "gamepaddisconnected");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
 },
  
  emscripten_get_num_gamepads: function() {
    if (!navigator.getGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    return navigator.getGamepads().length;
  },
  
  emscripten_get_gamepad_status: function(index, gamepadState) {
    if (!navigator.getGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    var gamepads = navigator.getGamepads();
    if (index < 0 || index >= gamepads.length) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
    }
    JSEvents.fillGamepadEventData(gamepadState, gamepads[index]);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_beforeunload_callback: function(userData, callbackfunc) {
    if (typeof window.onbeforeunload === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.registerBeforeUnloadEventCallback(window, userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BEFOREUNLOAD') }}}, "beforeunload"); 
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_batterychargingchange_callback: function(userData, callbackfunc) {
    if (!JSEvents.battery()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}}; 
    JSEvents.registerBatteryEventCallback(JSEvents.battery(), userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BATTERYCHARGINGCHANGE') }}}, "chargingchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_batterylevelchange_callback: function(userData, callbackfunc) {
    if (!JSEvents.battery()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}}; 
    JSEvents.registerBatteryEventCallback(JSEvents.battery(), userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BATTERYLEVELCHANGE') }}}, "levelchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_get_battery_status: function(batteryState) {
    if (!JSEvents.battery()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}}; 
    JSEvents.fillBatteryEventData(batteryState, JSEvents.battery());
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_webglcontextlost_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST') }}}, "webglcontextlost");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_webglcontextrestored_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED') }}}, "webglcontextrestored");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
};

autoAddDeps(LibraryJSEvents, '$JSEvents');
mergeInto(LibraryManager.library, LibraryJSEvents);
