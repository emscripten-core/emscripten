/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryHTML5 = {
  $JSEvents__deps: [
#if PTHREADS
    '_emscripten_run_callback_on_thread',
#endif
    '$addOnExit',
  ],
  $JSEvents: {

#if USE_CLOSURE_COMPILER
    // pointers to structs malloc()ed to Emscripten HEAP for JS->C interop.
    batteryEvent: 0,
    gamepadEvent: 0,
    keyEvent: 0,
    mouseEvent: 0,
    wheelEvent: 0,
    uiEvent: 0,
    focusEvent: 0,
    deviceOrientationEvent: 0,
    orientationChangeEvent: 0,
    deviceMotionEvent: 0,
    fullscreenChangeEvent: 0,
    pointerlockChangeEvent: 0,
    visibilityChangeEvent: 0,
    touchEvent: 0,
#endif

/* We do not depend on the exact initial values of falsey member fields - these
   fields can be populated on-demand to save code size.
   (but still documented here to keep track of what is supposed to be present)

    // When we transition from fullscreen to windowed mode, we remember here the
    // element that was just in fullscreen mode so that we can report
    // information about that element in the event message.
    previousFullscreenElement: null,

    // When the C runtime exits via exit(), we unregister all event handlers
    // added by this library to be nice and clean.
    // Track in this field whether we have yet registered that onExit handler.
    removeEventListenersRegistered: false,

#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // If we are in an event handler, specifies the event handler object from
    // the eventHandlers array that is currently running.
    currentEventHandler: null,
#endif
*/
    removeAllEventListeners() {
      while (JSEvents.eventHandlers.length) {
        JSEvents._removeHandler(JSEvents.eventHandlers.length - 1);
      }
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      JSEvents.deferredCalls = [];
#endif
    },

#if EXIT_RUNTIME
    registerRemoveEventListeners() {
      if (!JSEvents.removeEventListenersRegistered) {
        addOnExit(JSEvents.removeAllEventListeners);
        JSEvents.removeEventListenersRegistered = true;
      }
    },
#endif

#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // If positive, we are currently executing in a JS event handler.
    // (this particular property must be initialized to zero, as we ++/-- it)
    inEventHandler: 0,

    deferredCalls: [],

    // Queues the given function call to occur the next time we enter an event handler.
    // Existing implementations of pointerlock apis have required that
    // the target element is active in fullscreen mode first. Therefore give
    // fullscreen mode request a precedence of 1 and pointer lock a precedence of 2
    // and sort by that to always request fullscreen before pointer lock.
    deferCall(targetFunction, precedence, argsList) {
      function arraysHaveEqualContent(arrA, arrB) {
        if (arrA.length != arrB.length) return false;

        for (var i in arrA) {
          if (arrA[i] != arrB[i]) return false;
        }
        return true;
      }
      // Test if the given call was already queued, and if so, don't add it again.
      for (var call of JSEvents.deferredCalls) {
        if (call.targetFunction == targetFunction && arraysHaveEqualContent(call.argsList, argsList)) {
          return;
        }
      }
      JSEvents.deferredCalls.push({
        targetFunction,
        precedence,
        argsList
      });

      JSEvents.deferredCalls.sort((x,y) => x.precedence < y.precedence);
    },

    // Erases all deferred calls to the given target function from the queue list.
    removeDeferredCalls(targetFunction) {
      JSEvents.deferredCalls = JSEvents.deferredCalls.filter((call) => call.targetFunction != targetFunction);
    },

    canPerformEventHandlerRequests() {
      if (navigator.userActivation) {
        // Verify against transient activation status from UserActivation API
        // whether it is possible to perform a request here without needing to defer. See
        // https://developer.mozilla.org/en-US/docs/Web/Security/User_activation#transient_activation
        // and https://caniuse.com/mdn-api_useractivation
        // At the time of writing, Firefox does not support this API: https://bugzil.la/1791079
        return navigator.userActivation.isActive;
      }

      return JSEvents.inEventHandler && JSEvents.currentEventHandler.allowsDeferredCalls;
    },

    runDeferredCalls() {
      if (!JSEvents.canPerformEventHandlerRequests()) {
        return;
      }
      var deferredCalls = JSEvents.deferredCalls;
      JSEvents.deferredCalls = [];
      for (var call of deferredCalls) {
        call.targetFunction(...call.argsList);
      }
    },
#endif

    // Stores objects representing each currently registered JS event handler.
    eventHandlers: [],

    // Removes all event handlers on the given DOM element of the given type.
    // Pass in eventTypeString == undefined/null to remove all event handlers
    // regardless of the type.
    removeAllHandlersOnTarget: (target, eventTypeString) => {
      for (var i = 0; i < JSEvents.eventHandlers.length; ++i) {
        if (JSEvents.eventHandlers[i].target == target &&
          (!eventTypeString || eventTypeString == JSEvents.eventHandlers[i].eventTypeString)) {
           JSEvents._removeHandler(i--);
         }
      }
    },

    _removeHandler(i) {
      var h = JSEvents.eventHandlers[i];
      h.target.removeEventListener(h.eventTypeString, h.eventListenerFunc, h.useCapture);
      JSEvents.eventHandlers.splice(i, 1);
    },

    registerOrRemoveHandler(eventHandler) {
      if (!eventHandler.target) {
#if ASSERTIONS
        err('registerOrRemoveHandler: the target element for event handler registration does not exist, when processing the following event handler registration:');
        console.dir(eventHandler);
#endif
        return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
      }
      if (eventHandler.callbackfunc) {
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
        eventHandler.eventListenerFunc = function(event) {
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
        };
#else
        eventHandler.eventListenerFunc = eventHandler.handlerFunc;
#endif

        eventHandler.target.addEventListener(eventHandler.eventTypeString,
                                             eventHandler.eventListenerFunc,
                                             eventHandler.useCapture);
        JSEvents.eventHandlers.push(eventHandler);
#if EXIT_RUNTIME
        JSEvents.registerRemoveEventListeners();
#endif
      } else {
        for (var i = 0; i < JSEvents.eventHandlers.length; ++i) {
          if (JSEvents.eventHandlers[i].target == eventHandler.target
           && JSEvents.eventHandlers[i].eventTypeString == eventHandler.eventTypeString) {
             JSEvents._removeHandler(i--);
           }
        }
      }
      return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
    },

    removeSingleHandler(eventHandler) {
      let success = false;
      for (let i = 0; i < JSEvents.eventHandlers.length; ++i) {
        const handler = JSEvents.eventHandlers[i];
        if (handler.target === eventHandler.target
          && handler.eventTypeId === eventHandler.eventTypeId
          && handler.callbackfunc === eventHandler.callbackfunc
          && handler.userData === eventHandler.userData) {
          // in some very rare cases (ex: Safari / fullscreen events), there is more than 1 handler (eventTypeString is different)
          JSEvents._removeHandler(i--);
          success = true;
        }
      }
      return success ? {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}} : {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    },

#if PTHREADS
    getTargetThreadForEventCallback(targetThread) {
      switch (targetThread) {
        case {{{ cDefs.EM_CALLBACK_THREAD_CONTEXT_MAIN_RUNTIME_THREAD }}}:
          // The event callback for the current event should be called on the
          // main browser thread. (0 == don't proxy)
          return 0;
        case {{{ cDefs.EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD }}}:
          // The event callback for the current event should be backproxied to
          // the thread that is registering the event.
          // This can be 0 in the case that the caller uses
          // EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD but on the main thread
          // itself.
          return PThread.currentProxiedOperationCallerThread;
        default:
          // The event callback for the current event should be proxied to the
          // given specific thread.
          return targetThread;
      }
    },
#endif

    getNodeNameForTarget(target) {
      if (!target) return '';
      if (target == window) return '#window';
      if (target == screen) return '#screen';
      return target?.nodeName || '';
    },

    fullscreenEnabled() {
      return document.fullscreenEnabled
#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
      // Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitFullscreenEnabled.
      // TODO: If Safari at some point ships with unprefixed version, update the version check above.
      || document.webkitFullscreenEnabled
#endif
       ;
    },
  },

  $getFullscreenElement__internal: true,
  $getFullscreenElement() {
    return document.fullscreenElement || document.mozFullScreenElement ||
           document.webkitFullscreenElement || document.webkitCurrentFullScreenElement ||
           document.msFullscreenElement;
  },

  $registerKeyEventCallback__noleakcheck: true,
  $registerKeyEventCallback__deps: ['$JSEvents', '$findEventTarget', '$stringToUTF8', 'malloc'],
  $registerKeyEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenKeyboardEvent.__size__ }}};
    JSEvents.keyEvent ||= _malloc(eventSize);

    var keyEventHandlerFunc = (e) => {
#if ASSERTIONS
      assert(e);
#endif

      var keyEventData = JSEvents.keyEvent;
      {{{ makeSetValue('keyEventData', C_STRUCTS.EmscriptenKeyboardEvent.timestamp, 'e.timeStamp', 'double') }}};

      var idx = {{{ getHeapOffset('keyEventData', 'i32') }}};

      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.location / 4 }}}] = e.location;
      HEAP8[keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.ctrlKey }}}] = e.ctrlKey;
      HEAP8[keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.shiftKey }}}] = e.shiftKey;
      HEAP8[keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.altKey }}}] = e.altKey;
      HEAP8[keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.metaKey }}}] = e.metaKey;
      HEAP8[keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.repeat }}}] = e.repeat;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.charCode / 4 }}}] = e.charCode;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.keyCode / 4 }}}] = e.keyCode;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenKeyboardEvent.which / 4 }}}] = e.which;
      stringToUTF8(e.key || '', keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.key }}}, {{{ cDefs.EM_HTML5_SHORT_STRING_LEN_BYTES }}});
      stringToUTF8(e.code || '', keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.code }}}, {{{ cDefs.EM_HTML5_SHORT_STRING_LEN_BYTES }}});
      stringToUTF8(e.char || '', keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.charValue }}}, {{{ cDefs.EM_HTML5_SHORT_STRING_LEN_BYTES }}});
      stringToUTF8(e.locale || '', keyEventData + {{{ C_STRUCTS.EmscriptenKeyboardEvent.locale }}}, {{{ cDefs.EM_HTML5_SHORT_STRING_LEN_BYTES }}});

#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, keyEventData, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, keyEventData, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: keyEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  // In DOM capturing and bubbling sequence, there are two special elements at the top of the event chain that can be of interest
  // to register many events to: document and window. These cannot be addressed by using document.querySelector(), so
  // a special mechanism to address them is needed. (For any other special object, such as screen.orientation, no general access
  // scheme should be needed, but the object-specific event callback registration functions should handle them individually).
  //
  // Users can also add more special event targets, basically by just doing something like
  //    specialHTMLTargets["!canvas"] = Module.canvas;
  // (that will let !canvas map to the canvas held in Module.canvas).
  $specialHTMLTargets__docs: '/** @type {Object} */',
#if ENVIRONMENT_MAY_BE_WORKER || ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL || PTHREADS
  $specialHTMLTargets: "[0, globalThis.document ?? 0, globalThis.window ?? 0]",
#else
  $specialHTMLTargets: "[0, document, window]",
#endif

