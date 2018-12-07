// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var LibraryJSEvents = {
  $JSEvents__postset: 'JSEvents.staticInit();',
  $JSEvents__deps: ['_get_canvas_element_size', '_set_canvas_element_size'],
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
    numGamepadsConnected: 0, // Keep track of how many gamepads are connected, to optimize to not poll gamepads when none are connected.

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

    _onGamepadConnected: function() { ++JSEvents.numGamepadsConnected; },
    _onGamepadDisconnected: function() { --JSEvents.numGamepadsConnected; },

    staticInit: function() {
      if (typeof window !== 'undefined') {
        window.addEventListener("gamepadconnected", JSEvents._onGamepadConnected);
        window.addEventListener("gamepaddisconnected", JSEvents._onGamepadDisconnected);
        
        // Chromium does not fire the gamepadconnected event on reload, so we need to get the number of gamepads here as a workaround.
        // See https://bugs.chromium.org/p/chromium/issues/detail?id=502824
        var firstState = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads() : null);
        if (firstState) {
          JSEvents.numGamepadsConnected = firstState.length;
        }
      }
    },

    removeAllEventListeners: function() {
      for(var i = JSEvents.eventHandlers.length-1; i >= 0; --i) {
        JSEvents._removeHandler(i);
      }
      JSEvents.eventHandlers = [];
      JSEvents.deferredCalls = [];
      if (typeof window !== 'undefined') {
        window.removeEventListener("gamepadconnected", JSEvents._onGamepadConnected);
        window.removeEventListener("gamepaddisconnected", JSEvents._onGamepadDisconnected);
      }
    },

    registerRemoveEventListeners: function() {
      if (!JSEvents.removeEventListenersRegistered) {
        __ATEXIT__.push(JSEvents.removeAllEventListeners);
        JSEvents.removeEventListenersRegistered = true;
      }
    },

    // Find a DOM element with the given ID.
    findEventTarget: function(target) {
      try {
        // The sensible "default" target varies between events, but use window as the default
        // since DOM events mostly can default to that. Specific callback registrations
        // override their own defaults.
        if (!target) return window;
        if (typeof target === "number") target = Pointer_stringify(target);
        if (target === '#window') return window;
        else if (target === '#document') return document;
        else if (target === '#screen') return window.screen;
        else if (target === '#canvas') return Module['canvas'];
        return (typeof target === 'string') ? document.getElementById(target) : target;
      } catch(e) {
        // In Web Workers, some objects above, such as '#document' do not exist. Gracefully
        // return null for them.
        return null;
      }
    },

    // Like findEventTarget, but looks for OffscreenCanvas elements first
    findCanvasEventTarget: function(target) {
      if (typeof target === 'number') target = Pointer_stringify(target);
      if (!target || target === '#canvas') {
        if (typeof GL !== 'undefined' && GL.offscreenCanvases['canvas']) return GL.offscreenCanvases['canvas']; // TODO: Remove this line, target '#canvas' should refer only to Module['canvas'], not to GL.offscreenCanvases['canvas'] - but need stricter tests to be able to remove this line.
        return Module['canvas'];
      }
      if (typeof GL !== 'undefined' && GL.offscreenCanvases[target]) return GL.offscreenCanvases[target];
      return JSEvents.findEventTarget(target);
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

#if USE_PTHREADS
    queueEventHandlerOnThread_iiii: function(targetThread, eventHandlerFunc, eventTypeId, eventData, userData) {
      var stackTop = stackSave();
      var varargs = stackAlloc(12);
      {{{ makeSetValue('varargs', 0, 'eventTypeId', 'i32') }}};
      {{{ makeSetValue('varargs', 4, 'eventData', 'i32') }}};
      {{{ makeSetValue('varargs', 8, 'userData', 'i32') }}};
      _emscripten_async_queue_on_thread_(targetThread, {{{ cDefine('EM_FUNC_SIG_IIII') }}}, eventHandlerFunc, eventData, varargs);
      stackRestore(stackTop);
    },
#endif

#if USE_PTHREADS
    getTargetThreadForEventCallback: function(targetThread) {
      switch(targetThread) {
        case {{{ cDefine('EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD') }}}: return 0; // The event callback for the current event should be called on the main browser thread. (0 == don't proxy)
        case {{{ cDefine('EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD') }}}: return PThread.currentProxiedOperationCallerThread; // The event callback for the current event should be backproxied to the the thread that is registering the event.
        default: return targetThread; // The event callback for the current event should be proxied to the given specific thread.
      }
    },
#endif

    registerKeyEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.keyEvent) JSEvents.keyEvent = _malloc( {{{ C_STRUCTS.EmscriptenKeyboardEvent.__size__ }}} );

      var keyEventHandlerFunc = function(event) {
        var e = event || window.event;

#if USE_PTHREADS
        var keyEventData = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenKeyboardEvent.__size__ }}} ) : JSEvents.keyEvent; // This allocated block is passed as satellite data to the proxied function call, so the call frees up the data block when done.
#else
        var keyEventData = JSEvents.keyEvent;
#endif
        stringToUTF8(e.key ? e.key : "", keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.key }}}, {{{ cDefine('EM_HTML5_SHORT_STRING_LEN_BYTES') }}});
        stringToUTF8(e.code ? e.code : "", keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.code }}}, {{{ cDefine('EM_HTML5_SHORT_STRING_LEN_BYTES') }}});
        {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.location, 'e.location', 'i32') }}};
        {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.ctrlKey, 'e.ctrlKey', 'i32') }}};
        {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.shiftKey, 'e.shiftKey', 'i32') }}};
        {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.altKey, 'e.altKey', 'i32') }}};
        {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.metaKey, 'e.metaKey', 'i32') }}};
        {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.repeat, 'e.repeat', 'i32') }}};
        stringToUTF8(e.locale ? e.locale : "", keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.locale }}}, {{{ cDefine('EM_HTML5_SHORT_STRING_LEN_BYTES') }}});
        stringToUTF8(e.char ? e.char : "", keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.charValue }}}, {{{ cDefine('EM_HTML5_SHORT_STRING_LEN_BYTES') }}});
        {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.charCode, 'e.charCode', 'i32') }}};
        {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.keyCode, 'e.keyCode', 'i32') }}};
        {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.which, 'e.which', 'i32') }}};

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, keyEventData, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, keyEventData, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: JSEvents.isInternetExplorer() ? false : true, // MSIE doesn't allow fullscreen and pointerlock requests from key handlers, others do.
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: keyEventHandlerFunc,
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
      // wheel and mousewheel events contain wrong screenX/screenY on chrome/opera
      // https://github.com/kripken/emscripten/pull/4997
      // https://bugs.chromium.org/p/chromium/issues/detail?id=699956
      if (e.type !== 'wheel' && e.type !== 'mousewheel') {
        JSEvents.previousScreenX = e.screenX;
        JSEvents.previousScreenY = e.screenY;
      }
    },

    registerMouseEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.mouseEvent) JSEvents.mouseEvent = _malloc( {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}} );
      target = JSEvents.findEventTarget(target);

      var mouseEventHandlerFunc = function(event) {
        var e = event || window.event;

        // TODO: Make this access thread safe, or this could update live while app is reading it.
        JSEvents.fillMouseEventData(JSEvents.mouseEvent, e, target);

#if USE_PTHREADS
        if (targetThread) {
          var mouseEventData = _malloc( {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}} ); // This allocated block is passed as satellite data to the proxied function call, so the call frees up the data block when done.
          JSEvents.fillMouseEventData(mouseEventData, e, target);
          JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, mouseEventData, userData);
        } else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.mouseEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: eventTypeString != 'mousemove' && eventTypeString != 'mouseenter' && eventTypeString != 'mouseleave', // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: mouseEventHandlerFunc,
        useCapture: useCapture
      };
      // In IE, mousedown events don't either allow deferred calls to be run!
      if (JSEvents.isInternetExplorer() && eventTypeString == 'mousedown') eventHandler.allowsDeferredCalls = false;
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerWheelEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.wheelEvent) JSEvents.wheelEvent = _malloc( {{{ C_STRUCTS.EmscriptenWheelEvent.__size__ }}} );
      target = JSEvents.findEventTarget(target);


      // The DOM Level 3 events spec event 'wheel'
      var wheelHandlerFunc = function(event) {
        var e = event || window.event;
#if USE_PTHREADS
        var wheelEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenWheelEvent.__size__ }}} ) : JSEvents.wheelEvent; // This allocated block is passed as satellite data to the proxied function call, so the call frees up the data block when done.
#else
        var wheelEvent = JSEvents.wheelEvent;
#endif
        JSEvents.fillMouseEventData(wheelEvent, e, target);
        {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaX, 'e["deltaX"]', 'double') }}};
        {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaY, 'e["deltaY"]', 'double') }}};
        {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaZ, 'e["deltaZ"]', 'double') }}};
        {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaMode, 'e["deltaMode"]', 'i32') }}};
