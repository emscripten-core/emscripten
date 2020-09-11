/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryJSEvents = {
  $JSEvents: {

/* We do not depend on the exact initial values of falsey member fields - these fields can be populated on-demand
   to save code size.
   (but still documented here to keep track of what is supposed to be present)
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

    // When we transition from fullscreen to windowed mode, we remember here the element that was just in fullscreen mode
    // so that we can report information about that element in the event message.
    previousFullscreenElement: null,

#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION <= 8 || MIN_EDGE_VERSION <= 12 || MIN_CHROME_VERSION <= 21 // https://caniuse.com/#search=movementX
    // Remember the current mouse coordinates in case we need to emulate movementXY generation for browsers that don't support it.
    // Some browsers (e.g. Safari 6.0.5) only give movementXY when Pointerlock is active.
    previousScreenX: null,
    previousScreenY: null,
#endif

    // When the C runtime exits via exit(), we unregister all event handlers added by this library to be nice and clean.
    // Track in this field whether we have yet registered that __ATEXIT__ handler.
    removeEventListenersRegistered: false, 

#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // If we are in an event handler, specifies the event handler object from the eventHandlers array that is currently running.
    currentEventHandler: null,
#endif
*/

    // If positive, we are currently executing in a JS event handler.
    // (this particular property must be initialized to zero, as we ++/-- it)
    inEventHandler: 0,

    removeAllEventListeners: function() {
      for(var i = JSEvents.eventHandlers.length-1; i >= 0; --i) {
        JSEvents._removeHandler(i);
      }
      JSEvents.eventHandlers = [];
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      JSEvents.deferredCalls = [];
#endif
    },

#if !MINIMAL_RUNTIME || EXIT_RUNTIME // In minimal runtime, there is no concept of the page running vs being closed, and hence __ATEXIT__ is not present
    registerRemoveEventListeners: function() {
      if (!JSEvents.removeEventListenersRegistered) {
        __ATEXIT__.push(JSEvents.removeAllEventListeners);
        JSEvents.removeEventListenersRegistered = true;
      }
    },
#endif

#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    deferredCalls: [],

    // Queues the given function call to occur the next time we enter an event handler.
    // Existing implementations of pointerlock apis have required that 
    // the target element is active in fullscreen mode first. Therefore give
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
        call.targetFunction.apply(null, call.argsList);
      }
    },
#endif

    // Stores objects representing each currently registered JS event handler.
    eventHandlers: [],

#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED
    isInternetExplorer: function() { return navigator.userAgent.indexOf('MSIE') !== -1 || navigator.appVersion.indexOf('Trident/') > 0; },
#endif

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
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
        // Increment nesting count for the event handler.
        ++JSEvents.inEventHandler;
        JSEvents.currentEventHandler = eventHandler;
        // Process any old deferred calls the user has placed.
        JSEvents.runDeferredCalls();
#endif
        // Process the actual event, calls back to user C code handler.
        eventHandler.handlerFunc(event);
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
        // Process any new deferred calls that were placed right now from this event handler.
        JSEvents.runDeferredCalls();
        // Out of event handler - restore nesting count.
        --JSEvents.inEventHandler;
#endif
      };
      
      if (eventHandler.callbackfunc) {
        eventHandler.eventListenerFunc = jsEventHandler;
        eventHandler.target.addEventListener(eventHandler.eventTypeString, jsEventHandler, eventHandler.useCapture);
        JSEvents.eventHandlers.push(eventHandler);
#if !MINIMAL_RUNTIME // In minimal runtime, there is no concept of the page running vs being closed, and hence __ATEXIT__ is not present
        JSEvents.registerRemoveEventListeners();
#endif
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
      __emscripten_call_on_thread(0, targetThread, {{{ cDefine('EM_FUNC_SIG_IIII') }}}, eventHandlerFunc, eventData, varargs);
      stackRestore(stackTop);
    },
#endif

#if USE_PTHREADS
    getTargetThreadForEventCallback: function(targetThread) {
      switch(targetThread) {
        case {{{ cDefine('EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD') }}}: return 0; // The event callback for the current event should be called on the main browser thread. (0 == don't proxy)
        case {{{ cDefine('EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD') }}}: return PThread.currentProxiedOperationCallerThread; // The event callback for the current event should be backproxied to the thread that is registering the event.
        default: return targetThread; // The event callback for the current event should be proxied to the given specific thread.
      }
    },
#endif

    getNodeNameForTarget: function(target) {
      if (!target) return '';
      if (target == window) return '#window';
      if (target == screen) return '#screen';
      return (target && target.nodeName) ? target.nodeName : '';
    },

    fullscreenEnabled: function() {
      return document.fullscreenEnabled
#if MIN_FIREFOX_VERSION <= 63
      // Firefox 64 shipped unprefixed form of fullscreenEnabled (https://caniuse.com/#feat=mdn-api_document_fullscreenenabled)
      || document.mozFullScreenEnabled
#endif
#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
      // Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitFullscreenEnabled.
      // TODO: If Safari at some point ships with unprefixed version, update the version check above.
      || document.webkitFullscreenEnabled
#endif
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED
      // Last IE 11 shipped prefixed msFullscreenEnabled
       || document.msFullscreenEnabled
#endif
       ;
    },
  },

  _registerKeyEventCallback__deps: ['$JSEvents', '$findEventTarget'],
  _registerKeyEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.keyEvent) JSEvents.keyEvent = _malloc( {{{ C_STRUCTS.EmscriptenKeyboardEvent.__size__ }}} );

    var keyEventHandlerFunc = function(e) {
#if ASSERTIONS
      assert(e);
#endif

#if USE_PTHREADS
      var keyEventData = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenKeyboardEvent.__size__ }}} ) : JSEvents.keyEvent; // This allocated block is passed as satellite data to the proxied function call, so the call frees up the data block when done.
#else
      var keyEventData = JSEvents.keyEvent;
#endif
      var idx = keyEventData >> 2;

      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.location / 4}}}] = e.location;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.ctrlKey / 4}}}] = e.ctrlKey;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.shiftKey / 4}}}] = e.shiftKey;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.altKey / 4}}}] = e.altKey;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.metaKey / 4}}}] = e.metaKey;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.repeat / 4}}}] = e.repeat;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.charCode / 4}}}] = e.charCode;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.keyCode / 4}}}] = e.keyCode;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.which / 4}}}] = e.which;
      stringToUTF8(e.key || '', keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.key }}}, {{{ cDefine('EM_HTML5_SHORT_STRING_LEN_BYTES') }}});
      stringToUTF8(e.code || '', keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.code }}}, {{{ cDefine('EM_HTML5_SHORT_STRING_LEN_BYTES') }}});
      stringToUTF8(e.char || '', keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.charValue }}}, {{{ cDefine('EM_HTML5_SHORT_STRING_LEN_BYTES') }}});
      stringToUTF8(e.locale || '', keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.locale }}}, {{{ cDefine('EM_HTML5_SHORT_STRING_LEN_BYTES') }}});

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, keyEventData, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, keyEventData, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED
      allowsDeferredCalls: JSEvents.isInternetExplorer() ? false : true, // MSIE doesn't allow fullscreen and pointerlock requests from key handlers, others do.
#else
      allowsDeferredCalls: true,
#endif
#endif
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: keyEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  // In DOM capturing and bubbling sequence, there are two special elements at the top of the event chain that can be of interest
  // to register many events to: document and window. These cannot be addressed by using document.querySelector(), so
  // a special mechanism to address them is needed. (For any other special object, such as screen.orientation, no general access
  // scheme should be needed, but the object-specific event callback registration functions should handle them individually).
  //
  // Users can also add more special event targets, basically by just doing something like
  //    specialHTMLTargets["!canvas"] = Module.canvas;
  // (that will let !canvas map to the canvas held in Module.canvas).
#if ENVIRONMENT_MAY_BE_WORKER || ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL || USE_PTHREADS
  $specialHTMLTargets: "[0, typeof document !== 'undefined' ? document : 0, typeof window !== 'undefined' ? window : 0]",
#else
  $specialHTMLTargets: "[0, document, window]",
#endif

#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
  $maybeCStringToJsString: function(cString) {
    // "cString > 2" checks if the input is a number, and isn't of the special
    // values we accept here, EMSCRIPTEN_EVENT_TARGET_* (which map to 0, 1, 2).
    // In other words, if cString > 2 then it's a pointer to a valid place in
    // memory, and points to a C string.
    return cString > 2 ? UTF8ToString(cString) : cString;
  },

  $findEventTarget__deps: ['$maybeCStringToJsString', '$specialHTMLTargets'],
  $findEventTarget: function(target) {
    target = maybeCStringToJsString(target);
#if ENVIRONMENT_MAY_BE_WORKER || ENVIRONMENT_MAY_BE_NODE
    var domElement = specialHTMLTargets[target] || (typeof document !== 'undefined' ? document.querySelector(target) : undefined);
#else
    var domElement = specialHTMLTargets[target] || document.querySelector(target);
#endif
    return domElement;
  },

#if OFFSCREENCANVAS_SUPPORT
  $findCanvasEventTarget__deps: ['$GL', '$maybeCStringToJsString'],
  $findCanvasEventTarget: function(target) {
    target = maybeCStringToJsString(target);

    // When compiling with OffscreenCanvas support and looking up a canvas to target,
    // we first look up if the target Canvas has been transferred to OffscreenCanvas use.
    // These transfers are represented/tracked by GL.offscreenCanvases object, which contain
    // the OffscreenCanvas element for each regular Canvas element that has been transferred.

    // Note that each pthread/worker have their own set of GL.offscreenCanvases. That is,
    // when an OffscreenCanvas is transferred from a pthread/main thread to another pthread,
    // it will move in the GL.offscreenCanvases array between threads. Hence GL.offscreenCanvases
    // represents the set of OffscreenCanvases owned by the current calling thread.

    // First check out the list of OffscreenCanvases by CSS selector ID ('#myCanvasID')
    return GL.offscreenCanvases[target.substr(1)] // Remove '#' prefix
    // If not found, if one is querying by using DOM tag name selector 'canvas', grab the first
    // OffscreenCanvas that we can find.
     || (target == 'canvas' && Object.keys(GL.offscreenCanvases)[0])
    // If that is not found either, query via the regular DOM selector.
#if USE_PTHREADS
     || (typeof document !== 'undefined' && document.querySelector(target));
#else
     || document.querySelector(target);
#endif
  },
#else
  $findCanvasEventTarget__deps: ['$findEventTarget'],
  $findCanvasEventTarget: function(target) { return findEventTarget(target); },
#endif

#else
  // Find a DOM element with the given ID.
  $findEventTarget__deps: ['$specialHTMLTargets'],
  $findEventTarget: function(target) {
#if ASSERTIONS
    warnOnce('Rules for selecting event targets in HTML5 API are changing: instead of using document.getElementById() that only can refer to elements by their DOM ID, new event target selection mechanism uses the more flexible function document.querySelector() that can look up element names, classes, and complex CSS selectors. Build with -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 to change to the new lookup rules. See https://github.com/emscripten-core/emscripten/pull/7977 for more details.');
#endif
    try {
      // The sensible "default" target varies between events, but use window as the default
      // since DOM events mostly can default to that. Specific callback registrations
      // override their own defaults.
      if (!target) return window;
      if (typeof target === "number") target = specialHTMLTargets[target] || UTF8ToString(target);
      if (target === '#window') return window;
      else if (target === '#document') return document;
      else if (target === '#screen') return screen;
      else if (target === '#canvas') return Module['canvas'];
      return (typeof target === 'string') ? document.getElementById(target) : target;
    } catch(e) {
      // In Web Workers, some objects above, such as '#document' do not exist. Gracefully
      // return null for them.
      return null;
    }
  },

  // Like findEventTarget, but looks for OffscreenCanvas elements first
  $findCanvasEventTarget__deps: ['$findEventTarget'],
  $findCanvasEventTarget: function(target) {
    if (typeof target === 'number') target = UTF8ToString(target);
    if (!target || target === '#canvas') {
      if (typeof GL !== 'undefined' && GL.offscreenCanvases['canvas']) return GL.offscreenCanvases['canvas']; // TODO: Remove this line, target '#canvas' should refer only to Module['canvas'], not to GL.offscreenCanvases['canvas'] - but need stricter tests to be able to remove this line.
      return Module['canvas'];
    }
    if (typeof GL !== 'undefined' && GL.offscreenCanvases[target]) return GL.offscreenCanvases[target];
    return findEventTarget(target);
  },