#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
  $maybeCStringToJsString: (cString) => {
    // "cString > 2" checks if the input is a number, and isn't of the special
    // values we accept here, EMSCRIPTEN_EVENT_TARGET_* (which map to 0, 1, 2).
    // In other words, if cString > 2 then it's a pointer to a valid place in
    // memory, and points to a C string.
    return cString > 2 ? UTF8ToString(cString) : cString;
  },

  // Find a DOM element with the given ID, or null if none is found.
  $findEventTarget__deps: ['$maybeCStringToJsString', '$specialHTMLTargets'],
  $findEventTarget: (target) => {
    target = maybeCStringToJsString(target);
#if ENVIRONMENT_MAY_BE_WORKER || ENVIRONMENT_MAY_BE_NODE
    var domElement = specialHTMLTargets[target] || globalThis.document?.querySelector(target);
#else
    var domElement = specialHTMLTargets[target] || document.querySelector(target);
#endif
    return domElement;
  },

#if OFFSCREENCANVAS_SUPPORT
  $findCanvasEventTarget__deps: ['$GL', '$maybeCStringToJsString', '$specialHTMLTargets'],
  $findCanvasEventTarget: (target) => {
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
    return GL.offscreenCanvases[target.slice(1)] // Remove '#' prefix
    // If not found, if one is querying by using DOM tag name selector 'canvas', grab the first
    // OffscreenCanvas that we can find.
     || (target == 'canvas' && Object.values(GL.offscreenCanvases)[0])
    // If not found, check specialHTMLTargets
     || specialHTMLTargets[target]
    // If that is not found either, query via the regular DOM selector.
#if PTHREADS
     || globalThis.document?.querySelector(target);
#else
     || document.querySelector(target);
#endif
  },
#else
  $findCanvasEventTarget: '$findEventTarget',
#endif

#else
  // Find a DOM element with the given ID, or null if none is found.
  $findEventTarget__deps: ['$specialHTMLTargets'],
  $findEventTarget: (target) => {
#if ASSERTIONS
    warnOnce('Rules for selecting event targets in HTML5 API are changing: instead of using document.getElementById() that only can refer to elements by their DOM ID, new event target selection mechanism uses the more flexible function document.querySelector() that can look up element names, classes, and complex CSS selectors. Build with -sDISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR to change to the new lookup rules. See https://github.com/emscripten-core/emscripten/pull/7977 for more details.');
#endif
    // The sensible "default" target varies between events, but use window as the default
    // since DOM events mostly can default to that. Specific callback registrations
    // override their own defaults.
    if (!target) return window;
    if (typeof target == "number") target = specialHTMLTargets[target] || UTF8ToString(target);
    if (target === '#window') return window;
    else if (target === '#document') return document;
    else if (target === '#screen') return screen;
    else if (target === '#canvas') return Module['canvas'];
    else if (typeof target == 'string')
#if ENVIRONMENT_MAY_BE_WORKER || ENVIRONMENT_MAY_BE_NODE
      return globalThis.document?.getElementById(target);
#else
      return document.getElementById(target);
#endif
    return target;
  },

  // Like findEventTarget, but looks for OffscreenCanvas elements first
  $findCanvasEventTarget__deps: ['$findEventTarget'],
  $findCanvasEventTarget: (target) => {
    if (typeof target == 'number') target = UTF8ToString(target);
    if (!target || target === '#canvas') {
      if (typeof GL != 'undefined' && GL.offscreenCanvases['canvas']) return GL.offscreenCanvases['canvas']; // TODO: Remove this line, target '#canvas' should refer only to Module['canvas'], not to GL.offscreenCanvases['canvas'] - but need stricter tests to be able to remove this line.
      return Module['canvas'];
    }
    if (typeof GL != 'undefined' && GL.offscreenCanvases[target]) return GL.offscreenCanvases[target];
    return findEventTarget(target);
  },
#endif

  emscripten_html5_remove_event_listener__proxy: 'sync',
  emscripten_html5_remove_event_listener__deps: ['$JSEvents', '$findEventTarget'],
  emscripten_html5_remove_event_listener: (target, userData, eventTypeId, callback) => {
    var eventHandler = {
      target: findEventTarget(target),
      userData,
      eventTypeId,
      callbackfunc: callback,
    };
    return JSEvents.removeSingleHandler(eventHandler);
  },

  emscripten_set_keypress_callback_on_thread__proxy: 'sync',
  emscripten_set_keypress_callback_on_thread__deps: ['$registerKeyEventCallback'],
  emscripten_set_keypress_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_KEYPRESS }}}, "keypress", targetThread),

  emscripten_set_keydown_callback_on_thread__proxy: 'sync',
  emscripten_set_keydown_callback_on_thread__deps: ['$registerKeyEventCallback'],
  emscripten_set_keydown_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_KEYDOWN }}}, "keydown", targetThread),

  emscripten_set_keyup_callback_on_thread__proxy: 'sync',
  emscripten_set_keyup_callback_on_thread__deps: ['$registerKeyEventCallback'],
  emscripten_set_keyup_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerKeyEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_KEYUP }}}, "keyup", targetThread),

  // Outline access to function .getBoundingClientRect() since it is a long string. Closure compiler does not outline access to it by itself, but it can inline access if
  // there is only one caller to this function.
  $getBoundingClientRect__deps: ['$specialHTMLTargets'],
  $getBoundingClientRect: (e) => specialHTMLTargets.indexOf(e) < 0 ? e.getBoundingClientRect() : {'left':0,'top':0},

  // Copies mouse event data from the given JS mouse event 'e' to the specified Emscripten mouse event structure in the HEAP.
  // eventStruct: the structure to populate.
  // e: The JS mouse event to read data from.
  // target: Specifies a target DOM element that will be used as the reference to populate targetX and targetY parameters.
  $fillMouseEventData__deps: ['$getBoundingClientRect'],
  $fillMouseEventData: (eventStruct, e, target) => {
#if ASSERTIONS
    assert(eventStruct % 4 == 0);
#endif
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenMouseEvent.timestamp, 'e.timeStamp', 'double') }}};
    var idx = {{{ getHeapOffset('eventStruct', 'i32') }}};
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.screenX / 4 }}}] = e.screenX;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.screenY / 4 }}}] = e.screenY;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.clientX / 4 }}}] = e.clientX;
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.clientY / 4 }}}] = e.clientY;
    HEAP8[eventStruct + {{{ C_STRUCTS.EmscriptenMouseEvent.ctrlKey }}}] = e.ctrlKey;
    HEAP8[eventStruct + {{{ C_STRUCTS.EmscriptenMouseEvent.shiftKey }}}] = e.shiftKey;
    HEAP8[eventStruct + {{{ C_STRUCTS.EmscriptenMouseEvent.altKey }}}] = e.altKey;
    HEAP8[eventStruct + {{{ C_STRUCTS.EmscriptenMouseEvent.metaKey }}}] = e.metaKey;
    HEAP16[idx*2 + {{{ C_STRUCTS.EmscriptenMouseEvent.button / 2 }}}] = e.button;
    HEAP16[idx*2 + {{{ C_STRUCTS.EmscriptenMouseEvent.buttons / 2 }}}] = e.buttons;

    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.movementX / 4 }}}] = e["movementX"];

    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.movementY / 4 }}}] = e["movementY"];

#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    if (Module['canvas']) {
      var rect = getBoundingClientRect(Module['canvas']);
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.canvasX / 4 }}}] = e.clientX - (rect.left | 0);
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.canvasY / 4 }}}] = e.clientY - (rect.top  | 0);
    } else { // Canvas is not initialized, return 0.
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.canvasX / 4 }}}] = 0;
      HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.canvasY / 4 }}}] = 0;
    }
#endif
    // Note: rect contains doubles (truncated to placate SAFE_HEAP, which is the same behaviour when writing to HEAP32 anyway)
    var rect = getBoundingClientRect(target);
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.targetX / 4 }}}] = e.clientX - (rect.left | 0);
    HEAP32[idx + {{{ C_STRUCTS.EmscriptenMouseEvent.targetY / 4 }}}] = e.clientY - (rect.top  | 0);
  },

  $registerMouseEventCallback__noleakcheck: true,
  $registerMouseEventCallback__deps: ['$JSEvents', '$fillMouseEventData', '$findEventTarget', 'malloc'],
  $registerMouseEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenMouseEvent.__size__ }}};
    JSEvents.mouseEvent ||= _malloc(eventSize);
    target = findEventTarget(target);

    var mouseEventHandlerFunc = (e) => {
      // TODO: Make this access thread safe, or this could update live while app is reading it.
      fillMouseEventData(JSEvents.mouseEvent, e, target);

#if PTHREADS
      if (targetThread) {
        __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, JSEvents.mouseEvent, eventSize, userData);
      } else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, JSEvents.mouseEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target,
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      allowsDeferredCalls: eventTypeString != 'mousemove' && eventTypeString != 'mouseenter' && eventTypeString != 'mouseleave', // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
#endif
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: mouseEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_click_callback_on_thread__proxy: 'sync',
  emscripten_set_click_callback_on_thread__deps: ['$registerMouseEventCallback'],
  emscripten_set_click_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_CLICK }}}, "click", targetThread),

  emscripten_set_mousedown_callback_on_thread__proxy: 'sync',
  emscripten_set_mousedown_callback_on_thread__deps: ['$registerMouseEventCallback'],
  emscripten_set_mousedown_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_MOUSEDOWN }}}, "mousedown", targetThread),

  emscripten_set_mouseup_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseup_callback_on_thread__deps: ['$registerMouseEventCallback'],
  emscripten_set_mouseup_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_MOUSEUP }}}, "mouseup", targetThread),

  emscripten_set_dblclick_callback_on_thread__proxy: 'sync',
  emscripten_set_dblclick_callback_on_thread__deps: ['$registerMouseEventCallback'],
  emscripten_set_dblclick_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_DBLCLICK }}}, "dblclick", targetThread),

  emscripten_set_mousemove_callback_on_thread__proxy: 'sync',
  emscripten_set_mousemove_callback_on_thread__deps: ['$registerMouseEventCallback'],
  emscripten_set_mousemove_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_MOUSEMOVE }}}, "mousemove", targetThread),

  emscripten_set_mouseenter_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseenter_callback_on_thread__deps: ['$registerMouseEventCallback'],
  emscripten_set_mouseenter_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_MOUSEENTER }}}, "mouseenter", targetThread),

  emscripten_set_mouseleave_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseleave_callback_on_thread__deps: ['$registerMouseEventCallback'],
  emscripten_set_mouseleave_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_MOUSELEAVE }}}, "mouseleave", targetThread),

  emscripten_set_mouseover_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseover_callback_on_thread__deps: ['$registerMouseEventCallback'],
  emscripten_set_mouseover_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_MOUSEOVER }}}, "mouseover", targetThread),

  emscripten_set_mouseout_callback_on_thread__proxy: 'sync',
  emscripten_set_mouseout_callback_on_thread__deps: ['$registerMouseEventCallback'],
  emscripten_set_mouseout_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerMouseEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_MOUSEOUT }}}, "mouseout", targetThread),

  // HTML5 does not really have a polling API for mouse events, so implement one
  // manually by returning the data from the most recently received event. This
  // requires that user has registered at least some no-op function as an event
  // handler to any of the mouse function.
  _emscripten_get_last_mouse_event__proxy: 'sync',
  _emscripten_get_last_mouse_event__internal: true,
  _emscripten_get_last_mouse_event__deps: ['$JSEvents'],
  _emscripten_get_last_mouse_event: () => JSEvents.mouseEvent,

  $registerWheelEventCallback__noleakcheck: true,
  $registerWheelEventCallback__deps: ['$JSEvents', '$fillMouseEventData', 'malloc'],
  $registerWheelEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenWheelEvent.__size__ }}};
    JSEvents.wheelEvent ||= _malloc(eventSize)

    // The DOM Level 3 events spec event 'wheel'
    var wheelHandlerFunc = (e) => {
      var wheelEvent = JSEvents.wheelEvent;
      fillMouseEventData(wheelEvent, e, target);
      {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaX, 'e["deltaX"]', 'double') }}};
      {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaY, 'e["deltaY"]', 'double') }}};
      {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaZ, 'e["deltaZ"]', 'double') }}};
      {{{ makeSetValue('wheelEvent', C_STRUCTS.EmscriptenWheelEvent.deltaMode, 'e["deltaMode"]', 'i32') }}};