#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, wheelEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, wheelEvent, userData)) e.preventDefault();
      };
      // The 'mousewheel' event as implemented in Safari 6.0.5
      var mouseWheelHandlerFunc = function(event) {
        var e = event || window.event;
        JSEvents.fillMouseEventData(JSEvents.wheelEvent, e, target);
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaX, 'e["wheelDeltaX"] || 0', 'double') }}};
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaY, '-(e["wheelDeltaY"] ? e["wheelDeltaY"] : e["wheelDelta"]) /* 1. Invert to unify direction with the DOM Level 3 wheel event. 2. MSIE does not provide wheelDeltaY, so wheelDelta is used as a fallback. */', 'double') }}};
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaZ, '0 /* Not available */', 'double') }}};
        {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaMode, '0 /* DOM_DELTA_PIXEL */', 'i32') }}};
        var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.wheelEvent, userData);
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

    registerUiEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.uiEvent) JSEvents.uiEvent = _malloc( {{{ C_STRUCTS.EmscriptenUiEvent.__size__ }}} );

      if (eventTypeString == "scroll" && !target) {
        target = document; // By default read scroll events on document rather than window.
      } else {
        target = JSEvents.findEventTarget(target);
      }

      var uiEventHandlerFunc = function(event) {
        var e = event || window.event;
        if (e.target != target) {
          // Never take ui events such as scroll via a 'bubbled' route, but always from the direct element that
          // was targeted. Otherwise e.g. if app logs a message in response to a page scroll, the Emscripten log
          // message box could cause to scroll, generating a new (bubbled) scroll message, causing a new log print,
          // causing a new scroll, etc..
          return;
        }
        var scrollPos = JSEvents.pageScrollPos();

#if USE_PTHREADS
        var uiEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenUiEvent.__size__ }}} ) : JSEvents.uiEvent;
#else
        var uiEvent = JSEvents.uiEvent;
#endif
        {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.detail, 'e.detail', 'i32') }}};
        {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientWidth, 'document.body.clientWidth', 'i32') }}};
        {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientHeight, 'document.body.clientHeight', 'i32') }}};
        {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerWidth, 'window.innerWidth', 'i32') }}};
        {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerHeight, 'window.innerHeight', 'i32') }}};
        {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterWidth, 'window.outerWidth', 'i32') }}};
        {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterHeight, 'window.outerHeight', 'i32') }}};
        {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollTop, 'scrollPos[0]', 'i32') }}};
        {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollLeft, 'scrollPos[1]', 'i32') }}};
#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, uiEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, uiEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false, // Neither scroll or resize events allow running requests inside them.
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: uiEventHandlerFunc,
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

    registerFocusEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.focusEvent) JSEvents.focusEvent = _malloc( {{{ C_STRUCTS.EmscriptenFocusEvent.__size__ }}} );

      var focusEventHandlerFunc = function(event) {
        var e = event || window.event;

        var nodeName = JSEvents.getNodeNameForTarget(e.target);
        var id = e.target.id ? e.target.id : '';

#if USE_PTHREADS
        var focusEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenFocusEvent.__size__ }}} ) : JSEvents.focusEvent;
#else
        var focusEvent = JSEvents.focusEvent;
#endif
        stringToUTF8(nodeName, focusEvent + {{{ C_STRUCTS.EmscriptenFocusEvent.nodeName }}}, {{{ cDefine('EM_HTML5_LONG_STRING_LEN_BYTES') }}});
        stringToUTF8(id, focusEvent + {{{ C_STRUCTS.EmscriptenFocusEvent.id }}}, {{{ cDefine('EM_HTML5_LONG_STRING_LEN_BYTES') }}});

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, focusEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, focusEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: focusEventHandlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    tick: function() {
      if (window['performance'] && window['performance']['now']) return window['performance']['now']();
      else return Date.now();
    },

    fillDeviceOrientationEventData: function(eventStruct, e, target) {
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.timestamp, 'JSEvents.tick()', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.alpha, 'e.alpha', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.beta, 'e.beta', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.gamma, 'e.gamma', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.absolute, 'e.absolute', 'i32') }}};
    },

    registerDeviceOrientationEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.deviceOrientationEvent) JSEvents.deviceOrientationEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceOrientationEvent.__size__ }}} );

      var deviceOrientationEventHandlerFunc = function(event) {
        var e = event || window.event;

        JSEvents.fillDeviceOrientationEventData(JSEvents.deviceOrientationEvent, e, target); // TODO: Thread-safety with respect to emscripten_get_deviceorientation_status()

#if USE_PTHREADS
        if (targetThread) {
          var deviceOrientationEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceOrientationEvent.__size__ }}} );
          JSEvents.fillDeviceOrientationEventData(deviceOrientationEvent, e, target);
          JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, deviceOrientationEvent, userData);
        } else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.deviceOrientationEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: deviceOrientationEventHandlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    fillDeviceMotionEventData: function(eventStruct, e, target) {
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.timestamp, 'JSEvents.tick()', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationX, 'e.acceleration.x', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationY, 'e.acceleration.y', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationZ, 'e.acceleration.z', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityX, 'e.accelerationIncludingGravity.x', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityY, 'e.accelerationIncludingGravity.y', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityZ, 'e.accelerationIncludingGravity.z', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateAlpha, 'e.rotationRate.alpha', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateBeta, 'e.rotationRate.beta', 'double') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateGamma, 'e.rotationRate.gamma', 'double') }}};
    },

    registerDeviceMotionEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.deviceMotionEvent) JSEvents.deviceMotionEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}} );

      var deviceMotionEventHandlerFunc = function(event) {
        var e = event || window.event;

        JSEvents.fillDeviceMotionEventData(JSEvents.deviceMotionEvent, e, target); // TODO: Thread-safety with respect to emscripten_get_devicemotion_status()

#if USE_PTHREADS
        if (targetThread) {
          var deviceMotionEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}} );
          JSEvents.fillDeviceMotionEventData(deviceMotionEvent, e, target);
          JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, deviceMotionEvent, userData);
        } else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.deviceMotionEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: deviceMotionEventHandlerFunc,
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

    registerOrientationChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.orientationChangeEvent) JSEvents.orientationChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenOrientationChangeEvent.__size__ }}} );

      if (!target) {
        target = window.screen; // Orientation events need to be captured from 'window.screen' instead of 'window'
      } else {
        target = JSEvents.findEventTarget(target);
      }

      var orientationChangeEventHandlerFunc = function(event) {
        var e = event || window.event;

#if USE_PTHREADS
        var orientationChangeEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}} ) : JSEvents.orientationChangeEvent;
#else
        var orientationChangeEvent = JSEvents.orientationChangeEvent;
#endif

        JSEvents.fillOrientationChangeEventData(orientationChangeEvent, e);

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, orientationChangeEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, orientationChangeEvent, userData)) e.preventDefault();
      };

      if (eventTypeString == "orientationchange" && window.screen.mozOrientation !== undefined) {
        eventTypeString = "mozorientationchange";
      }

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: orientationChangeEventHandlerFunc,
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
      stringToUTF8(nodeName, eventStruct + {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.nodeName }}}, {{{ cDefine('EM_HTML5_LONG_STRING_LEN_BYTES') }}});
      stringToUTF8(id, eventStruct + {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.id }}}, {{{ cDefine('EM_HTML5_LONG_STRING_LEN_BYTES') }}});
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.elementWidth, 'reportedElement ? reportedElement.clientWidth : 0', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.elementHeight, 'reportedElement ? reportedElement.clientHeight : 0', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.screenWidth, 'screen.width', 'i32') }}};
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.screenHeight, 'screen.height', 'i32') }}};
      if (isFullscreen) {
        JSEvents.previousFullscreenElement = fullscreenElement;
      }
    },

    registerFullscreenChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.fullscreenChangeEvent) JSEvents.fullscreenChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.__size__ }}} );

      if (!target) target = document; // Fullscreen change events need to be captured from 'document' by default instead of 'window'
      else target = JSEvents.findEventTarget(target);

      var fullscreenChangeEventhandlerFunc = function(event) {
        var e = event || window.event;

#if USE_PTHREADS
        var fullscreenChangeEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.__size__ }}} ) : JSEvents.fullscreenChangeEvent;
#else
        var fullscreenChangeEvent = JSEvents.fullscreenChangeEvent;
#endif

        JSEvents.fillFullscreenChangeEventData(fullscreenChangeEvent, e);

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, fullscreenChangeEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, fullscreenChangeEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: fullscreenChangeEventhandlerFunc,
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
      var canvasSize = __get_canvas_element_size(target);
      var windowedRttWidth = canvasSize[0];
      var windowedRttHeight = canvasSize[1];

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
        var newWidth = (cssWidth * dpiScale)|0;
        var newHeight = (cssHeight * dpiScale)|0;
        __set_canvas_element_size(target, newWidth, newHeight);
        if (target.GLctxObject) target.GLctxObject.GLctx.viewport(0, 0, newWidth, newHeight);
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
#if USE_PTHREADS
        if (strategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
        else
#endif
        Module['dynCall_iiii'](strategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
      }

      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    },

    fillPointerlockChangeEventData: function(eventStruct, e) {
      var pointerLockElement = document.pointerLockElement || document.mozPointerLockElement || document.webkitPointerLockElement || document.msPointerLockElement;
      var isPointerlocked = !!pointerLockElement;
      {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenPointerlockChangeEvent.isActive, 'isPointerlocked', 'i32') }}};
      var nodeName = JSEvents.getNodeNameForTarget(pointerLockElement);
      var id = (pointerLockElement && pointerLockElement.id) ? pointerLockElement.id : '';
      stringToUTF8(nodeName, eventStruct + {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.nodeName }}}, {{{ cDefine('EM_HTML5_LONG_STRING_LEN_BYTES') }}});
      stringToUTF8(id, eventStruct + {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.id }}}, {{{ cDefine('EM_HTML5_LONG_STRING_LEN_BYTES') }}});
    },

    registerPointerlockChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.pointerlockChangeEvent) JSEvents.pointerlockChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.__size__ }}} );

      if (!target) target = document; // Pointer lock change events need to be captured from 'document' by default instead of 'window'
      else target = JSEvents.findEventTarget(target);

      var pointerlockChangeEventHandlerFunc = function(event) {
        var e = event || window.event;

#if USE_PTHREADS
        var pointerlockChangeEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.__size__ }}} ) : JSEvents.pointerlockChangeEvent;