#endif

  emscripten_set_keypress_callback_on_thread__proxy: 'sync',
  emscripten_set_keypress_callback_on_thread__sig: 'iiiiii',
  emscripten_set_keypress_callback_on_thread__deps: ['_registerKeyEventCallback'],
  emscripten_set_keypress_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYPRESS') }}}, "keypress", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_keydown_callback_on_thread__proxy: 'sync',
  emscripten_set_keydown_callback_on_thread__sig: 'iiiiii',
  emscripten_set_keydown_callback_on_thread__deps: ['_registerKeyEventCallback'],
  emscripten_set_keydown_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYDOWN') }}}, "keydown", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_keyup_callback_on_thread__proxy: 'sync',
  emscripten_set_keyup_callback_on_thread__sig: 'iiiiii',
  emscripten_set_keyup_callback_on_thread__deps: ['_registerKeyEventCallback'],
  emscripten_set_keyup_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_KEYUP') }}}, "keyup", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  // Outline access to function .getBoundingClientRect() since it is a long string. Closure compiler does not outline access to it by itself, but it can inline access if
  // there is only one caller to this function.
  _getBoundingClientRect__deps: ['$specialHTMLTargets'],
  _getBoundingClientRect: function(e) {
    return specialHTMLTargets.indexOf(e) < 0 ? e.getBoundingClientRect() : {'left':0,'top':0};
  },

  // Copies mouse event data from the given JS mouse event 'e' to the specified Emscripten mouse event structure in the HEAP.
  // eventStruct: the structure to populate.
  // e: The JS mouse event to read data from.
  // target: Specifies a target DOM element that will be used as the reference to populate targetX and targetY parameters.
  _fillMouseEventData__deps: ['$JSEvents', '_getBoundingClientRect', '$specialHTMLTargets'],
  _fillMouseEventData: function(eventStruct, e, target) {
#if ASSERTIONS
    assert(eventStruct % 4 == 0);
#endif
    var idx = eventStruct >> 2;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.screenX / 4 }}}] = e.screenX;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.screenY / 4 }}}] = e.screenY;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.clientX / 4 }}}] = e.clientX;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.clientY / 4 }}}] = e.clientY;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.ctrlKey / 4 }}}] = e.ctrlKey;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.shiftKey / 4 }}}] = e.shiftKey;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.altKey / 4 }}}] = e.altKey;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.metaKey / 4 }}}] = e.metaKey;
    HEAP16[idx*2 + {{{ C_STRUCTS.EmscriptenMouseEvent.button / 2 }}}] = e.button;
    HEAP16[idx*2 + {{{ C_STRUCTS.EmscriptenMouseEvent.buttons / 2 }}}] = e.buttons;

    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.movementX / 4 }}}] = e["movementX"]
#if MIN_FIREFOX_VERSION <= 40
      //     https://caniuse.com/#feat=mdn-api_mouseevent_movementx
      || e["mozMovementX"]
#endif
#if MIN_CHROME_VERSION <= 36 // || MIN_ANDROID_BROWSER_VERSION <= 4.4.4
      || e["webkitMovementX"]
#endif
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION <= 80000 || MIN_EDGE_VERSION <= 12 || MIN_CHROME_VERSION <= 21 // https://caniuse.com/#search=movementX
      || (e.screenX-JSEvents.previousScreenX)
#endif
      ;

    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.movementY / 4 }}}] = e["movementY"]
#if MIN_FIREFOX_VERSION <= 40
      || e["mozMovementY"]
#endif
#if MIN_CHROME_VERSION <= 36 // || MIN_ANDROID_BROWSER_VERSION <= 4.4.4
      || e["webkitMovementY"]
#endif
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION <= 80000 || MIN_EDGE_VERSION <= 12 || MIN_CHROME_VERSION <= 21 // https://caniuse.com/#search=movementX
      || (e.screenY-JSEvents.previousScreenY)
#endif
      ;

#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    if (Module['canvas']) {
      var rect = __getBoundingClientRect(Module['canvas']);
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.canvasX / 4 }}}] = e.clientX - rect.left;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.canvasY / 4 }}}] = e.clientY - rect.top;
    } else { // Canvas is not initialized, return 0.
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.canvasX / 4 }}}] = 0;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.canvasY / 4 }}}] = 0;
    }
#endif
    var rect = __getBoundingClientRect(target);
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.targetX / 4 }}}] = e.clientX - rect.left;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.targetY / 4 }}}] = e.clientY - rect.top;

#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION <= 80000 || MIN_EDGE_VERSION <= 12 || MIN_CHROME_VERSION <= 21 // https://caniuse.com/#search=movementX
#if MIN_CHROME_VERSION <= 76
    // wheel and mousewheel events contain wrong screenX/screenY on chrome/opera <= 76,
    // so there we should not record previous screen coordinates on wheel events.
    // https://bugs.chromium.org/p/chromium/issues/detail?id=699956
    // https://github.com/emscripten-core/emscripten/pull/4997
    if (e.type !== 'wheel') {
#endif
      JSEvents.previousScreenX = e.screenX;
      JSEvents.previousScreenY = e.screenY;
#if MIN_CHROME_VERSION <= 76
    }
#endif
#endif
  },

  _registerMouseEventCallback__deps: ['$JSEvents', '_fillMouseEventData', '$findEventTarget'],
  _registerMouseEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.mouseEvent) JSEvents.mouseEvent = _malloc( {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}} );
    target = findEventTarget(target);

    var mouseEventHandlerFunc = function(ev) {
      var e = ev || event;

      // TODO: Make this access thread safe, or this could update live while app is reading it.
      __fillMouseEventData(JSEvents.mouseEvent, e, target);

#if USE_PTHREADS
      if (targetThread) {
        var mouseEventData = _malloc( {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}} ); // This allocated block is passed as satellite data to the proxied function call, so the call frees up the data block when done.
        __fillMouseEventData(mouseEventData, e, target);
        JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, mouseEventData, userData);
      } else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, JSEvents.mouseEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: target,
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      allowsDeferredCalls: eventTypeString != 'mousemove' && eventTypeString != 'mouseenter' && eventTypeString != 'mouseleave', // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
#endif
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: mouseEventHandlerFunc,
      useCapture: useCapture
    };
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED && HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // In IE, mousedown events don't either allow deferred calls to be run!
    if (JSEvents.isInternetExplorer() && eventTypeString == 'mousedown') eventHandler.allowsDeferredCalls = false;
#endif
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_click_callback_on_thread__proxy: 'sync',
  emscripten_set_click_callback_on_thread__sig: 'iiiiii',
  emscripten_set_click_callback_on_thread__deps: ['_registerMouseEventCallback'],
  emscripten_set_click_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_CLICK') }}}, "click", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mousedown_callback_on_thread__proxy: 'sync',
  emscripten_set_mousedown_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mousedown_callback_on_thread__deps: ['_registerMouseEventCallback'],
  emscripten_set_mousedown_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEDOWN') }}}, "mousedown", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseup_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseup_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseup_callback_on_thread__deps: ['_registerMouseEventCallback'],
  emscripten_set_mouseup_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEUP') }}}, "mouseup", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_dblclick_callback_on_thread__proxy: 'sync',
  emscripten_set_dblclick_callback_on_thread__sig: 'iiiiii',
  emscripten_set_dblclick_callback_on_thread__deps: ['_registerMouseEventCallback'],
  emscripten_set_dblclick_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DBLCLICK') }}}, "dblclick", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mousemove_callback_on_thread__proxy: 'sync',
  emscripten_set_mousemove_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mousemove_callback_on_thread__deps: ['_registerMouseEventCallback'],
  emscripten_set_mousemove_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEMOVE') }}}, "mousemove", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseenter_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseenter_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseenter_callback_on_thread__deps: ['_registerMouseEventCallback'],
  emscripten_set_mouseenter_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEENTER') }}}, "mouseenter", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseleave_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseleave_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseleave_callback_on_thread__deps: ['_registerMouseEventCallback'],
  emscripten_set_mouseleave_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSELEAVE') }}}, "mouseleave", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseover_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseover_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseover_callback_on_thread__deps: ['_registerMouseEventCallback'],
  emscripten_set_mouseover_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEOVER') }}}, "mouseover", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_mouseout_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseout_callback_on_thread__sig: 'iiiiii',
  emscripten_set_mouseout_callback_on_thread__deps: ['_registerMouseEventCallback'],
  emscripten_set_mouseout_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_MOUSEOUT') }}}, "mouseout", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_mouse_status__proxy: 'sync',
  emscripten_get_mouse_status__sig: 'ii',
  emscripten_get_mouse_status__deps: ['$JSEvents'],
  emscripten_get_mouse_status: function(mouseState) {
    if (!JSEvents.mouseEvent) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    // HTML5 does not really have a polling API for mouse events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler to any of the mouse function.
    HEAP8.set(HEAP8.subarray(JSEvents.mouseEvent, JSEvents.mouseEvent + {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}}), mouseState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _registerWheelEventCallback__deps: ['$JSEvents', '_fillMouseEventData', '$findEventTarget'],
  _registerWheelEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.wheelEvent) JSEvents.wheelEvent = _malloc( {{{ C_STRUCTS.EmscriptenWheelEvent.__size__ }}} );

    // The DOM Level 3 events spec event 'wheel'
    var wheelHandlerFunc = function(ev) {
      var e = ev || event;
#if USE_PTHREADS
      var wheelEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenWheelEvent.__size__ }}} ) : JSEvents.wheelEvent; // This allocated block is passed as satellite data to the proxied function call, so the call frees up the data block when done.
#else
      var wheelEvent = JSEvents.wheelEvent;
#endif
      __fillMouseEventData(wheelEvent, e, target);
      {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaX, 'e["deltaX"]', 'double') }}};
      {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaY, 'e["deltaY"]', 'double') }}};
      {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaZ, 'e["deltaZ"]', 'double') }}};
      {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaMode, 'e["deltaMode"]', 'i32') }}};
#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, wheelEvent, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, wheelEvent, userData)) e.preventDefault();
    };