#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, wheelEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, wheelEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target,
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      allowsDeferredCalls: true,
#endif
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: wheelHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_wheel_callback_on_thread__proxy: 'sync',
  emscripten_set_wheel_callback_on_thread__deps: ['$registerWheelEventCallback', '$findEventTarget'],
  emscripten_set_wheel_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) => {
    target = findEventTarget(target);
    if (!target) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
    if (typeof target.onwheel != 'undefined') {
      return registerWheelEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_WHEEL }}}, "wheel", targetThread);
    } else {
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }
  },

  $registerUiEventCallback__noleakcheck: true,
  $registerUiEventCallback__deps: ['$JSEvents', '$findEventTarget', 'malloc'],
  $registerUiEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenUiEvent.__size__ }}};
    JSEvents.uiEvent ||= _malloc(eventSize);

#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    if (eventTypeId == {{{ cDefs.EMSCRIPTEN_EVENT_SCROLL }}} && !target) {
      target = document; // By default read scroll events on document rather than window.
    } else {
      target = findEventTarget(target);
    }
#else
#endif

    var uiEventHandlerFunc = (e) => {
      if (e.target != target) {
        // Never take ui events such as scroll via a 'bubbled' route, but always from the direct element that
        // was targeted. Otherwise e.g. if app logs a message in response to a page scroll, the Emscripten log
        // message box could cause to scroll, generating a new (bubbled) scroll message, causing a new log print,
        // causing a new scroll, etc..
        return;
      }
      var b = document.body; // Take document.body to a variable, Closure compiler does not outline access to it on its own.
      if (!b) {
        // During a page unload 'body' can be null, with "Cannot read property 'clientWidth' of null" being thrown
        return;
      }
      var uiEvent = JSEvents.uiEvent;
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.detail, '0', 'i32') }}}; // always zero for resize and scroll
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientWidth, 'b.clientWidth', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.documentBodyClientHeight, 'b.clientHeight', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerWidth, 'innerWidth', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowInnerHeight, 'innerHeight', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterWidth, 'outerWidth', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.windowOuterHeight, 'outerHeight', 'i32') }}};
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollTop, 'pageXOffset | 0', 'i32') }}}; // scroll offsets are float
      {{{ makeSetValue('uiEvent', C_STRUCTS.EmscriptenUiEvent.scrollLeft, 'pageYOffset | 0', 'i32') }}};
#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, uiEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, uiEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target,
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: uiEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_resize_callback_on_thread__proxy: 'sync',
  emscripten_set_resize_callback_on_thread__deps: ['$registerUiEventCallback'],
  emscripten_set_resize_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_RESIZE }}}, "resize", targetThread),

  emscripten_set_scroll_callback_on_thread__proxy: 'sync',
  emscripten_set_scroll_callback_on_thread__deps: ['$registerUiEventCallback'],
  emscripten_set_scroll_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerUiEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_SCROLL }}}, "scroll", targetThread),

  $registerFocusEventCallback__noleakcheck: true,
  $registerFocusEventCallback__deps: ['$JSEvents', '$findEventTarget', 'malloc', '$stringToUTF8'],
  $registerFocusEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenFocusEvent.__size__ }}};
    JSEvents.focusEvent ||= _malloc(eventSize);

    var focusEventHandlerFunc = (e) => {
      var nodeName = JSEvents.getNodeNameForTarget(e.target);
      var id = e.target.id ? e.target.id : '';

      var focusEvent = JSEvents.focusEvent;
      stringToUTF8(nodeName, focusEvent + {{{ C_STRUCTS.EmscriptenFocusEvent.nodeName }}}, {{{ cDefs.EM_HTML5_LONG_STRING_LEN_BYTES }}});
      stringToUTF8(id, focusEvent + {{{ C_STRUCTS.EmscriptenFocusEvent.id }}}, {{{ cDefs.EM_HTML5_LONG_STRING_LEN_BYTES }}});

#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, focusEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, focusEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: focusEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_blur_callback_on_thread__proxy: 'sync',
  emscripten_set_blur_callback_on_thread__deps: ['$registerFocusEventCallback'],
  emscripten_set_blur_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_BLUR }}}, "blur", targetThread),

  emscripten_set_focus_callback_on_thread__proxy: 'sync',
  emscripten_set_focus_callback_on_thread__deps: ['$registerFocusEventCallback'],
  emscripten_set_focus_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_FOCUS }}}, "focus", targetThread),

  emscripten_set_focusin_callback_on_thread__proxy: 'sync',
  emscripten_set_focusin_callback_on_thread__deps: ['$registerFocusEventCallback'],
  emscripten_set_focusin_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_FOCUSIN }}}, "focusin", targetThread),

  emscripten_set_focusout_callback_on_thread__proxy: 'sync',
  emscripten_set_focusout_callback_on_thread__deps: ['$registerFocusEventCallback'],
  emscripten_set_focusout_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerFocusEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_FOCUSOUT }}}, "focusout", targetThread),

  $fillDeviceOrientationEventData: (eventStruct, e, target) => {
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.alpha, 'e.alpha', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.beta, 'e.beta', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.gamma, 'e.gamma', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceOrientationEvent.absolute, 'e.absolute', 'i8') }}};
  },

  $registerDeviceOrientationEventCallback__noleakcheck: true,
  $registerDeviceOrientationEventCallback__deps: ['$JSEvents', '$fillDeviceOrientationEventData', '$findEventTarget'],
  $registerDeviceOrientationEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenDeviceOrientationEvent.__size__ }}};
    JSEvents.deviceOrientationEvent ||= _malloc(eventSize);

    var deviceOrientationEventHandlerFunc = (e) => {
      fillDeviceOrientationEventData(JSEvents.deviceOrientationEvent, e, target); // TODO: Thread-safety with respect to emscripten_get_deviceorientation_status()

#if PTHREADS
      if (targetThread) {
        __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, JSEvents.deviceOrientationEvent, eventSize, userData);
      } else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, JSEvents.deviceOrientationEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: deviceOrientationEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_deviceorientation_callback_on_thread__proxy: 'sync',
  emscripten_set_deviceorientation_callback_on_thread__deps: ['$registerDeviceOrientationEventCallback'],
  emscripten_set_deviceorientation_callback_on_thread: (userData, useCapture, callbackfunc, targetThread) => {
    return registerDeviceOrientationEventCallback({{{ cDefs.EMSCRIPTEN_EVENT_TARGET_WINDOW }}}, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_DEVICEORIENTATION }}}, "deviceorientation", targetThread);
  },

  // HTML5 does not really have a polling API for device orientation events, so
  // implement one manually by returning the data from the most recently
  // received event. This requires that user has registered at least some
  // no-op function as an event handler.
  _emscripten_get_last_deviceorientation_event__proxy: 'sync',
  _emscripten_get_last_deviceorientation_event__internal: true,
  _emscripten_get_last_deviceorientation_event__deps: ['$JSEvents'],
  _emscripten_get_last_deviceorientation_event: () => JSEvents.deviceOrientationEvent,

  $fillDeviceMotionEventData: (eventStruct, e, target) => {
    var supportedFields = 0;
    var a = e['acceleration'];
    supportedFields |= a && {{{ cDefs.EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ACCELERATION }}};
    var ag = e['accelerationIncludingGravity'];
    supportedFields |= ag && {{{ cDefs.EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ACCELERATION_INCLUDING_GRAVITY }}};
    var rr = e['rotationRate'];
    supportedFields |= rr && {{{ cDefs.EMSCRIPTEN_DEVICE_MOTION_EVENT_SUPPORTS_ROTATION_RATE }}};
    a = a || {};
    ag = ag || {};
    rr = rr || {};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenDeviceMotionEvent.supportedFields, 'supportedFields', 'i32') }}};
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

  $registerDeviceMotionEventCallback__noleakcheck: true,
  $registerDeviceMotionEventCallback__deps: ['$JSEvents', '$fillDeviceMotionEventData', '$findEventTarget', 'malloc'],
  $registerDeviceMotionEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenDeviceMotionEvent.__size__ }}};
    JSEvents.deviceMotionEvent ||= _malloc(eventSize);

    var deviceMotionEventHandlerFunc = (e) => {
      fillDeviceMotionEventData(JSEvents.deviceMotionEvent, e, target); // TODO: Thread-safety with respect to emscripten_get_devicemotion_status()

#if PTHREADS
      if (targetThread) {
        __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, JSEvents.deviceMotionEvent, eventSize, userData);
      } else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, JSEvents.deviceMotionEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: deviceMotionEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_devicemotion_callback_on_thread__proxy: 'sync',
  emscripten_set_devicemotion_callback_on_thread__deps: ['$registerDeviceMotionEventCallback'],
  emscripten_set_devicemotion_callback_on_thread: (userData, useCapture, callbackfunc, targetThread) =>
    registerDeviceMotionEventCallback({{{ cDefs.EMSCRIPTEN_EVENT_TARGET_WINDOW }}}, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_DEVICEMOTION }}}, "devicemotion", targetThread),

  // HTML5 does not really have a polling API for device motion events, so
  // implement one manually by returning the data from the most recently
  // received event. This requires that user has registered at least some
  // no-op function as an event handler.
  _emscripten_get_last_devicemotion_event__proxy: 'sync',
  _emscripten_get_last_devicemotion_event__internal: true,
  _emscripten_get_last_devicemotion_event__deps: ['$JSEvents'],
  _emscripten_get_last_devicemotion_event: () => JSEvents.deviceMotionEvent,

  $screenOrientation: () => {
    if (!window.screen) return undefined;
    return screen.orientation || screen['mozOrientation'] || screen['webkitOrientation'];
  },

  $fillOrientationChangeEventData__deps: ['$screenOrientation'],
  $fillOrientationChangeEventData: (eventStruct) => {
    // OrientationType enum
    var orientationsType1 = ['portrait-primary', 'portrait-secondary', 'landscape-primary', 'landscape-secondary'];
    // alternative selection from OrientationLockType enum
    var orientationsType2 = ['portrait',         'portrait',           'landscape',         'landscape'];

    var orientationIndex = {{{ cDefs.EMSCRIPTEN_ORIENTATION_UNSUPPORTED }}};
    var orientationAngle = 0;
    var screenOrientObj  = screenOrientation();
    if (typeof screenOrientObj === 'object') {
      orientationIndex = orientationsType1.indexOf(screenOrientObj.type);
      if (orientationIndex < 0) {
        orientationIndex = orientationsType2.indexOf(screenOrientObj.type);
      }
      if (orientationIndex >= 0) {
        orientationIndex = 1 << orientationIndex;
      }
      orientationAngle = screenOrientObj.angle;
    }
#if MIN_SAFARI_VERSION < 160400
    else {
      // fallback for Safari earlier than 16.4 (March 2023)
      orientationAngle = window.orientation;
    }
#endif

    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenOrientationChangeEvent.orientationIndex, 'orientationIndex', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenOrientationChangeEvent.orientationAngle, 'orientationAngle', 'i32') }}};
  },

  $registerOrientationChangeEventCallback__noleakcheck: true,
  $registerOrientationChangeEventCallback__deps: ['$JSEvents', '$fillOrientationChangeEventData', 'malloc'],
  $registerOrientationChangeEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenOrientationChangeEvent.__size__ }}};
    JSEvents.orientationChangeEvent ||= _malloc(eventSize);

    var orientationChangeEventHandlerFunc = (e) => {
      var orientationChangeEvent = JSEvents.orientationChangeEvent;
      fillOrientationChangeEventData(orientationChangeEvent);

#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, orientationChangeEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, orientationChangeEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target,
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: orientationChangeEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_orientationchange_callback_on_thread__proxy: 'sync',
  emscripten_set_orientationchange_callback_on_thread__deps: ['$registerOrientationChangeEventCallback'],
  emscripten_set_orientationchange_callback_on_thread: (userData, useCapture, callbackfunc, targetThread) => {
    if (!window.screen || !screen.orientation) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    return registerOrientationChangeEventCallback(screen.orientation, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_ORIENTATIONCHANGE }}}, 'change', targetThread);
  },

  emscripten_get_orientation_status__proxy: 'sync',
  emscripten_get_orientation_status__deps: ['$fillOrientationChangeEventData', '$screenOrientation'],
  emscripten_get_orientation_status: (orientationChangeEvent) => {
    // screenOrientation() resolving standard, window.orientation being the deprecated mobile-only
    if (!screenOrientation() && typeof orientation == 'undefined') return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    fillOrientationChangeEventData(orientationChangeEvent);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_lock_orientation__proxy: 'sync',
  emscripten_lock_orientation: (allowedOrientations) => {
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
    } else {
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }
    if (succeeded) {
      return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_FAILED }}};
  },

  emscripten_unlock_orientation__proxy: 'sync',
  emscripten_unlock_orientation: () => {
    if (screen.unlockOrientation) {
      screen.unlockOrientation();
    } else if (screen.mozUnlockOrientation) {
      screen.mozUnlockOrientation();
    } else if (screen.webkitUnlockOrientation) {
      screen.webkitUnlockOrientation();
    } else {
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  $fillFullscreenChangeEventData__deps: ['$JSEvents', '$stringToUTF8', '$getFullscreenElement'],
  $fillFullscreenChangeEventData: (eventStruct) => {
    var fullscreenElement = getFullscreenElement();
    var isFullscreen = !!fullscreenElement;
    // Assigning a boolean to HEAP32 with expected type coercion.
    /** @suppress{checkTypes} */
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.isFullscreen, 'isFullscreen', 'i8') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.fullscreenEnabled, 'JSEvents.fullscreenEnabled()', 'i8') }}};
    // If transitioning to fullscreen, report info about the element that is now fullscreen.
    // If transitioning to windowed mode, report info about the element that just was fullscreen.
    var reportedElement = isFullscreen ? fullscreenElement : JSEvents.previousFullscreenElement;
    var nodeName = JSEvents.getNodeNameForTarget(reportedElement);
    var id = reportedElement?.id || '';
    stringToUTF8(nodeName, eventStruct + {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.nodeName }}}, {{{ cDefs.EM_HTML5_LONG_STRING_LEN_BYTES }}});
    stringToUTF8(id, eventStruct + {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.id }}}, {{{ cDefs.EM_HTML5_LONG_STRING_LEN_BYTES }}});
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.elementWidth, 'reportedElement ? reportedElement.clientWidth : 0', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.elementHeight, 'reportedElement ? reportedElement.clientHeight : 0', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.screenWidth, 'screen.width', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenFullscreenChangeEvent.screenHeight, 'screen.height', 'i32') }}};
    if (isFullscreen) {
      JSEvents.previousFullscreenElement = fullscreenElement;
    }
  },

  $registerFullscreenChangeEventCallback__noleakcheck: true,
  $registerFullscreenChangeEventCallback__deps: ['$JSEvents', '$fillFullscreenChangeEventData', 'malloc'],
  $registerFullscreenChangeEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenFullscreenChangeEvent.__size__ }}};
    JSEvents.fullscreenChangeEvent ||= _malloc(eventSize);

    var fullscreenChangeEventHandlerFunc = (e) => {
      var fullscreenChangeEvent = JSEvents.fullscreenChangeEvent;
      fillFullscreenChangeEventData(fullscreenChangeEvent);

#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, fullscreenChangeEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, fullscreenChangeEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target,
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: fullscreenChangeEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_fullscreenchange_callback_on_thread__proxy: 'sync',
  emscripten_set_fullscreenchange_callback_on_thread__deps: ['$JSEvents', '$registerFullscreenChangeEventCallback', '$findEventTarget',
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    '$specialHTMLTargets'
#endif
  ],
  emscripten_set_fullscreenchange_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) => {
    if (!JSEvents.fullscreenEnabled()) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : specialHTMLTargets[{{{ cDefs.EMSCRIPTEN_EVENT_TARGET_DOCUMENT }}}];
#endif
    if (!target) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};