#else
        var pointerlockChangeEvent = JSEvents.pointerlockChangeEvent;
#endif
        JSEvents.fillPointerlockChangeEventData(pointerlockChangeEvent, e);

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, pointerlockChangeEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, pointerlockChangeEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: pointerlockChangeEventHandlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerPointerlockErrorEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      if (!target) target = document; // Pointer lock events need to be captured from 'document' by default instead of 'window'
      else target = JSEvents.findEventTarget(target);

      var pointerlockErrorEventHandlerFunc = function(event) {
        var e = event || window.event;

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, 0, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, 0, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: pointerlockErrorEventHandlerFunc,
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

    registerVisibilityChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.visibilityChangeEvent) JSEvents.visibilityChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenVisibilityChangeEvent.__size__ }}} );

      if (!target) target = document; // Visibility change events need to be captured from 'document' by default instead of 'window'
      else target = JSEvents.findEventTarget(target);

      var visibilityChangeEventHandlerFunc = function(event) {
        var e = event || window.event;

#if USE_PTHREADS
        var visibilityChangeEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenVisibilityChangeEvent.__size__ }}} ) : JSEvents.visibilityChangeEvent;
#else
        var visibilityChangeEvent = JSEvents.visibilityChangeEvent;
#endif

        JSEvents.fillVisibilityChangeEventData(visibilityChangeEvent, e);

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, visibilityChangeEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, visibilityChangeEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: visibilityChangeEventHandlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerTouchEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.touchEvent) JSEvents.touchEvent = _malloc( {{{ C_STRUCTS.EmscriptenTouchEvent.__size__ }}} );

      target = JSEvents.findEventTarget(target);

      var touchEventHandlerFunc = function(event) {
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

#if USE_PTHREADS
        var touchEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenTouchEvent.__size__ }}} ) : JSEvents.touchEvent;
#else
        var touchEvent = JSEvents.touchEvent;
#endif
        var ptr = touchEvent;
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
        {{{ makeSetValue('touchEvent', C_STRUCTS.EmscriptenTouchEvent.numTouches, 'numTouches', 'i32') }}};

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, touchEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, touchEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: target,
        allowsDeferredCalls: eventTypeString == 'touchstart' || eventTypeString == 'touchend',
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: touchEventHandlerFunc,
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
      stringToUTF8(e.id, eventStruct + {{{ C_STRUCTS.EmscriptenGamepadEvent.id }}}, {{{ cDefine('EM_HTML5_MEDIUM_STRING_LEN_BYTES') }}});
      stringToUTF8(e.mapping, eventStruct + {{{ C_STRUCTS.EmscriptenGamepadEvent.mapping }}}, {{{ cDefine('EM_HTML5_MEDIUM_STRING_LEN_BYTES') }}});
    },

    registerGamepadEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.gamepadEvent) JSEvents.gamepadEvent = _malloc( {{{ C_STRUCTS.EmscriptenGamepadEvent.__size__ }}} );

      var gamepadEventHandlerFunc = function(event) {
        var e = event || window.event;

#if USE_PTHREADS
        var gamepadEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenGamepadEvent.__size__ }}} ) : JSEvents.gamepadEvent;
#else
        var gamepadEvent = JSEvents.gamepadEvent;
#endif
        JSEvents.fillGamepadEventData(gamepadEvent, e.gamepad);

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, gamepadEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, gamepadEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: true,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: gamepadEventHandlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerBeforeUnloadEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
      var beforeUnloadEventHandlerFunc = function(event) {
        var e = event || window.event;

        // Note: This is always called on the main browser thread, since it needs synchronously return a value!
        var confirmationMessage = Module['dynCall_iiii'](callbackfunc, eventTypeId, 0, userData);
        
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
        handlerFunc: beforeUnloadEventHandlerFunc,
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
    
    registerBatteryEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!JSEvents.batteryEvent) JSEvents.batteryEvent = _malloc( {{{ C_STRUCTS.EmscriptenBatteryEvent.__size__ }}} );

      var batteryEventHandlerFunc = function(event) {
        var e = event || window.event;

#if USE_PTHREADS
        var batteryEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenBatteryEvent.__size__ }}} ) : JSEvents.batteryEvent;
#else
        var batteryEvent = JSEvents.batteryEvent;
#endif
        JSEvents.fillBatteryEventData(batteryEvent, JSEvents.battery());

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, batteryEvent, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, batteryEvent, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: batteryEventHandlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },

    registerWebGlEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
      targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
      if (!target) target = Module['canvas'];

      var webGlEventHandlerFunc = function(event) {
        var e = event || window.event;

#if USE_PTHREADS
        if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, 0, userData);
        else