#if MIN_IE_VERSION <= 8 || MIN_SAFARI_VERSION < 60100 // Browsers that do not support https://caniuse.com/#feat=mdn-api_wheelevent
    // The 'mousewheel' event as implemented in Safari 6.0.5
    var mouseWheelHandlerFunc = function(ev) {
      var e = ev || event;
      __fillMouseEventData(JSEvents.wheelEvent, e, target);
      {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaX, 'e["wheelDeltaX"] || 0', 'double') }}};
      /* 1. Invert to unify direction with the DOM Level 3 wheel event. 2. MSIE does not provide wheelDeltaY, so wheelDelta is used as a fallback. */
      var wheelDeltaY = -(e["wheelDeltaY"] || e["wheelDelta"])
      {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaY, 'wheelDeltaY', 'double') }}};
      {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaZ, '0 /* Not available */', 'double') }}};
      {{{ makeSetValue('JSEvents.wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaMode, '0 /* DOM_DELTA_PIXEL */', 'i32') }}};
      var shouldCancel = {{{ makeDynCall('iiii', 'callbackfunc') }}}( eventTypeId, JSEvents.wheelEvent, userData);
      if (shouldCancel) {
        e.preventDefault();
      }
    };
#endif

    var eventHandler = {
      target: target,
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      allowsDeferredCalls: true,
#endif
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
#if MIN_IE_VERSION <= 8 || MIN_SAFARI_VERSION < 60100 // Browsers that do not support https://caniuse.com/#feat=mdn-api_wheelevent
      handlerFunc: (eventTypeString == 'wheel') ? wheelHandlerFunc : mouseWheelHandlerFunc,
#else
      handlerFunc: wheelHandlerFunc,
#endif
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_wheel_callback_on_thread__proxy: 'sync',
  emscripten_set_wheel_callback_on_thread__sig: 'iiiiii',
  emscripten_set_wheel_callback_on_thread__deps: ['$JSEvents', '_registerWheelEventCallback', '$findEventTarget'],
  emscripten_set_wheel_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    target = findEventTarget(target);
    if (typeof target.onwheel !== 'undefined') {
      __registerWheelEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WHEEL') }}}, "wheel", targetThread);
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
#if MIN_IE_VERSION <= 8 || MIN_SAFARI_VERSION < 60100 // Browsers that do not support https://caniuse.com/#feat=mdn-api_wheelevent
    } else if (typeof target.onmousewheel !== 'undefined') {
      __registerWheelEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WHEEL') }}}, "mousewheel", targetThread);
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
#endif
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
  },

  _registerUiEventCallback__deps: ['$JSEvents', '$findEventTarget'],
  _registerUiEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.uiEvent) JSEvents.uiEvent = _malloc( {{{ C_STRUCTS.EmscriptenUiEvent.__size__ }}} );

#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    if (eventTypeString == "scroll" && !target) {
      target = document; // By default read scroll events on document rather than window.
    } else {
      target = findEventTarget(target);
    }
#else
#endif

    var uiEventHandlerFunc = function(ev) {
      var e = ev || event;
      if (e.target != target) {
        // Never take ui events such as scroll via a 'bubbled' route, but always from the direct element that
        // was targeted. Otherwise e.g. if app logs a message in response to a page scroll, the Emscripten log
        // message box could cause to scroll, generating a new (bubbled) scroll message, causing a new log print,
        // causing a new scroll, etc..
        return;
      }
#if USE_PTHREADS
      var uiEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenUiEvent.__size__ }}} ) : JSEvents.uiEvent;
#else
      var uiEvent = JSEvents.uiEvent;
#endif
      var b = document.body; // Take document.body to a variable, Closure compiler does not outline access to it on its own.
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.detail, 'e.detail', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientWidth, 'b.clientWidth', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientHeight, 'b.clientHeight', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerWidth, 'innerWidth', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerHeight, 'innerHeight', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterWidth, 'outerWidth', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterHeight, 'outerHeight', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollTop, 'pageXOffset', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollLeft, 'pageYOffset', 'i32') }}};
#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, uiEvent, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, uiEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: target,
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: uiEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_resize_callback_on_thread__proxy: 'sync',
  emscripten_set_resize_callback_on_thread__sig: 'iiiiii',
  emscripten_set_resize_callback_on_thread__deps: ['_registerUiEventCallback'],
  emscripten_set_resize_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_RESIZE') }}}, "resize", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_scroll_callback_on_thread__proxy: 'sync',
  emscripten_set_scroll_callback_on_thread__sig: 'iiiiii',
  emscripten_set_scroll_callback_on_thread__deps: ['_registerUiEventCallback'],
  emscripten_set_scroll_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_SCROLL') }}}, "scroll", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _registerFocusEventCallback__deps: ['$JSEvents', '$findEventTarget'],
  _registerFocusEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.focusEvent) JSEvents.focusEvent = _malloc( {{{ C_STRUCTS.EmscriptenFocusEvent.__size__ }}} );

    var focusEventHandlerFunc = function(ev) {
      var e = ev || event;

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
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, focusEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: focusEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_blur_callback_on_thread__proxy: 'sync',
  emscripten_set_blur_callback_on_thread__sig: 'iiiiii',
  emscripten_set_blur_callback_on_thread__deps: ['_registerFocusEventCallback'],
  emscripten_set_blur_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BLUR') }}}, "blur", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focus_callback_on_thread__proxy: 'sync',
  emscripten_set_focus_callback_on_thread__sig: 'iiiiii',
  emscripten_set_focus_callback_on_thread__deps: ['_registerFocusEventCallback'],
  emscripten_set_focus_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUS') }}}, "focus", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focusin_callback_on_thread__proxy: 'sync',
  emscripten_set_focusin_callback_on_thread__sig: 'iiiiii',
  emscripten_set_focusin_callback_on_thread__deps: ['_registerFocusEventCallback'],
  emscripten_set_focusin_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUSIN') }}}, "focusin", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_focusout_callback_on_thread__proxy: 'sync',
  emscripten_set_focusout_callback_on_thread__sig: 'iiiiii',
  emscripten_set_focusout_callback_on_thread__deps: ['_registerFocusEventCallback'],
  emscripten_set_focusout_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FOCUSOUT') }}}, "focusout", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _fillDeviceOrientationEventData__deps: ['$JSEvents'],
  _fillDeviceOrientationEventData: function(eventStruct, e, target) {
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.alpha, 'e.alpha', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.beta, 'e.beta', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.gamma, 'e.gamma', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.absolute, 'e.absolute', 'i32') }}};
  },

  _registerDeviceOrientationEventCallback__deps: ['$JSEvents', '_fillDeviceOrientationEventData', '$findEventTarget'],
  _registerDeviceOrientationEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.deviceOrientationEvent) JSEvents.deviceOrientationEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceOrientationEvent.__size__ }}} );

    var deviceOrientationEventHandlerFunc = function(ev) {
      var e = ev || event;

      __fillDeviceOrientationEventData(JSEvents.deviceOrientationEvent, e, target); // TODO: Thread-safety with respect to emscripten_get_deviceorientation_status()

#if USE_PTHREADS
      if (targetThread) {
        var deviceOrientationEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceOrientationEvent.__size__ }}} );
        __fillDeviceOrientationEventData(deviceOrientationEvent, e, target);
        JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, deviceOrientationEvent, userData);
      } else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, JSEvents.deviceOrientationEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: deviceOrientationEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_deviceorientation_callback_on_thread__proxy: 'sync',
  emscripten_set_deviceorientation_callback_on_thread__sig: 'iiiii',
  emscripten_set_deviceorientation_callback_on_thread__deps: ['_registerDeviceOrientationEventCallback'],
  emscripten_set_deviceorientation_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    __registerDeviceOrientationEventCallback({{{ cDefine('EMSCRIPTEN_EVENT_TARGET_WINDOW') }}}, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DEVICEORIENTATION') }}}, "deviceorientation", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_deviceorientation_status__proxy: 'sync',
  emscripten_get_deviceorientation_status__sig: 'ii',
  emscripten_get_deviceorientation_status__deps: ['$JSEvents', '_registerDeviceOrientationEventCallback'],
  emscripten_get_deviceorientation_status: function(orientationState) {
    if (!JSEvents.deviceOrientationEvent) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    // HTML5 does not really have a polling API for device orientation events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler.
    HEAP32.set(HEAP32.subarray(JSEvents.deviceOrientationEvent, {{{ C_STRUCTS.EmscriptenDeviceOrientationEvent.__size__ }}}), orientationState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _fillDeviceMotionEventData__deps: ['$JSEvents'],
  _fillDeviceMotionEventData: function(eventStruct, e, target) {
    var supportedFields = 0;
    var a = e['acceleration'];
    supportedFields |= a && {{{ cDefine('EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ACCELERATION') }}};
    var ag = e['accelerationIncludingGravity'];
    supportedFields |= ag && {{{ cDefine('EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ACCELERATION_INCLUDING_GRAVITY') }}};
    var rr = e['rotationRate'];
    supportedFields |= rr && {{{ cDefine('EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ROTATION_RATE') }}};
    a = a || {};
    ag = ag || {};
    rr = rr || {};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationX, 'a["x"]', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationY, 'a["y"]', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationZ, 'a["z"]', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityX, 'ag["x"]', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityY, 'ag["y"]', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.accelerationIncludingGravityZ, 'ag["z"]', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateAlpha, 'rr["alpha"]', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateBeta, 'rr["beta"]', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.rotationRateGamma, 'rr["gamma"]', 'double') }}};
  },

  _registerDeviceMotionEventCallback__deps: ['$JSEvents', '_fillDeviceMotionEventData', '$findEventTarget'],
  _registerDeviceMotionEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.deviceMotionEvent) JSEvents.deviceMotionEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}} );

    var deviceMotionEventHandlerFunc = function(ev) {
      var e = ev || event;

      __fillDeviceMotionEventData(JSEvents.deviceMotionEvent, e, target); // TODO: Thread-safety with respect to emscripten_get_devicemotion_status()

#if USE_PTHREADS
      if (targetThread) {
        var deviceMotionEvent = _malloc( {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}} );
        __fillDeviceMotionEventData(deviceMotionEvent, e, target);
        JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, deviceMotionEvent, userData);
      } else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, JSEvents.deviceMotionEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: deviceMotionEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_devicemotion_callback_on_thread__proxy: 'sync',
  emscripten_set_devicemotion_callback_on_thread__sig: 'iiiii',
  emscripten_set_devicemotion_callback_on_thread__deps: ['_registerDeviceMotionEventCallback'],
  emscripten_set_devicemotion_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    __registerDeviceMotionEventCallback({{{ cDefine('EMSCRIPTEN_EVENT_TARGET_WINDOW') }}}, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_DEVICEMOTION') }}}, "devicemotion", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_devicemotion_status__proxy: 'sync',
  emscripten_get_devicemotion_status__sig: 'ii',
  emscripten_get_devicemotion_status__deps: ['$JSEvents'],
  emscripten_get_devicemotion_status: function(motionState) {
    if (!JSEvents.deviceMotionEvent) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};
    // HTML5 does not really have a polling API for device motion events, so implement one manually by
    // returning the data from the most recently received event. This requires that user has registered
    // at least some no-op function as an event handler.
    HEAP32.set(HEAP32.subarray(JSEvents.deviceMotionEvent, {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}}), motionState);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _screenOrientation: function() {
    if (!screen) return undefined;
    return screen.orientation || screen.mozOrientation || screen.webkitOrientation || screen.msOrientation;
  },

  _fillOrientationChangeEventData__deps: ['_screenOrientation'],
  _fillOrientationChangeEventData: function(eventStruct) {
    var orientations  = ["portrait-primary", "portrait-secondary", "landscape-primary", "landscape-secondary"];
    var orientations2 = ["portrait",         "portrait",           "landscape",         "landscape"];

    var orientationString = __screenOrientation();
    var orientation = orientations.indexOf(orientationString);
    if (orientation == -1) {
      orientation = orientations2.indexOf(orientationString);
    }

    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenOrientationChangeEvent.orientationIndex, '1 << orientation', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenOrientationChangeEvent.orientationAngle, 'orientation', 'i32') }}};
  },

  _registerOrientationChangeEventCallback__deps: ['$JSEvents', '_fillOrientationChangeEventData', '$findEventTarget'],
  _registerOrientationChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.orientationChangeEvent) JSEvents.orientationChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenOrientationChangeEvent.__size__ }}} );

    var orientationChangeEventHandlerFunc = function(ev) {
      var e = ev || event;

#if USE_PTHREADS
      var orientationChangeEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}} ) : JSEvents.orientationChangeEvent;