#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
    // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
    // TODO: When this block is removed, also change test/test_html5_remove_event_listener.c test expectation on emscripten_set_fullscreenchange_callback().
    registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_FULLSCREENCHANGE }}}, "webkitfullscreenchange", targetThread);
#endif

    return registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_FULLSCREENCHANGE }}}, "fullscreenchange", targetThread);
  },

  emscripten_get_fullscreen_status__proxy: 'sync',
  emscripten_get_fullscreen_status__deps: ['$JSEvents', '$fillFullscreenChangeEventData'],
  emscripten_get_fullscreen_status: (fullscreenStatus) => {
    if (!JSEvents.fullscreenEnabled()) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    fillFullscreenChangeEventData(fullscreenStatus);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  $JSEvents_requestFullscreen__deps: ['$JSEvents', '$JSEvents_resizeCanvasForFullscreen'],
  $JSEvents_requestFullscreen: (target, strategy) => {
    // EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT + EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE is a mode where no extra logic is performed to the DOM elements.
    if (strategy.scaleMode != {{{ cDefs.EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT }}} || strategy.canvasResolutionScaleMode != {{{ cDefs.EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE }}}) {
      JSEvents_resizeCanvasForFullscreen(target, strategy);
    }

    if (target.requestFullscreen) {
      target.requestFullscreen();
#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
    } else if (target.webkitRequestFullscreen) {
      target.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
#endif
    } else {
      return JSEvents.fullscreenEnabled() ? {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}} : {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }

    currentFullscreenStrategy = strategy;

    if (strategy.canvasResizedCallback) {
#if PTHREADS
      if (strategy.canvasResizedCallbackTargetThread) __emscripten_run_callback_on_thread(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, {{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, strategy.canvasResizedCallbackUserData);
      else
#endif
      {{{ makeDynCall('iipp', 'strategy.canvasResizedCallback') }}}({{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, strategy.canvasResizedCallbackUserData);
    }

    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  $JSEvents_resizeCanvasForFullscreen__deps: ['$registerRestoreOldStyle', '$getCanvasElementSize', '$setLetterbox', '$setCanvasElementSize', '$getBoundingClientRect'],
  $JSEvents_resizeCanvasForFullscreen: (target, strategy) => {
    var restoreOldStyle = registerRestoreOldStyle(target);
    var cssWidth = strategy.softFullscreen ? innerWidth : screen.width;
    var cssHeight = strategy.softFullscreen ? innerHeight : screen.height;
    var rect = getBoundingClientRect(target);
    var windowedCssWidth = rect.width;
    var windowedCssHeight = rect.height;
    var canvasSize = getCanvasElementSize(target);
    var windowedRttWidth = canvasSize[0];
    var windowedRttHeight = canvasSize[1];

    if (strategy.scaleMode == {{{ cDefs.EMSCRIPTEN_FULLSCREEN_SCALE_CENTER }}}) {
      setLetterbox(target, (cssHeight - windowedCssHeight) / 2, (cssWidth - windowedCssWidth) / 2);
      cssWidth = windowedCssWidth;
      cssHeight = windowedCssHeight;
    } else if (strategy.scaleMode == {{{ cDefs.EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT }}}) {
      if (cssWidth*windowedRttHeight < windowedRttWidth*cssHeight) {
        var desiredCssHeight = windowedRttHeight * cssWidth / windowedRttWidth;
        setLetterbox(target, (cssHeight - desiredCssHeight) / 2, 0);
        cssHeight = desiredCssHeight;
      } else {
        var desiredCssWidth = windowedRttWidth * cssHeight / windowedRttHeight;
        setLetterbox(target, 0, (cssWidth - desiredCssWidth) / 2);
        cssWidth = desiredCssWidth;
      }
    }

    // If we are adding padding, must choose a background color or otherwise Chrome will give the
    // padding a default white color. Do it only if user has not customized their own background color.
    target.style.backgroundColor ||= 'black';
    // IE11 does the same, but requires the color to be set in the document body.
    document.body.style.backgroundColor ||= 'black'; // IE11
    // Firefox always shows black letterboxes independent of style color.

    target.style.width = cssWidth + 'px';
    target.style.height = cssHeight + 'px';

    if (strategy.filteringMode == {{{ cDefs.EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST }}}) {
      target.style.imageRendering = 'optimizeSpeed';
      target.style.imageRendering = '-moz-crisp-edges';
      target.style.imageRendering = '-o-crisp-edges';
      target.style.imageRendering = '-webkit-optimize-contrast';
      target.style.imageRendering = 'optimize-contrast';
      target.style.imageRendering = 'crisp-edges';
      target.style.imageRendering = 'pixelated';
    }

    var dpiScale = (strategy.canvasResolutionScaleMode == {{{ cDefs.EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF }}}) ? devicePixelRatio : 1;
    if (strategy.canvasResolutionScaleMode != {{{ cDefs.EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE }}}) {
      var newWidth = (cssWidth * dpiScale)|0;
      var newHeight = (cssHeight * dpiScale)|0;
      setCanvasElementSize(target, newWidth, newHeight);
      if (target.GLctxObject) target.GLctxObject.GLctx.viewport(0, 0, newWidth, newHeight);
    }
    return restoreOldStyle;
  },

  $registerRestoreOldStyle__deps: ['$getCanvasElementSize', '$setCanvasElementSize', '$currentFullscreenStrategy'],
  $registerRestoreOldStyle: (canvas) => {
    var canvasSize = getCanvasElementSize(canvas);
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
      if (!getFullscreenElement()) {
        document.removeEventListener('fullscreenchange', restoreOldStyle);

#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
        // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
        document.removeEventListener('webkitfullscreenchange', restoreOldStyle);
#endif

        setCanvasElementSize(canvas, oldWidth, oldHeight);

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

        if (currentFullscreenStrategy.canvasResizedCallback) {
#if PTHREADS
          if (currentFullscreenStrategy.canvasResizedCallbackTargetThread) __emscripten_run_callback_on_thread(currentFullscreenStrategy.canvasResizedCallbackTargetThread, currentFullscreenStrategy.canvasResizedCallback, {{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, currentFullscreenStrategy.canvasResizedCallbackUserData);
          else
#endif
          {{{ makeDynCall('iipp', 'currentFullscreenStrategy.canvasResizedCallback') }}}({{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, currentFullscreenStrategy.canvasResizedCallbackUserData);
        }
      }
    }
    document.addEventListener('fullscreenchange', restoreOldStyle);
#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
    // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
    document.addEventListener('webkitfullscreenchange', restoreOldStyle);
#endif
    return restoreOldStyle;
  },

  // Walks the DOM tree and hides every element by setting "display: none;" except the given element.
  // Returns a list of [{node: element, displayState: oldDisplayStyle}] entries to allow restoring previous
  // visibility states after done.
  $hideEverythingExceptGivenElement: (onlyVisibleElement) => {
    var child = onlyVisibleElement;
    var parent = child.parentNode;
    var hiddenElements = [];
    while (child != document.body) {
      var children = parent.children;
      for (var currChild of children) {
        if (currChild != child) {
          hiddenElements.push({ node: currChild, displayState: currChild.style.display });
          currChild.style.display = 'none';
        }
      }
      child = parent;
      parent = parent.parentNode;
    }
    return hiddenElements;
  },

  // Applies old visibility states, given a list of changes returned by hideEverythingExceptGivenElement().
  $restoreHiddenElements: (hiddenElements) => {
    for (var elem of hiddenElements) {
      elem.node.style.display = elem.displayState;
    }
  },

  // Add letterboxes to a fullscreen element in a cross-browser way.
  $setLetterbox: (element, topBottom, leftRight) => {
    // Cannot use margin to specify letterboxes in FF or Chrome, since those ignore margins in fullscreen mode.
    element.style.paddingLeft = element.style.paddingRight = leftRight + 'px';
    element.style.paddingTop = element.style.paddingBottom = topBottom + 'px';
  },

  $currentFullscreenStrategy: {},
  $restoreOldWindowedStyle: null,

  $softFullscreenResizeWebGLRenderTarget__deps: ['$setLetterbox', '$currentFullscreenStrategy', '$getCanvasElementSize', '$setCanvasElementSize', '$jstoi_q'],
  $softFullscreenResizeWebGLRenderTarget: () => {
    var dpr = devicePixelRatio;
    var inHiDPIFullscreenMode = currentFullscreenStrategy.canvasResolutionScaleMode == {{{ cDefs.EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF }}};
    var inAspectRatioFixedFullscreenMode = currentFullscreenStrategy.scaleMode == {{{ cDefs.EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT }}};
    var inPixelPerfectFullscreenMode = currentFullscreenStrategy.canvasResolutionScaleMode != {{{ cDefs.EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE }}};
    var inCenteredWithoutScalingFullscreenMode = currentFullscreenStrategy.scaleMode == {{{ cDefs.EMSCRIPTEN_FULLSCREEN_SCALE_CENTER }}};
    var screenWidth = inHiDPIFullscreenMode ? Math.round(innerWidth*dpr) : innerWidth;
    var screenHeight = inHiDPIFullscreenMode ? Math.round(innerHeight*dpr) : innerHeight;
    var w = screenWidth;
    var h = screenHeight;
    var canvas = currentFullscreenStrategy.target;
    var canvasSize = getCanvasElementSize(canvas);
    var x = canvasSize[0];
    var y = canvasSize[1];
    var topMargin;

    if (inAspectRatioFixedFullscreenMode) {
      if (w*y < x*h) h = (w * y / x) | 0;
      else if (w*y > x*h) w = (h * x / y) | 0;
      topMargin = ((screenHeight - h) / 2) | 0;
    }

    if (inPixelPerfectFullscreenMode) {
      setCanvasElementSize(canvas, w, h);
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
      setLetterbox(canvas, t, b);
    } else {
      canvas.style.width = w + 'px';
      canvas.style.height = h + 'px';
      var b = (innerWidth - w) / 2;
      setLetterbox(canvas, topMargin, b);
    }

    if (!inCenteredWithoutScalingFullscreenMode && currentFullscreenStrategy.canvasResizedCallback) {
#if PTHREADS
      if (currentFullscreenStrategy.canvasResizedCallbackTargetThread) __emscripten_run_callback_on_thread(currentFullscreenStrategy.canvasResizedCallbackTargetThread, currentFullscreenStrategy.canvasResizedCallback, {{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, currentFullscreenStrategy.canvasResizedCallbackUserData);
      else
#endif
      {{{ makeDynCall('iipp', 'currentFullscreenStrategy.canvasResizedCallback') }}}({{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, currentFullscreenStrategy.canvasResizedCallbackUserData);
    }
  },

  // https://developer.mozilla.org/en-US/docs/Web/Guide/API/DOM/Using_full_screen_mode
  $doRequestFullscreen__deps: ['$JSEvents', '$JSEvents_requestFullscreen', '$findEventTarget'],
  $doRequestFullscreen: (target, strategy) => {
    if (!JSEvents.fullscreenEnabled()) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target ||= '#canvas';
#endif
    target = findEventTarget(target);
    if (!target) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};

    if (!target.requestFullscreen
#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
      && !target.webkitRequestFullscreen
#endif
      ) {
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // Queue this function call if we're not currently in an event handler and
    // the user saw it appropriate to do so.
    if (!JSEvents.canPerformEventHandlerRequests()) {
      if (strategy.deferUntilInEventHandler) {
        JSEvents.deferCall(JSEvents_requestFullscreen, 1 /* priority over pointer lock */, [target, strategy]);
        return {{{ cDefs.EMSCRIPTEN_RESULT_DEFERRED }}};
      }
      return {{{ cDefs.EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED }}};
    }
#endif

    return JSEvents_requestFullscreen(target, strategy);
  },

  emscripten_request_fullscreen__deps: ['$doRequestFullscreen'],
  emscripten_request_fullscreen__proxy: 'sync',
  emscripten_request_fullscreen: (target, deferUntilInEventHandler) => {
    var strategy = {
      // These options perform no added logic, but just bare request fullscreen.
      scaleMode: {{{ cDefs.EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT }}},
      canvasResolutionScaleMode: {{{ cDefs.EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE }}},
      filteringMode: {{{ cDefs.EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT }}},
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      deferUntilInEventHandler,
#endif
      canvasResizedCallbackTargetThread: {{{ cDefs.EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD }}}
    };
    return doRequestFullscreen(target, strategy);
  },

  emscripten_request_fullscreen_strategy__deps: ['$doRequestFullscreen'],
  emscripten_request_fullscreen_strategy__proxy: 'sync',
  emscripten_request_fullscreen_strategy: (target, deferUntilInEventHandler, fullscreenStrategy) => {
    var strategy = {
      scaleMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.scaleMode, 'i32') }}},
      canvasResolutionScaleMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResolutionScaleMode, 'i32') }}},
      filteringMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.filteringMode, 'i32') }}},
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      deferUntilInEventHandler,
#endif
#if PTHREADS
      canvasResizedCallbackTargetThread: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackTargetThread, 'i32') }}},