#endif
        if (Module['dynCall_iiii'](callbackfunc, eventTypeId, 0, userData)) e.preventDefault();
      };

      var eventHandler = {
        target: JSEvents.findEventTarget(target),
        allowsDeferredCalls: false,
        eventTypeString: eventTypeString,
        callbackfunc: callbackfunc,
        handlerFunc: webGlEventHandlerFunc,
        useCapture: useCapture
      };
      JSEvents.registerOrRemoveHandler(eventHandler);
    },
  },

  emscripten_set_keypress_callback_on_thread__proxy: 'sync',
  emscripten_set_keypress_callback_on_thread__sig: 'iiiiii',
  emscripten_set_keypress_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYPRESS') }}}, "keypress", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_keydown_callback_on_thread__proxy: 'sync',
  emscripten_set_keydown_callback_on_thread__sig: 'iiiiii',
  emscripten_set_keydown_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYDOWN') }}}, "keydown", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_keyup_callback_on_thread__proxy: 'sync',
  emscripten_set_keyup_callback_on_thread__sig: 'iiiiii',
  emscripten_set_keyup_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYUP') }}}, "keyup", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_click_callback_on_thread__proxy: 'sync',
  emscripten_set_click_callback_on_thread__sig: 'iiiiii',
  emscripten_set_click_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_CLICK') }}}, "click", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mousedown_callback_on_thread__proxy: 'sync',
  emscripten_set_mousedown_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mousedown_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEDOWN') }}}, "mousedown", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseup_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseup_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseup_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEUP') }}}, "mouseup", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_dblclick_callback_on_thread__proxy: 'sync',
  emscripten_set_dblclick_callback_on_thread__sig: 'iiiiii',
  emscripten_set_dblclick_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DBLCLICK') }}}, "dblclick", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mousemove_callback_on_thread__proxy: 'sync',
  emscripten_set_mousemove_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mousemove_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEMOVE') }}}, "mousemove", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseenter_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseenter_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseenter_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEENTER') }}}, "mouseenter", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseleave_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseleave_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseleave_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSELEAVE') }}}, "mouseleave", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseover_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseover_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseover_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEOVER') }}}, "mouseover", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseout_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseout_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseout_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEOUT') }}}, "mouseout", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_mouse_status__proxy: 'sync',
  emscripten_get_mouse_status__sig: 'ii',
  emscripten_get_mouse_status: function(mouseState) {
    if (!JSEvents.mouseEvent) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    // HTML5 does not really have a polling API for mouse events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler to any of the mouse function.
    HEAP8.set(HEAP8.subarray(JSEvents.mouseEvent, JSEvents.mouseEvent + {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}}), mouseState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_wheel_callback_on_thread__proxy: 'sync',
  emscripten_set_wheel_callback_on_thread__sig: 'iiiiii',
  emscripten_set_wheel_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    target = JSEvents.findEventTarget(target);
    if (typeof target.onwheel !== 'undefined') {
      JSEvents.registerWheelEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WHEEL') }}}, "wheel", targetThread);
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    } else if (typeof target.onmousewheel !== 'undefined') {
      JSEvents.registerWheelEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WHEEL') }}}, "mousewheel", targetThread);
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
  },

  emscripten_set_resize_callback_on_thread__proxy: 'sync',
  emscripten_set_resize_callback_on_thread__sig: 'iiiiii',
  emscripten_set_resize_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_RESIZE') }}}, "resize", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_scroll_callback_on_thread__proxy: 'sync',
  emscripten_set_scroll_callback_on_thread__sig: 'iiiiii',
  emscripten_set_scroll_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_SCROLL') }}}, "scroll", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_blur_callback_on_thread__proxy: 'sync',
  emscripten_set_blur_callback_on_thread__sig: 'iiiiii',
  emscripten_set_blur_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BLUR') }}}, "blur", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focus_callback_on_thread__proxy: 'sync',
  emscripten_set_focus_callback_on_thread__sig: 'iiiiii',
  emscripten_set_focus_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUS') }}}, "focus", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focusin_callback_on_thread__proxy: 'sync',
  emscripten_set_focusin_callback_on_thread__sig: 'iiiiii',
  emscripten_set_focusin_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUSIN') }}}, "focusin", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focusout_callback_on_thread__proxy: 'sync',
  emscripten_set_focusout_callback_on_thread__sig: 'iiiiii',
  emscripten_set_focusout_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUSOUT') }}}, "focusout", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_deviceorientation_callback_on_thread__proxy: 'sync',
  emscripten_set_deviceorientation_callback_on_thread__sig: 'iiiii',
  emscripten_set_deviceorientation_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerDeviceOrientationEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DEVICEORIENTATION') }}}, "deviceorientation", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_deviceorientation_status__proxy: 'sync',
  emscripten_get_deviceorientation_status__sig: 'ii',
  emscripten_get_deviceorientation_status: function(orientationState) {
    if (!JSEvents.deviceOrientationEvent) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    // HTML5 does not really have a polling API for device orientation events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler.
    HEAP32.set(HEAP32.subarray(JSEvents.deviceOrientationEvent, {{{ C_STRUCTS.EmscriptenDeviceOrientationEvent.__size__ }}}), orientationState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_devicemotion_callback_on_thread__proxy: 'sync',
  emscripten_set_devicemotion_callback_on_thread__sig: 'iiiii',
  emscripten_set_devicemotion_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerDeviceMotionEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DEVICEMOTION') }}}, "devicemotion", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_devicemotion_status__proxy: 'sync',
  emscripten_get_devicemotion_status__sig: 'ii',
  emscripten_get_devicemotion_status: function(motionState) {
    if (!JSEvents.deviceMotionEvent) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    // HTML5 does not really have a polling API for device motion events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler.
    HEAP32.set(HEAP32.subarray(JSEvents.deviceMotionEvent, {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}}), motionState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_orientationchange_callback_on_thread__proxy: 'sync',
  emscripten_set_orientationchange_callback_on_thread__sig: 'iiiii',
  emscripten_set_orientationchange_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    if (!window.screen || !window.screen.addEventListener) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.registerOrientationChangeEventCallback(window.screen, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_ORIENTATIONCHANGE') }}}, "orientationchange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_get_orientation_status__proxy: 'sync',
  emscripten_get_orientation_status__sig: 'ii',
  emscripten_get_orientation_status: function(orientationChangeEvent) {
    if (!JSEvents.screenOrientation() && typeof window.orientation === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.fillOrientationChangeEventData(orientationChangeEvent);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_lock_orientation__proxy: 'sync',
  emscripten_lock_orientation__sig: 'ii',
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
  
  emscripten_unlock_orientation__proxy: 'sync',
  emscripten_unlock_orientation__sig: 'i',
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

  emscripten_set_fullscreenchange_callback_on_thread__proxy: 'sync',
  emscripten_set_fullscreenchange_callback_on_thread__sig: 'iiiiii',
  emscripten_set_fullscreenchange_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    if (typeof JSEvents.fullscreenEnabled() === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    if (!target) target = document;
    else {
      target = JSEvents.findEventTarget(target);
      if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    }
    JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "fullscreenchange", targetThread);
    JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "mozfullscreenchange", targetThread);
    JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "webkitfullscreenchange", targetThread);
    JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "msfullscreenchange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_fullscreen_status__proxy: 'sync',
  emscripten_get_fullscreen_status__sig: 'ii',
  emscripten_get_fullscreen_status: function(fullscreenStatus) {
    if (typeof JSEvents.fullscreenEnabled() === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.fillFullscreenChangeEventData(fullscreenStatus);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _registerRestoreOldStyle__deps: ['_get_canvas_element_size', '_set_canvas_element_size'],
  _registerRestoreOldStyle: function(canvas) {
    var canvasSize = __get_canvas_element_size(canvas);
    var oldWidth = canvasSize[0];
    var oldHeight = canvasSize[1];
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

        __set_canvas_element_size(canvas, oldWidth, oldHeight);

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
#if USE_PTHREADS
          if (__currentFullscreenStrategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(__currentFullscreenStrategy.canvasResizedCallbackTargetThread, __currentFullscreenStrategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData);
          else
#endif
          Module['dynCall_iiii'](__currentFullscreenStrategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData);
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

  _softFullscreenResizeWebGLRenderTarget__deps: ['_setLetterbox', '_currentFullscreenStrategy', '_get_canvas_element_size', '_set_canvas_element_size'],
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
    var canvasSize = __get_canvas_element_size(canvas);
    var x = canvasSize[0];
    var y = canvasSize[1];
    var topMargin;

    if (inAspectRatioFixedFullscreenMode) {
      if (w*y < x*h) h = (w * y / x) | 0;
      else if (w*y > x*h) w = (h * x / y) | 0;
      topMargin = ((screenHeight - h) / 2) | 0;
    }

    if (inPixelPerfectFullscreenMode) {
      __set_canvas_element_size(canvas, w, h);
      if (canvas.GLctxObject) canvas.GLctxObject.GLctx.viewport(0, 0, w, h);
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
#if USE_PTHREADS
      if (__currentFullscreenStrategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(__currentFullscreenStrategy.canvasResizedCallbackTargetThread, __currentFullscreenStrategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData);
      else
#endif
      Module['dynCall_iiii'](__currentFullscreenStrategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData);
    }
  },

  // https://developer.mozilla.org/en-US/docs/Web/Guide/API/DOM/Using_full_screen_mode  
  _emscripten_do_request_fullscreen__deps: ['_setLetterbox', 'emscripten_set_canvas_element_size', 'emscripten_get_canvas_element_size', '_get_canvas_element_size', '_set_canvas_element_size'],
  _emscripten_do_request_fullscreen: function(target, strategy) {
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

  emscripten_request_fullscreen__deps: ['_emscripten_do_request_fullscreen'],
  emscripten_request_fullscreen__proxy: 'sync',
  emscripten_request_fullscreen__sig: 'iii',
  emscripten_request_fullscreen: function(target, deferUntilInEventHandler) {
    var strategy = {};
    // These options perform no added logic, but just bare request fullscreen.
    strategy.scaleMode = {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT') }}};
    strategy.canvasResolutionScaleMode = {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE') }}};
    strategy.filteringMode = {{{ cDefine('EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT') }}};
    strategy.deferUntilInEventHandler = deferUntilInEventHandler;
    strategy.canvasResizedCallbackTargetThread = {{{ cDefine('EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD') }}};

    return __emscripten_do_request_fullscreen(target, strategy);
  },

  emscripten_request_fullscreen_strategy__deps: ['_emscripten_do_request_fullscreen', '_currentFullscreenStrategy', '_registerRestoreOldStyle'],
  emscripten_request_fullscreen_strategy__proxy: 'sync',
  emscripten_request_fullscreen_strategy__sig: 'iiii',
  emscripten_request_fullscreen_strategy: function(target, deferUntilInEventHandler, fullscreenStrategy) {
    var strategy = {};
    strategy.scaleMode = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.scaleMode, 'i32') }}};
    strategy.canvasResolutionScaleMode = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResolutionScaleMode, 'i32') }}};
    strategy.filteringMode = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.filteringMode, 'i32') }}};
    strategy.deferUntilInEventHandler = deferUntilInEventHandler;
    strategy.canvasResizedCallback = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallback, 'i32') }}};
    strategy.canvasResizedCallbackUserData = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackUserData, 'i32') }}};
#if USE_PTHREADS
    strategy.canvasResizedCallbackTargetThread = {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackTargetThread, 'i32') }}};
#endif
    __currentFullscreenStrategy = strategy;

    return __emscripten_do_request_fullscreen(target, strategy);
  },

  emscripten_enter_soft_fullscreen__deps: ['_setLetterbox', '_hideEverythingExceptGivenElement', '_restoreOldWindowedStyle', '_registerRestoreOldStyle', '_restoreHiddenElements', '_currentFullscreenStrategy', '_softFullscreenResizeWebGLRenderTarget', '_get_canvas_element_size', '_set_canvas_element_size'],
  emscripten_enter_soft_fullscreen__proxy: 'sync',
  emscripten_enter_soft_fullscreen__sig: 'iii',
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
#if USE_PTHREADS
    strategy.canvasResizedCallbackTargetThread = JSEvents.getTargetThreadForEventCallback();