#else
      var orientationChangeEvent = JSEvents.orientationChangeEvent;
#endif

      __fillOrientationChangeEventData(orientationChangeEvent);

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, orientationChangeEvent, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, orientationChangeEvent, userData)) e.preventDefault();
    };

    if (eventTypeString == "orientationchange" && screen.mozOrientation !== undefined) {
      eventTypeString = "mozorientationchange";
    }

    var eventHandler = {
      target: target,
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: orientationChangeEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_orientationchange_callback_on_thread__proxy: 'sync',
  emscripten_set_orientationchange_callback_on_thread__sig: 'iiiii',
  emscripten_set_orientationchange_callback_on_thread__deps: ['_registerOrientationChangeEventCallback'],
  emscripten_set_orientationchange_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    if (!screen || !screen['addEventListener']) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    __registerOrientationChangeEventCallback(screen, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_ORIENTATIONCHANGE') }}}, "orientationchange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_get_orientation_status__proxy: 'sync',
  emscripten_get_orientation_status__sig: 'ii',
  emscripten_get_orientation_status__deps: ['_fillOrientationChangeEventData', '_screenOrientation'],
  emscripten_get_orientation_status: function(orientationChangeEvent) {
    if (!__screenOrientation() && typeof orientation === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    __fillOrientationChangeEventData(orientationChangeEvent);
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
    if (screen.lockOrientation) {
      succeeded = screen.lockOrientation(orientations);
    } else if (screen.mozLockOrientation) {
      succeeded = screen.mozLockOrientation(orientations);
    } else if (screen.webkitLockOrientation) {
      succeeded = screen.webkitLockOrientation(orientations);
    } else if (screen.msLockOrientation) {
      succeeded = screen.msLockOrientation(orientations);
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
    if (screen.unlockOrientation) {
      screen.unlockOrientation();
    } else if (screen.mozUnlockOrientation) {
      screen.mozUnlockOrientation();
    } else if (screen.webkitUnlockOrientation) {
      screen.webkitUnlockOrientation();
    } else if (screen.msUnlockOrientation) {
      screen.msUnlockOrientation();
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _fillFullscreenChangeEventData__deps: ['$JSEvents'],
  _fillFullscreenChangeEventData: function(eventStruct) {
    var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
    var isFullscreen = !!fullscreenElement;
    /** @suppress{checkTypes} */
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

  _registerFullscreenChangeEventCallback__deps: ['$JSEvents', '_fillFullscreenChangeEventData', '$findEventTarget'],
  _registerFullscreenChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.fullscreenChangeEvent) JSEvents.fullscreenChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.__size__ }}} );

    var fullscreenChangeEventhandlerFunc = function(ev) {
      var e = ev || event;

#if USE_PTHREADS
      var fullscreenChangeEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.__size__ }}} ) : JSEvents.fullscreenChangeEvent;
#else
      var fullscreenChangeEvent = JSEvents.fullscreenChangeEvent;
#endif

      __fillFullscreenChangeEventData(fullscreenChangeEvent);

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, fullscreenChangeEvent, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, fullscreenChangeEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: target,
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: fullscreenChangeEventhandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_fullscreenchange_callback_on_thread__proxy: 'sync',
  emscripten_set_fullscreenchange_callback_on_thread__sig: 'iiiiii',
  emscripten_set_fullscreenchange_callback_on_thread__deps: ['$JSEvents', '_registerFullscreenChangeEventCallback', '$findEventTarget', '$specialHTMLTargets'],
  emscripten_set_fullscreenchange_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    if (!JSEvents.fullscreenEnabled()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : specialHTMLTargets[{{{ cDefine('EMSCRIPTEN_EVENT_TARGET_DOCUMENT') }}}];
#endif
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    __registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "fullscreenchange", targetThread);

#if MIN_FIREFOX_VERSION <= 63 // https://caniuse.com/#feat=mdn-api_element_fullscreenchange_event
    __registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "mozfullscreenchange", targetThread);
#endif

#if MIN_CHROME_VERSION < 71 || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
    // Unprefixed Fullscreen API shipped in Chromium 71 (https://bugs.chromium.org/p/chromium/issues/detail?id=383813)
    // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
    __registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "webkitfullscreenchange", targetThread);
#endif

#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED // https://caniuse.com/#feat=mdn-api_document_fullscreenchange_event
    __registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_FULLSCREENCHANGE') }}}, "MSFullscreenChange", targetThread);
#endif
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_fullscreen_status__proxy: 'sync',
  emscripten_get_fullscreen_status__sig: 'ii',
  emscripten_get_fullscreen_status__deps: ['$JSEvents', '_fillFullscreenChangeEventData'],
  emscripten_get_fullscreen_status: function(fullscreenStatus) {
    if (!JSEvents.fullscreenEnabled()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    __fillFullscreenChangeEventData(fullscreenStatus);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  JSEvents_requestFullscreen__deps: ['$JSEvents', 'JSEvents_resizeCanvasForFullscreen'],
  JSEvents_requestFullscreen: function(target, strategy) {
    // EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT + EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE is a mode where no extra logic is performed to the DOM elements.
    if (strategy.scaleMode != {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT') }}} || strategy.canvasResolutionScaleMode != {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE') }}}) {
      _JSEvents_resizeCanvasForFullscreen(target, strategy);
    }

    if (target.requestFullscreen) {
      target.requestFullscreen();
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED // https://caniuse.com/#feat=fullscreen
    } else if (target.msRequestFullscreen) {
      target.msRequestFullscreen();
#endif
#if MIN_FIREFOX_VERSION <= 63 // https://caniuse.com/#feat=fullscreen
    } else if (target.mozRequestFullScreen) {
      target.mozRequestFullScreen();
    } else if (target.mozRequestFullscreen) {
      target.mozRequestFullscreen();
#endif
#if MIN_CHROME_VERSION <= 70 || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
    } else if (target.webkitRequestFullscreen) {
      target.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
#endif
    } else {
      return JSEvents.fullscreenEnabled() ? {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}} : {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }

    __currentFullscreenStrategy = strategy;

    if (strategy.canvasResizedCallback) {
#if USE_PTHREADS
      if (strategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
      else
#endif
      {{{ makeDynCall('iiii', 'strategy.canvasResizedCallback') }}}({{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
    }

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  JSEvents_resizeCanvasForFullscreen__deps: ['_registerRestoreOldStyle', '_get_canvas_element_size', '_setLetterbox', '_set_canvas_element_size', '_getBoundingClientRect'],
  JSEvents_resizeCanvasForFullscreen: function(target, strategy) {
    var restoreOldStyle = __registerRestoreOldStyle(target);
    var cssWidth = strategy.softFullscreen ? innerWidth : screen.width;
    var cssHeight = strategy.softFullscreen ? innerHeight : screen.height;
    var rect = __getBoundingClientRect(target);
#if MIN_IE_VERSION < 9
    // .getBoundingClientRect(element).width & .height do not work on IE 8 and older, IE 9+ is required
    // (https://developer.mozilla.org/en-US/docs/Web/API/Element/getBoundingClientRect)
    var windowedCssWidth = rect.right - rect.left;
    var windowedCssHeight = rect.bottom - rect.top;
#else
    var windowedCssWidth = rect.width;
    var windowedCssHeight = rect.height;
#endif
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

    var dpiScale = (strategy.canvasResolutionScaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF') }}}) ? devicePixelRatio : 1;
    if (strategy.canvasResolutionScaleMode != {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE') }}}) {
      var newWidth = (cssWidth * dpiScale)|0;
      var newHeight = (cssHeight * dpiScale)|0;
      __set_canvas_element_size(target, newWidth, newHeight);
      if (target.GLctxObject) target.GLctxObject.GLctx.viewport(0, 0, newWidth, newHeight);
    }
    return restoreOldStyle;
  },

  _registerRestoreOldStyle__deps: ['$JSEvents', '_get_canvas_element_size', '_set_canvas_element_size'],
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
      var fullscreenElement = document.fullscreenElement
#if MIN_FIREFOX_VERSION <= 63 // https://caniuse.com/#feat=mdn-api_documentorshadowroot_fullscreenelement
        || document.mozFullScreenElement
#endif
#if MIN_EDGE_VERSION != TARGET_NOT_SUPPORTED || MIN_CHROME_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED // https://caniuse.com/#feat=mdn-api_documentorshadowroot_fullscreenelement
        || document.webkitFullscreenElement
#endif
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_EDGE_VERSION < 76
        || document.msFullscreenElement
#endif
        ;
      if (!fullscreenElement) {
        document.removeEventListener('fullscreenchange', restoreOldStyle);

#if MIN_FIREFOX_VERSION <= 63 // https://caniuse.com/#feat=mdn-api_element_fullscreenchange_event
        document.removeEventListener('mozfullscreenchange', restoreOldStyle);
#endif

#if MIN_CHROME_VERSION < 71 || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
        // Unprefixed Fullscreen API shipped in Chromium 71 (https://bugs.chromium.org/p/chromium/issues/detail?id=383813)
        // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
        document.removeEventListener('webkitfullscreenchange', restoreOldStyle);
#endif

#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED // https://caniuse.com/#feat=mdn-api_document_fullscreenchange_event
        document.removeEventListener('MSFullscreenChange', restoreOldStyle);
#endif

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
          {{{ makeDynCall('iiii', '__currentFullscreenStrategy.canvasResizedCallback') }}}({{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData);
        }
      }
    }
    document.addEventListener('fullscreenchange', restoreOldStyle);
#if MIN_FIREFOX_VERSION <= 63 // https://caniuse.com/#feat=mdn-api_element_fullscreenchange_event
    document.addEventListener('mozfullscreenchange', restoreOldStyle);
#endif
#if MIN_CHROME_VERSION < 71 || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
    // Unprefixed Fullscreen API shipped in Chromium 71 (https://bugs.chromium.org/p/chromium/issues/detail?id=383813)
    // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
    document.addEventListener('webkitfullscreenchange', restoreOldStyle);
#endif
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED // https://caniuse.com/#feat=mdn-api_document_fullscreenchange_event
    document.addEventListener('MSFullscreenChange', restoreOldStyle);
#endif
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
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED
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
#endif
      // Cannot use margin to specify letterboxes in FF or Chrome, since those ignore margins in fullscreen mode.
      element.style.paddingLeft = element.style.paddingRight = leftRight + 'px';
      element.style.paddingTop = element.style.paddingBottom = topBottom + 'px';
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED
    }
#endif
  },

  _currentFullscreenStrategy: {},
  _restoreOldWindowedStyle: null,

  _softFullscreenResizeWebGLRenderTarget__deps: ['$JSEvents', '_setLetterbox', '_currentFullscreenStrategy', '_get_canvas_element_size', '_set_canvas_element_size', '$jstoi_q'],
  _softFullscreenResizeWebGLRenderTarget: function() {
    var dpr = devicePixelRatio;
    var inHiDPIFullscreenMode = __currentFullscreenStrategy.canvasResolutionScaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF') }}};
    var inAspectRatioFixedFullscreenMode = __currentFullscreenStrategy.scaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT') }}};
    var inPixelPerfectFullscreenMode = __currentFullscreenStrategy.canvasResolutionScaleMode != {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE') }}};
    var inCenteredWithoutScalingFullscreenMode = __currentFullscreenStrategy.scaleMode == {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_CENTER') }}};
    var screenWidth = inHiDPIFullscreenMode ? Math.round(innerWidth*dpr) : innerWidth;
    var screenHeight = inHiDPIFullscreenMode ? Math.round(innerHeight*dpr) : innerHeight;
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
      topMargin /= dpr;
      w /= dpr;
      h /= dpr;
      // Round to nearest 4 digits of precision.
      w = Math.round(w*1e4)/1e4;
      h = Math.round(h*1e4)/1e4;
      topMargin = Math.round(topMargin*1e4)/1e4;
    }

    if (inCenteredWithoutScalingFullscreenMode) {
      var t = (innerHeight - jstoi_q(canvas.style.height)) / 2;
      var b = (innerWidth - jstoi_q(canvas.style.width)) / 2;
      __setLetterbox(canvas, t, b);
    } else {
      canvas.style.width = w + 'px';
      canvas.style.height = h + 'px';
      var b = (innerWidth - w) / 2;
      __setLetterbox(canvas, topMargin, b);
    }

    if (!inCenteredWithoutScalingFullscreenMode && __currentFullscreenStrategy.canvasResizedCallback) {
#if USE_PTHREADS
      if (__currentFullscreenStrategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(__currentFullscreenStrategy.canvasResizedCallbackTargetThread, __currentFullscreenStrategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData);
      else
#endif
      {{{ makeDynCall('iiii', '__currentFullscreenStrategy.canvasResizedCallback') }}}({{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, __currentFullscreenStrategy.canvasResizedCallbackUserData);
    }
  },

  // https://developer.mozilla.org/en-US/docs/Web/Guide/API/DOM/Using_full_screen_mode  
  _emscripten_do_request_fullscreen__deps: ['$JSEvents', '_setLetterbox', 'emscripten_set_canvas_element_size', 'emscripten_get_canvas_element_size', '_get_canvas_element_size', '_set_canvas_element_size', 'JSEvents_requestFullscreen', '$findEventTarget'],
  _emscripten_do_request_fullscreen: function(target, strategy) {
    if (!JSEvents.fullscreenEnabled()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    if (!target) target = '#canvas';
#endif
    target = findEventTarget(target);
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    if (!target.requestFullscreen
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED // https://caniuse.com/#feat=fullscreen
      && !target.msRequestFullscreen
#endif
#if MIN_FIREFOX_VERSION <= 63 // https://caniuse.com/#feat=fullscreen
      && !target.mozRequestFullScreen
      && !target.mozRequestFullscreen
#endif
#if MIN_CHROME_VERSION <= 70 || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
      && !target.webkitRequestFullscreen
#endif
      ) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    var canPerformRequests = JSEvents.canPerformEventHandlerRequests();

    // Queue this function call if we're not currently in an event handler and the user saw it appropriate to do so.
    if (!canPerformRequests) {
      if (strategy.deferUntilInEventHandler) {
        JSEvents.deferCall(_JSEvents_requestFullscreen, 1 /* priority over pointer lock */, [target, strategy]);
        return {{{ cDefine('EMSCRIPTEN_RESULT_DEFERRED') }}};
      } else {
        return {{{ cDefine('EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED') }}};
      }
    }
#endif

    return _JSEvents_requestFullscreen(target, strategy);
  },

  emscripten_request_fullscreen__deps: ['_emscripten_do_request_fullscreen'],
  emscripten_request_fullscreen__proxy: 'sync',
  emscripten_request_fullscreen__sig: 'iii',
  emscripten_request_fullscreen: function(target, deferUntilInEventHandler) {
    var strategy = {
      // These options perform no added logic, but just bare request fullscreen.
      scaleMode: {{{ cDefine('EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT') }}},
      canvasResolutionScaleMode: {{{ cDefine('EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE') }}},
      filteringMode: {{{ cDefine('EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT') }}},
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      deferUntilInEventHandler: deferUntilInEventHandler,
#endif
      canvasResizedCallbackTargetThread: {{{ cDefine('EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD') }}}
    };
    return __emscripten_do_request_fullscreen(target, strategy);
  },

  emscripten_request_fullscreen_strategy__deps: ['_emscripten_do_request_fullscreen', '_currentFullscreenStrategy', '_registerRestoreOldStyle'],
  emscripten_request_fullscreen_strategy__proxy: 'sync',
  emscripten_request_fullscreen_strategy__sig: 'iiii',
  emscripten_request_fullscreen_strategy: function(target, deferUntilInEventHandler, fullscreenStrategy) {
    var strategy = {
      scaleMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.scaleMode, 'i32') }}},
      canvasResolutionScaleMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResolutionScaleMode, 'i32') }}},
      filteringMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.filteringMode, 'i32') }}},
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      deferUntilInEventHandler: deferUntilInEventHandler,
#endif
#if USE_PTHREADS
      canvasResizedCallbackTargetThread: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackTargetThread, 'i32') }}},
