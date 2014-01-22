var LibraryJsEvents = {
  $JsEvents: {
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
      if (!JsEvents.removeEventListenersRegistered) {
      __ATEXIT__.push({ func: function() {
          for(var i = JsEvents.eventHandlers.length-1; i >= 0; --i) {
            JsEvents._removeHandler(i);
          }
         } });
        JsEvents.removeEventListenersRegistered = true;
      }
    },

    findEventTarget: function(target) {
      if (target) {
        if (typeof target == "number") {
          target = Pointer_stringify(target);
        }
        if (target == '#window') {
          return window;
        } else if (target == '#document') {
          return document;
        } else if (target == '#screen') {
          return window.screen;
        } else if (target == '#canvas') {
          return Module['canvas'];
        }
        if (typeof target == 'string') {
          return document.getElementById(target);
        } else {
          return target;
        }
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
        if (arrA.length != arrB.length) {
          return false;
        }
        for(var i in arrA) {
          if (arrA[i] != arrB[i]) {
            return false;
          }
        }
        return true;
      }
      // Test if the given call was already queued, and if so, don't add it again.
      for(var i in JsEvents.deferredCalls) {
        var call = JsEvents.deferredCalls[i];
        if (call.targetFunction == targetFunction && arraysHaveEqualContent(call.argsList, argsList)) {
          return;
        }
      }
      JsEvents.deferredCalls.push({
        targetFunction: targetFunction,
        precedence: precedence,
        argsList: argsList
      });

      JsEvents.deferredCalls.sort(function(x,y) { return x.precedence < y.precedence; });
    },
    
    // Erases all deferred calls to the given target function from the queue list.
    removeDeferredCalls: function(targetFunction) {
      for(var i = 0; i < JsEvents.deferredCalls.length; ++i) {
        if (JsEvents.deferredCalls[i].targetFunction == targetFunction) {
          JsEvents.deferredCalls.splice(i, 1);
          --i;
        }
      }
    },
    
    canPerformEventHandlerRequests: function() {
      return JsEvents.inEventHandler && JsEvents.currentEventHandler.allowsDeferredCalls;
    },
    
    runDeferredCalls: function() {
      if (!JsEvents.canPerformEventHandlerRequests()) {
        return;
      }
      for(var i = 0; i < JsEvents.deferredCalls.length; ++i) {
        var call = JsEvents.deferredCalls[i];
        JsEvents.deferredCalls.splice(i, 1);
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
      JsEvents.eventHandlers[i].target.removeEventListener(JsEvents.eventHandlers[i].eventTypeString, JsEvents.eventHandlers[i].handlerFunc, true);
      JsEvents.eventHandlers.splice(i, 1);
    },
    
    registerOrRemoveHandler: function(eventHandler) {
      var jsEventHandler = function jsEventHandler(event) {
        // Increment nesting count for the event handler.
        ++JsEvents.inEventHandler;
        JsEvents.currentEventHandler = eventHandler;
        // Process any old deferred calls the user has placed.
        JsEvents.runDeferredCalls();
        // Process the actual event, calls back to user C code handler.
        eventHandler.handlerFunc(event);
        // Process any new deferred calls that were placed right now from this event handler.
        JsEvents.runDeferredCalls();
        // Out of event handler - restore nesting count.
        --JsEvents.inEventHandler;
      }
      
      if (eventHandler.callbackfunc) {
        eventHandler.target.addEventListener(eventHandler.eventTypeString, jsEventHandler, eventHandler.useCapture);
        JsEvents.eventHandlers.push(eventHandler);
        JsEvents.registerRemoveEventListeners();
      } else {
        for(var i = 0; i < JsEvents.eventHandlers.length; ++i) {
          if (JsEvents.eventHandlers[i].target == eventHandler.target
           && JsEvents.eventHandlers[i].eventTypeString == eventHandler.eventTypeString) {
             JsEvents._removeHandler(i--);
           }
        }
      }
    },

    registerKeyEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.keyEvent) {
        JsEvents.keyEvent = _malloc( {{{ C_STRUCTS.emscripten_KeyboardEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;
        writeStringToMemory(e.key ? e.key : "", JsEvents.keyEvent + {{{ C_STRUCTS.emscripten_KeyboardEvent.key }}} );
        writeStringToMemory(e.code ? e.code : "", JsEvents.keyEvent + {{{ C_STRUCTS.emscripten_KeyboardEvent.code }}} );
        {{{ makeSetValue('JsEvents.keyEvent', C_STRUCTS.emscripten_KeyboardEvent.location, 'e.location', 'i32') }}}
        {{{ makeSetValue('JsEvents.keyEvent', C_STRUCTS.emscripten_KeyboardEvent.ctrlKey, 'e.ctrlKey', 'i32') }}}
        {{{ makeSetValue('JsEvents.keyEvent', C_STRUCTS.emscripten_KeyboardEvent.shiftKey, 'e.shiftKey', 'i32') }}}
        {{{ makeSetValue('JsEvents.keyEvent', C_STRUCTS.emscripten_KeyboardEvent.altKey, 'e.altKey', 'i32') }}}
        {{{ makeSetValue('JsEvents.keyEvent', C_STRUCTS.emscripten_KeyboardEvent.metaKey, 'e.metaKey', 'i32') }}}
        {{{ makeSetValue('JsEvents.keyEvent', C_STRUCTS.emscripten_KeyboardEvent.repeat, 'e.repeat', 'i32') }}}
        writeStringToMemory(e.locale ? e.locale : "", JsEvents.keyEvent + {{{ C_STRUCTS.emscripten_KeyboardEvent.locale }}} );
        writeStringToMemory(e.char ? e.char : "", JsEvents.keyEvent + {{{ C_STRUCTS.emscripten_KeyboardEvent.charValue }}} );
        {{{ makeSetValue('JsEvents.keyEvent', C_STRUCTS.emscripten_KeyboardEvent.charCode, 'e.charCode', 'i32') }}}
        {{{ makeSetValue('JsEvents.keyEvent', C_STRUCTS.emscripten_KeyboardEvent.keyCode, 'e.keyCode', 'i32') }}}
        {{{ makeSetValue('JsEvents.keyEvent', C_STRUCTS.emscripten_KeyboardEvent.which, 'e.which', 'i32') }}}
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.keyEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: true,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    fillMouseEventData: function(eventStruct, e) {
      var rect = Module['canvas'].getBoundingClientRect();
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.timestamp, 'JsEvents.tick()', 'double') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.screenX, 'e.screenX', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.screenY, 'e.screenY', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.clientX, 'e.clientX', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.clientY, 'e.clientY', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.ctrlKey, 'e.ctrlKey', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.shiftKey, 'e.shiftKey', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.altKey, 'e.altKey', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.metaKey, 'e.metaKey', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.button, 'e.button', 'i16') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.buttons, 'e.buttons', 'i16') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.movementX, 'e.movementX', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.movementY, 'e.movementY', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.canvasX, 'e.clientX - rect.left', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_MouseEvent.canvasY, 'e.clientY - rect.top', 'i32') }}}
    },
    
    registerMouseEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.mouseEvent) {
        JsEvents.mouseEvent = _malloc( {{{ C_STRUCTS.emscripten_MouseEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;
        JsEvents.fillMouseEventData(JsEvents.mouseEvent, e);
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.mouseEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: eventTypeString != 'mousemove', // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    registerWheelEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.wheelEvent) {
        JsEvents.wheelEvent = _malloc( {{{ C_STRUCTS.emscripten_WheelEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;
        JsEvents.fillMouseEventData(JsEvents.wheelEvent, e);
        {{{ makeSetValue('JsEvents.wheelEvent', C_STRUCTS.emscripten_WheelEvent.deltaX, 'e.deltaX', 'double') }}}
        {{{ makeSetValue('JsEvents.wheelEvent', C_STRUCTS.emscripten_WheelEvent.deltaY, 'e.deltaY', 'double') }}}
        {{{ makeSetValue('JsEvents.wheelEvent', C_STRUCTS.emscripten_WheelEvent.deltaZ, 'e.deltaZ', 'double') }}}
        {{{ makeSetValue('JsEvents.wheelEvent', C_STRUCTS.emscripten_WheelEvent.deltaMode, 'e.deltaMode', 'i32') }}}
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.wheelEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: true,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    pageScrollPos: function() {
      if (window.pageXOffset > 0 || window.pageYOffset > 0) {
        return [window.pageXOffset, window.pageYOffset];
      }
      if (document.documentElement.scrollLeft > 0 || document.documentElement.scrollTop > 0) {
        return [document.documentElement.scrollLeft, document.documentElement.scrollTop];
      }
      return [document.body.scrollLeft|0, document.body.scrollTop|0];
    },

    registerUiEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.uiEvent) {
        JsEvents.uiEvent = _malloc( {{{ C_STRUCTS.emscripten_UiEvent.__size__ }}} );
      }

      if (eventTypeString == "scroll" && !target) {
        target = document; // By default read scroll events on document rather than window.
      } else {
        target = JsEvents.findEventTarget(target);
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
        var scrollPos = JsEvents.pageScrollPos();
        {{{ makeSetValue('JsEvents.uiEvent', C_STRUCTS.emscripten_UiEvent.detail, 'e.detail', 'i32') }}}
        {{{ makeSetValue('JsEvents.uiEvent', C_STRUCTS.emscripten_UiEvent.documentBodyClientWidth, 'document.body.clientWidth', 'i32') }}}
        {{{ makeSetValue('JsEvents.uiEvent', C_STRUCTS.emscripten_UiEvent.documentBodyClientHeight, 'document.body.clientHeight', 'i32') }}}
        {{{ makeSetValue('JsEvents.uiEvent', C_STRUCTS.emscripten_UiEvent.windowInnerWidth, 'window.innerWidth', 'i32') }}}
        {{{ makeSetValue('JsEvents.uiEvent', C_STRUCTS.emscripten_UiEvent.windowInnerHeight, 'window.innerHeight', 'i32') }}}
        {{{ makeSetValue('JsEvents.uiEvent', C_STRUCTS.emscripten_UiEvent.windowOuterWidth, 'window.outerWidth', 'i32') }}}
        {{{ makeSetValue('JsEvents.uiEvent', C_STRUCTS.emscripten_UiEvent.windowOuterHeight, 'window.outerHeight', 'i32') }}}
        {{{ makeSetValue('JsEvents.uiEvent', C_STRUCTS.emscripten_UiEvent.scrollTop, 'scrollPos[0]', 'i32') }}}
        {{{ makeSetValue('JsEvents.uiEvent', C_STRUCTS.emscripten_UiEvent.scrollLeft, 'scrollPos[1]', 'i32') }}}
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.uiEvent, userData]);
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
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    getNodeNameForTarget: function(target) {
      if (!target) {
        return '';
      }
      if (target == window) {
        return '#window';
      }
      if (target == window.screen) {
        return '#screen';
      }
      return (target && target.nodeName) ? target.nodeName : '';
    },

    registerFocusEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.focusEvent) {
        JsEvents.focusEvent = _malloc( {{{ C_STRUCTS.emscripten_FocusEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;

        var nodeName = JsEvents.getNodeNameForTarget(e.target);
        var id = e.target.id ? e.target.id : '';
        writeStringToMemory(nodeName, JsEvents.focusEvent + {{{ C_STRUCTS.emscripten_FocusEvent.nodeName }}} );
        writeStringToMemory(id, JsEvents.focusEvent + {{{ C_STRUCTS.emscripten_FocusEvent.id }}} );
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.focusEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    tick: function() {
      if (window['performance'] && window['performance']['now']) {
        return window['performance']['now']();
      } else {
        return Date.now();
      }
    },

    registerDeviceOrientationEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.deviceOrientationEvent) {
        JsEvents.deviceOrientationEvent = _malloc( {{{ C_STRUCTS.emscripten_DeviceOrientationEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;

        {{{ makeSetValue('JsEvents.deviceOrientationEvent', C_STRUCTS.emscripten_DeviceOrientationEvent.timestamp, 'JsEvents.tick()', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceOrientationEvent', C_STRUCTS.emscripten_DeviceOrientationEvent.alpha, 'e.alpha', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceOrientationEvent', C_STRUCTS.emscripten_DeviceOrientationEvent.beta, 'e.beta', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceOrientationEvent', C_STRUCTS.emscripten_DeviceOrientationEvent.gamma, 'e.gamma', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceOrientationEvent', C_STRUCTS.emscripten_DeviceOrientationEvent.absolute, 'e.absolute', 'i32') }}}

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.deviceOrientationEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    registerDeviceMotionEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.deviceMotionEvent) {
        JsEvents.deviceMotionEvent = _malloc( {{{ C_STRUCTS.emscripten_DeviceMotionEvent.__size__ }}} );
      }
      var handlerFunc = function(event) {
        var e = event || window.event;

        {{{ makeSetValue('JsEvents.deviceOrientationEvent', C_STRUCTS.emscripten_DeviceMotionEvent.timestamp, 'JsEvents.tick()', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceMotionEvent', C_STRUCTS.emscripten_DeviceMotionEvent.accelerationX, 'e.acceleration.x', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceMotionEvent', C_STRUCTS.emscripten_DeviceMotionEvent.accelerationY, 'e.acceleration.y', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceMotionEvent', C_STRUCTS.emscripten_DeviceMotionEvent.accelerationZ, 'e.acceleration.z', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceMotionEvent', C_STRUCTS.emscripten_DeviceMotionEvent.accelerationIncludingGravityX, 'e.accelerationIncludingGravity.x', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceMotionEvent', C_STRUCTS.emscripten_DeviceMotionEvent.accelerationIncludingGravityY, 'e.accelerationIncludingGravity.y', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceMotionEvent', C_STRUCTS.emscripten_DeviceMotionEvent.accelerationIncludingGravityZ, 'e.accelerationIncludingGravity.z', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceMotionEvent', C_STRUCTS.emscripten_DeviceMotionEvent.rotationRateAlpha, 'e.rotationRate.alpha', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceMotionEvent', C_STRUCTS.emscripten_DeviceMotionEvent.rotationRateBeta, 'e.rotationRate.beta', 'double') }}}
        {{{ makeSetValue('JsEvents.deviceMotionEvent', C_STRUCTS.emscripten_DeviceMotionEvent.rotationRateGamma, 'e.rotationRate.gamma', 'double') }}}

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.deviceMotionEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    fillOrientationChangeEventData: function(eventStruct, e) {
      var orientations  = ["portrait-primary", "portrait-secondary", "landscape-primary", "landscape-secondary"];
      var orientations2 = ["portrait",         "portrait",           "landscape",         "landscape"];

      var orientationString = window.screen.orientation || window.screen.mozOrientation;
      var orientation = orientations.indexOf(orientationString);
      if (orientation == -1) {
        orientation = orientations2.indexOf(orientationString);
      }

      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_OrientationChangeEvent.orientationIndex, '1 << orientation', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_OrientationChangeEvent.orientationAngle, 'window.orientation', 'i32') }}}
    },

    registerOrientationChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.orientationChangeEvent) {
        JsEvents.orientationChangeEvent = _malloc( {{{ C_STRUCTS.emscripten_OrientationChangeEvent.__size__ }}} );
      }

      if (!target) {
        target = window.screen; // Orientation events need to be captured from 'window.screen' instead of 'window'
      } else {
        target = JsEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JsEvents.fillOrientationChangeEventData(JsEvents.orientationChangeEvent, e);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.orientationChangeEvent, userData]);
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
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    fillFullscreenChangeEventData: function(eventStruct, e) {
      var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
      var isFullscreen = !!fullscreenElement;
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_FullscreenChangeEvent.isFullscreen, 'isFullscreen', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_FullscreenChangeEvent.fullscreenEnabled, 'document.fullscreenEnabled', 'i32') }}}
      var nodeName = JsEvents.getNodeNameForTarget(fullscreenElement);
      var id = (fullscreenElement && fullscreenElement.id) ? fullscreenElement.id : '';
      writeStringToMemory(nodeName, eventStruct + {{{ C_STRUCTS.emscripten_FullscreenChangeEvent.nodeName }}} );
      writeStringToMemory(id, eventStruct + {{{ C_STRUCTS.emscripten_FullscreenChangeEvent.id }}} );
    },

    registerFullscreenChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.fullscreenChangeEvent) {
        JsEvents.fullscreenChangeEvent = _malloc( {{{ C_STRUCTS.emscripten_FullscreenChangeEvent.__size__ }}} );
      }

      if (!target) {
        target = document; // Fullscreen change events need to be captured from 'document' by default instead of 'window'
      } else {
        target = JsEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JsEvents.fillFullscreenChangeEventData(JsEvents.fullscreenChangeEvent, e);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.fullscreenChangeEvent, userData]);
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
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    requestFullscreen: function(target) {
      if (target.requestFullscreen) {
        target.requestFullscreen();
      } else if (target.msRequestFullscreen) {
        target.msRequestFullscreen();
      } else if (target.mozRequestFullScreen) {
        target.mozRequestFullScreen();
      } else if (target.webkitRequestFullscreen) {
        target.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
      } else {
        return 2 /* Target does not support requesting fullscreen */;
      }
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    },

    fillPointerlockChangeEventData: function(eventStruct, e) {
      var pointerLockElement = document.pointerLockElement || document.mozPointerLockElement || document.webkitPointerLockElement || document.msPointerLockElement;
      var isPointerlocked = !!pointerLockElement;
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_PointerlockChangeEvent.isActive, 'isPointerlocked', 'i32') }}}
      var nodeName = JsEvents.getNodeNameForTarget(pointerLockElement);
      var id = (pointerLockElement && pointerLockElement.id) ? pointerLockElement.id : '';
      writeStringToMemory(nodeName, eventStruct + {{{ C_STRUCTS.emscripten_PointerlockChangeEvent.nodeName }}} );
      writeStringToMemory(id, eventStruct + {{{ C_STRUCTS.emscripten_PointerlockChangeEvent.id }}});
    },

    registerPointerlockChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.pointerlockChangeEvent) {
        JsEvents.pointerlockChangeEvent = _malloc( {{{ C_STRUCTS.emscripten_PointerlockChangeEvent.__size__ }}} );
      }

      if (!target) {
        target = document; // Pointer lock change events need to be captured from 'document' by default instead of 'window'
      } else {
        target = JsEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JsEvents.fillPointerlockChangeEventData(JsEvents.pointerlockChangeEvent, e);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.pointerlockChangeEvent, userData]);
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
      JsEvents.registerOrRemoveHandler(eventHandler);
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
        return 2 /* Target does not support requesting pointer lock */;
      }
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    },

    fillVisibilityChangeEventData: function(eventStruct, e) {
      var visibilityStates = [ "hidden", "visible", "prerender", "unloaded" ];
      var visibilityState = visibilityStates.indexOf(document.visibilityState);

      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_VisibilityChangeEvent.hidden, 'document.hidden', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_VisibilityChangeEvent.visibilityState, 'visibilityState', 'i32') }}}
    },

    registerVisibilityChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.visibilityChangeEvent) {
        JsEvents.visibilityChangeEvent = _malloc( {{{ C_STRUCTS.emscripten_VisibilityChangeEvent.__size__ }}} );
      }

      if (!target) {
        target = document; // Visibility change events need to be captured from 'document' by default instead of 'window'
      } else {
        target = JsEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JsEvents.fillVisibilityChangeEventData(JsEvents.visibilityChangeEvent, e);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.visibilityChangeEvent, userData]);
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
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    registerTouchEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.touchEvent) {
        JsEvents.touchEvent = _malloc( {{{ C_STRUCTS.emscripten_TouchEvent.__size__ }}} );
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
        
        var ptr = JsEvents.touchEvent;
        {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchEvent.ctrlKey, 'e.ctrlKey', 'i32') }}}
        {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchEvent.shiftKey, 'e.shiftKey', 'i32') }}}
        {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchEvent.altKey, 'e.altKey', 'i32') }}}
        {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchEvent.metaKey, 'e.metaKey', 'i32') }}}
        ptr += {{{ C_STRUCTS.emscripten_TouchEvent.touches }}}; // Advance to the start of the touch array.
        var rect = Module['canvas'].getBoundingClientRect();
        var numTouches = 0;
        for(var i in touches) {
          var t = touches[i];
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.identifier, 't.identifier', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.screenX, 't.screenX', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.screenY, 't.screenY', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.clientX, 't.clientX', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.clientY, 't.clientY', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.pageX, 't.pageX', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.pageY, 't.pageY', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.isChanged, 't.changed', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.onTarget, 't.onTarget', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.canvasX, 't.clientX - rect.left', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.emscripten_TouchPoint.canvasY, 't.clientY - rect.top', 'i32') }}}
          ptr += {{{ C_STRUCTS.emscripten_TouchPoint.__size__ }}};

          if (++numTouches >= 32) {
            break;
          }
        }
        {{{ makeSetValue('JsEvents.touchEvent', C_STRUCTS.emscripten_TouchEvent.numTouches, 'numTouches', 'i32') }}}

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.touchEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: true,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    fillGamepadEventData: function(eventStruct, e) {
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_GamepadEvent.timestamp, 'e.timestamp', 'double') }}}
      for(var i = 0; i < e.axes.length; ++i) {
        {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.emscripten_GamepadEvent.axis, 'e.axes[i]', 'double') }}}
      }
      for(var i = 0; i < e.buttons.length; ++i) {
        {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.emscripten_GamepadEvent.analogButton, 'e.buttons[i].value', 'double') }}}
      }
      for(var i = 0; i < e.buttons.length; ++i) {
        {{{ makeSetValue('eventStruct+i*4', C_STRUCTS.emscripten_GamepadEvent.digitalButton, 'e.buttons[i].pressed', 'i32') }}}
      }
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_GamepadEvent.connected, 'e.connected', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_GamepadEvent.index, 'e.index', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_GamepadEvent.numAxes, 'e.axes.length', 'i32') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_GamepadEvent.numButtons, 'e.buttons.length', 'i32') }}}
      writeStringToMemory(e.id, eventStruct + {{{ C_STRUCTS.emscripten_GamepadEvent.id }}} );
      writeStringToMemory(e.mapping, eventStruct + {{{ C_STRUCTS.emscripten_GamepadEvent.mapping }}} );
    },

    registerGamepadEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.gamepadEvent) {
        JsEvents.gamepadEvent = _malloc( {{{ C_STRUCTS.emscripten_GamepadEvent.__size__ }}} );
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JsEvents.fillGamepadEventData(JsEvents.gamepadEvent, e.gamepad);

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.gamepadEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: true,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
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
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
    },

    battery: function() { return navigator.battery || navigator.mozBattery || navigator.webkitBattery; },

    fillBatteryEventData: function(eventStruct, e) {
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_BatteryEvent.chargingTime, 'e.chargingTime', 'double') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_BatteryEvent.dischargingTime, 'e.dischargingTime', 'double') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_BatteryEvent.level, 'e.level', 'double') }}}
      {{{ makeSetValue('eventStruct', C_STRUCTS.emscripten_BatteryEvent.charging, 'e.charging', 'i32') }}}
    },
    
    registerBatteryEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JsEvents.batteryEvent) {
        JsEvents.batteryEvent = _malloc( {{{ C_STRUCTS.emscripten_BatteryEvent.__size__ }}} );
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        JsEvents.fillBatteryEventData(JsEvents.batteryEvent, JsEvents.battery());

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JsEvents.batteryEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
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
        target: JsEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JsEvents.registerOrRemoveHandler(eventHandler);
    },
  },

  emscripten_set_keypress_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYPRESS') }}}, "keypress");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_keydown_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYDOWN') }}}, "keydown");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_keyup_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYUP') }}}, "keyup");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_click_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_CLICK') }}}, "click");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mousedown_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEDOWN') }}}, "mousedown");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseup_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEUP') }}}, "mouseup");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_dblclick_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DBLCLICK') }}}, "dblclick");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mousemove_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEMOVE') }}}, "mousemove");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_mouse_status: function(mouseState) {
    if (!JsEvents.mouseEvent) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    }
    // HTML5 does not really have a polling API for mouse events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler to any of the mouse function.
    HEAP32.set(HEAP32.subarray(JsEvents.mouseEvent, {{{ C_STRUCTS.emscripten_MouseEvent.__size__ }}}), mouseState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_wheel_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerWheelEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WHEEL') }}}, "wheel");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_resize_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_RESIZE') }}}, "resize");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_scroll_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_SCROLL') }}}, "scroll");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_blur_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BLUR') }}}, "blur");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focus_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUS') }}}, "focus");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focusin_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUSIN') }}}, "focusin");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focusout_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUSOUT') }}}, "focusout");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_deviceorientation_callback: function(userData, useCapture, callbackfunc) {
    JsEvents.registerDeviceOrientationEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DEVICEORIENTATION') }}}, "deviceorientation");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_deviceorientation_status: function(orientationState) {
    if (!JsEvents.deviceOrientationEvent) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    }
    // HTML5 does not really have a polling API for device orientation events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler.
    HEAP32.set(HEAP32.subarray(JsEvents.deviceOrientationEvent, {{{ C_STRUCTS.emscripten_DeviceOrientationEvent.__size__ }}}), orientationState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_devicemotion_callback: function(userData, useCapture, callbackfunc) {
    JsEvents.registerDeviceMotionEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DEVICEMOTION') }}}, "devicemotion");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_devicemotion_status: function(motionState) {
    if (!JsEvents.deviceMotionEvent) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    }
    // HTML5 does not really have a polling API for device motion events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler.
    HEAP32.set(HEAP32.subarray(JsEvents.deviceMotionEvent, {{{ C_STRUCTS.emscripten_DeviceMotionEvent.__size__ }}}), motionState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_orientationchange_callback: function(userData, useCapture, callbackfunc) {
    if (!window.screen || !window.screen.addEventListener) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    JsEvents.registerOrientationChangeEventCallback(window.screen, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_ORIENTATIONCHANGE') }}}, "orientationchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_get_orientation_status: function(orientationChangeEvent) {
    JsEvents.fillOrientationChangeEventData(orientationChangeEvent);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_lock_orientation: function(allowedOrientations) {
    var orientations = [];
    if (allowedOrientations & 1) orientations.push("portrait-primary");
    if (allowedOrientations & 2) orientations.push("portrait-secondary");
    if (allowedOrientations & 4) orientations.push("landscape-primary");
    if (allowedOrientations & 8) orientations.push("landscape-secondary");
    if (window.screen.lockOrientation) {
      window.screen.lockOrientation(orientations);
    } else if (window.screen.mozLockOrientation) {
      window.screen.mozLockOrientation(orientations);
    } else if (window.screen.webkitLockOrientation) {
      window.screen.webkitLockOrientation(orientations);
    } else if (window.screen.msLockOrientation) {
      window.screen.msLockOrientation(orientations);
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
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

  emscripten_set_fullscreenchange_callback: function(userData, useCapture, callbackfunc) {
    JsEvents.registerFullscreenChangeEventCallback(userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "fullscreenchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_fullscreen_status: function(fullscreenStatus) {
    JsEvents.fillFullscreenChangeEventData(fullscreenStatus);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  // https://developer.mozilla.org/en-US/docs/Web/Guide/API/DOM/Using_full_screen_mode
  emscripten_request_fullscreen: function(target, deferUntilInEventHandler) {
    if (!target) {
      target = '#canvas';
    }
    target = JsEvents.findEventTarget(target);
    if (!target) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

    if (!target.requestFullscreen && !target.msRequestFullscreen && !target.mozRequestFullScreen && !target.webkitRequestFullscreen) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }

    var canPerformRequests = JsEvents.canPerformEventHandlerRequests();

    // Queue this function call if we're not currently in an event handler and the user saw it appropriate to do so.
    if (!canPerformRequests) {
      if (deferUntilInEventHandler) {
        JsEvents.deferCall(JsEvents.requestFullscreen, 1 /* priority over pointer lock */, [target]);
        return {{{ cDefine('EMSCRIPTEN_RESULT_DEFERRED') }}};
      } else {
        return {{{ cDefine('EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED') }}};
      }
    }

    return JsEvents.requestFullscreen(target);
  },
  
  emscripten_exit_fullscreen: function() {
    // Make sure no queued up calls will fire after this.
    JsEvents.removeDeferredCalls(JsEvents.requestFullscreen);

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

  emscripten_set_pointerlockchange_callback: function(userData, useCapture, callbackfunc) {
    JsEvents.registerPointerlockChangeEventCallback(document, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "pointerlockchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_pointerlock_status: function(pointerlockStatus) {
    JsEvents.fillPointerlockChangeEventData(pointerlockStatus);  
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_request_pointerlock: function(target, deferUntilInEventHandler) {
    if (!target) {
      target = '#canvas';
    }
    target = JsEvents.findEventTarget(target);
    if (!target) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }
    if (!target.requestPointerLock && !target.mozRequestPointerLock && !target.webkitRequestPointerLock && !target.msRequestPointerLock) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }

    var canPerformRequests = JsEvents.canPerformEventHandlerRequests();

    // Queue this function call if we're not currently in an event handler and the user saw it appropriate to do so.
    if (!canPerformRequests) {
      if (deferUntilInEventHandler) {
        JsEvents.deferCall(JsEvents.requestPointerLock, 2 /* priority below fullscreen */, [target]);
        return {{{ cDefine('EMSCRIPTEN_RESULT_DEFERRED') }}};
      } else {
        return {{{ cDefine('EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED') }}};
      }
    }

    return JsEvents.requestPointerLock(target);
  },

  emscripten_exit_pointerlock: function() {
    // Make sure no queued up calls will fire after this.
    JsEvents.removeDeferredCalls(JsEvents.requestPointerLock);

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
    if (!navigator.vibrate) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    
    navigator.vibrate(msecs);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_vibrate_pattern: function(msecsArray, numEntries) {
    if (!navigator.vibrate) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }

    var vibrateList = [];
    for(var i = 0; i < numEntries; ++i) {
      var msecs = {{{ makeGetValue('msecsArray', 'i*4', 'i32') }}}
      vibrateList.push(msecs);
    }
    navigator.vibrate(vibrateList);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_visibilitychange_callback: function(userData, useCapture, callbackfunc) {
    JsEvents.registerVisibilityChangeEventCallback(document, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_VISIBILITYCHANGE') }}}, "visibilitychange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_visibility_status: function(visibilityStatus) {
    JsEvents.fillVisibilityChangeEventData(visibilityStatus);  
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchstart_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHSTART') }}}, "touchstart");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchend_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHEND') }}}, "touchend");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchmove_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHMOVE') }}}, "touchmove");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchcancel_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHCANCEL') }}}, "touchcancel");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_gamepadconnected_callback: function(userData, useCapture, callbackfunc) {
    JsEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADCONNECTED') }}}, "gamepadconnected");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_gamepaddisconnected_callback: function(userData, useCapture, callbackfunc) {
    JsEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED') }}}, "gamepaddisconnected");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
 },
  
  emscripten_get_num_gamepads: function() {
    if (!navigator.getGamepads) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    return navigator.getGamepads().length;
  },
  
  emscripten_get_gamepad_status: function(index, gamepadState) {
    if (!navigator.getGamepads) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    var gamepads = navigator.getGamepads();
    if (index < 0 || index >= gamepads.length) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
    }
    JsEvents.fillGamepadEventData(gamepadState, gamepads[index]);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_beforeunload_callback: function(userData, callbackfunc) {
    JsEvents.registerBeforeUnloadEventCallback(window, userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BEFOREUNLOAD') }}}, "beforeunload"); 
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_batterychargingchange_callback: function(userData, callbackfunc) {
    JsEvents.registerBatteryEventCallback(JsEvents.battery(), userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BATTERYCHARGINGCHANGE') }}}, "chargingchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_batterylevelchange_callback: function(userData, callbackfunc) {
    JsEvents.registerBatteryEventCallback(JsEvents.battery(), userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BATTERYLEVELCHANGE') }}}, "levelchange");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_get_battery_status: function(batteryState) {
    JsEvents.fillBatteryEventData(batteryState, JsEvents.battery());
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_webglcontextlost_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST') }}}, "webglcontextlost");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_webglcontextrestored_callback: function(target, userData, useCapture, callbackfunc) {
    JsEvents.registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED') }}}, "webglcontextrestored");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
};

autoAddDeps(LibraryJsEvents, '$JsEvents');
mergeInto(LibraryManager.library, LibraryJsEvents);