#endif
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
#if USE_PTHREADS
        if (strategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
        else
#endif
        Module['dynCall_iiii'](strategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
      }
    }
    __restoreOldWindowedStyle = restoreWindowedState;
    __currentFullscreenStrategy = strategy;
    window.addEventListener('resize', __softFullscreenResizeWebGLRenderTarget);

    // Inform the caller that the canvas size has changed.
    if (strategy.canvasResizedCallback) {
#if USE_PTHREADS
      if (strategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
      else
#endif
      Module['dynCall_iiii'](strategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
    }

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_exit_soft_fullscreen__deps: ['_restoreOldWindowedStyle'],
  emscripten_exit_soft_fullscreen__proxy: 'sync',
  emscripten_exit_soft_fullscreen__sig: 'i',
  emscripten_exit_soft_fullscreen: function() {
    if (__restoreOldWindowedStyle) __restoreOldWindowedStyle();
    __restoreOldWindowedStyle = null;

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_exit_fullscreen__deps: ['_currentFullscreenStrategy'],
  emscripten_exit_fullscreen__proxy: 'sync',
  emscripten_exit_fullscreen__sig: 'i',
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
#if USE_PTHREADS
        if (__currentFullscreenStrategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(__currentFullscreenStrategy.canvasResizedCallbackTargetThread, __currentFullscreenStrategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData);
        else
#endif
      Module['dynCall_iiii'](__currentFullscreenStrategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData);
    }

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_pointerlockchange_callback_on_thread__proxy: 'sync',
  emscripten_set_pointerlockchange_callback_on_thread__sig: 'iiiiii',
  emscripten_set_pointerlockchange_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    // TODO: Currently not supported in pthreads or in --proxy-to-worker mode. (In pthreads mode, document object is not defined)
    if (!document || !document.body || (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock)) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    if (!target) target = document;
    else {
      target = JSEvents.findEventTarget(target);
      if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    }
    JSEvents.registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "pointerlockchange", targetThread);
    JSEvents.registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "mozpointerlockchange", targetThread);
    JSEvents.registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "webkitpointerlockchange", targetThread);
    JSEvents.registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "mspointerlockchange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_pointerlockerror_callback_on_thread__proxy: 'sync',
  emscripten_set_pointerlockerror_callback_on_thread__sig: 'iiiiii',
  emscripten_set_pointerlockerror_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    // TODO: Currently not supported in pthreads or in --proxy-to-worker mode. (In pthreads mode, document object is not defined)
    if (!document || !document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    if (!target) target = document;
    else {
      target = JSEvents.findEventTarget(target);
      if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    }
    JSEvents.registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "pointerlockerror", targetThread);
    JSEvents.registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "mozpointerlockerror", targetThread);
    JSEvents.registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "webkitpointerlockerror", targetThread);
    JSEvents.registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "mspointerlockerror", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_pointerlock_status__proxy: 'sync',
  emscripten_get_pointerlock_status__sig: 'ii',
  emscripten_get_pointerlock_status: function(pointerlockStatus) {
    if (pointerlockStatus) JSEvents.fillPointerlockChangeEventData(pointerlockStatus);
    if (!document.body || (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock)) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_request_pointerlock__proxy: 'sync',
  emscripten_request_pointerlock__sig: 'iii',
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

  emscripten_exit_pointerlock__proxy: 'sync',
  emscripten_exit_pointerlock__sig: 'i',
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
  
  emscripten_vibrate__proxy: 'sync',
  emscripten_vibrate__sig: 'ii',
  emscripten_vibrate: function(msecs) {
    if (!navigator.vibrate) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};    
    navigator.vibrate(msecs);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_vibrate_pattern__proxy: 'sync',
  emscripten_vibrate_pattern__sig: 'iii',
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

  emscripten_set_visibilitychange_callback_on_thread__proxy: 'sync',
  emscripten_set_visibilitychange_callback_on_thread__sig: 'iiiii',
  emscripten_set_visibilitychange_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerVisibilityChangeEventCallback(document, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_VISIBILITYCHANGE') }}}, "visibilitychange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_visibility_status__proxy: 'sync',
  emscripten_get_visibility_status__sig: 'ii',
  emscripten_get_visibility_status: function(visibilityStatus) {
    if (typeof document.visibilityState === 'undefined' && typeof document.hidden === 'undefined') {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    JSEvents.fillVisibilityChangeEventData(visibilityStatus);  
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchstart_callback_on_thread__proxy: 'sync',
  emscripten_set_touchstart_callback_on_thread__sig: 'iiiiii',
  emscripten_set_touchstart_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHSTART') }}}, "touchstart", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchend_callback_on_thread__proxy: 'sync',
  emscripten_set_touchend_callback_on_thread__sig: 'iiiiii',
  emscripten_set_touchend_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHEND') }}}, "touchend", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchmove_callback_on_thread__proxy: 'sync',
  emscripten_set_touchmove_callback_on_thread__sig: 'iiiiii',
  emscripten_set_touchmove_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHMOVE') }}}, "touchmove", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchcancel_callback_on_thread__proxy: 'sync',
  emscripten_set_touchcancel_callback_on_thread__sig: 'iiiiii',
  emscripten_set_touchcancel_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHCANCEL') }}}, "touchcancel", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_gamepadconnected_callback_on_thread__proxy: 'sync',
  emscripten_set_gamepadconnected_callback_on_thread__sig: 'iiiii',
  emscripten_set_gamepadconnected_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    if (!navigator.getGamepads && !navigator.webkitGetGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADCONNECTED') }}}, "gamepadconnected", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_gamepaddisconnected_callback_on_thread__proxy: 'sync',
  emscripten_set_gamepaddisconnected_callback_on_thread__sig: 'iiiii',
  emscripten_set_gamepaddisconnected_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    if (!navigator.getGamepads && !navigator.webkitGetGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    JSEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED') }}}, "gamepaddisconnected", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
 },
  
  _emscripten_sample_gamepad_data: function() {
    // Polling gamepads generates garbage, so don't do it when we know there are no gamepads connected.
    if (!JSEvents.numGamepadsConnected) return;

    // Produce a new Gamepad API sample if we are ticking a new game frame, or if not using emscripten_set_main_loop() at all to drive animation.
    if (Browser.mainLoop.currentFrameNumber !== JSEvents.lastGamepadStateFrame || !Browser.mainLoop.currentFrameNumber) {
      JSEvents.lastGamepadState = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads : null);
      JSEvents.lastGamepadStateFrame = Browser.mainLoop.currentFrameNumber;
    }
  },

  emscripten_get_num_gamepads__deps: ['_emscripten_sample_gamepad_data'],
  emscripten_get_num_gamepads__proxy: 'sync',
  emscripten_get_num_gamepads__sig: 'i',
  emscripten_get_num_gamepads: function() {
    // Polling gamepads generates garbage, so don't do it when we know there are no gamepads connected.
    if (!JSEvents.numGamepadsConnected) return 0;

    __emscripten_sample_gamepad_data();
    if (!JSEvents.lastGamepadState) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    return JSEvents.lastGamepadState.length;
  },
  
  emscripten_get_gamepad_status__deps: ['_emscripten_sample_gamepad_data'],
  emscripten_get_gamepad_status__proxy: 'sync',
  emscripten_get_gamepad_status__sig: 'iii',
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
  
  emscripten_set_beforeunload_callback_on_thread__proxy: 'sync',
  emscripten_set_beforeunload_callback_on_thread__sig: 'iii',
  emscripten_set_beforeunload_callback_on_thread: function(userData, callbackfunc, targetThread) {
    if (typeof window.onbeforeunload === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    // beforeunload callback can only be registered on the main browser thread, because the page will go away immediately after returning from the handler,
    // and there is no time to start proxying it anywhere.
    if (targetThread !== {{{ cDefine('EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD') }}}) return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
    JSEvents.registerBeforeUnloadEventCallback(window, userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BEFOREUNLOAD') }}}, "beforeunload"); 
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_batterychargingchange_callback_on_thread__proxy: 'sync',
  emscripten_set_batterychargingchange_callback_on_thread__sig: 'iii',
  emscripten_set_batterychargingchange_callback_on_thread: function(userData, callbackfunc, targetThread) {
    if (!JSEvents.battery()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}}; 
    JSEvents.registerBatteryEventCallback(JSEvents.battery(), userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BATTERYCHARGINGCHANGE') }}}, "chargingchange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_batterylevelchange_callback_on_thread__proxy: 'sync',
  emscripten_set_batterylevelchange_callback_on_thread__sig: 'iii',
  emscripten_set_batterylevelchange_callback_on_thread: function(userData, callbackfunc, targetThread) {
    if (!JSEvents.battery()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}}; 
    JSEvents.registerBatteryEventCallback(JSEvents.battery(), userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BATTERYLEVELCHANGE') }}}, "levelchange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_get_battery_status__proxy: 'sync',
  emscripten_get_battery_status__sig: 'ii',
  emscripten_get_battery_status: function(batteryState) {
    if (!JSEvents.battery()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}}; 
    JSEvents.fillBatteryEventData(batteryState, JSEvents.battery());
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  // Execute in calling thread without proxying needed.
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
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.explicitSwapControl, 0, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.proxyContextToMainThread, 1, 'i32') }}};
    {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.renderViaOffscreenBackBuffer, 0, 'i32') }}};
  },

#if !USE_PTHREADS
  emscripten_webgl_create_context: 'emscripten_webgl_do_create_context',
  emscripten_webgl_get_current_context: 'emscripten_webgl_do_get_current_context',
  emscripten_webgl_commit_frame: 'emscripten_webgl_do_commit_frame',
#endif

  emscripten_webgl_do_create_context__deps: ['$GL'],
  // This function performs proxying manually, depending on the style of context that is to be created.
  emscripten_webgl_do_create_context: function(target, attributes) {
    var contextAttributes = {};
    contextAttributes['alpha'] = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.alpha, 'i32') }}};
    contextAttributes['depth'] = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.depth, 'i32') }}};
    contextAttributes['stencil'] = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.stencil, 'i32') }}};
    contextAttributes['antialias'] = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.antialias, 'i32') }}};
    contextAttributes['premultipliedAlpha'] = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.premultipliedAlpha, 'i32') }}};
    contextAttributes['preserveDrawingBuffer'] = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer, 'i32') }}};
    contextAttributes['preferLowPowerToHighPerformance'] = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.preferLowPowerToHighPerformance, 'i32') }}};
    contextAttributes['failIfMajorPerformanceCaveat'] = !!{{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.failIfMajorPerformanceCaveat, 'i32') }}};
    contextAttributes['majorVersion'] = {{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.majorVersion, 'i32') }}};
    contextAttributes['minorVersion'] = {{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.minorVersion, 'i32') }}};
    contextAttributes['enableExtensionsByDefault'] = {{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.enableExtensionsByDefault, 'i32') }}};
    contextAttributes['explicitSwapControl'] = {{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.explicitSwapControl, 'i32') }}};
    contextAttributes['proxyContextToMainThread'] = {{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.proxyContextToMainThread, 'i32') }}};
    contextAttributes['renderViaOffscreenBackBuffer'] = {{{ makeGetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.renderViaOffscreenBackBuffer, 'i32') }}};

    target = Pointer_stringify(target);
    var canvas;
    if ((!target || target === '#canvas') && Module['canvas']) {
      canvas = (Module['canvas'].id && GL.offscreenCanvases[Module['canvas'].id]) ? (GL.offscreenCanvases[Module['canvas'].id].offscreenCanvas || JSEvents.findEventTarget(Module['canvas'].id)) : Module['canvas'];
    } else {
      canvas = GL.offscreenCanvases[target] ? GL.offscreenCanvases[target].offscreenCanvas : JSEvents.findEventTarget(target);
    }