#endif
      canvasResizedCallback: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallback, 'i32') }}},
      canvasResizedCallbackUserData: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackUserData, 'i32') }}}
    };

    return __emscripten_do_request_fullscreen(target, strategy);
  },

  emscripten_enter_soft_fullscreen__deps: ['$JSEvents', '_setLetterbox', '_hideEverythingExceptGivenElement', '_restoreOldWindowedStyle', '_registerRestoreOldStyle', '_restoreHiddenElements', '_currentFullscreenStrategy', '_softFullscreenResizeWebGLRenderTarget', '_get_canvas_element_size', '_set_canvas_element_size', 'JSEvents_resizeCanvasForFullscreen', '$findEventTarget'],
  emscripten_enter_soft_fullscreen__proxy: 'sync',
  emscripten_enter_soft_fullscreen__sig: 'iii',
  emscripten_enter_soft_fullscreen: function(target, fullscreenStrategy) {
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    if (!target) target = '#canvas';
#endif
    target = findEventTarget(target);
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    var strategy = {
        scaleMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.scaleMode, 'i32') }}},
        canvasResolutionScaleMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResolutionScaleMode, 'i32') }}},
        filteringMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.filteringMode, 'i32') }}},
        canvasResizedCallback: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallback, 'i32') }}},
        canvasResizedCallbackUserData: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackUserData, 'i32') }}},
#if USE_PTHREADS
        canvasResizedCallbackTargetThread: JSEvents.getTargetThreadForEventCallback(),
#endif
        target: target,
        softFullscreen: true
    };

    var restoreOldStyle = _JSEvents_resizeCanvasForFullscreen(target, strategy);

    document.documentElement.style.overflow = 'hidden';  // Firefox, Chrome
    document.body.scroll = "no"; // IE11
    document.body.style.margin = '0px'; // Override default document margin area on all browsers.

    var hiddenElements = __hideEverythingExceptGivenElement(target);

    function restoreWindowedState() {
      restoreOldStyle();
      __restoreHiddenElements(hiddenElements);
      removeEventListener('resize', __softFullscreenResizeWebGLRenderTarget);
      if (strategy.canvasResizedCallback) {
#if USE_PTHREADS
        if (strategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
        else
#endif
        {{{ makeDynCall('iiii', 'strategy.canvasResizedCallback') }}}({{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
      }
      __currentFullscreenStrategy = 0;
    }
    __restoreOldWindowedStyle = restoreWindowedState;
    __currentFullscreenStrategy = strategy;
    addEventListener('resize', __softFullscreenResizeWebGLRenderTarget);

    // Inform the caller that the canvas size has changed.
    if (strategy.canvasResizedCallback) {
#if USE_PTHREADS
      if (strategy.canvasResizedCallbackTargetThread) JSEvents.queueEventHandlerOnThread_iiii(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, {{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
      else
#endif
      {{{ makeDynCall('iiii', 'strategy.canvasResizedCallback') }}}({{{ cDefine('EMSCRIPTEN_EVENT_CANVASRESIZED') }}}, 0, strategy.canvasResizedCallbackUserData);
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

  emscripten_exit_fullscreen__deps: ['$JSEvents', '_currentFullscreenStrategy', 'JSEvents_requestFullscreen', '$specialHTMLTargets'],
  emscripten_exit_fullscreen__proxy: 'sync',
  emscripten_exit_fullscreen__sig: 'i',
  emscripten_exit_fullscreen: function() {
    if (!JSEvents.fullscreenEnabled()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // Make sure no queued up calls will fire after this.
    JSEvents.removeDeferredCalls(_JSEvents_requestFullscreen);
#endif

    var d = specialHTMLTargets[{{{ cDefine('EMSCRIPTEN_EVENT_TARGET_DOCUMENT') }}}];
    if (d.exitFullscreen) {
      d.fullscreenElement && d.exitFullscreen();
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED // https://caniuse.com/#feat=mdn-api_document_exitfullscreen
    } else if (d.msExitFullscreen) {
      d.msFullscreenElement && d.msExitFullscreen();
#endif
#if MIN_FIREFOX_VERSION < 64 // https://caniuse.com/#feat=mdn-api_document_exitfullscreen
    } else if (d.mozCancelFullScreen) {
      d.mozFullScreenElement && d.mozCancelFullScreen();
#endif
#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED // https://caniuse.com/#feat=mdn-api_document_exitfullscreen
    } else if (d.webkitExitFullscreen) {
      d.webkitFullscreenElement && d.webkitExitFullscreen();
#endif
    } else {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _fillPointerlockChangeEventData__deps: ['$JSEvents'],
  _fillPointerlockChangeEventData: function(eventStruct) {
    var pointerLockElement = document.pointerLockElement || document.mozPointerLockElement || document.webkitPointerLockElement || document.msPointerLockElement;
    var isPointerlocked = !!pointerLockElement;
    /** @suppress {checkTypes} */
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenPointerlockChangeEvent.isActive, 'isPointerlocked', 'i32') }}};
    var nodeName = JSEvents.getNodeNameForTarget(pointerLockElement);
    var id = (pointerLockElement && pointerLockElement.id) ? pointerLockElement.id : '';
    stringToUTF8(nodeName, eventStruct + {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.nodeName }}}, {{{ cDefine('EM_HTML5_LONG_STRING_LEN_BYTES') }}});
    stringToUTF8(id, eventStruct + {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.id }}}, {{{ cDefine('EM_HTML5_LONG_STRING_LEN_BYTES') }}});
  },

  _registerPointerlockChangeEventCallback__deps: ['$JSEvents', '_fillPointerlockChangeEventData', '$findEventTarget'],
  _registerPointerlockChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.pointerlockChangeEvent) JSEvents.pointerlockChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.__size__ }}} );

    var pointerlockChangeEventHandlerFunc = function(ev) {
      var e = ev || event;

#if USE_PTHREADS
      var pointerlockChangeEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.__size__ }}} ) : JSEvents.pointerlockChangeEvent;
#else
      var pointerlockChangeEvent = JSEvents.pointerlockChangeEvent;
#endif
      __fillPointerlockChangeEventData(pointerlockChangeEvent);

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, pointerlockChangeEvent, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, pointerlockChangeEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: target,
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: pointerlockChangeEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_pointerlockchange_callback_on_thread__proxy: 'sync',
  emscripten_set_pointerlockchange_callback_on_thread__sig: 'iiiiii',
  emscripten_set_pointerlockchange_callback_on_thread__deps: ['$JSEvents', '_registerPointerlockChangeEventCallback', '$findEventTarget', '$specialHTMLTargets'],
  emscripten_set_pointerlockchange_callback_on_thread__docs: '/** @suppress {missingProperties} */', // Closure does not see document.body.mozRequestPointerLock etc.
  emscripten_set_pointerlockchange_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    // TODO: Currently not supported in pthreads or in --proxy-to-worker mode. (In pthreads mode, document object is not defined)
    if (!document || !document.body || (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock)) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }

