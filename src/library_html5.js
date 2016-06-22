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

    // In order to ensure most coherent Gamepad API state as possible (https://github.com/w3c/gamepad/issues/22) and
    // to minimize the amount of garbage created, we sample the gamepad state at most once per frame, and not e.g. once per
    // each controller or similar. To implement that, the following variables retain a cache of the most recent polled gamepad
    // state.
    lastGamepadState: null,
    lastGamepadStateFrame: null, // The integer value of Browser.mainLoop.currentFrameNumber of when the last gamepad state was produced.

    // When we transition from fullscreen to windowed mode, we remember here the element that was just in fullscreen mode
    // so that we can report information about that element in the event message.
    previousFullscreenElement: null,

    // Remember the current mouse coordinates in case we need to emulate movementXY generation for browsers that don't support it.
    // Some browsers (e.g. Safari 6.0.5) only give movementXY when Pointerlock is active.
    previousScreenX: null,
    previousScreenY: null,

    // When the C runtime exits via exit(), we unregister all event handlers added by this library to be nice and clean.
    // Track in this field whether we have yet registered that __ATEXIT__ handler.
    removeEventListenersRegistered: false, 

    registerRemoveEventListeners: function() {
      if (!JSEvents.removeEventListenersRegistered) {
      __ATEXIT__.push(function() {
          for(var i = JSEvents.eventHandlers.length-1; i >= 0; --i) {
            JSEvents._removeHandler(i);
          }
         });
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

    isInternetExplorer: function() { return navigator.userAgent.indexOf('MSIE') !== -1 || navigator.appVersion.indexOf('Trident/') > 0; },

    // Removes all event handlers on the given DOM element of the given type. Pass in eventTypeString == undefined/null to remove all event handlers regardless of the type.
    removeAllHandlersOnTarget: function(target, eventTypeString) {
      for(var i = 0; i < JSEvents.eventHandlers.length; ++i) {
        if (JSEvents.eventHandlers[i].target == target && 
          (!eventTypeString || eventTypeString == JSEvents.eventHandlers[i].eventTypeString)) {
           JSEvents._removeHandler(i--);
         }
      }
    },

    _removeHandler: function(i) {
      var h = JSEvents.eventHandlers[i];
      h.target.removeEventListener(h.eventTypeString, h.eventListenerFunc, h.useCapture);
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
        eventHandler.eventListenerFunc = jsEventHandler;
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
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.location, 'e.location', 'i32') }}};
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.ctrlKey, 'e.ctrlKey', 'i32') }}};
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.shiftKey, 'e.shiftKey', 'i32') }}};
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.altKey, 'e.altKey', 'i32') }}};
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.metaKey, 'e.metaKey', 'i32') }}};
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.repeat, 'e.repeat', 'i32') }}};
        writeStringToMemory(e.locale ? e.locale : "", JSEvents.keyEvent + {{{ C_STRUCTS.EmscriptenKeyboardEvent.locale }}} );
        writeStringToMemory(e.char ? e.char : "", JSEvents.keyEvent + {{{ C_STRUCTS.EmscriptenKeyboardEvent.charValue }}} );
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.charCode, 'e.charCode', 'i32') }}};
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.keyCode, 'e.keyCode', 'i32') }}};
        {{{ makeSetValue('JSEvents.keyEvent', C_STRUCTS.EmscriptenKeyboardEvent.which, 'e.which', 'i32') }}};
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.keyEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: JSEvents.isInternetExplorer() ? false : true, // MSIE doesn't allow fullscreen and pointerlock requests from key handlers, others do.
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    getBoundingClientRectOrZeros: function(target) {
      return target.getBoundingClientRect ? target.getBoundingClientRect() : { left: 0, top: 0 };
    },

    // Copies mouse event data from the given JS mouse event 'e' to the specified Emscripten mouse event structure in the HEAP.
    // eventStruct: the structure to populate.
    // e: The JS mouse event to read data from.
    // target: Specifies a target DOM element that will be used as the reference to populate targetX and targetY parameters.
    fillMouseEventData: function(eventStruct, e, target) {
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.timestamp, 'JSEvents.tick()', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.screenX, 'e.screenX', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.screenY, 'e.screenY', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.clientX, 'e.clientX', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.clientY, 'e.clientY', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.ctrlKey, 'e.ctrlKey', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.shiftKey, 'e.shiftKey', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.altKey, 'e.altKey', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.metaKey, 'e.metaKey', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.button, 'e.button', 'i16') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.buttons, 'e.buttons', 'i16') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.movementX, 'e["movementX"] || e["mozMovementX"] || e["webkitMovementX"] || (e.screenX-JSEvents.previousScreenX)', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.movementY, 'e["movementY"] || e["mozMovementY"] || e["webkitMovementY"] || (e.screenY-JSEvents.previousScreenY)', 'i32') }}};

      if (Module['canvas']) {
        var rect = Module['canvas'].getBoundingClientRect();
        {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.canvasX, 'e.clientX - rect.left', 'i32') }}};
        {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.canvasY, 'e.clientY - rect.top', 'i32') }}};
      } else { // Canvas is not initialized, return 0.
        {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.canvasX, '0', 'i32') }}};
        {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.canvasY, '0', 'i32') }}};
      }
      if (target) {
        var rect = JSEvents.getBoundingClientRectOrZeros(target);
        {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.targetX, 'e.clientX - rect.left', 'i32') }}};
        {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.targetY, 'e.clientY - rect.top', 'i32') }}};        
      } else { // No specific target passed, return 0.
        {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.targetX, '0', 'i32') }}};
        {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.targetY, '0', 'i32') }}};
      }
      JSEvents.previousScreenX = e.screenX;
      JSEvents.previousScreenY = e.screenY;
    },
    
    registerMouseEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.mouseEvent) {
        JSEvents.mouseEvent = _malloc( {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}} );
      }
      target = JSEvents.findEventTarget(target);
      var handlerFunc = function(event) {
        var e = event || window.event;
        JSEvents.fillMouseEventData(JSEvents.mouseEvent, e, target);
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.mouseEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: eventTypeString != 'mousemove' && eventTypeString != 'mouseenter' && eventTypeString != 'mouseleave', // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: handlerFunc,
        useCapture: useCapture
      };
      // In IE, mousedown events don't either allow deferred calls to be run!
      if (JSEvents.isInternetExplorer() && eventTypeString == 'mousedown') eventHandler.allowsDeferredCalls = false;
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerWheelEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!JSEvents.wheelEvent) {
        JSEvents.wheelEvent = _malloc( {{{ C_STRUCTS.EmscriptenWheelEvent.__size__ }}} );
      }
      target = JSEvents.findEventTarget(target);
      // The DOM Level 3 events spec event 'wheel'
      var wheelHandlerFunc = function(event) {
        var e = event || window.event;
        JSEvents.fillMouseEventData(JSEvents.wheelEvent, e, target);
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaX, 'e["deltaX"]', 'double') }}};
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaY, 'e["deltaY"]', 'double') }}};
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaZ, 'e["deltaZ"]', 'double') }}};
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaMode, 'e["deltaMode"]', 'i32') }}};
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.wheelEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };
      // The 'mousewheel' event as implemented in Safari 6.0.5
      var mouseWheelHandlerFunc = function(event) {
        var e = event || window.event;
        JSEvents.fillMouseEventData(JSEvents.wheelEvent, e, target);
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaX, 'e["wheelDeltaX"]', 'double') }}};
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaY, '-(e["wheelDeltaY"] ? e["wheelDeltaY"] : e["wheelDelta"]) /* 1. Invert to unify direction with the DOM Level 3 wheel event. 2. MSIE does not provide wheelDeltaY, so wheelDelta is used as a fallback. */', 'double') }}};
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaZ, '0 /* Not available */', 'double') }}};
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaMode, '0 /* DOM_DELTA_PIXEL */', 'i32') }}};
        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.wheelEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: true,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: (eventTypeString == 'wheel') ? wheelHandlerFunc : mouseWheelHandlerFunc,
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
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.detail, 'e.detail', 'i32') }}};
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientWidth, 'document.body.clientWidth', 'i32') }}};
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientHeight, 'document.body.clientHeight', 'i32') }}};
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerWidth, 'window.innerWidth', 'i32') }}};
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerHeight, 'window.innerHeight', 'i32') }}};
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterWidth, 'window.outerWidth', 'i32') }}};
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterHeight, 'window.outerHeight', 'i32') }}};
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollTop, 'scrollPos[0]', 'i32') }}};
        {{{ makeSetValue('JSEvents.uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollLeft, 'scrollPos[1]', 'i32') }}};
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

        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.timestamp, 'JSEvents.tick()', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.alpha, 'e.alpha', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.beta, 'e.beta', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.gamma, 'e.gamma', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceOrientationEvent.absolute, 'e.absolute', 'i32') }}};

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

        {{{ makeSetValue('JSEvents.deviceOrientationEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.timestamp, 'JSEvents.tick()', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationX, 'e.acceleration.x', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationY, 'e.acceleration.y', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationZ, 'e.acceleration.z', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityX, 'e.accelerationIncludingGravity.x', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityY, 'e.accelerationIncludingGravity.y', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityZ, 'e.accelerationIncludingGravity.z', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateAlpha, 'e.rotationRate.alpha', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateBeta, 'e.rotationRate.beta', 'double') }}};
        {{{ makeSetValue('JSEvents.deviceMotionEvent', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateGamma, 'e.rotationRate.gamma', 'double') }}};

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

      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenOrientationChangeEvent.orientationIndex, '1 << orientation', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenOrientationChangeEvent.orientationAngle, 'window.orientation', 'i32') }}};
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
      return document.fullscreenEnabled || document.mozFullScreenEnabled || document.webkitFullscreenEnabled || document.msFullscreenEnabled;
    },
    
    fillFullscreenChangeEventData: function(eventStruct, e) {
      var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
      var isFullscreen = !!fullscreenElement;
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.isFullscreen, 'isFullscreen', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.fullscreenEnabled, 'JSEvents.fullscreenEnabled()', 'i32') }}};
      // If transitioning to fullscreen, report info about the element that is now fullscreen.
      // If transitioning to windowed mode, report info about the element that just was fullscreen.
      var reportedElement = isFullscreen ? fullscreenElement : JSEvents.previousFullscreenElement;
      var nodeName = JSEvents.getNodeNameForTarget(reportedElement);
      var id = (reportedElement && reportedElement.id) ? reportedElement.id : '';
      writeStringToMemory(nodeName, eventStruct + {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.nodeName }}} );
      writeStringToMemory(id, eventStruct + {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.id }}} );
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.elementWidth, 'reportedElement ? reportedElement.clientWidth : 0', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.elementHeight, 'reportedElement ? reportedElement.clientHeight : 0', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.screenWidth, 'screen.width', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.screenHeight, 'screen.height', 'i32') }}};
      if (isFullscreen) {
        JSEvents.previousFullscreenElement = fullscreenElement;
      }
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

    resizeCanvasForFullscreen: function(target, strategy) {
      var restoreOldStyle = __registerRestoreOldStyle(target);
      var cssWidth = strategy.softFullscreen ? window.innerWidth : screen.width;
      var cssHeight = strategy.softFullscreen ? window.innerHeight : screen.height;
      var rect = target.getBoundingClientRect();
      var windowedCssWidth = rect.right - rect.left;
      var windowedCssHeight = rect.bottom - rect.top;
      var windowedRttWidth = target.width;
      var windowedRttHeight = target.height;

      if (strategy.scaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_CENTER') }}}) {
        __setLetterbox(target, (cssHeight - windowedCssHeight) / 2, (cssWidth - windowedCssWidth) / 2);
        cssWidth = windowedCssWidth;
        cssHeight = windowedCssHeight;
      } else if (strategy.scaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT') }}}) {
        if (cssWidth*windowedRttHeight < windowedRttWidth*cssHeight) {
          var desiredCssHeight = windowedRttHeight * cssWidth / windowedRttWidth;
          __setLetterbox(target, (cssHeight - desiredCssHeight) / 2, 0);
          cssHeight = desiredCssHeight;
        } else {
          var desiredCssWidth = windowedRttWidth * cssHeight / windowedRttHeight;
          __setLetterbox(target, 0, (cssWidth - desiredCssWidth) / 2);
          cssWidth = desiredCssWidth;
        }
      }

      // If we are adding padding, must choose a background color or otherwise Chrome will give the
      // padding a default white color. Do it only if user has not customized their own background color.
      if (!target.style.backgroundColor) target.style.backgroundColor = 'black';
      // IE11 does the same, but requires the color to be set in the document body.
      if (!document.body.style.backgroundColor) document.body.style.backgroundColor = 'black'; // IE11
      // Firefox always shows black letterboxes independent of style color.

      target.style.width = cssWidth + 'px';
      target.style.height = cssHeight + 'px';

      if (strategy.filteringMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST') }}}) {
        target.style.imageRendering = 'optimizeSpeed';
        target.style.imageRendering = '-moz-crisp-edges';
        target.style.imageRendering = '-o-crisp-edges';
        target.style.imageRendering = '-webkit-optimize-contrast';
        target.style.imageRendering = 'optimize-contrast';
        target.style.imageRendering = 'crisp-edges';
        target.style.imageRendering = 'pixelated';
      }

      var dpiScale = (strategy.canvasResolutionScaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF') }}}) ? window.devicePixelRatio : 1;
      if (strategy.canvasResolutionScaleMode != {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE') }}}) {
        target.width = cssWidth * dpiScale;
        target.height = cssHeight * dpiScale;
        if (target.GLctxObject) target.GLctxObject.GLctx.viewport(0, 0, target.width, target.height);
      }
      return restoreOldStyle;
    },

    requestFullscreen: function(target, strategy) {
      // EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT + EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE is a mode where no extra logic is performed to the DOM elements.
      if (strategy.scaleMode != {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT') }}} || strategy.canvasResolutionScaleMode != {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE') }}}) {
        JSEvents.resizeCanvasForFullscreen(target, strategy);
      }

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

      if (strategy.canvasResizedCallback) {
        Runtime.dynCall('iiii', strategy.canvasResizedCallback, [{{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData]);
      }

      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    },

    fillPointerlockChangeEventData: function(eventStruct, e) {
      var pointerLockElement = document.pointerLockElement || document.mozPointerLockElement || document.webkitPointerLockElement || document.msPointerLockElement;
      var isPointerlocked = !!pointerLockElement;
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenPointerlockChangeEvent.isActive, 'isPointerlocked', 'i32') }}};
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

    registerPointerlockErrorEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!target) {
        target = document; // Pointer lock events need to be captured from 'document' by default instead of 'window'
      } else {
        target = JSEvents.findEventTarget(target);
      }

      var handlerFunc = function(event) {
        var e = event || window.event;

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, 0, userData]);
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

      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenVisibilityChangeEvent.hidden, 'document.hidden', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenVisibilityChangeEvent.visibilityState, 'visibilityState', 'i32') }}};
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

      target = JSEvents.findEventTarget(target);

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
        {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchEvent.ctrlKey, 'e.ctrlKey', 'i32') }}};
        {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchEvent.shiftKey, 'e.shiftKey', 'i32') }}};
        {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchEvent.altKey, 'e.altKey', 'i32') }}};
        {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchEvent.metaKey, 'e.metaKey', 'i32') }}};
        ptr += {{{ C_STRUCTS.EmscriptenTouchEvent.touches }}}; // Advance to the start of the touch array.
        var canvasRect = Module['canvas'] ? Module['canvas'].getBoundingClientRect() : undefined;
        var targetRect = JSEvents.getBoundingClientRectOrZeros(target);
        var numTouches = 0;
        for(var i in touches) {
          var t = touches[i];
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.identifier, 't.identifier', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.screenX, 't.screenX', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.screenY, 't.screenY', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.clientX, 't.clientX', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.clientY, 't.clientY', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.pageX, 't.pageX', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.pageY, 't.pageY', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.isChanged, 't.changed', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.onTarget, 't.onTarget', 'i32') }}};
          if (canvasRect) {
            {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.canvasX, 't.clientX - canvasRect.left', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.canvasY, 't.clientY - canvasRect.top', 'i32') }}};
          } else {
            {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.canvasX, '0', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.canvasY, '0', 'i32') }}};            
          }
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.targetX, 't.clientX - targetRect.left', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.EmscriptenTouchPoint.targetY, 't.clientY - targetRect.top', 'i32') }}};
          
          ptr += {{{ C_STRUCTS.EmscriptenTouchPoint.__size__ }}};

          if (++numTouches >= 32) {
            break;
          }
        }
        {{{ makeSetValue('JSEvents.touchEvent', C_STRUCTS.EmscriptenTouchEvent.numTouches, 'numTouches', 'i32') }}};

        var shouldCancel = Runtime.dynCall('iiii', callbackfunc, [eventTypeId, JSEvents.touchEvent, userData]);
        if (shouldCancel) {
          e.preventDefault();
        }
      };

      var eventHandler = {
        target: target,
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
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.timestamp, 'e.timestamp', 'double') }}};
      for(var i = 0; i < e.axes.length; ++i) {
        {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.EmscriptenGamepadEvent.axis, 'e.axes[i]', 'double') }}};
      }
      for(var i = 0; i < e.buttons.length; ++i) {
        if (typeof(e.buttons[i]) === 'object') {
          {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.EmscriptenGamepadEvent.analogButton, 'e.buttons[i].value', 'double') }}};
        } else {
          {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.EmscriptenGamepadEvent.analogButton, 'e.buttons[i]', 'double') }}};
        }
      }
      for(var i = 0; i < e.buttons.length; ++i) {
        if (typeof(e.buttons[i]) === 'object') {
          {{{ makeSetValue('eventStruct+i*4', C_STRUCTS.EmscriptenGamepadEvent.digitalButton, 'e.buttons[i].pressed', 'i32') }}};
        } else {
          {{{ makeSetValue('eventStruct+i*4', C_STRUCTS.EmscriptenGamepadEvent.digitalButton, 'e.buttons[i] == 1.0', 'i32') }}};
        }
      }
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.connected, 'e.connected', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.index, 'e.index', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.numAxes, 'e.axes.length', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.numButtons, 'e.buttons.length', 'i32') }}};
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
        
        if (confirmationMessage) {
          confirmationMessage = Pointer_stringify(confirmationMessage);
        }
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
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.chargingTime, 'e.chargingTime', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.dischargingTime, 'e.dischargingTime', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.level, 'e.level', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.charging, 'e.charging', 'i32') }}};
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

  emscripten_set_mouseenter_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEENTER') }}}, "mouseenter");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseleave_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSELEAVE') }}}, "mouseleave");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseover_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEOVER') }}}, "mouseover");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseout_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEOUT') }}}, "mouseout");
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
    target = JSEvents.findEventTarget(target);
    if (typeof target.onwheel !== 'undefined') {
      JSEvents.registerWheelEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WHEEL') }}}, "wheel");
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    } else if (typeof target.onmousewheel !== 'undefined') {
      JSEvents.registerWheelEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WHEEL') }}}, "mousewheel");
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
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

  _registerRestoreOldStyle: function(canvas) {
    var oldWidth = canvas.width;
    var oldHeight = canvas.height;
    var oldCssWidth = canvas.style.width;
    var oldCssHeight = canvas.style.height;
    var oldBackgroundColor = canvas.style.backgroundColor; // Chrome reads color from here.
    var oldDocumentBackgroundColor = document.body.style.backgroundColor; // IE11 reads color from here.
    // Firefox always has black background color.
    var oldPaddingLeft = canvas.style.paddingLeft; // Chrome, FF, Safari
    var oldPaddingRight = canvas.style.paddingRight;
    var oldPaddingTop = canvas.style.paddingTop;
    var oldPaddingBottom = canvas.style.paddingBottom;
    var oldMarginLeft = canvas.style.marginLeft; // IE11
    var oldMarginRight = canvas.style.marginRight;
    var oldMarginTop = canvas.style.marginTop;
    var oldMarginBottom = canvas.style.marginBottom;
    var oldDocumentBodyMargin = document.body.style.margin;
    var oldDocumentOverflow = document.documentElement.style.overflow; // Chrome, Firefox
    var oldDocumentScroll = document.body.scroll; // IE
    var oldImageRendering = canvas.style.imageRendering;

    function restoreOldStyle() {
      var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
      if (!fullscreenElement) {
        document.removeEventListener('fullscreenchange', restoreOldStyle);
        document.removeEventListener('mozfullscreenchange', restoreOldStyle);
        document.removeEventListener('webkitfullscreenchange', restoreOldStyle);
        document.removeEventListener('MSFullscreenChange', restoreOldStyle);

        canvas.width = oldWidth;
        canvas.height = oldHeight;
        canvas.style.width = oldCssWidth;
        canvas.style.height = oldCssHeight;
        canvas.style.backgroundColor = oldBackgroundColor; // Chrome
        // IE11 hack: assigning 'undefined' or an empty string to document.body.style.backgroundColor has no effect, so first assign back the default color
        // before setting the undefined value. Setting undefined value is also important, or otherwise we would later treat that as something that the user
        // had explicitly set so subsequent fullscreen transitions would not set background color properly.
        if (!oldDocumentBackgroundColor) document.body.style.backgroundColor = 'white';
        document.body.style.backgroundColor = oldDocumentBackgroundColor; // IE11
        canvas.style.paddingLeft = oldPaddingLeft; // Chrome, FF, Safari
        canvas.style.paddingRight = oldPaddingRight;
        canvas.style.paddingTop = oldPaddingTop;
        canvas.style.paddingBottom = oldPaddingBottom;
        canvas.style.marginLeft = oldMarginLeft; // IE11
        canvas.style.marginRight = oldMarginRight;
        canvas.style.marginTop = oldMarginTop;
        canvas.style.marginBottom = oldMarginBottom;
        document.body.style.margin = oldDocumentBodyMargin;
        document.documentElement.style.overflow = oldDocumentOverflow; // Chrome, Firefox
        document.body.scroll = oldDocumentScroll; // IE
        canvas.style.imageRendering = oldImageRendering;
        if (canvas.GLctxObject) canvas.GLctxObject.GLctx.viewport(0, 0, oldWidth, oldHeight);

        if (__currentFullscreenStrategy.canvasResizedCallback) {
          Runtime.dynCall('iiii', __currentFullscreenStrategy.canvasResizedCallback, [{{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData]);
        }
      }
    }
    document.addEventListener('fullscreenchange', restoreOldStyle);
    document.addEventListener('mozfullscreenchange', restoreOldStyle);
    document.addEventListener('webkitfullscreenchange', restoreOldStyle);
    document.addEventListener('MSFullscreenChange', restoreOldStyle);
    return restoreOldStyle;
  },

  // Walks the DOM tree and hides every element by setting "display: none;" except the given element.
  // Returns a list of [{node: element, displayState: oldDisplayStyle}] entries to allow restoring previous
  // visibility states after done.
  _hideEverythingExceptGivenElement: function (onlyVisibleElement) {
    var child = onlyVisibleElement;
    var parent = child.parentNode;
    var hiddenElements = [];
    while (child != document.body) {
      var children = parent.children;
      for (var i = 0; i < children.length; ++i) {
        if (children[i] != child) {
          hiddenElements.push({ node: children[i], displayState: children[i].style.display });
          children[i].style.display = 'none';
        }
      }
      child = parent;
      parent = parent.parentNode;
    }
    return hiddenElements;
  },

  // Applies old visibility states, given a list of changes returned by hideEverythingExceptGivenElement().
  _restoreHiddenElements: function(hiddenElements) {
    for (var i = 0; i < hiddenElements.length; ++i) {
      hiddenElements[i].node.style.display = hiddenElements[i].displayState;
    }
  },

  // Add letterboxes to a fullscreen element in a cross-browser way.
  _setLetterbox__deps: ['$JSEvents'],
  _setLetterbox: function(element, topBottom, leftRight) {
    if (JSEvents.isInternetExplorer()) {
      // Cannot use padding on IE11, because IE11 computes padding in addition to the size, unlike
      // other browsers, which treat padding to be part of the size.
      // e.g.
      // FF, Chrome: If CSS size = 1920x1080, padding-leftright = 460, padding-topbottomx40, then content size = (1920 - 2*460) x (1080-2*40) = 1000x1000px, and total element size = 1920x1080px.
      //       IE11: If CSS size = 1920x1080, padding-leftright = 460, padding-topbottomx40, then content size = 1920x1080px and total element size = (1920+2*460) x (1080+2*40)px.
      // IE11  treats margin like Chrome and FF treat padding.
      element.style.marginLeft = element.style.marginRight = leftRight + 'px';
      element.style.marginTop = element.style.marginBottom = topBottom + 'px';
    } else {
      // Cannot use margin to specify letterboxes in FF or Chrome, since those ignore margins in fullscreen mode.
      element.style.paddingLeft = element.style.paddingRight = leftRight + 'px';
      element.style.paddingTop = element.style.paddingBottom = topBottom + 'px';
    }
  },

  _currentFullscreenStrategy: {},
  _restoreOldWindowedStyle: null,

  _softFullscreenResizeWebGLRenderTarget__deps: ['_setLetterbox', '_currentFullscreenStrategy'],
  _softFullscreenResizeWebGLRenderTarget: function() {
    var inHiDPIFullscreenMode = __currentFullscreenStrategy.canvasResolutionScaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF') }}};
    var inAspectRatioFixedFullscreenMode = __currentFullscreenStrategy.scaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT') }}};
    var inPixelPerfectFullscreenMode = __currentFullscreenStrategy.canvasResolutionScaleMode != {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE') }}};
    var inCenteredWithoutScalingFullscreenMode = __currentFullscreenStrategy.scaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_CENTER') }}};
    var screenWidth = inHiDPIFullscreenMode ? Math.round(window.innerWidth*window.devicePixelRatio) : window.innerWidth;
    var screenHeight = inHiDPIFullscreenMode ? Math.round(window.innerHeight*window.devicePixelRatio) : window.innerHeight;
    var w = screenWidth;
    var h = screenHeight;
    var canvas = __currentFullscreenStrategy.target;
    var x = canvas.width;
    var y = canvas.height;
    var topMargin;

    if (inAspectRatioFixedFullscreenMode) {
      if (w*y < x*h) h = (w * y / x) | 0;
      else if (w*y > x*h) w = (h * x / y) | 0;
      topMargin = ((screenHeight - h) / 2) | 0;
    }

    if (inPixelPerfectFullscreenMode) {
      canvas.width = w;
      canvas.height = h;
      if (canvas.GLctxObject) canvas.GLctxObject.GLctx.viewport(0, 0, canvas.width, canvas.height);
    }

    // Back to CSS pixels.
    if (inHiDPIFullscreenMode) {
      topMargin /= window.devicePixelRatio;
      w /= window.devicePixelRatio;
      h /= window.devicePixelRatio;
      // Round to nearest 4 digits of precision.
      w = Math.round(w*1e4)/1e4;
      h = Math.round(h*1e4)/1e4;
      topMargin = Math.round(topMargin*1e4)/1e4;
    }

    if (inCenteredWithoutScalingFullscreenMode) {
      var t = (window.innerHeight - parseInt(canvas.style.height)) / 2;
      var b = (window.innerWidth - parseInt(canvas.style.width)) / 2;
      __setLetterbox(canvas, t, b);
    } else {
      canvas.style.width = w + 'px';
      canvas.style.height = h + 'px';
      var b = (window.innerWidth - w) / 2;
      __setLetterbox(canvas, topMargin, b);
    }

    if (!inCenteredWithoutScalingFullscreenMode && __currentFullscreenStrategy.canvasResizedCallback) {
      Runtime.dynCall('iiii', __currentFullscreenStrategy.canvasResizedCallback, [{{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData]);
    }
  },

  // https://developer.mozilla.org/en-US/docs/Web/Guide/API/DOM/Using_full_screen_mode  
  emscripten_do_request_fullscreen__deps: ['_setLetterbox'],
  emscripten_do_request_fullscreen: function(target, strategy) {
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
      if (strategy.deferUntilInEventHandler) {
        JSEvents.deferCall(JSEvents.requestFullscreen, 1 /* priority over pointer lock */, [target, strategy]);
        return {{{ cDefine('EMSCRIPTEN_RESULT_DEFERRED') }}};
      } else {
        return {{{ cDefine('EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED') }}};
      }
    }

    return JSEvents.requestFullscreen(target, strategy);
  },

  emscripten_request_fullscreen__deps: ['emscripten_do_request_fullscreen'],
  emscripten_request_fullscreen: function(target, deferUntilInEventHandler) {
    var strategy = {};
    // These options perform no added logic, but just bare request fullscreen.
    strategy.scaleMode = {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT') }}};
    strategy.canvasResolutionScaleMode = {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE') }}};
    strategy.filteringMode = {{{ cDefine('EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT') }}};
    strategy.deferUntilInEventHandler = deferUntilInEventHandler;

    return _emscripten_do_request_fullscreen(target, strategy);
  },

  emscripten_request_fullscreen_strategy__deps: ['emscripten_do_request_fullscreen', '_currentFullscreenStrategy', '_registerRestoreOldStyle'],
  emscripten_request_fullscreen_strategy: function(target, deferUntilInEventHandler, fullscreenStrategy) {
    var strategy = {};
    strategy.scaleMode = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.scaleMode, 'i32') }}};
    strategy.canvasResolutionScaleMode = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResolutionScaleMode, 'i32') }}};
    strategy.filteringMode = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.filteringMode, 'i32') }}};
    strategy.deferUntilInEventHandler = deferUntilInEventHandler;
    strategy.canvasResizedCallback = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallback, 'i32') }}};
    strategy.canvasResizedCallbackUserData = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackUserData, 'i32') }}};
    __currentFullscreenStrategy = strategy;

    return _emscripten_do_request_fullscreen(target, strategy);
  },

  emscripten_enter_soft_fullscreen__deps: ['_setLetterbox', '_hideEverythingExceptGivenElement', '_restoreOldWindowedStyle', '_registerRestoreOldStyle', '_restoreHiddenElements', '_currentFullscreenStrategy', '_softFullscreenResizeWebGLRenderTarget'],
  emscripten_enter_soft_fullscreen: function(target, fullscreenStrategy) {
    if (!target) target = '#canvas';
    target = JSEvents.findEventTarget(target);
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    var strategy = {};
    strategy.scaleMode = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.scaleMode, 'i32') }}};
    strategy.canvasResolutionScaleMode = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResolutionScaleMode, 'i32') }}};
    strategy.filteringMode = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.filteringMode, 'i32') }}};
    strategy.canvasResizedCallback = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallback, 'i32') }}};
    strategy.canvasResizedCallbackUserData = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackUserData, 'i32') }}};
    strategy.target = target;
    strategy.softFullscreen = true;

    var restoreOldStyle = JSEvents.resizeCanvasForFullscreen(target, strategy);

    document.documentElement.style.overflow = 'hidden';  // Firefox, Chrome
    document.body.scroll = "no"; // IE11
    document.body.style.margin = '0px'; // Override default document margin area on all browsers.

    var hiddenElements = __hideEverythingExceptGivenElement(target);

    function restoreWindowedState() {
      restoreOldStyle();
      __restoreHiddenElements(hiddenElements);
      window.removeEventListener('resize', __softFullscreenResizeWebGLRenderTarget);
      if (strategy.canvasResizedCallback) {
        Runtime.dynCall('iiii', strategy.canvasResizedCallback, [{{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData]);
      }
    }
    __restoreOldWindowedStyle = restoreWindowedState;
    __currentFullscreenStrategy = strategy;
    window.addEventListener('resize', __softFullscreenResizeWebGLRenderTarget);

    // Inform the caller that the canvas size has changed.
    if (strategy.canvasResizedCallback) {
      Runtime.dynCall('iiii', strategy.canvasResizedCallback, [{{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData]);
    }

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_exit_soft_fullscreen__deps: ['_restoreOldWindowedStyle'],
  emscripten_exit_soft_fullscreen: function() {
    if (__restoreOldWindowedStyle) __restoreOldWindowedStyle();
    __restoreOldWindowedStyle = null;

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_exit_fullscreen__deps: ['_currentFullscreenStrategy'],
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

    if (__currentFullscreenStrategy.canvasResizedCallback) {
      Runtime.dynCall('iiii', __currentFullscreenStrategy.canvasResizedCallback, [{{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData]);
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

  emscripten_set_pointerlockerror_callback: function(target, userData, useCapture, callbackfunc) {
    if (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    if (!target) target = document;
    else {
      target = JSEvents.findEventTarget(target);
      if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    }
    JSEvents.registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "pointerlockerror");
    JSEvents.registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "mozpointerlockerror");
    JSEvents.registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "webkitpointerlockerror");
    JSEvents.registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "mspointerlockerror");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_pointerlock_status: function(pointerlockStatus) {
    if (pointerlockStatus) JSEvents.fillPointerlockChangeEventData(pointerlockStatus);
    if (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
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
      var msecs = {{{ makeGetValue('msecsArray', 'i*4', 'i32') }}};
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
    if (!navigator.getGamepads && !navigator.webkitGetGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADCONNECTED') }}}, "gamepadconnected");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_gamepaddisconnected_callback: function(userData, useCapture, callbackfunc) {
    if (!navigator.getGamepads && !navigator.webkitGetGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED') }}}, "gamepaddisconnected");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
 },
  
  _emscripten_sample_gamepad_data: function() {
    // Produce a new Gamepad API sample if we are ticking a new game frame, or if not using emscripten_set_main_loop() at all to drive animation.
    if (Browser.mainLoop.currentFrameNumber !== JSEvents.lastGamepadStateFrame || !Browser.mainLoop.currentFrameNumber) {
      JSEvents.lastGamepadState = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads : null);
      JSEvents.lastGamepadStateFrame = Browser.mainLoop.currentFrameNumber;
    }
  },

  emscripten_get_num_gamepads__deps: ['_emscripten_sample_gamepad_data'],
  emscripten_get_num_gamepads: function() {
    __emscripten_sample_gamepad_data();
    if (!JSEvents.lastGamepadState) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    return JSEvents.lastGamepadState.length;
  },
  
  emscripten_get_gamepad_status__deps: ['_emscripten_sample_gamepad_data'],
  emscripten_get_gamepad_status: function(index, gamepadState) {
    __emscripten_sample_gamepad_data();
    if (!JSEvents.lastGamepadState) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};

    // INVALID_PARAM is returned on a Gamepad index that never was there.
    if (index < 0 || index >= JSEvents.lastGamepadState.length) return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};

    // NO_DATA is returned on a Gamepad index that was removed.
    // For previously disconnected gamepads there should be an empty slot (null/undefined/false) at the index.
    // This is because gamepads must keep their original position in the array.
    // For example, removing the first of two gamepads produces [null/undefined/false, gamepad].
    if (!JSEvents.lastGamepadState[index]) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};

    JSEvents.fillGamepadEventData(gamepadState, JSEvents.lastGamepadState[index]);
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
  
  emscripten_webgl_init_context_attributes: function(attributes) {
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.alpha, 1, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.depth, 1, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.stencil, 0, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.antialias, 1, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.premultipliedAlpha, 1, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer, 0, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.preferLowPowerToHighPerformance, 0, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.failIfMajorPerformanceCaveat, 0, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.majorVersion, 1, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.minorVersion, 0, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.enableExtensionsByDefault, 1, 'i32') }}};
  },

  emscripten_webgl_create_context__deps: ['$GL'],
  emscripten_webgl_create_context: function(target, attributes) {
    var contextAttributes = {};
    contextAttributes.alpha = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.alpha, 'i32') }}};
    contextAttributes.depth = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.depth, 'i32') }}};
    contextAttributes.stencil = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.stencil, 'i32') }}};
    contextAttributes.antialias = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.antialias, 'i32') }}};
    contextAttributes.premultipliedAlpha = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.premultipliedAlpha, 'i32') }}};
    contextAttributes.preserveDrawingBuffer = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer, 'i32') }}};
    contextAttributes.preferLowPowerToHighPerformance = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.preferLowPowerToHighPerformance, 'i32') }}};
    contextAttributes.failIfMajorPerformanceCaveat = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.failIfMajorPerformanceCaveat, 'i32') }}};
    contextAttributes.majorVersion = {{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.majorVersion, 'i32') }}};
    contextAttributes.minorVersion = {{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.minorVersion, 'i32') }}};
    var enableExtensionsByDefault = {{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.enableExtensionsByDefault, 'i32') }}};

    if (!target) {
      target = Module['canvas'];
    } else {
      target = JSEvents.findEventTarget(target);
    }
    var contextHandle = GL.createContext(target, contextAttributes);
    return contextHandle;
  },

  emscripten_webgl_make_context_current: function(contextHandle) {
    var success = GL.makeContextCurrent(contextHandle);
    return success ? {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}} : {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
  },

  emscripten_webgl_get_current_context: function() {
    return GL.currentContext ? GL.currentContext.handle : 0;
  },

  emscripten_webgl_destroy_context: function(contextHandle) {
    GL.deleteContext(contextHandle);
  },

  emscripten_webgl_enable_extension: function(contextHandle, extension) {
    var context = GL.getContext(contextHandle);
    var extString = Pointer_stringify(extension);
    if (extString.indexOf('GL_') == 0) extString = extString.substr(3); // Allow enabling extensions both with "GL_" prefix and without.
    var ext = context.GLctx.getExtension(extString);
    return ext ? 1 : 0;
  },

  emscripten_set_webglcontextlost_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST') }}}, "webglcontextlost");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_webglcontextrestored_callback: function(target, userData, useCapture, callbackfunc) {
    JSEvents.registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED') }}}, "webglcontextrestored");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_is_webgl_context_lost: function(target) {
    // TODO: In the future if multiple GL contexts are supported, use the 'target' parameter to find the canvas to query.
    if (!Module['ctx']) return true; // No context ~> lost context.
    return Module['ctx'].isContextLost();
  },

  emscripten_set_element_css_size: function(target, width, height) {
    if (!target) {
      target = Module['canvas'];
    } else {
      target = JSEvents.findEventTarget(target);
    }

    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    target.style.setProperty("width", width + "px");
    target.style.setProperty("height", height + "px");

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_element_css_size: function(target, width, height) {
    if (!target) {
      target = Module['canvas'];
    } else {
      target = JSEvents.findEventTarget(target);
    }

    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    if (target.getBoundingClientRect) {
      var rect = target.getBoundingClientRect();
      {{{ makeSetValue('width', '0', 'rect.right - rect.left', 'double') }}};
      {{{ makeSetValue('height', '0', 'rect.bottom - rect.top', 'double') }}};
    } else {
      {{{ makeSetValue('width', '0', 'target.clientWidth', 'double') }}};
      {{{ makeSetValue('height', '0', 'target.clientHeight', 'double') }}};
    }

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  }
};

autoAddDeps(LibraryJSEvents, '$JSEvents');
mergeInto(LibraryManager.library, LibraryJSEvents);