#if USE_PTHREADS
    // Create a WebGL context that is proxied to main thread if canvas was not found on worker, or if explicitly requested to do so.
    if (ENVIRONMENT_IS_PTHREAD) {
      if (contextAttributes['proxyContextToMainThread'] === {{{ cDefine('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS') }}} ||
         (!canvas && contextAttributes['proxyContextToMainThread'] === {{{ cDefine('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK') }}})) {
        // When WebGL context is being proxied via the main thread, we must render using an offscreen FBO render target to avoid WebGL's
        // "implicit swap when callback exits" behavior. TODO: If OffscreenCanvas is supported, explicitSwapControl=true and still proxying,
        // then this can be avoided, since OffscreenCanvas enables explicit swap control.
#if GL_DEBUG
        if (contextAttributes['proxyContextToMainThread'] === {{{ cDefine('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS') }}}) console.error('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS enabled, proxying WebGL rendering from pthread to main thread.');
        if (!canvas && contextAttributes['proxyContextToMainThread'] === {{{ cDefine('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK') }}}) console.error('Specified canvas target "' + target + '" is not an OffscreenCanvas in the current pthread, but EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK is set. Proxying WebGL rendering from pthread to main thread.');
        console.error('Performance warning: forcing renderViaOffscreenBackBuffer=true and preserveDrawingBuffer=true since proxying WebGL rendering.');
#endif
        // We will be proxying - if OffscreenCanvas is supported, we can proxy a bit more efficiently by avoiding having to create an Offscreen FBO.
        if (typeof OffscreenCanvas === 'undefined') {
          {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.renderViaOffscreenBackBuffer, '1', 'i32') }}}
          {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer, '1', 'i32') }}}
        }
        return _emscripten_sync_run_in_main_thread_2({{{ cDefine('EM_PROXIED_CREATE_CONTEXT') }}}, target, attributes);
      }
    }
#endif

    if (!canvas) {
#if GL_DEBUG
      console.error('emscripten_webgl_create_context failed: Unknown canvas target "' + target + '"!');
#endif
      return 0;
    }

#if OFFSCREENCANVAS_SUPPORT
#if GL_DEBUG
    if (typeof OffscreenCanvas !== 'undefined' && canvas instanceof OffscreenCanvas) console.log('emscripten_webgl_create_context: Creating an OffscreenCanvas-based WebGL context on target "' + target + '"');
    else if (typeof HTMLCanvasElement !== 'undefined' && canvas instanceof HTMLCanvasElement) console.log('emscripten_webgl_create_context: Creating an HTMLCanvasElement-based WebGL context on target "' + target + '"');
#endif

    if (contextAttributes['explicitSwapControl']) {
      var supportsOffscreenCanvas = canvas.transferControlToOffscreen || (typeof OffscreenCanvas !== 'undefined' && canvas instanceof OffscreenCanvas);

      if (!supportsOffscreenCanvas) {
#if OFFSCREEN_FRAMEBUFFER
        if (!contextAttributes['renderViaOffscreenBackBuffer']) {
          contextAttributes['renderViaOffscreenBackBuffer'] = true;
#if GL_DEBUG
          console.error('emscripten_webgl_create_context: Performance warning, OffscreenCanvas is not supported but explicitSwapControl was requested, so force-enabling renderViaOffscreenBackBuffer=true to allow explicit swapping!');
#endif
        }
#else
#if GL_DEBUG
        console.error('emscripten_webgl_create_context failed: OffscreenCanvas is not supported but explicitSwapControl was requested!');
#endif
        return 0;
#endif
      }

      if (canvas.transferControlToOffscreen) {
#if GL_DEBUG
        console.log('explicitSwapControl requested: canvas.transferControlToOffscreen() on canvas "' + target + '" to get .commit() function and not rely on implicit WebGL swap');
#endif
        if (!canvas.controlTransferredOffscreen) {
          GL.offscreenCanvases[canvas.id] = canvas.transferControlToOffscreen();
          canvas.controlTransferredOffscreen = true;
          GL.offscreenCanvases[canvas.id].id = canvas.id;
        } else if (!GL.offscreenCanvases[canvas.id]) {
#if GL_DEBUG
          console.error('OffscreenCanvas is supported, and canvas "' + canvas.id + '" has already before been transferred offscreen, but there is no known OffscreenCanvas with that name!');
#endif
          return 0;
        }
        canvas = GL.offscreenCanvases[canvas.id];
      }
    }
#else // !OFFSCREENCANVAS_SUPPORT
#if OFFSCREEN_FRAMEBUFFER
    if (contextAttributes['explicitSwapControl'] && !contextAttributes['renderViaOffscreenBackBuffer']) {
      contextAttributes['renderViaOffscreenBackBuffer'] = true;
#if GL_DEBUG
      console.error('emscripten_webgl_create_context: Performance warning, not building with OffscreenCanvas support enabled but explicitSwapControl was requested, so force-enabling renderViaOffscreenBackBuffer=true to allow explicit swapping!');
#endif
    }
#else
    if (contextAttributes['explicitSwapControl']) {
#if GL_DEBUG
      console.error('emscripten_webgl_create_context failed: explicitSwapControl is not supported, please rebuild with -s OFFSCREENCANVAS_SUPPORT=1 to enable targeting the experimental OffscreenCanvas specification, or rebuild with -s OFFSCREEN_FRAMEBUFFER=1 to emulate explicitSwapControl in the absence of OffscreenCanvas support!');
#endif
      return 0;
    }
#endif // ~!OFFSCREEN_FRAMEBUFFER

#endif // ~!OFFSCREENCANVAS_SUPPORT

    var contextHandle = GL.createContext(canvas, contextAttributes);
    return contextHandle;
  },
#if USE_PTHREADS
  // Runs on the calling thread, proxies if needed.
  emscripten_webgl_make_context_current_calling_thread: function(contextHandle) {
    var success = GL.makeContextCurrent(contextHandle);
    if (success) GL.currentContextIsProxied = false; // If succeeded above, we will have a local GL context from this thread (worker or main).
    return success ? {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}} : {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
  },
  // This function gets called in a pthread, after it has successfully activated (with make_current()) a proxied GL context to itself from the main thread.
  // In this scenario, the pthread does not hold a high-level JS object to the GL context, because it lives on the main thread, in which case we record
  // an integer pointer as a token value to represent the GL context activation from another thread. (when this function is called, the main browser thread
  // has already accepted the GL context activation for our pthread, so that side is good)
  _emscripten_proxied_gl_context_activated_from_main_browser_thread: function(contextHandle) {
    GLctx = Module.ctx = GL.currentContext = contextHandle;
    GL.currentContextIsProxied = true;
  },
#else
  emscripten_webgl_make_context_current: function(contextHandle) {
    var success = GL.makeContextCurrent(contextHandle);
    return success ? {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}} : {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
  },