#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : specialHTMLTargets[{{{ cDefine('EMSCRIPTEN_EVENT_TARGET_DOCUMENT') }}}]; // Pointer lock change events need to be captured from 'document' by default instead of 'window'
#endif
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    __registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "pointerlockchange", targetThread);
    __registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "mozpointerlockchange", targetThread);
    __registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "webkitpointerlockchange", targetThread);
    __registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKCHANGE') }}}, "mspointerlockchange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _registerPointerlockErrorEventCallback__deps: ['$JSEvents', '$findEventTarget', '$specialHTMLTargets'],
  _registerPointerlockErrorEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif

    var pointerlockErrorEventHandlerFunc = function(ev) {
      var e = ev || event;

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, 0, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, 0, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: target,
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: pointerlockErrorEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_pointerlockerror_callback_on_thread__proxy: 'sync',
  emscripten_set_pointerlockerror_callback_on_thread__sig: 'iiiiii',
  emscripten_set_pointerlockerror_callback_on_thread__deps: ['$JSEvents', '_registerPointerlockErrorEventCallback', '$findEventTarget', '$specialHTMLTargets'],
  emscripten_set_pointerlockerror_callback_on_thread__docs: '/** @suppress {missingProperties} */', // Closure does not see document.body.mozRequestPointerLock etc.
  emscripten_set_pointerlockerror_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    // TODO: Currently not supported in pthreads or in --proxy-to-worker mode. (In pthreads mode, document object is not defined)
    if (!document || !document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }

#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : specialHTMLTargets[{{{ cDefine('EMSCRIPTEN_EVENT_TARGET_DOCUMENT') }}}]; // Pointer lock change events need to be captured from 'document' by default instead of 'window'
#endif

    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    __registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "pointerlockerror", targetThread);
    __registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "mozpointerlockerror", targetThread);
    __registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "webkitpointerlockerror", targetThread);
    __registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_POINTERLOCKERROR') }}}, "mspointerlockerror", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_pointerlock_status__proxy: 'sync',
  emscripten_get_pointerlock_status__sig: 'ii',
  emscripten_get_pointerlock_status__deps: ['_fillPointerlockChangeEventData'],
  emscripten_get_pointerlock_status__docs: '/** @suppress {missingProperties} */', // Closure does not see document.body.mozRequestPointerLock etc.
  emscripten_get_pointerlock_status: function(pointerlockStatus) {
    if (pointerlockStatus) __fillPointerlockChangeEventData(pointerlockStatus);
    if (!document.body || (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock)) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _requestPointerLock: function(target) {
    if (target.requestPointerLock) {
      target.requestPointerLock();
#if MIN_FIREFOX_VERSION <= 40 // https://caniuse.com/#feat=pointerlock
    } else if (target.mozRequestPointerLock) {
      target.mozRequestPointerLock();
#endif
#if MIN_CHROME_VERSION <= 36 // https://caniuse.com/#feat=pointerlock
    } else if (target.webkitRequestPointerLock) {
      target.webkitRequestPointerLock();
#endif
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_EDGE_VERSION < 76
    } else if (target.msRequestPointerLock) {
      target.msRequestPointerLock();
#endif
    } else {
      // document.body is known to accept pointer lock, so use that to differentiate if the user passed a bad element,
      // or if the whole browser just doesn't support the feature.
      if (document.body.requestPointerLock
#if MIN_FIREFOX_VERSION <= 40 // https://caniuse.com/#feat=pointerlock
        || document.body.mozRequestPointerLock
#endif
#if MIN_CHROME_VERSION <= 36 // https://caniuse.com/#feat=pointerlock
        || document.body.webkitRequestPointerLock
#endif
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_EDGE_VERSION < 76
        || document.body.msRequestPointerLock
#endif
        ) {
        return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
      } else {
        return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
      }
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_request_pointerlock__proxy: 'sync',
  emscripten_request_pointerlock__sig: 'iii',
  emscripten_request_pointerlock__deps: ['$JSEvents', '_requestPointerLock', '$findEventTarget'],
  emscripten_request_pointerlock: function(target, deferUntilInEventHandler) {
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    if (!target) target = '#canvas';
#endif
    target = findEventTarget(target);
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
    if (!target.requestPointerLock
#if MIN_FIREFOX_VERSION <= 40 // https://caniuse.com/#feat=pointerlock
      && !target.mozRequestPointerLock
#endif
#if MIN_CHROME_VERSION <= 36 // https://caniuse.com/#feat=pointerlock
      && !target.webkitRequestPointerLock
#endif
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_EDGE_VERSION < 76
      && !target.msRequestPointerLock
#endif
      ) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }

#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    var canPerformRequests = JSEvents.canPerformEventHandlerRequests();

    // Queue this function call if we're not currently in an event handler and the user saw it appropriate to do so.
    if (!canPerformRequests) {
      if (deferUntilInEventHandler) {
        JSEvents.deferCall(__requestPointerLock, 2 /* priority below fullscreen */, [target]);
        return {{{ cDefine('EMSCRIPTEN_RESULT_DEFERRED') }}};
      } else {
        return {{{ cDefine('EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED') }}};
      }
    }
#endif

    return __requestPointerLock(target);
  },

  emscripten_exit_pointerlock__proxy: 'sync',
  emscripten_exit_pointerlock__sig: 'i',
  emscripten_exit_pointerlock__deps: ['$JSEvents', '_requestPointerLock'],
  emscripten_exit_pointerlock: function() {
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // Make sure no queued up calls will fire after this.
    JSEvents.removeDeferredCalls(__requestPointerLock);
#endif

    if (document.exitPointerLock) {
      document.exitPointerLock();
#if MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_EDGE_VERSION < 76
    } else if (document.msExitPointerLock) {
      document.msExitPointerLock();
#endif
#if MIN_FIREFOX_VERSION <= 40 // https://caniuse.com/#feat=pointerlock
    } else if (document.mozExitPointerLock) {
      document.mozExitPointerLock();
#endif
#if MIN_CHROME_VERSION <= 36 // https://caniuse.com/#feat=pointerlock
    } else if (document.webkitExitPointerLock) {
      document.webkitExitPointerLock();
#endif
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

  _fillVisibilityChangeEventData: function(eventStruct) {
    var visibilityStates = [ "hidden", "visible", "prerender", "unloaded" ];
    var visibilityState = visibilityStates.indexOf(document.visibilityState);

    // Assigning a boolean to HEAP32 with expected type coercion.
    /** @suppress {checkTypes} */
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenVisibilityChangeEvent.hidden, 'document.hidden', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenVisibilityChangeEvent.visibilityState, 'visibilityState', 'i32') }}};
  },

  _registerVisibilityChangeEventCallback__deps: ['$JSEvents', '_fillVisibilityChangeEventData', '$findEventTarget'],
  _registerVisibilityChangeEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.visibilityChangeEvent) JSEvents.visibilityChangeEvent = _malloc( {{{ C_STRUCTS.EmscriptenVisibilityChangeEvent.__size__ }}} );

    var visibilityChangeEventHandlerFunc = function(ev) {
      var e = ev || event;

#if USE_PTHREADS
      var visibilityChangeEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenVisibilityChangeEvent.__size__ }}} ) : JSEvents.visibilityChangeEvent;
#else
      var visibilityChangeEvent = JSEvents.visibilityChangeEvent;
#endif

      __fillVisibilityChangeEventData(visibilityChangeEvent);

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, visibilityChangeEvent, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, visibilityChangeEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: target,
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: visibilityChangeEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_visibilitychange_callback_on_thread__proxy: 'sync',
  emscripten_set_visibilitychange_callback_on_thread__sig: 'iiiii',
  emscripten_set_visibilitychange_callback_on_thread__deps: ['_registerVisibilityChangeEventCallback', '$specialHTMLTargets'],
  emscripten_set_visibilitychange_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
#if ENVIRONMENT_MAY_BE_WORKER || ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
  if (!specialHTMLTargets[{{{ cDefine('EMSCRIPTEN_EVENT_TARGET_DOCUMENT') }}}]) {
    return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};
  }
#endif
    __registerVisibilityChangeEventCallback(specialHTMLTargets[{{{ cDefine('EMSCRIPTEN_EVENT_TARGET_DOCUMENT') }}}], userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_VISIBILITYCHANGE') }}}, "visibilitychange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_visibility_status__proxy: 'sync',
  emscripten_get_visibility_status__sig: 'ii',
  emscripten_get_visibility_status__deps: ['_fillVisibilityChangeEventData'],
  emscripten_get_visibility_status: function(visibilityStatus) {
    if (typeof document.visibilityState === 'undefined' && typeof document.hidden === 'undefined') {
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    __fillVisibilityChangeEventData(visibilityStatus);  
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _registerTouchEventCallback__deps: ['$JSEvents', '$findEventTarget', '_getBoundingClientRect'],
  _registerTouchEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.touchEvent) JSEvents.touchEvent = _malloc( {{{ C_STRUCTS.EmscriptenTouchEvent.__size__ }}} );

    target = findEventTarget(target);

    var touchEventHandlerFunc = function(e) {
#if ASSERTIONS
      assert(e);
#endif
      var touches = {};
      var et = e.touches;
      for(var i = 0; i < et.length; ++i) {
        var touch = et[i];
#if ASSERTIONS
        // Verify that browser does not recycle touch objects with old stale data, but reports new ones each time.
        assert(!touch.isChanged);
        assert(!touch.onTarget);
#endif
        touches[touch.identifier] = touch;
      }
      et = e.changedTouches;
      for(var i = 0; i < et.length; ++i) {
        var touch = et[i];
#if ASSERTIONS
        // Verify that browser does not recycle touch objects with old stale data, but reports new ones each time.
        assert(!touch.onTarget);
#endif
        touch.isChanged = 1;
        touches[touch.identifier] = touch;
      }
      et = e.targetTouches;
      for(var i = 0; i < et.length; ++i) {
        touches[et[i].identifier].onTarget = 1;
      }

#if USE_PTHREADS
      var touchEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenTouchEvent.__size__ }}} ) : JSEvents.touchEvent;
#else
      var touchEvent = JSEvents.touchEvent;
#endif
      var idx = touchEvent>>2; // Pre-shift the ptr to index to HEAP32 to save code size
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchEvent.ctrlKey / 4}}}] = e.ctrlKey;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchEvent.shiftKey / 4}}}] = e.shiftKey;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchEvent.altKey / 4}}}] = e.altKey;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchEvent.metaKey / 4}}}] = e.metaKey;
      idx += {{{ C_STRUCTS.EmscriptenTouchEvent.touches / 4 }}}; // Advance to the start of the touch array.
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
      var canvasRect = Module['canvas'] ? __getBoundingClientRect(Module['canvas']) : undefined;
#endif
      var targetRect = __getBoundingClientRect(target);
      var numTouches = 0;
      for(var i in touches) {
        var t = touches[i];
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.identifier / 4}}}] = t.identifier;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.screenX / 4}}}] = t.screenX;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.screenY / 4}}}] = t.screenY;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.clientX / 4}}}] = t.clientX;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.clientY / 4}}}] = t.clientY;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.pageX / 4}}}] = t.pageX;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.pageY / 4}}}] = t.pageY;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.isChanged / 4}}}] = t.isChanged;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.onTarget / 4}}}] = t.onTarget;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.targetX / 4}}}] = t.clientX - targetRect.left;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.targetY / 4}}}] = t.clientY - targetRect.top;
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.canvasX / 4}}}] = canvasRect ? t.clientX - canvasRect.left : 0;
        HEAP32[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.canvasY / 4}}}] = canvasRect ? t.clientY - canvasRect.top : 0;