#endif
      canvasResizedCallback: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallback, 'i32') }}},
      canvasResizedCallbackUserData: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackUserData, 'i32') }}}
    };

    return doRequestFullscreen(target, strategy);
  },

  emscripten_enter_soft_fullscreen__deps: ['$JSEvents', '$hideEverythingExceptGivenElement', '$restoreOldWindowedStyle', '$restoreHiddenElements', '$currentFullscreenStrategy', '$softFullscreenResizeWebGLRenderTarget', '$JSEvents_resizeCanvasForFullscreen', '$findEventTarget'],
  emscripten_enter_soft_fullscreen__proxy: 'sync',
  emscripten_enter_soft_fullscreen: (target, fullscreenStrategy) => {
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target ||= '#canvas';
#endif
    target = findEventTarget(target);
    if (!target) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};

    var strategy = {
        scaleMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.scaleMode, 'i32') }}},
        canvasResolutionScaleMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResolutionScaleMode, 'i32') }}},
        filteringMode: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.filteringMode, 'i32') }}},
        canvasResizedCallback: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallback, 'i32') }}},
        canvasResizedCallbackUserData: {{{ makeGetValue('fullscreenStrategy', C_STRUCTS.EmscriptenFullscreenStrategy.canvasResizedCallbackUserData, 'i32') }}},
#if PTHREADS
        canvasResizedCallbackTargetThread: JSEvents.getTargetThreadForEventCallback(),