#endif

  emscripten_webgl_do_get_current_context: function() {
    return GL.currentContext ? GL.currentContext.handle : 0;
  },

  emscripten_webgl_get_drawing_buffer_size_calling_thread: function(contextHandle, width, height) {
    var GLContext = GL.getContext(contextHandle);

    if (!GLContext || !GLContext.GLctx || !width || !height) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
    }
    {{{ makeSetValue('width', '0', 'GLContext.GLctx.drawingBufferWidth', 'i32') }}};
    {{{ makeSetValue('height', '0', 'GLContext.GLctx.drawingBufferHeight', 'i32') }}};
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

#if USE_PTHREADS
  emscripten_webgl_get_drawing_buffer_size_main_thread__proxy: 'sync',
  emscripten_webgl_get_drawing_buffer_size_main_thread__sig: 'iiii',
  emscripten_webgl_get_drawing_buffer_size_main_thread__deps: ['emscripten_webgl_get_drawing_buffer_size_calling_thread'],
  emscripten_webgl_get_drawing_buffer_size_main_thread: function(contextHandle, width, height) { return _emscripten_webgl_get_drawing_buffer_size_calling_thread(contextHandle, width, height); },

  emscripten_webgl_get_drawing_buffer_size__deps: ['emscripten_webgl_get_drawing_buffer_size_calling_thread', 'emscripten_webgl_get_drawing_buffer_size_main_thread'],
  emscripten_webgl_get_drawing_buffer_size: function(contextHandle, width, height) {
    if (GL.contexts[contextHandle]) return _emscripten_webgl_get_drawing_buffer_size_calling_thread(contextHandle, width, height);
    else _emscripten_webgl_get_drawing_buffer_size_main_thread(contextHandle, width, height);
  },
#else
  emscripten_webgl_get_drawing_buffer_size: 'emscripten_webgl_get_drawing_buffer_size_calling_thread',