#endif

        idx += {{{ C_STRUCTS.EmscriptenTouchPoint.__size__ / 4 }}};

        if (++numTouches > 31) {
          break;
        }
      }
      {{{ makeSetValue('touchEvent', C_STRUCTS.EmscriptenTouchEvent.numTouches, 'numTouches', 'i32') }}};

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, touchEvent, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, touchEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: target,
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      allowsDeferredCalls: eventTypeString == 'touchstart' || eventTypeString == 'touchend',
#endif
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: touchEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_touchstart_callback_on_thread__proxy: 'sync',
  emscripten_set_touchstart_callback_on_thread__sig: 'iiiiii',
  emscripten_set_touchstart_callback_on_thread__deps: ['_registerTouchEventCallback'],
  emscripten_set_touchstart_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHSTART') }}}, "touchstart", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchend_callback_on_thread__proxy: 'sync',
  emscripten_set_touchend_callback_on_thread__sig: 'iiiiii',
  emscripten_set_touchend_callback_on_thread__deps: ['_registerTouchEventCallback'],
  emscripten_set_touchend_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHEND') }}}, "touchend", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchmove_callback_on_thread__proxy: 'sync',
  emscripten_set_touchmove_callback_on_thread__sig: 'iiiiii',
  emscripten_set_touchmove_callback_on_thread__deps: ['_registerTouchEventCallback'],
  emscripten_set_touchmove_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHMOVE') }}}, "touchmove", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_touchcancel_callback_on_thread__proxy: 'sync',
  emscripten_set_touchcancel_callback_on_thread__sig: 'iiiiii',
  emscripten_set_touchcancel_callback_on_thread__deps: ['_registerTouchEventCallback'],
  emscripten_set_touchcancel_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_TOUCHCANCEL') }}}, "touchcancel", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _fillGamepadEventData: function(eventStruct, e) {
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
        // Assigning a boolean to HEAP32, that's ok, but Closure would like to warn about it:
        /** @suppress {checkTypes} */
        {{{ makeSetValue('eventStruct+i*4', C_STRUCTS.EmscriptenGamepadEvent.digitalButton, 'e.buttons[i] == 1', 'i32') }}};
      }
    }
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.connected, 'e.connected', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.index, 'e.index', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.numAxes, 'e.axes.length', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.numButtons, 'e.buttons.length', 'i32') }}};
    stringToUTF8(e.id, eventStruct + {{{ C_STRUCTS.EmscriptenGamepadEvent.id }}}, {{{ cDefine('EM_HTML5_MEDIUM_STRING_LEN_BYTES') }}});
    stringToUTF8(e.mapping, eventStruct + {{{ C_STRUCTS.EmscriptenGamepadEvent.mapping }}}, {{{ cDefine('EM_HTML5_MEDIUM_STRING_LEN_BYTES') }}});
  },

  _registerGamepadEventCallback__deps: ['$JSEvents', '_fillGamepadEventData', '$findEventTarget'],
  _registerGamepadEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.gamepadEvent) JSEvents.gamepadEvent = _malloc( {{{ C_STRUCTS.EmscriptenGamepadEvent.__size__ }}} );

    var gamepadEventHandlerFunc = function(ev) {
      var e = ev || event;

#if USE_PTHREADS
      var gamepadEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenGamepadEvent.__size__ }}} ) : JSEvents.gamepadEvent;
#else
      var gamepadEvent = JSEvents.gamepadEvent;