#endif
        target,
        softFullscreen: true
    };

    var restoreOldStyle = JSEvents_resizeCanvasForFullscreen(target, strategy);

    document.documentElement.style.overflow = 'hidden';  // Firefox, Chrome
    document.body.scroll = "no"; // IE11
    document.body.style.margin = '0px'; // Override default document margin area on all browsers.

    var hiddenElements = hideEverythingExceptGivenElement(target);

    function restoreWindowedState() {
      restoreOldStyle();
      restoreHiddenElements(hiddenElements);
      removeEventListener('resize', softFullscreenResizeWebGLRenderTarget);
      if (strategy.canvasResizedCallback) {
#if PTHREADS
        if (strategy.canvasResizedCallbackTargetThread) __emscripten_run_callback_on_thread(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, {{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, strategy.canvasResizedCallbackUserData);
        else
#endif
        {{{ makeDynCall('iipp', 'strategy.canvasResizedCallback') }}}({{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, strategy.canvasResizedCallbackUserData);
      }
      currentFullscreenStrategy = 0;
    }
    restoreOldWindowedStyle = restoreWindowedState;
    currentFullscreenStrategy = strategy;
    addEventListener('resize', softFullscreenResizeWebGLRenderTarget);

    // Inform the caller that the canvas size has changed.
    if (strategy.canvasResizedCallback) {
#if PTHREADS
      if (strategy.canvasResizedCallbackTargetThread) __emscripten_run_callback_on_thread(strategy.canvasResizedCallbackTargetThread, strategy.canvasResizedCallback, {{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, strategy.canvasResizedCallbackUserData);
      else
#endif
      {{{ makeDynCall('iipp', 'strategy.canvasResizedCallback') }}}({{{ cDefs.EMSCRIPTEN_EVENT_CANVASRESIZED }}}, 0, strategy.canvasResizedCallbackUserData);
    }

    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_exit_soft_fullscreen__deps: ['$restoreOldWindowedStyle'],
  emscripten_exit_soft_fullscreen__proxy: 'sync',
  emscripten_exit_soft_fullscreen: () => {
    restoreOldWindowedStyle?.();
    restoreOldWindowedStyle = null;

    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_exit_fullscreen__deps: [
    '$JSEvents',
    '$specialHTMLTargets',
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    '$JSEvents_requestFullscreen',
#endif
  ],
  emscripten_exit_fullscreen__proxy: 'sync',
  emscripten_exit_fullscreen: () => {
    if (!JSEvents.fullscreenEnabled()) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // Make sure no queued up calls will fire after this.
    JSEvents.removeDeferredCalls(JSEvents_requestFullscreen);
#endif

    var d = specialHTMLTargets[{{{ cDefs.EMSCRIPTEN_EVENT_TARGET_DOCUMENT }}}];
    if (d.exitFullscreen) {
      d.fullscreenElement && d.exitFullscreen();
#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED // https://caniuse.com/#feat=mdn-api_document_exitfullscreen
    } else if (d.webkitExitFullscreen) {
      d.webkitFullscreenElement && d.webkitExitFullscreen();
#endif
    } else {
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }

    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  $fillPointerlockChangeEventData__deps: ['$JSEvents', '$stringToUTF8'],
  $fillPointerlockChangeEventData: (eventStruct) => {
    var pointerLockElement = document.pointerLockElement;
    var isPointerlocked = !!pointerLockElement;
    // Assigning a boolean to HEAP32 with expected type coercion.
    /** @suppress{checkTypes} */
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenPointerlockChangeEvent.isActive, 'isPointerlocked', 'i8') }}};
    var nodeName = JSEvents.getNodeNameForTarget(pointerLockElement);
    var id = pointerLockElement?.id || '';
    stringToUTF8(nodeName, eventStruct + {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.nodeName }}}, {{{ cDefs.EM_HTML5_LONG_STRING_LEN_BYTES }}});
    stringToUTF8(id, eventStruct + {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.id }}}, {{{ cDefs.EM_HTML5_LONG_STRING_LEN_BYTES }}});
  },

  $registerPointerlockChangeEventCallback__noleakcheck: true,
  $registerPointerlockChangeEventCallback__deps: ['$JSEvents', '$fillPointerlockChangeEventData', 'malloc'],
  $registerPointerlockChangeEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenPointerlockChangeEvent.__size__ }}};
    JSEvents.pointerlockChangeEvent ||= _malloc(eventSize);

    var pointerlockChangeEventHandlerFunc = (e) => {
      var pointerlockChangeEvent = JSEvents.pointerlockChangeEvent;
      fillPointerlockChangeEventData(pointerlockChangeEvent);

#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, pointerlockChangeEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, pointerlockChangeEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target,
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: pointerlockChangeEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_pointerlockchange_callback_on_thread__proxy: 'sync',
  emscripten_set_pointerlockchange_callback_on_thread__deps: ['$registerPointerlockChangeEventCallback', '$findEventTarget',
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    '$specialHTMLTargets'
#endif
  ],
  emscripten_set_pointerlockchange_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) => {
    if (!document.body?.requestPointerLock) {
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }

#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : specialHTMLTargets[{{{ cDefs.EMSCRIPTEN_EVENT_TARGET_DOCUMENT }}}]; // Pointer lock change events need to be captured from 'document' by default instead of 'window'
#endif
    if (!target) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
    return registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_POINTERLOCKCHANGE }}}, "pointerlockchange", targetThread);
  },

  $registerPointerlockErrorEventCallback__deps: ['$JSEvents'],
  $registerPointerlockErrorEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif

    var pointerlockErrorEventHandlerFunc = (e) => {
#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, 0, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, 0, userData)) e.preventDefault();
    };

    var eventHandler = {
      target,
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: pointerlockErrorEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_pointerlockerror_callback_on_thread__proxy: 'sync',
  emscripten_set_pointerlockerror_callback_on_thread__deps: ['$registerPointerlockErrorEventCallback', '$findEventTarget',
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    '$specialHTMLTargets'
#endif
  ],
  emscripten_set_pointerlockerror_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) => {
    if (!document.body?.requestPointerLock) {
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }

#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : specialHTMLTargets[{{{ cDefs.EMSCRIPTEN_EVENT_TARGET_DOCUMENT }}}]; // Pointer lock change events need to be captured from 'document' by default instead of 'window'
#endif

    if (!target) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
    return registerPointerlockErrorEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_POINTERLOCKERROR }}}, "pointerlockerror", targetThread);
  },

  emscripten_get_pointerlock_status__proxy: 'sync',
  emscripten_get_pointerlock_status__deps: ['$fillPointerlockChangeEventData'],
  emscripten_get_pointerlock_status: (pointerlockStatus) => {
    if (pointerlockStatus) fillPointerlockChangeEventData(pointerlockStatus);
    if (!document.body?.requestPointerLock) {
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  $requestPointerLock: (target) => {
    if (target.requestPointerLock) {
      target.requestPointerLock();
    } else {
      // document.body is known to accept pointer lock, so use that to differentiate if the user passed a bad element,
      // or if the whole browser just doesn't support the feature.
      if (document.body.requestPointerLock) {
        return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
      }
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_request_pointerlock__proxy: 'sync',
  emscripten_request_pointerlock__deps: ['$requestPointerLock', '$findEventTarget',
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    '$JSEvents',
#endif
  ],
  emscripten_request_pointerlock: (target, deferUntilInEventHandler) => {
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target ||= '#canvas';
#endif
    target = findEventTarget(target);
    if (!target) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
    if (!target.requestPointerLock) {
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }

#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // Queue this function call if we're not currently in an event handler and
    // the user saw it appropriate to do so.
    if (!JSEvents.canPerformEventHandlerRequests()) {
      if (deferUntilInEventHandler) {
        JSEvents.deferCall(requestPointerLock, 2 /* priority below fullscreen */, [target]);
        return {{{ cDefs.EMSCRIPTEN_RESULT_DEFERRED }}};
      }
      return {{{ cDefs.EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED }}};
    }
#endif

    return requestPointerLock(target);
  },

#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
  emscripten_exit_pointerlock__deps: ['$JSEvents', '$requestPointerLock'],
#endif
  emscripten_exit_pointerlock__proxy: 'sync',
  emscripten_exit_pointerlock: () => {
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
    // Make sure no queued up calls will fire after this.
    JSEvents.removeDeferredCalls(requestPointerLock);
#endif
    if (!document.exitPointerLock) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    document.exitPointerLock();
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_vibrate__proxy: 'sync',
  emscripten_vibrate: (msecs) => {
    if (!navigator.vibrate) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    navigator.vibrate(msecs);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_vibrate_pattern__proxy: 'sync',
  emscripten_vibrate_pattern: (msecsArray, numEntries) => {
    if (!navigator.vibrate) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};

    var vibrateList = [];
    for (var i = 0; i < numEntries; ++i) {
      var msecs = {{{ makeGetValue('msecsArray', 'i*4', 'i32') }}};
      vibrateList.push(msecs);
    }
    navigator.vibrate(vibrateList);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  $fillVisibilityChangeEventData: (eventStruct) => {
    var visibilityStates = [ "hidden", "visible", "prerender", "unloaded" ];
    var visibilityState = visibilityStates.indexOf(document.visibilityState);

    // Assigning a boolean to HEAP32 with expected type coercion.
    /** @suppress{checkTypes} */
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenVisibilityChangeEvent.hidden, 'document.hidden', 'i8') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenVisibilityChangeEvent.visibilityState, 'visibilityState', 'i32') }}};
  },

  $registerVisibilityChangeEventCallback__noleakcheck: true,
  $registerVisibilityChangeEventCallback__deps: ['$JSEvents', '$fillVisibilityChangeEventData', 'malloc'],
  $registerVisibilityChangeEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenVisibilityChangeEvent.__size__ }}};
    JSEvents.visibilityChangeEvent ||= _malloc(eventSize);

    var visibilityChangeEventHandlerFunc = (e) => {
      var visibilityChangeEvent = JSEvents.visibilityChangeEvent;
      fillVisibilityChangeEventData(visibilityChangeEvent);

#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, visibilityChangeEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, visibilityChangeEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target,
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: visibilityChangeEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_visibilitychange_callback_on_thread__proxy: 'sync',
  emscripten_set_visibilitychange_callback_on_thread__deps: ['$registerVisibilityChangeEventCallback', '$specialHTMLTargets'],
  emscripten_set_visibilitychange_callback_on_thread: (userData, useCapture, callbackfunc, targetThread) => {
#if ENVIRONMENT_MAY_BE_WORKER || ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
  if (!specialHTMLTargets[{{{ cDefs.EMSCRIPTEN_EVENT_TARGET_DOCUMENT }}}]) {
    return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
  }
#endif
    return registerVisibilityChangeEventCallback(specialHTMLTargets[{{{ cDefs.EMSCRIPTEN_EVENT_TARGET_DOCUMENT }}}], userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_VISIBILITYCHANGE }}}, "visibilitychange", targetThread);
  },

  emscripten_get_visibility_status__proxy: 'sync',
  emscripten_get_visibility_status__deps: ['$fillVisibilityChangeEventData'],
  emscripten_get_visibility_status: (visibilityStatus) => {
    if (typeof document.visibilityState == 'undefined' && typeof document.hidden == 'undefined') {
      return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    }
    fillVisibilityChangeEventData(visibilityStatus);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  $registerTouchEventCallback__noleakcheck: true,
  $registerTouchEventCallback__deps: ['$JSEvents', '$findEventTarget', '$getBoundingClientRect', 'malloc'],
  $registerTouchEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenTouchEvent.__size__ }}};
    JSEvents.touchEvent ||= _malloc(eventSize);

    target = findEventTarget(target);

    var touchEventHandlerFunc = (e) => {
#if ASSERTIONS
      assert(e);
#endif
      var t, touches = {}, et = e.touches;
      // To ease marshalling different kinds of touches that browser reports (all touches are listed in e.touches,
      // only changed touches in e.changedTouches, and touches on target at a.targetTouches), mark a boolean in
      // each Touch object so that we can later loop only once over all touches we see to marshall over to Wasm.

      for (let t of et) {
        // Browser might recycle the generated Touch objects between each frame (Firefox on Android), so reset any
        // changed/target states we may have set from previous frame.
        t.isChanged = t.onTarget = 0;
        touches[t.identifier] = t;
      }
      // Mark which touches are part of the changedTouches list.
      for (let t of e.changedTouches) {
        t.isChanged = 1;
        touches[t.identifier] = t;
      }
      // Mark which touches are part of the targetTouches list.
      for (let t of e.targetTouches) {
        touches[t.identifier].onTarget = 1;
      }

      var touchEvent = JSEvents.touchEvent;
      {{{ makeSetValue('touchEvent', C_STRUCTS.EmscriptenTouchEvent.timestamp, 'e.timeStamp', 'double') }}};
      HEAP8[touchEvent + {{{ C_STRUCTS.EmscriptenTouchEvent.ctrlKey }}}] = e.ctrlKey;
      HEAP8[touchEvent + {{{ C_STRUCTS.EmscriptenTouchEvent.shiftKey }}}] = e.shiftKey;
      HEAP8[touchEvent + {{{ C_STRUCTS.EmscriptenTouchEvent.altKey }}}] = e.altKey;
      HEAP8[touchEvent + {{{ C_STRUCTS.EmscriptenTouchEvent.metaKey }}}] = e.metaKey;
      var idx = touchEvent + {{{ C_STRUCTS.EmscriptenTouchEvent.touches }}};
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
      var canvasRect = Module['canvas'] ? getBoundingClientRect(Module['canvas']) : undefined;
#endif
      var targetRect = getBoundingClientRect(target);
      var numTouches = 0;
      for (let t of Object.values(touches)) {
        var idx32 = {{{ getHeapOffset('idx', 'i32') }}}; // Pre-shift the ptr to index to HEAP32 to save code size
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.identifier / 4 }}}] = t.identifier;
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.screenX / 4 }}}] = t.screenX;
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.screenY / 4 }}}] = t.screenY;
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.clientX / 4 }}}] = t.clientX;
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.clientY / 4 }}}] = t.clientY;
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.pageX / 4 }}}] = t.pageX;
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.pageY / 4 }}}] = t.pageY;
        HEAP8[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.isChanged }}}] = t.isChanged;
        HEAP8[idx + {{{ C_STRUCTS.EmscriptenTouchPoint.onTarget }}}] = t.onTarget;
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.targetX / 4 }}}] = t.clientX - (targetRect.left | 0);
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.targetY / 4 }}}] = t.clientY - (targetRect.top  | 0);
#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.canvasX / 4 }}}] = canvasRect ? t.clientX - (canvasRect.left | 0) : 0;
        HEAP32[idx32 + {{{ C_STRUCTS.EmscriptenTouchPoint.canvasY / 4 }}}] = canvasRect ? t.clientY - (canvasRect.top  | 0) : 0;