#endif

  emscripten_webgl_do_commit_frame: function() {
#if TRACE_WEBGL_CALLS
    var threadId = (typeof _pthread_self !== 'undefined') ? _pthread_self : function() { return 1; };
    console.error('[Thread ' + threadId() + ', GL ctx: ' + GL.currentContext.handle + ']: emscripten_webgl_do_commit_frame()');
#endif
    if (!GL.currentContext || !GL.currentContext.GLctx) {
#if GL_DEBUG
      console.error('emscripten_webgl_commit_frame() failed: no GL context set current via emscripten_webgl_make_context_current()!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

#if OFFSCREEN_FRAMEBUFFER
    if (GL.currentContext.defaultFbo) {
      GL.blitOffscreenFramebuffer(GL.currentContext);
#if GL_DEBUG
      if (GL.currentContext.GLctx.commit) console.error('emscripten_webgl_commit_frame(): Offscreen framebuffer should never have gotten created when canvas is in OffscreenCanvas mode, since it is redundant and not necessary');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    }
#endif
    if (!GL.currentContext.GLctx.commit) {
#if GL_DEBUG
      console.error('emscripten_webgl_commit_frame() failed: OffscreenCanvas is not supported by the current GL context!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    if (!GL.currentContext.attributes.explicitSwapControl) {
#if GL_DEBUG
      console.error('emscripten_webgl_commit_frame() cannot be called for canvases with implicit swap control mode!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }
    GL.currentContext.GLctx.commit();
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_webgl_destroy_context_calling_thread: function(contextHandle) {
    if (GL.currentContext == contextHandle) GL.currentContext = 0;
    GL.deleteContext(contextHandle);
  },

#if USE_PTHREADS
  emscripten_webgl_destroy_context_main_thread__proxy: 'sync',
  emscripten_webgl_destroy_context_main_thread__sig: 'vi',
  emscripten_webgl_destroy_context_main_thread__deps: ['emscripten_webgl_destroy_context_calling_thread'],
  emscripten_webgl_destroy_context_main_thread: function(contextHandle) { return _emscripten_webgl_destroy_context_calling_thread(contextHandle); },

  emscripten_webgl_destroy_context__deps: ['emscripten_webgl_destroy_context_main_thread', 'emscripten_webgl_destroy_context_calling_thread', 'emscripten_webgl_get_current_context', 'emscripten_webgl_make_context_current'],
  emscripten_webgl_destroy_context: function(contextHandle) {
    if (_emscripten_webgl_get_current_context() == contextHandle) _emscripten_webgl_make_context_current(0);
    return GL.contexts[contextHandle] ? _emscripten_webgl_destroy_context_calling_thread(contextHandle) : _emscripten_webgl_destroy_context_main_thread(contextHandle);
  },
#else
  emscripten_webgl_destroy_context: 'emscripten_webgl_destroy_context_calling_thread',
#endif

  emscripten_webgl_enable_extension_calling_thread: function(contextHandle, extension) {
    var context = GL.getContext(contextHandle);
    var extString = Pointer_stringify(extension);
    if (extString.indexOf('GL_') == 0) extString = extString.substr(3); // Allow enabling extensions both with "GL_" prefix and without.
    var ext = context.GLctx.getExtension(extString);
    return !!ext;
  },

  emscripten_supports_offscreencanvas: function() {
#if OFFSCREENCANVAS_SUPPORT
    return typeof OffscreenCanvas !== 'undefined';
#else
    return 0;
#endif
  },

#if USE_PTHREADS
  emscripten_webgl_enable_extension_main_thread__proxy: 'sync',
  emscripten_webgl_enable_extension_main_thread__sig: 'iii',
  emscripten_webgl_enable_extension_main_thread__deps: ['emscripten_webgl_enable_extension_calling_thread'],
  emscripten_webgl_enable_extension_main_thread: function(contextHandle, extension) { return _emscripten_webgl_enable_extension_calling_thread(contextHandle, extension); },

  emscripten_webgl_enable_extension__deps: ['emscripten_webgl_enable_extension_main_thread', 'emscripten_webgl_enable_extension_calling_thread'],
  emscripten_webgl_enable_extension: function(contextHandle, extension) {
    return GL.contexts[contextHandle] ? _emscripten_webgl_enable_extension_calling_thread(contextHandle, extension) : _emscripten_webgl_enable_extension_main_thread(contextHandle, extension);
  },
#else
  emscripten_webgl_enable_extension: 'emscripten_webgl_enable_extension_calling_thread',
#endif

  emscripten_set_webglcontextlost_callback_on_thread__proxy: 'sync',
  emscripten_set_webglcontextlost_callback_on_thread__sig: 'iiiiii',
  emscripten_set_webglcontextlost_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST') }}}, "webglcontextlost", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_webglcontextrestored_callback_on_thread__proxy: 'sync',
  emscripten_set_webglcontextrestored_callback_on_thread__sig: 'iiiiii',
  emscripten_set_webglcontextrestored_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    JSEvents.registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED') }}}, "webglcontextrestored", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_is_webgl_context_lost__proxy: 'sync',
  emscripten_is_webgl_context_lost__sig: 'ii',
  emscripten_is_webgl_context_lost: function(target) {
    // TODO: In the future if multiple GL contexts are supported, use the 'target' parameter to find the canvas to query.
    return Module.ctx ? Module.ctx.isContextLost() : true; // No context ~> lost context.
  },

#if USE_PTHREADS
  emscripten_set_canvas_element_size_calling_thread__deps: ['emscripten_set_offscreencanvas_size_on_target_thread'],
  emscripten_set_canvas_element_size_calling_thread: function(target, width, height) {
    var canvas = JSEvents.findCanvasEventTarget(target);
    if (!canvas) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    if (canvas.canvasSharedPtr) {
      // N.B. We hold the canvasSharedPtr info structure as the authoritative source for specifying the size of a canvas
      // since the actual canvas size changes are asynchronous if the canvas is owned by an OffscreenCanvas on another thread.
      // Therefore when setting the size, eagerly set the size of the canvas on the calling thread here, though this thread
      // might not be the one that actually ends up specifying the size, but the actual size change may be dispatched
      // as an asynchronous event below.
      {{{ makeSetValue('canvas.canvasSharedPtr', 0, 'width', 'i32') }}};
      {{{ makeSetValue('canvas.canvasSharedPtr', 4, 'height', 'i32') }}};
    }

    if (canvas.offscreenCanvas || !canvas.controlTransferredOffscreen) {
      if (canvas.offscreenCanvas) canvas = canvas.offscreenCanvas;
      var autoResizeViewport = false;
      if (canvas.GLctxObject && canvas.GLctxObject.GLctx) {
        var prevViewport = canvas.GLctxObject.GLctx.getParameter(canvas.GLctxObject.GLctx.VIEWPORT);
        // TODO: Perhaps autoResizeViewport should only be true if FBO 0 is currently active?
        autoResizeViewport = (prevViewport[0] === 0 && prevViewport[1] === 0 && prevViewport[2] === canvas.width && prevViewport[3] === canvas.height);
#if GL_DEBUG
        console.error('Resizing canvas from ' + canvas.width + 'x' + canvas.height + ' to ' + width + 'x' + height + '. Previous GL viewport size was ' 
          + prevViewport + ', so autoResizeViewport=' + autoResizeViewport);
#endif
      }
      canvas.width = width;
      canvas.height = height;
      if (autoResizeViewport) {
#if GL_DEBUG
        console.error('Automatically resizing GL viewport to cover whole render target ' + width + 'x' + height);
#endif
        // TODO: Add -s CANVAS_RESIZE_SETS_GL_VIEWPORT=0/1 option (default=1). This is commonly done and several graphics engines depend on this,
        // but this can be quite disruptive.
        canvas.GLctxObject.GLctx.viewport(0, 0, width, height);
      }
    } else if (canvas.canvasSharedPtr) {
      var targetThread = {{{ makeGetValue('canvas.canvasSharedPtr', 8, 'i32') }}};
      _emscripten_set_offscreencanvas_size_on_target_thread(targetThread, target, width, height);
      return {{{ cDefine('EMSCRIPTEN_RESULT_DEFERRED') }}}; // This will have to be done asynchronously
    } else {
#if GL_DEBUG
      console.error('canvas.controlTransferredOffscreen but we do not own the canvas, and do not know who has (no canvas.canvasSharedPtr present, an internal bug?)!\n');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    }
#if OFFSCREEN_FRAMEBUFFER
    if (canvas.GLctxObject) GL.resizeOffscreenFramebuffer(canvas.GLctxObject);
#endif
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_offscreencanvas_size_on_target_thread: function(targetThread, targetCanvas, width, height) {
    var stackTop = stackSave();
    var varargs = stackAlloc(12);

    // TODO: This could be optimized a bit (basically a dumb encoding agnostic strdup)
    var targetStr = targetCanvas
                      ? typeof targetCanvas === 'string'
                        ? targetCanvas
                        : Pointer_stringify(targetCanvas)
                      : 0;
    var targetStrHeap = targetStr ? _malloc(targetStr.length+1) : 0;
    if (targetStrHeap) stringToUTF8(targetStr, targetStrHeap, targetStr.length+1);

    {{{ makeSetValue('varargs', 0, 'targetStrHeap', 'i32')}}};
    {{{ makeSetValue('varargs', 4, 'width', 'i32')}}};
    {{{ makeSetValue('varargs', 8, 'height', 'i32')}}};
    // Note: If we are also a pthread, the call below could theoretically be done synchronously. However if the target pthread is waiting for a mutex from us, then
    // these two threads will deadlock. At the moment, we'd like to consider that this kind of deadlock would be an Emscripten runtime bug, although if
    // emscripten_set_canvas_element_size() was documented to require running an event in the queue of thread that owns the OffscreenCanvas, then that might be ok.
    // (safer this way however)
    _emscripten_async_queue_on_thread_(targetThread, {{{ cDefine('EM_PROXIED_RESIZE_OFFSCREENCANVAS') }}}, 0, targetStrHeap /* satellite data */, varargs);
    stackRestore(stackTop);
  },

  emscripten_set_canvas_element_size_main_thread__proxy: 'sync',
  emscripten_set_canvas_element_size_main_thread__sig: 'iiii',
  emscripten_set_canvas_element_size_main_thread__deps: ['emscripten_set_canvas_element_size_calling_thread'],
  emscripten_set_canvas_element_size_main_thread: function(target, width, height) { return _emscripten_set_canvas_element_size_calling_thread(target, width, height); },

  emscripten_set_canvas_element_size__deps: ['emscripten_set_canvas_element_size_calling_thread', 'emscripten_set_canvas_element_size_main_thread'],
  emscripten_set_canvas_element_size: function(target, width, height) {
#if GL_DEBUG
    console.error('emscripten_set_canvas_element_size(target='+target+',width='+width+',height='+height);
#endif
    var canvas = JSEvents.findCanvasEventTarget(target);
    if (canvas) return _emscripten_set_canvas_element_size_calling_thread(target, width, height);
    else return _emscripten_set_canvas_element_size_main_thread(target, width, height);
  }, 
#else
  emscripten_set_canvas_element_size: function(target, width, height) {
#if GL_DEBUG
    console.error('emscripten_set_canvas_element_size(target='+target+',width='+width+',height='+height);
#endif
    var canvas = JSEvents.findCanvasEventTarget(target);
    if (!canvas) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    canvas.width = width;
    canvas.height = height;
#if OFFSCREEN_FRAMEBUFFER
    if (canvas.GLctxObject) GL.resizeOffscreenFramebuffer(canvas.GLctxObject);
#endif
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
#endif

  _set_canvas_element_size__deps: ['emscripten_set_canvas_element_size'],
  _set_canvas_element_size: function(target, width, height) {
#if GL_DEBUG
    console.error('_set_canvas_element_size(target='+target+',width='+width+',height='+height);
#endif
    if (!target.controlTransferredOffscreen) {
      target.width = width;
      target.height = height;
    } else {
      // This function is being called from high-level JavaScript code instead of asm.js/Wasm,
      // and it needs to synchronously proxy over to another thread, so marshal the string onto the heap to do the call.
      var stackTop = stackSave();
      var targetInt = stackAlloc(target.id.length+1);
      stringToUTF8(target.id, targetInt, target.id.length+1);
      _emscripten_set_canvas_element_size(targetInt, width, height);
      stackRestore(stackTop);
    }
  }, 

#if USE_PTHREADS
  emscripten_get_canvas_element_size_calling_thread: function(target, width, height) {
    var canvas = JSEvents.findCanvasEventTarget(target);
    if (!canvas) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    if (canvas.canvasSharedPtr) {
      // N.B. Reading the size of the Canvas takes priority from our shared state structure, which is not the actual size.
      // However if is possible that there is a canvas size set event pending on an OffscreenCanvas owned by another thread,
      // so that the real sizes of the canvas have not updated yet. Therefore reading the real values would be racy.
      var w = {{{ makeGetValue('canvas.canvasSharedPtr', 0, 'i32') }}};
      var h = {{{ makeGetValue('canvas.canvasSharedPtr', 4, 'i32') }}};
      {{{ makeSetValue('width', 0, 'w', 'i32') }}};
      {{{ makeSetValue('height', 0, 'h', 'i32') }}};
    } else if (canvas.offscreenCanvas) {
      {{{ makeSetValue('width', 0, 'canvas.offscreenCanvas.width', 'i32') }}};
      {{{ makeSetValue('height', 0, 'canvas.offscreenCanvas.height', 'i32') }}};
    } else if (!canvas.controlTransferredOffscreen) {
      {{{ makeSetValue('width', 0, 'canvas.width', 'i32') }}};
      {{{ makeSetValue('height', 0, 'canvas.height', 'i32') }}};
    } else {
#if GL_DEBUG
      console.error('canvas.controlTransferredOffscreen but we do not own the canvas, and do not know who has (no canvas.canvasSharedPtr present, an internal bug?)!\n');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_canvas_element_size_main_thread__proxy: 'sync',
  emscripten_get_canvas_element_size_main_thread__sig: 'iiii',
  emscripten_get_canvas_element_size_main_thread__deps: ['emscripten_get_canvas_element_size_calling_thread'],
  emscripten_get_canvas_element_size_main_thread: function(target, width, height) { return _emscripten_get_canvas_element_size_calling_thread(target, width, height); },

  emscripten_get_canvas_element_size__deps: ['emscripten_get_canvas_element_size_calling_thread', 'emscripten_get_canvas_element_size_main_thread'],
  emscripten_get_canvas_element_size: function(target, width, height) {
    var canvas = JSEvents.findCanvasEventTarget(target);
    if (canvas) return _emscripten_get_canvas_element_size_calling_thread(target, width, height);
    else return _emscripten_get_canvas_element_size_main_thread(target, width, height);
  }, 
#else
  emscripten_get_canvas_element_size: function(target, width, height) {
    var canvas = JSEvents.findCanvasEventTarget(target);
    if (!canvas) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    {{{ makeSetValue('width', '0', 'canvas.width', 'i32') }}};
    {{{ makeSetValue('height', '0', 'canvas.height', 'i32') }}};
  },
#endif

  // JavaScript-friendly API, returns pair [width, height]
  _get_canvas_element_size__deps: ['emscripten_get_canvas_element_size'],
  _get_canvas_element_size: function(target) {
    var stackTop = stackSave();
    var w = stackAlloc(8);
    var h = w + 4;

    var targetInt = stackAlloc(target.id.length+1);
    stringToUTF8(target.id, targetInt, target.id.length+1);
    var ret = _emscripten_get_canvas_element_size(targetInt, w, h);
    var size = [{{{ makeGetValue('w', 0, 'i32')}}}, {{{ makeGetValue('h', 0, 'i32')}}}];
    stackRestore(stackTop);
    return size;
  }, 

  emscripten_set_element_css_size__proxy: 'sync',
  emscripten_set_element_css_size__sig: 'iiii',
  emscripten_set_element_css_size: function(target, width, height) {
    if (target) target = JSEvents.findEventTarget(target);
    else target = Module['canvas'];
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    target.style.setProperty("width", width + "px");
    target.style.setProperty("height", height + "px");

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_element_css_size__proxy: 'sync',
  emscripten_get_element_css_size__sig: 'iiii',
  emscripten_get_element_css_size: function(target, width, height) {
    if (target) target = JSEvents.findEventTarget(target);
    else target = Module['canvas'];
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
  },

  emscripten_html5_remove_all_event_listeners: function() {
    JSEvents.removeAllEventListeners();
  }
};

autoAddDeps(LibraryJSEvents, '$JSEvents');
mergeInto(LibraryManager.library, LibraryJSEvents);