#endif
      __fillGamepadEventData(gamepadEvent, e["gamepad"]);

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, gamepadEvent, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, gamepadEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      allowsDeferredCalls: true,
#endif
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: gamepadEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_gamepadconnected_callback_on_thread__proxy: 'sync',
  emscripten_set_gamepadconnected_callback_on_thread__sig: 'iiiii',
  emscripten_set_gamepadconnected_callback_on_thread__deps: ['_registerGamepadEventCallback'],
  emscripten_set_gamepadconnected_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    if (!navigator.getGamepads && !navigator.webkitGetGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    __registerGamepadEventCallback({{{ cDefine('EMSCRIPTEN_EVENT_TARGET_WINDOW') }}}, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADCONNECTED') }}}, "gamepadconnected", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_set_gamepaddisconnected_callback_on_thread__proxy: 'sync',
  emscripten_set_gamepaddisconnected_callback_on_thread__sig: 'iiiii',
  emscripten_set_gamepaddisconnected_callback_on_thread__deps: ['_registerGamepadEventCallback'],
  emscripten_set_gamepaddisconnected_callback_on_thread: function(userData, useCapture, callbackfunc, targetThread) {
    if (!navigator.getGamepads && !navigator.webkitGetGamepads) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    __registerGamepadEventCallback({{{ cDefine('EMSCRIPTEN_EVENT_TARGET_WINDOW') }}}, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED') }}}, "gamepaddisconnected", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_sample_gamepad_data__proxy: 'sync',
  emscripten_sample_gamepad_data__sig: 'i',
  emscripten_sample_gamepad_data__deps: ['$JSEvents'],
  emscripten_sample_gamepad_data: function() {
    return (JSEvents.lastGamepadState = (navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads() : null)))
      ? {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}} : {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
  },

  emscripten_get_num_gamepads__proxy: 'sync',
  emscripten_get_num_gamepads__sig: 'i',
  emscripten_get_num_gamepads__deps: ['$JSEvents'],
  emscripten_get_num_gamepads: function() {
#if ASSERTIONS
    if (!JSEvents.lastGamepadState) throw 'emscripten_get_num_gamepads() can only be called after having first called emscripten_sample_gamepad_data() and that function has returned EMSCRIPTEN_RESULT_SUCCESS!';
#endif
    // N.B. Do not call emscripten_get_num_gamepads() unless having first called emscripten_sample_gamepad_data(), and that has returned EMSCRIPTEN_RESULT_SUCCESS.
    // Otherwise the following line will throw an exception.
    return JSEvents.lastGamepadState.length;
  },
  
  emscripten_get_gamepad_status__proxy: 'sync',
  emscripten_get_gamepad_status__sig: 'iii',
  emscripten_get_gamepad_status__deps: ['$JSEvents', '_fillGamepadEventData'],
  emscripten_get_gamepad_status: function(index, gamepadState) {
#if ASSERTIONS
    if (!JSEvents.lastGamepadState) throw 'emscripten_get_gamepad_status() can only be called after having first called emscripten_sample_gamepad_data() and that function has returned EMSCRIPTEN_RESULT_SUCCESS!';
#endif

    // INVALID_PARAM is returned on a Gamepad index that never was there.
    if (index < 0 || index >= JSEvents.lastGamepadState.length) return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};

    // NO_DATA is returned on a Gamepad index that was removed.
    // For previously disconnected gamepads there should be an empty slot (null/undefined/false) at the index.
    // This is because gamepads must keep their original position in the array.
    // For example, removing the first of two gamepads produces [null/undefined/false, gamepad].
    if (!JSEvents.lastGamepadState[index]) return {{{ cDefine('EMSCRIPTEN_RESULT_NO_DATA') }}};

    __fillGamepadEventData(gamepadState, JSEvents.lastGamepadState[index]);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  _registerBeforeUnloadEventCallback__deps: ['$JSEvents', '$findEventTarget'],
  _registerBeforeUnloadEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
    var beforeUnloadEventHandlerFunc = function(ev) {
      var e = ev || event;

      // Note: This is always called on the main browser thread, since it needs synchronously return a value!
      var confirmationMessage = {{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, 0, userData);
      
      if (confirmationMessage) {
        confirmationMessage = UTF8ToString(confirmationMessage);
      }
      if (confirmationMessage) {
        e.preventDefault();
        e.returnValue = confirmationMessage;
        return confirmationMessage;
      }
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: beforeUnloadEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_beforeunload_callback_on_thread__proxy: 'sync',
  emscripten_set_beforeunload_callback_on_thread__sig: 'iii',
  emscripten_set_beforeunload_callback_on_thread__deps: ['_registerBeforeUnloadEventCallback'],
  emscripten_set_beforeunload_callback_on_thread: function(userData, callbackfunc, targetThread) {
    if (typeof onbeforeunload === 'undefined') return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    // beforeunload callback can only be registered on the main browser thread, because the page will go away immediately after returning from the handler,
    // and there is no time to start proxying it anywhere.
    if (targetThread !== {{{ cDefine('EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD') }}}) return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
    __registerBeforeUnloadEventCallback({{{ cDefine('EMSCRIPTEN_EVENT_TARGET_WINDOW') }}}, userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BEFOREUNLOAD') }}}, "beforeunload");
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  _fillBatteryEventData: function(eventStruct, e) {
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.chargingTime, 'e.chargingTime', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.dischargingTime, 'e.dischargingTime', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.level, 'e.level', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.charging, 'e.charging', 'i32') }}};
  },

  _battery: function() { return navigator.battery || navigator.mozBattery || navigator.webkitBattery; },

  _registerBatteryEventCallback__deps: ['$JSEvents', '_fillBatteryEventData', '_battery', '$findEventTarget'],
  _registerBatteryEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    if (!JSEvents.batteryEvent) JSEvents.batteryEvent = _malloc( {{{ C_STRUCTS.EmscriptenBatteryEvent.__size__ }}} );

    var batteryEventHandlerFunc = function(ev) {
      var e = ev || event;

#if USE_PTHREADS
      var batteryEvent = targetThread ? _malloc( {{{ C_STRUCTS.EmscriptenBatteryEvent.__size__ }}} ) : JSEvents.batteryEvent;
#else
      var batteryEvent = JSEvents.batteryEvent;
#endif
      __fillBatteryEventData(batteryEvent, __battery());

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, batteryEvent, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, batteryEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: batteryEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_batterychargingchange_callback_on_thread__proxy: 'sync',
  emscripten_set_batterychargingchange_callback_on_thread__sig: 'iii',
  emscripten_set_batterychargingchange_callback_on_thread__deps: ['_registerBatteryEventCallback', '_battery'],
  emscripten_set_batterychargingchange_callback_on_thread: function(userData, callbackfunc, targetThread) {
    if (!__battery()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}}; 
    __registerBatteryEventCallback(__battery(), userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BATTERYCHARGINGCHANGE') }}}, "chargingchange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_batterylevelchange_callback_on_thread__proxy: 'sync',
  emscripten_set_batterylevelchange_callback_on_thread__sig: 'iii',
  emscripten_set_batterylevelchange_callback_on_thread__deps: ['_registerBatteryEventCallback', '_battery'],
  emscripten_set_batterylevelchange_callback_on_thread: function(userData, callbackfunc, targetThread) {
    if (!__battery()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}}; 
    __registerBatteryEventCallback(__battery(), userData, true, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_BATTERYLEVELCHANGE') }}}, "levelchange", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },
  
  emscripten_get_battery_status__proxy: 'sync',
  emscripten_get_battery_status__sig: 'ii',
  emscripten_get_battery_status__deps: ['_fillBatteryEventData', '_battery'],
  emscripten_get_battery_status: function(batteryState) {
    if (!__battery()) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}}; 
    __fillBatteryEventData(batteryState, __battery());
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

#if USE_PTHREADS
  emscripten_set_canvas_element_size_calling_thread__deps: ['$JSEvents', 'emscripten_set_offscreencanvas_size_on_target_thread', '$findCanvasEventTarget'],
  emscripten_set_canvas_element_size_calling_thread: function(target, width, height) {
    var canvas = findCanvasEventTarget(target);
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
        var prevViewport = canvas.GLctxObject.GLctx.getParameter(0xBA2 /* GL_VIEWPORT */);
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

  emscripten_set_offscreencanvas_size_on_target_thread_js__deps: ['$stringToNewUTF8', '_emscripten_call_on_thread'],
  emscripten_set_offscreencanvas_size_on_target_thread_js: function(targetThread, targetCanvas, width, height) {
    var stackTop = stackSave();
    var varargs = stackAlloc(12);
    var targetCanvasPtr = 0;
    if (targetCanvas) {
      targetCanvasPtr = stringToNewUTF8(targetCanvas);
    }
    {{{ makeSetValue('varargs', 0, 'targetCanvasPtr', 'i32')}}};
    {{{ makeSetValue('varargs', 4, 'width', 'i32')}}};
    {{{ makeSetValue('varargs', 8, 'height', 'i32')}}};
    // Note: If we are also a pthread, the call below could theoretically be done synchronously. However if the target pthread is waiting for a mutex from us, then
    // these two threads will deadlock. At the moment, we'd like to consider that this kind of deadlock would be an Emscripten runtime bug, although if
    // emscripten_set_canvas_element_size() was documented to require running an event in the queue of thread that owns the OffscreenCanvas, then that might be ok.
    // (safer this way however)
    __emscripten_call_on_thread(0, targetThread, {{{ cDefine('EM_PROXIED_RESIZE_OFFSCREENCANVAS') }}}, 0, targetCanvasPtr /* satellite data */, varargs);
    stackRestore(stackTop);
  },

  emscripten_set_offscreencanvas_size_on_target_thread__deps: ['emscripten_set_offscreencanvas_size_on_target_thread_js'],
  emscripten_set_offscreencanvas_size_on_target_thread: function(targetThread, targetCanvas, width, height) {
    targetCanvas = targetCanvas ? UTF8ToString(targetCanvas) : '';
    _emscripten_set_offscreencanvas_size_on_target_thread_js(targetThread, targetCanvas, width, height);
  },

  emscripten_set_canvas_element_size_main_thread__proxy: 'sync',
  emscripten_set_canvas_element_size_main_thread__sig: 'iiii',
  emscripten_set_canvas_element_size_main_thread__deps: ['emscripten_set_canvas_element_size_calling_thread'],
  emscripten_set_canvas_element_size_main_thread: function(target, width, height) { return _emscripten_set_canvas_element_size_calling_thread(target, width, height); },

  emscripten_set_canvas_element_size__deps: ['$JSEvents', 'emscripten_set_canvas_element_size_calling_thread', 'emscripten_set_canvas_element_size_main_thread', '$findCanvasEventTarget'],
  emscripten_set_canvas_element_size: function(target, width, height) {
#if GL_DEBUG
    console.error('emscripten_set_canvas_element_size(target='+target+',width='+width+',height='+height);
#endif
    var canvas = findCanvasEventTarget(target);
    if (canvas) {
      return _emscripten_set_canvas_element_size_calling_thread(target, width, height);
    } else {
      return _emscripten_set_canvas_element_size_main_thread(target, width, height);
    }
  }, 
#else
  emscripten_set_canvas_element_size__deps: ['$JSEvents', '$findCanvasEventTarget'],
  emscripten_set_canvas_element_size: function(target, width, height) {
#if GL_DEBUG
    console.error('emscripten_set_canvas_element_size(target='+target+',width='+width+',height='+height);
#endif
    var canvas = findCanvasEventTarget(target);
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
  emscripten_get_canvas_element_size_calling_thread__deps: ['$JSEvents', '$findCanvasEventTarget'],
  emscripten_get_canvas_element_size_calling_thread: function(target, width, height) {
    var canvas = findCanvasEventTarget(target);
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

  emscripten_get_canvas_element_size__deps: ['$JSEvents', 'emscripten_get_canvas_element_size_calling_thread', 'emscripten_get_canvas_element_size_main_thread', '$findCanvasEventTarget'],
  emscripten_get_canvas_element_size: function(target, width, height) {
    var canvas = findCanvasEventTarget(target);
    if (canvas) {
      return _emscripten_get_canvas_element_size_calling_thread(target, width, height);
    } else {
      return _emscripten_get_canvas_element_size_main_thread(target, width, height);
    }
  }, 
#else
  emscripten_get_canvas_element_size__deps: ['$JSEvents', '$findCanvasEventTarget'],
  emscripten_get_canvas_element_size: function(target, width, height) {
    var canvas = findCanvasEventTarget(target);
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
  emscripten_set_element_css_size__deps: ['$JSEvents', '$findEventTarget'],
  emscripten_set_element_css_size: function(target, width, height) {
#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : Module['canvas'];
#endif
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    target.style.width = width + "px";
    target.style.height = height + "px";

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_get_element_css_size__proxy: 'sync',
  emscripten_get_element_css_size__sig: 'iiii',
  emscripten_get_element_css_size__deps: ['$JSEvents', '$findEventTarget', '_getBoundingClientRect'],
  emscripten_get_element_css_size: function(target, width, height) {
#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : Module['canvas'];
#endif
    if (!target) return {{{ cDefine('EMSCRIPTEN_RESULT_UNKNOWN_TARGET') }}};

    var rect = __getBoundingClientRect(target);
#if MIN_IE_VERSION < 9
    // .getBoundingClientRect(element).width & .height do not work on IE 8 and older, IE 9+ is required
    // (https://developer.mozilla.org/en-US/docs/Web/API/Element/getBoundingClientRect)
    {{{ makeSetValue('width', '0', 'rect.right - rect.left', 'double') }}};
    {{{ makeSetValue('height', '0', 'rect.bottom - rect.top', 'double') }}};
#else
    {{{ makeSetValue('width', '0', 'rect.width', 'double') }}};
    {{{ makeSetValue('height', '0', 'rect.height', 'double') }}};
#endif

    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_html5_remove_all_event_listeners__deps: ['$JSEvents'],
  emscripten_html5_remove_all_event_listeners: function() {
    JSEvents.removeAllEventListeners();
  },

  emscripten_request_animation_frame: function(cb, userData) {
    return requestAnimationFrame(function(timeStamp) {
      {{{ makeDynCall('idi', 'cb') }}}(timeStamp, userData);
    });
  },

  emscripten_cancel_animation_frame: function(id) {
    cancelAnimationFrame(id);
  },

  emscripten_request_animation_frame_loop: function(cb, userData) {
    function tick(timeStamp) {
      if ({{{ makeDynCall('idi', 'cb') }}}(timeStamp, userData)) {
        requestAnimationFrame(tick);
      }
    }
    return requestAnimationFrame(tick);
  },

  $polyfillSetImmediate__postset:
    'var __setImmediate_id_counter = 0;\n' +
    'var __setImmediate_queue = [];\n' +
    'var __setImmediate_message_id = "_si";\n' +
    'function __setImmediate_cb(e) {\n' +
      'if (e.data === __setImmediate_message_id) {\n' +
        'e.stopPropagation();\n' +
        '__setImmediate_queue.shift()();\n' +
        '++__setImmediate_id_counter;\n' +
      '}\n' +
    '}\n' +
    'if (typeof setImmediate === "undefined" && typeof addEventListener === "function") {\n' +
      'addEventListener("message", __setImmediate_cb, true);\n' +
      'setImmediate = function(func) {\n' +
        'postMessage(__setImmediate_message_id, "*");\n' +
        'return __setImmediate_id_counter + __setImmediate_queue.push(func) - 1;\n' +
      '}\n' +
      'clearImmediate = /**@type{function(number=)}*/(function(id) {\n' +
        'var index = id - __setImmediate_id_counter;\n' +
        'if (index >= 0 && index < __setImmediate_queue.length) __setImmediate_queue[index] = function(){};\n' + // must preserve the order and count of elements in the queue, so replace the pending callback with an empty function
      '})\n' +
    '}',

  $polyfillSetImmediate: function() { /* nop, used for its postset to ensure setImmediate() polyfill is not duplicated between emscripten_set_immediate() and emscripten_set_immediate_loop() if application links to both of them.*/ },

  emscripten_set_immediate__deps: ['$polyfillSetImmediate'],
  emscripten_set_immediate: function(cb, userData) {
    polyfillSetImmediate();
    return setImmediate(function() {
      {{{ makeDynCall('vi', 'cb') }}}(userData);
    });
  },

  emscripten_clear_immediate: function(id) {
    clearImmediate(id);
  },

  emscripten_set_immediate_loop__deps: ['$polyfillSetImmediate'],
  emscripten_set_immediate_loop: function(cb, userData) {
    polyfillSetImmediate();
    function tick() {
      if ({{{ makeDynCall('ii', 'cb') }}}(userData)) {
        setImmediate(tick);
      }
    }
    return setImmediate(tick);
  },

  emscripten_set_timeout: function(cb, msecs, userData) {
    return setTimeout(function() {
      {{{ makeDynCall('vi', 'cb') }}}(userData);
    }, msecs);
  },

  emscripten_clear_timeout: function(id) {
    clearTimeout(id);
  },

  emscripten_set_timeout_loop: function(cb, msecs, userData) {
    function tick() {
      var t = performance.now();
      var n = t + msecs;
      if ({{{ makeDynCall('idi', 'cb') }}}(t, userData)) {
        // Save a little bit of code space: modern browsers should treat
        // negative setTimeout as timeout of 0
        // (https://stackoverflow.com/questions/8430966/is-calling-settimeout-with-a-negative-delay-ok)
        setTimeout(tick, t - performance.now());
      }
    }
    return setTimeout(tick, 0);
  },

  emscripten_set_interval: function(cb, msecs, userData) {
    return setInterval(function() {
      {{{ makeDynCall('vi', 'cb') }}}(userData)
    }, msecs);
  },

  emscripten_clear_interval: function(id) {
    clearInterval(id);
  },

  emscripten_date_now: function() {
    return Date.now();
  },

  emscripten_performance_now: function() {
    return performance.now();
  },

  emscripten_console_log: function(str) {
#if ASSERTIONS
    assert(typeof str === 'number');
#endif
    console.log(UTF8ToString(str));
  },

  emscripten_console_warn: function(str) {
#if ASSERTIONS
    assert(typeof str === 'number');
#endif
    console.warn(UTF8ToString(str));
  },

  emscripten_console_error: function(str) {
#if ASSERTIONS
    assert(typeof str === 'number');
#endif
    console.error(UTF8ToString(str));
  },

  emscripten_throw_number: function(number) {
    throw number;
  },

  emscripten_throw_string: function(str) {
#if ASSERTIONS
    assert(typeof str === 'number');
#endif
    throw UTF8ToString(str);
  },

  emscripten_unwind_to_js_event_loop: function() {
    throw 'unwind';
  },

  emscripten_get_device_pixel_ratio__proxy: 'sync',
  emscripten_get_device_pixel_ratio__sig: 'd',
  emscripten_get_device_pixel_ratio: function() {
#if ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
    return (typeof devicePixelRatio === 'number' && devicePixelRatio) || 1.0;
#else // otherwise, on the web and in workers, things are simpler
#if MIN_IE_VERSION < 11 || MIN_FIREFOX_VERSION < 18 || MIN_CHROME_VERSION < 4 || MIN_SAFARI_VERSION < 3010 // https://caniuse.com/#feat=devicepixelratio
    return window.devicePixelRatio || 1.0;
#else
    return devicePixelRatio;
#endif
#endif
  }
};

mergeInto(LibraryManager.library, LibraryJSEvents);