#endif

        idx += {{{ C_STRUCTS.EmscriptenTouchPoint.__size__ }}};

        if (++numTouches > 31) {
          break;
        }
      }
      {{{ makeSetValue('touchEvent', C_STRUCTS.EmscriptenTouchEvent.numTouches, 'numTouches', 'i32') }}};

#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, touchEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, touchEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target,
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      allowsDeferredCalls: eventTypeString == 'touchstart' || eventTypeString == 'touchend',
#endif
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: touchEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_touchstart_callback_on_thread__proxy: 'sync',
  emscripten_set_touchstart_callback_on_thread__deps: ['$registerTouchEventCallback'],
  emscripten_set_touchstart_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_TOUCHSTART }}}, "touchstart", targetThread),

  emscripten_set_touchend_callback_on_thread__proxy: 'sync',
  emscripten_set_touchend_callback_on_thread__deps: ['$registerTouchEventCallback'],
  emscripten_set_touchend_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_TOUCHEND }}}, "touchend", targetThread),

  emscripten_set_touchmove_callback_on_thread__proxy: 'sync',
  emscripten_set_touchmove_callback_on_thread__deps: ['$registerTouchEventCallback'],
  emscripten_set_touchmove_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_TOUCHMOVE }}}, "touchmove", targetThread),

  emscripten_set_touchcancel_callback_on_thread__proxy: 'sync',
  emscripten_set_touchcancel_callback_on_thread__deps: ['$registerTouchEventCallback'],
  emscripten_set_touchcancel_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) =>
    registerTouchEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_TOUCHCANCEL }}}, "touchcancel", targetThread),

  $fillGamepadEventData__deps: ['$stringToUTF8'],
  $fillGamepadEventData: (eventStruct, e) => {
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.timestamp, 'e.timestamp', 'double') }}};
    for (var i = 0; i < e.axes.length; ++i) {
      {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.EmscriptenGamepadEvent.axis, 'e.axes[i]', 'double') }}};
    }
    for (var i = 0; i < e.buttons.length; ++i) {
      if (typeof e.buttons[i] == 'object') {
        {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.EmscriptenGamepadEvent.analogButton, 'e.buttons[i].value', 'double') }}};
      } else {
        {{{ makeSetValue('eventStruct+i*8', C_STRUCTS.EmscriptenGamepadEvent.analogButton, 'e.buttons[i]', 'double') }}};
      }
    }
    for (var i = 0; i < e.buttons.length; ++i) {
      if (typeof e.buttons[i] == 'object') {
        {{{ makeSetValue('eventStruct+i', C_STRUCTS.EmscriptenGamepadEvent.digitalButton, 'e.buttons[i].pressed', 'i8') }}};
      } else {
        // Assigning a boolean to HEAP32, that's ok, but Closure would like to warn about it:
        /** @suppress {checkTypes} */
        {{{ makeSetValue('eventStruct+i', C_STRUCTS.EmscriptenGamepadEvent.digitalButton, 'e.buttons[i] == 1', 'i8') }}};
      }
    }
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.connected, 'e.connected', 'i8') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.index, 'e.index', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.numAxes, 'e.axes.length', 'i32') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenGamepadEvent.numButtons, 'e.buttons.length', 'i32') }}};
    stringToUTF8(e.id, eventStruct + {{{ C_STRUCTS.EmscriptenGamepadEvent.id }}}, {{{ cDefs.EM_HTML5_MEDIUM_STRING_LEN_BYTES }}});
    stringToUTF8(e.mapping, eventStruct + {{{ C_STRUCTS.EmscriptenGamepadEvent.mapping }}}, {{{ cDefs.EM_HTML5_MEDIUM_STRING_LEN_BYTES }}});
  },

  $registerGamepadEventCallback__noleakcheck: true,
  $registerGamepadEventCallback__deps: ['$JSEvents', '$fillGamepadEventData', '$findEventTarget', 'malloc'],
  $registerGamepadEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenGamepadEvent.__size__ }}};
    JSEvents.gamepadEvent ||= _malloc(eventSize);

    var gamepadEventHandlerFunc = (e) => {
      var gamepadEvent = JSEvents.gamepadEvent;
      fillGamepadEventData(gamepadEvent, e["gamepad"]);

#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, gamepadEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, gamepadEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
#if HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS
      allowsDeferredCalls: true,
#endif
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: gamepadEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_gamepadconnected_callback_on_thread__proxy: 'sync',
  emscripten_set_gamepadconnected_callback_on_thread__deps: ['$registerGamepadEventCallback', 'emscripten_sample_gamepad_data'],
  emscripten_set_gamepadconnected_callback_on_thread: (userData, useCapture, callbackfunc, targetThread) => {
    if (_emscripten_sample_gamepad_data()) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    return registerGamepadEventCallback({{{ cDefs.EMSCRIPTEN_EVENT_TARGET_WINDOW }}}, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_GAMEPADCONNECTED }}}, "gamepadconnected", targetThread);
  },

  emscripten_set_gamepaddisconnected_callback_on_thread__proxy: 'sync',
  emscripten_set_gamepaddisconnected_callback_on_thread__deps: ['$registerGamepadEventCallback', 'emscripten_sample_gamepad_data'],
  emscripten_set_gamepaddisconnected_callback_on_thread: (userData, useCapture, callbackfunc, targetThread) => {
    if (_emscripten_sample_gamepad_data()) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    return registerGamepadEventCallback({{{ cDefs.EMSCRIPTEN_EVENT_TARGET_WINDOW }}}, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED }}}, "gamepaddisconnected", targetThread);
  },

  emscripten_sample_gamepad_data__docs: '/** @suppress {checkTypes} */', // We assign null to navigator.getGamepads, which Closure would like to complain about.
  emscripten_sample_gamepad_data__proxy: 'sync',
  emscripten_sample_gamepad_data__deps: ['$JSEvents'],
  emscripten_sample_gamepad_data: () => {
    try {
      if (navigator.getGamepads) return (JSEvents.lastGamepadState = navigator.getGamepads())
        ? {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}} : {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    } catch(e) {
#if ASSERTIONS
      err(`navigator.getGamepads() exists, but failed to execute with exception ${e}. Disabling Gamepad access.`);
#endif
      navigator.getGamepads = null; // Disable getGamepads() so that it won't be attempted to be used again.
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
  },

  emscripten_get_num_gamepads__proxy: 'sync',
  emscripten_get_num_gamepads__deps: ['$JSEvents'],
  emscripten_get_num_gamepads: () => {
#if ASSERTIONS
    assert(JSEvents.lastGamepadState, 'emscripten_get_num_gamepads() can only be called after having first called emscripten_sample_gamepad_data() and that function has returned EMSCRIPTEN_RESULT_SUCCESS!');
#endif
    // N.B. Do not call emscripten_get_num_gamepads() unless having first called emscripten_sample_gamepad_data(), and that has returned EMSCRIPTEN_RESULT_SUCCESS.
    // Otherwise the following line will throw an exception.
    return JSEvents.lastGamepadState.length;
  },

  emscripten_get_gamepad_status__proxy: 'sync',
  emscripten_get_gamepad_status__deps: ['$JSEvents', '$fillGamepadEventData'],
  emscripten_get_gamepad_status: (index, gamepadState) => {
#if ASSERTIONS
    assert(JSEvents.lastGamepadState, 'emscripten_get_gamepad_status() can only be called after having first called emscripten_sample_gamepad_data() and that function has returned EMSCRIPTEN_RESULT_SUCCESS!');
#endif
    // INVALID_PARAM is returned on a Gamepad index that never was there.
    if (index < 0 || index >= JSEvents.lastGamepadState.length) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};

    // NO_DATA is returned on a Gamepad index that was removed.
    // For previously disconnected gamepads there should be an empty slot (null/undefined/false) at the index.
    // This is because gamepads must keep their original position in the array.
    // For example, removing the first of two gamepads produces [null/undefined/false, gamepad].
    if (!JSEvents.lastGamepadState[index]) return {{{ cDefs.EMSCRIPTEN_RESULT_NO_DATA }}};

    fillGamepadEventData(gamepadState, JSEvents.lastGamepadState[index]);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  $registerBeforeUnloadEventCallback__deps: ['$JSEvents', '$findEventTarget'],
  $registerBeforeUnloadEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) => {
    var beforeUnloadEventHandlerFunc = (e) => {
      // Note: This is always called on the main browser thread, since it needs synchronously return a value!
      var confirmationMessage = {{{ makeDynCall('pipp', 'callbackfunc') }}}(eventTypeId, 0, userData);

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
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: beforeUnloadEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_beforeunload_callback_on_thread__proxy: 'sync',
  emscripten_set_beforeunload_callback_on_thread__deps: ['$registerBeforeUnloadEventCallback'],
  emscripten_set_beforeunload_callback_on_thread: (userData, callbackfunc, targetThread) => {
    if (typeof onbeforeunload == 'undefined') return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    // beforeunload callback can only be registered on the main browser thread, because the page will go away immediately after returning from the handler,
    // and there is no time to start proxying it anywhere.
    if (targetThread !== {{{ cDefs.EM_CALLBACK_THREAD_CONTEXT_MAIN_RUNTIME_THREAD }}}) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    return registerBeforeUnloadEventCallback({{{ cDefs.EMSCRIPTEN_EVENT_TARGET_WINDOW }}}, userData, true, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_BEFOREUNLOAD }}}, "beforeunload");
  },

  $fillBatteryEventData: (eventStruct, battery) => {
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.chargingTime, 'battery.chargingTime', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.dischargingTime, 'battery.dischargingTime', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.level, 'battery.level', 'double') }}};
    {{{ makeSetValue('eventStruct', C_STRUCTS.EmscriptenBatteryEvent.charging, 'battery.charging', 'i8') }}};
  },

  $hasBatteryAPI__internal: true,
  $hasBatteryAPI: () => globalThis.navigator?.getBattery,

  $registerBatteryEventCallback__noleakcheck: true,
  $registerBatteryEventCallback__deps: ['$JSEvents', '$fillBatteryEventData', 'malloc'],
  $registerBatteryEventCallback: (battery, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif
    var eventSize = {{{ C_STRUCTS.EmscriptenBatteryEvent.__size__ }}};
    JSEvents.batteryEvent ||= _malloc(eventSize)

    var batteryEventHandlerFunc = (e) => {
      var batteryEvent = JSEvents.batteryEvent;
      fillBatteryEventData(batteryEvent, battery);

#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, batteryEvent, eventSize, userData);
      else
#endif
      if ({{{ makeDynCall('iipp', 'callbackfunc') }}}(eventTypeId, batteryEvent, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: battery,
      eventTypeString,
      eventTypeId,
      userData,
      callbackfunc,
      handlerFunc: batteryEventHandlerFunc,
      useCapture
    };
    return JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_batterychargingchange_callback_on_thread__proxy: 'sync',
  emscripten_set_batterychargingchange_callback_on_thread__deps: ['$registerBatteryEventCallback', '$hasBatteryAPI'],
  emscripten_set_batterychargingchange_callback_on_thread: (userData, callbackfunc, targetThread) => {
    if (!hasBatteryAPI()) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    navigator.getBattery().then((b) => {
      registerBatteryEventCallback(b, userData, true, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_BATTERYCHARGINGCHANGE }}}, "chargingchange", targetThread);
    });
  },

  emscripten_set_batterylevelchange_callback_on_thread__proxy: 'sync',
  emscripten_set_batterylevelchange_callback_on_thread__deps: ['$registerBatteryEventCallback', '$hasBatteryAPI'],
  emscripten_set_batterylevelchange_callback_on_thread: (userData, callbackfunc, targetThread) => {
    if (!hasBatteryAPI()) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    navigator.getBattery().then((b) => {
      registerBatteryEventCallback(b, userData, true, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_BATTERYLEVELCHANGE }}}, "levelchange", targetThread);
    });
  },

  $batteryManager: undefined,
  $batteryManager__internal: true,

  emscripten_get_battery_status__proxy: 'sync',
  emscripten_get_battery_status__deps: ['$fillBatteryEventData', '$hasBatteryAPI', '$batteryManager'],
  emscripten_get_battery_status: (batteryState) => {
    if (!hasBatteryAPI()) return {{{ cDefs.EMSCRIPTEN_RESULT_NOT_SUPPORTED }}};
    if (!batteryManager) {
      navigator.getBattery().then((b) => {
        batteryManager = b;
      });
      return {{{ cDefs.EMSCRIPTEN_RESULT_NO_DATA }}};
    }
    fillBatteryEventData(batteryState, batteryManager);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

#if PTHREADS
  $setCanvasElementSizeCallingThread__deps: [
#if OFFSCREENCANVAS_SUPPORT
    '$setOffscreenCanvasSizeOnTargetThread',
#endif
    '$findCanvasEventTarget'],
  $setCanvasElementSizeCallingThread: (target, width, height) => {
    var canvas = findCanvasEventTarget(target);
    if (!canvas) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};

#if OFFSCREENCANVAS_SUPPORT
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
#else
    if (!canvas.controlTransferredOffscreen) {
#endif
      var autoResizeViewport = false;
      if (canvas.GLctxObject?.GLctx) {
        var prevViewport = canvas.GLctxObject.GLctx.getParameter(0xBA2 /* GL_VIEWPORT */);
        // TODO: Perhaps autoResizeViewport should only be true if FBO 0 is currently active?
        autoResizeViewport = (prevViewport[0] === 0 && prevViewport[1] === 0 && prevViewport[2] === canvas.width && prevViewport[3] === canvas.height);
#if GL_DEBUG
        dbg(`Resizing canvas from ${canvas.width}x${canvas.height} to ${width}x${height}. Previous GL viewport size was ${prevViewport}, so autoResizeViewport=${autoResizeViewport}`);
#endif
      }
      canvas.width = width;
      canvas.height = height;
      if (autoResizeViewport) {
#if GL_DEBUG
        dbg(`Automatically resizing GL viewport to cover whole render target ${width}x${height}`);
#endif
        // TODO: Add -sCANVAS_RESIZE_SETS_GL_VIEWPORT=0/1 option (default=1). This is commonly done and several graphics engines depend on this,
        // but this can be quite disruptive.
        canvas.GLctxObject.GLctx.viewport(0, 0, width, height);
      }
#if OFFSCREENCANVAS_SUPPORT
    } else if (canvas.canvasSharedPtr) {
      var targetThread = {{{ makeGetValue('canvas.canvasSharedPtr', 8, '*') }}};
      setOffscreenCanvasSizeOnTargetThread(targetThread, target, width, height);
      return {{{ cDefs.EMSCRIPTEN_RESULT_DEFERRED }}}; // This will have to be done asynchronously
#endif
    } else {
#if GL_DEBUG
      dbg('canvas.controlTransferredOffscreen but we do not own the canvas, and do not know who has (no canvas.canvasSharedPtr present, an internal bug?)!\n');
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
    }
#if OFFSCREEN_FRAMEBUFFER
    if (canvas.GLctxObject) GL.resizeOffscreenFramebuffer(canvas.GLctxObject);
#endif
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

#if OFFSCREENCANVAS_SUPPORT
  $setOffscreenCanvasSizeOnTargetThread__deps: ['$stringToNewUTF8', '_emscripten_set_offscreencanvas_size_on_thread'],
  $setOffscreenCanvasSizeOnTargetThread: (targetThread, targetCanvas, width, height) => {
    targetCanvas = targetCanvas ? UTF8ToString(targetCanvas) : '';
    var targetCanvasPtr = 0;
    if (targetCanvas) {
      targetCanvasPtr = stringToNewUTF8(targetCanvas);
    }
    __emscripten_set_offscreencanvas_size_on_thread(targetThread, targetCanvasPtr, width, height);
  },
#endif

  $setCanvasElementSizeMainThread__proxy: 'sync',
  $setCanvasElementSizeMainThread__deps: ['$setCanvasElementSizeCallingThread'],
  $setCanvasElementSizeMainThread: (target, width, height) => setCanvasElementSizeCallingThread(target, width, height),

  emscripten_set_canvas_element_size__deps: ['$setCanvasElementSizeCallingThread', '$setCanvasElementSizeMainThread', '$findCanvasEventTarget'],
  emscripten_set_canvas_element_size: (target, width, height) => {
#if GL_DEBUG
    dbg(`emscripten_set_canvas_element_size(target=${target},width=${width},height=${height}`);
#endif
    var canvas = findCanvasEventTarget(target);
    if (canvas) {
      return setCanvasElementSizeCallingThread(target, width, height);
    }
    return setCanvasElementSizeMainThread(target, width, height);
  },
#else
  emscripten_set_canvas_element_size__deps: ['$findCanvasEventTarget'],
  emscripten_set_canvas_element_size: (target, width, height) => {
#if GL_DEBUG
    dbg(`emscripten_set_canvas_element_size(target=${target},width=${width},height=${height}`);
#endif
    var canvas = findCanvasEventTarget(target);
    if (!canvas) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
    canvas.width = width;
    canvas.height = height;
#if OFFSCREEN_FRAMEBUFFER
    if (canvas.GLctxObject) GL.resizeOffscreenFramebuffer(canvas.GLctxObject);
#endif
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },
#endif

  $setCanvasElementSize__deps: ['emscripten_set_canvas_element_size', '$stackSave', '$stackRestore', '$stringToUTF8OnStack'],
  $setCanvasElementSize: (target, width, height) => {
#if GL_DEBUG
    dbg(`setCanvasElementSize(target=${target},width=${width},height=${height}`);
#endif
    if (!target.controlTransferredOffscreen) {
      target.width = width;
      target.height = height;
    } else {
      // This function is being called from high-level JavaScript code instead of asm.js/Wasm,
      // and it needs to synchronously proxy over to another thread, so marshal the string onto the heap to do the call.
      var sp = stackSave();
      var targetInt = stringToUTF8OnStack(target.id);
      _emscripten_set_canvas_element_size(targetInt, width, height);
      stackRestore(sp);
    }
  },

#if PTHREADS
  $getCanvasSizeCallingThread__deps: ['$findCanvasEventTarget'],
  $getCanvasSizeCallingThread: (target, width, height) => {
    var canvas = findCanvasEventTarget(target);
    if (!canvas) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};

#if OFFSCREENCANVAS_SUPPORT
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
    } else
#endif
    if (!canvas.controlTransferredOffscreen) {
      {{{ makeSetValue('width', 0, 'canvas.width', 'i32') }}};
      {{{ makeSetValue('height', 0, 'canvas.height', 'i32') }}};
    } else {
#if GL_DEBUG
      dbg('canvas.controlTransferredOffscreen but we do not own the canvas, and do not know who has (no canvas.canvasSharedPtr present, an internal bug?)!\n');
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
    }
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  $getCanvasSizeMainThread__proxy: 'sync',
  $getCanvasSizeMainThread__deps: ['$getCanvasSizeCallingThread'],
  $getCanvasSizeMainThread: (target, width, height) => getCanvasSizeCallingThread(target, width, height),

  emscripten_get_canvas_element_size__deps: ['$getCanvasSizeCallingThread', '$getCanvasSizeMainThread', '$findCanvasEventTarget'],
  emscripten_get_canvas_element_size: (target, width, height) => {
    var canvas = findCanvasEventTarget(target);
    if (canvas) {
      return getCanvasSizeCallingThread(target, width, height);
    }
    return getCanvasSizeMainThread(target, width, height);
  },
#else
  emscripten_get_canvas_element_size__deps: ['$findCanvasEventTarget'],
  emscripten_get_canvas_element_size: (target, width, height) => {
    var canvas = findCanvasEventTarget(target);
    if (!canvas) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};
    {{{ makeSetValue('width', '0', 'canvas.width', 'i32') }}};
    {{{ makeSetValue('height', '0', 'canvas.height', 'i32') }}};
  },
#endif

  // JavaScript-friendly API, returns pair [width, height]
  $getCanvasElementSize__deps: ['emscripten_get_canvas_element_size', '$stackSave', '$stackRestore', '$stringToUTF8OnStack'],
  $getCanvasElementSize: (target) => {
    var sp = stackSave();
    var w = stackAlloc(8);
    var h = w + 4;

    var targetInt = stringToUTF8OnStack(target.id);
    var ret = _emscripten_get_canvas_element_size(targetInt, w, h);
    var size = [{{{ makeGetValue('w', 0, 'i32')}}}, {{{ makeGetValue('h', 0, 'i32')}}}];
    stackRestore(sp);
    return size;
  },

  emscripten_set_element_css_size__proxy: 'sync',
  emscripten_set_element_css_size__deps: ['$findEventTarget'],
  emscripten_set_element_css_size: (target, width, height) => {
#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : Module['canvas'];
#endif
    if (!target) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};

    target.style.width = width + "px";
    target.style.height = height + "px";

    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_get_element_css_size__proxy: 'sync',
  emscripten_get_element_css_size__deps: ['$findEventTarget', '$getBoundingClientRect'],
  emscripten_get_element_css_size: (target, width, height) => {
#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target = findEventTarget(target);
#else
    target = target ? findEventTarget(target) : Module['canvas'];
#endif
    if (!target) return {{{ cDefs.EMSCRIPTEN_RESULT_UNKNOWN_TARGET }}};

    var rect = getBoundingClientRect(target);
    {{{ makeSetValue('width', '0', 'rect.width', 'double') }}};
    {{{ makeSetValue('height', '0', 'rect.height', 'double') }}};

    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_html5_remove_all_event_listeners__deps: ['$JSEvents'],
  emscripten_html5_remove_all_event_listeners: () => JSEvents.removeAllEventListeners(),

  emscripten_request_animation_frame: (cb, userData) =>
    requestAnimationFrame((timeStamp) => {{{ makeDynCall('idp', 'cb') }}}(timeStamp, userData)),

  emscripten_cancel_animation_frame: (id) => cancelAnimationFrame(id),

  emscripten_request_animation_frame_loop: (cb, userData) => {
    function tick(timeStamp) {
      if ({{{ makeDynCall('idp', 'cb') }}}(timeStamp, userData)) {
        requestAnimationFrame(tick);
      }
    }
    return requestAnimationFrame(tick);
  },

  emscripten_get_device_pixel_ratio__proxy: 'sync',
  emscripten_get_device_pixel_ratio: () => {
#if ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
    return globalThis.devicePixelRatio ?? 1.0;
#else // otherwise, on the web and in workers, things are simpler
    return devicePixelRatio;
#endif
  }
};

addToLibrary(LibraryHTML5);
