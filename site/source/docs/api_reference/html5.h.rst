.. _html5-h:

=======
html5.h
=======

The C++ APIs in `html5.h <https://github.com/emscripten-core/emscripten/blob/master/system/include/emscripten/html5.h>`_ define the Emscripten low-level glue bindings to interact with HTML5 events from native code.

.. tip:: The C++ APIs map closely to their :ref:`equivalent HTML5 JavaScript APIs <specifications-html5-api>`. The HTML5 specifications listed below provide additional detailed reference "over and above" the information provided in this document.

  In addition, the :ref:`test-example-code-html5-api` can be reviewed to see how the code is used.

.. _specifications-html5-api:

The HTML5 specifications for APIs that are mapped by **html5.h** include:

  - `DOM Level 3 Events: Keyboard, Mouse, Mouse Wheel, Resize, Scroll, Focus <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html>`_.
  - `Device Orientation Events for gyro and accelerometer <http://www.w3.org/TR/orientation-event/>`_.
  - `Screen Orientation Events for portrait/landscape handling <https://dvcs.w3.org/hg/screen-orientation/raw-file/tip/Overview.html>`_.
  - `Fullscreen Events for browser canvas fullscreen modes transitioning <https://dvcs.w3.org/hg/fullscreen/raw-file/tip/Overview.html>`_.
  - `Pointer Lock Events for relative-mode mouse motion control <http://www.w3.org/TR/pointerlock/>`_.
  - `Vibration API for mobile device haptic vibration feedback control <http://dev.w3.org/2009/dap/vibration/>`_.
  - `Page Visibility Events for power management control <http://www.w3.org/TR/page-visibility/>`_.
  - `Touch Events <http://www.w3.org/TR/touch-events/>`_.
  - `Gamepad API <http://www.w3.org/TR/gamepad/>`_.
  - `Beforeunload event <http://www.whatwg.org/specs/web-apps/current-work/multipage/history.html#beforeunloadevent>`_.
  - `WebGL context events <http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2>`_.
  - `Animation and timing <https://developer.mozilla.org/en-US/docs/Web/API/window/requestAnimationFrame>`_.
  - `Console <https://developer.mozilla.org/en-US/docs/Web/API/console>`_.
  - `Throw <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/throw>`_.


.. contents:: Table of Contents
    :local:
    :depth: 1

How to use this API
===================

Most of these APIs use an event-based architecture; functionality is accessed by registering a callback function that will be called when the event occurs.

.. note:: The Gamepad API is currently an exception, as only a polling API is available. For some APIs, both an event-based and a polling-based model are exposed.



Registration functions
----------------------

The typical format of registration functions is as follows (some methods may omit various parameters):

  .. code-block:: cpp

    EMSCRIPTEN_RESULT emscripten_set_some_callback(
      const char *target,   // ID of the target HTML element.
      void *userData,   // User-defined data to be passed to the callback.
      EM_BOOL useCapture,   // Whether or not to use capture.
      em_someevent_callback_func callback   // Callback function.
    );


.. _target-parameter-html5-api:

The ``target`` parameter is the ID of the HTML element to which the callback registration is to be applied. This field has the following special meanings:

  - ``0`` or ``NULL``: A default element is chosen automatically based on the event type, which should be reasonable most of the time.
  - ``#window``: The event listener is applied to the JavaScript ``window`` object.
  - ``#document``: The event listener is applied to the JavaScript ``document`` object.
  - ``#screen``: The event listener is applied to the JavaScript ``window.screen`` object.
  - ``#canvas``: The event listener is applied to the Emscripten default WebGL canvas element.
  - Any other string **without a leading hash "#"** sign: The event listener is applied to the element on the page with the given ID.

.. _userdata-parameter-html5-api:

The ``userData`` parameter is a user-defined value that is passed (unchanged) to the registered event callback. This can be used to, for example, pass a pointer to a C++ class or similarly to enclose the C API in a clean object-oriented manner.

.. _usecapture-parameter-html5-api:

The ``useCapture`` parameter  maps to ``useCapture`` in `EventTarget.addEventListener <https://developer.mozilla.org/en-US/docs/Web/API/EventTarget.addEventListener>`_. It indicates whether or not to initiate *capture*: if ``true`` the callback will be invoked only for the DOM capture and target phases; if ``false`` the callback will be triggered during the target and bubbling phases. See `DOM Level 3 Events <http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107/events.html#Events-phases>`_ for a more detailed explanation.

Most functions return the result using the type :c:data:`EMSCRIPTEN_RESULT`. Zero and positive values denote success. Negative values signal failure. None of the functions fail or abort by throwing a JavaScript or C++ exception. If a particular browser does not support the given feature, the value :c:data:`EMSCRIPTEN_RESULT_NOT_SUPPORTED` will be returned at the time the callback is registered.


Callback functions
------------------

When the event occurs the callback is invoked with the relevant event "type" (for example :c:data:`EMSCRIPTEN_EVENT_CLICK`), a ``struct`` containing the details of the event that occurred, and the ``userData`` that was originally passed to the registration function. The general format of the callback function is: ::

  typedef EM_BOOL (*em_someevent_callback_func) // Callback function. Return true if event is "consumed".
    (
    int eventType, // The type of event.
    const EmscriptenSomeEvent *someEvent, // Information about the event.
    void *userData // User data passed from the registration function.
    );


.. _callback-handler-return-em_bool-html5-api:

Callback handlers that return an :c:data:`EM_BOOL` may specify ``true`` to signal that the handler *consumed* the event (this suppresses the default action for that event by calling its ``.preventDefault();`` member). Returning ``false`` indicates that the event was not consumed — the default browser event action is carried out and the event is allowed to pass on/bubble up as normal.

Calling a registration function with a ``null`` pointer for the callback causes a de-registration of that callback from the given ``target`` element. All event handlers are also automatically unregistered when the C ``exit()`` function is invoked during the ``atexit`` handler pass. Either use the function :c:func:`emscripten_set_main_loop` or set ``Module.noExitRuntime = true;`` to make sure that leaving ``main()`` will not immediately cause an ``exit()`` and clean up the event handlers.

.. _web-security-functions-html5-api:

Functions affected by web security
----------------------------------

Some functions, including :c:func:`emscripten_request_pointerlock` and :c:func:`emscripten_request_fullscreen`, are affected by web security.

While the functions can be called anywhere, the actual "requests" can only be raised inside the handler for a user-generated event (for example a key, mouse or touch press/release).

When porting code, it may be difficult to ensure that the functions are called inside appropriate event handlers (so that the requests are raised immediately). As a convenience, developers can set ``deferUntilInEventHandler=true`` to automatically defer insecure requests until the user next presses a keyboard or mouse button. This simplifies porting, but often results in a poorer user experience. For example, the user must click once on the canvas to hide the pointer or transition to full screen.

Where possible, the functions should only be called inside appropriate event handlers. Setting ``deferUntilInEventHandler=false`` causes the functions to abort with an error if the request is refused due to a security restriction: this is a useful mechanism for discovering instances where the functions are called outside the handler for a user-generated event.


.. _test-example-code-html5-api:

Test/Example code
-----------------

The HTML5 test code demonstrates how to use this API:

  - `test_html5.c <https://github.com/emscripten-core/emscripten/blob/master/tests/test_html5.c>`_
  - `test_html5_fullscreen.c <https://github.com/emscripten-core/emscripten/blob/master/tests/test_html5_fullscreen.c>`_
  - `test_html5_mouse.c <https://github.com/emscripten-core/emscripten/blob/master/tests/test_html5_mouse.c>`_


General types
=============


.. c:macro:: EM_BOOL

  This is the Emscripten type for a ``bool``.
  Possible values:

  .. c:macro:: EM_TRUE

    This is the Emscripten value for ``true``.

  .. c:macro:: EM_FALSE

    This is the Emscripten value for ``false``.


.. c:macro:: EM_UTF8

  This is the Emscripten type for a UTF8 string (maps to a ``char``). This is used for node names, element ids, etc.



Function result values
======================

Most functions in this API return a result of type :c:data:`EMSCRIPTEN_RESULT`. None of the functions fail or abort by throwing a JavaScript or C++ exception. If a particular browser does not support the given feature, the value :c:data:`EMSCRIPTEN_RESULT_NOT_SUPPORTED` will be returned at the time the callback is registered.


.. c:macro:: EMSCRIPTEN_RESULT

  This type is used to return the result of most functions in this API. Zero and positive values denote success, while negative values signal failure. Possible values are listed below.


  .. c:macro:: EMSCRIPTEN_RESULT_SUCCESS

    The operation succeeded.

  .. c:macro:: EMSCRIPTEN_RESULT_DEFERRED

    The requested operation cannot be completed now for :ref:`web security reasons<web-security-functions-html5-api>`, and has been deferred for completion in the next event handler.

  .. c:macro:: EMSCRIPTEN_RESULT_NOT_SUPPORTED

    The given operation is not supported by this browser or the target element. This value will be returned at the time the callback is registered if the operation is not supported.


  .. c:macro:: EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED

    The requested operation could not be completed now for :ref:`web security reasons<web-security-functions-html5-api>`. It failed because the user requested the operation not be deferred.

  .. c:macro:: EMSCRIPTEN_RESULT_INVALID_TARGET

    The operation failed because the specified target element is invalid.

  .. c:macro:: EMSCRIPTEN_RESULT_UNKNOWN_TARGET

    The operation failed because the specified target element was not found.

  .. c:macro:: EMSCRIPTEN_RESULT_INVALID_PARAM

    The operation failed because an invalid parameter was passed to the function.

  .. c:macro:: EMSCRIPTEN_RESULT_FAILED

    Generic failure result message, returned if no specific result is available.

  .. c:macro:: EMSCRIPTEN_RESULT_NO_DATA

    The operation failed because no data is currently available.



Keys
====

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_KEYPRESS
  EMSCRIPTEN_EVENT_KEYDOWN
  EMSCRIPTEN_EVENT_KEYUP

    Emscripten key events.

.. c:macro:: DOM_KEY_LOCATION

  The location of the key on the keyboard; one of the values below.

  .. c:macro:: DOM_KEY_LOCATION_STANDARD
    DOM_KEY_LOCATION_LEFT
    DOM_KEY_LOCATION_RIGHT
    DOM_KEY_LOCATION_NUMPAD

    Locations of the key on the keyboard.

Struct
------

.. c:type:: EmscriptenKeyboardEvent

  The event structure passed in `keyboard events <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#keys>`_: ``keypress``, ``keydown`` and ``keyup``.

  Note that since the `DOM Level 3 Events spec <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#keys>`_ is very recent at the time of writing (2014-03), uniform support for the different fields in the spec is still in flux. Be sure to check the results in multiple browsers. See the `unmerged pull request #2222 <https://github.com/emscripten-core/emscripten/pull/2222>`_ for an example of how to interpret the legacy key events.


  .. c:member:: EM_UTF8 key

    The printed representation of the pressed key.

    Maximum size 32 ``char`` (i.e. ``EM_UTF8 key[32]``).

  .. c:member:: EM_UTF8 code

    A string that identifies the physical key being pressed. The value is not affected by the current keyboard layout or modifier state, so a particular key will always return the same value.

    Maximum size 32 ``char`` (i.e. ``EM_UTF8 code[32]``).

  .. c:member:: unsigned long location

    Indicates the location of the key on the keyboard. One of the :c:data:`DOM_KEY_LOCATION <DOM_KEY_LOCATION_STANDARD>` values.

  .. c:member:: EM_BOOL ctrlKey
    EM_BOOL shiftKey
    EM_BOOL altKey
    EM_BOOL metaKey

    Specifies which modifiers were active during the key event.

  .. c:member:: EM_BOOL repeat

    Specifies if this keyboard event represents a repeated press.

  .. c:member:: EM_UTF8 locale

    A locale string indicating the configured keyboard locale. This may be an empty string if the browser or device doesn't know the keyboard's locale.

    Maximum size 32 char (i.e. ``EM_UTF8 locale[32]``).

  .. c:member:: EM_UTF8 charValue

    The following fields are values from previous versions of the DOM key events specifications. See `the character representation of the key <https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent?redirectlocale=en-US&redirectslug=DOM%2FKeyboardEvent>`_. This is the field ``char`` from the docs, but renamed to ``charValue`` to avoid a C reserved word.

    Maximum size 32 ``char`` (i.e. ``EM_UTF8 charValue[32]``).

    .. warning:: This attribute has been dropped from DOM Level 3 events.

  .. c:member:: unsigned long charCode

    The Unicode reference number of the key; this attribute is used only by the keypress event. For keys whose ``char`` attribute contains multiple characters, this is the Unicode value of the first character in that attribute.

    .. warning:: This attribute is deprecated, you should use the field ``key`` instead, if available.

  .. c:member:: unsigned long keyCode

    A system and implementation dependent numerical code identifying the unmodified value of the pressed key.

    .. warning:: This attribute is deprecated, you should use the field ``key`` instead, if available.


  .. c:member:: unsigned long which

    A system and implementation dependent numeric code identifying the unmodified value of the pressed key; this is usually the same as ``keyCode``.

    .. warning:: This attribute is deprecated, you should use the field ``key`` instead, if available. Note thought that while this field is deprecated, the cross-browser support for ``which`` may be better than for the other fields, so experimentation is recommended. Read issue https://github.com/emscripten-core/emscripten/issues/2817 for more information.


Callback functions
------------------

.. c:type:: em_key_callback_func

  Function pointer for the :c:func:`keypress callback functions <emscripten_set_keypress_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_key_callback_func)(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData);

  :param int eventType: The type of :c:data:`key event <EMSCRIPTEN_EVENT_KEYPRESS>`.
  :param keyEvent: Information about the key event that occurred.
  :type keyEvent: const EmscriptenKeyboardEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|


Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_keypress_callback(const char *target, void *userData, EM_BOOL useCapture, em_key_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_keydown_callback(const char *target, void *userData, EM_BOOL useCapture, em_key_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_keyup_callback(const char *target, void *userData, EM_BOOL useCapture, em_key_callback_func callback)

  Registers a callback function for receiving browser-generated keyboard input events.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL  useCapture: |useCapture-parameter-doc|
  :param em_key_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|

  :see also:
    - https://developer.mozilla.org/en/DOM/Event/UIEvent/KeyEvent
    - http://www.javascriptkit.com/jsref/eventkeyboardmouse.shtml

    .. note:: To receive events, the element must be focusable, see https://github.com/emscripten-core/emscripten/pull/7484#issuecomment-437887001

Mouse
=====

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_CLICK
  EMSCRIPTEN_EVENT_MOUSEDOWN
  EMSCRIPTEN_EVENT_MOUSEUP
  EMSCRIPTEN_EVENT_DBLCLICK
  EMSCRIPTEN_EVENT_MOUSEMOVE
  EMSCRIPTEN_EVENT_MOUSEENTER
  EMSCRIPTEN_EVENT_MOUSELEAVE

    Emscripten mouse events.


Struct
------

.. c:type:: EmscriptenMouseEvent

  The event structure passed in `mouse events <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#interface-MouseEvent>`_: `click <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-click>`_, `mousedown <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-mousedown>`_, `mouseup <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-mouseup>`_, `dblclick <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-dblclick>`_, `mousemove <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-mousemove>`_, `mouseenter <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-mouseenter>`_ and `mouseleave <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-mouseleave>`_.


  .. c:member:: double timestamp;

    A timestamp of when this data was generated by the browser. This is an absolute wallclock time in milliseconds.

  .. c:member:: long screenX
    long screenY

    The coordinates relative to the browser screen coordinate system.

  .. c:member:: long clientX
    long clientY

    The coordinates relative to the viewport associated with the event.


  .. c:member:: EM_BOOL ctrlKey
    EM_BOOL shiftKey
    EM_BOOL altKey
    EM_BOOL metaKey

    Specifies which modifiers were active during the mouse event.


  .. c:member:: unsigned short button

    Identifies which pointer device button changed state (see `MouseEvent.button <https://developer.mozilla.org/en-US/docs/Web/API/MouseEvent.button>`_):

      - 0 : Left button
      - 1 : Middle button (if present)
      - 2 : Right button


  .. c:member:: unsigned short buttons

    A bitmask that indicates which combinations of mouse buttons were being held down at the time of the event.

  .. c:member:: long movementX
    long movementY;

    If pointer lock is active, these two extra fields give relative mouse movement since the last event.

  .. c:member:: long targetX
     long targetY

    These fields give the mouse coordinates mapped relative to the coordinate space of the target DOM element receiving the input events (Emscripten-specific extension).


  .. c:member:: long canvasX
     long canvasY

    These fields give the mouse coordinates mapped to the Emscripten canvas client area (Emscripten-specific extension).


  .. c:member:: long padding

    Internal, and can be ignored.

    .. note:: Implementers only: pad this struct to multiple of 8 bytes to make ``WheelEvent`` unambiguously align to 8 bytes.


Callback functions
------------------

.. c:type:: em_mouse_callback_func

  Function pointer for the :c:func:`mouse event callback functions <emscripten_set_click_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_mouse_callback_func)(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);

  :param int eventType: The type of :c:data:`mouse event <EMSCRIPTEN_EVENT_CLICK>`.
  :param mouseEvent: Information about the mouse event that occurred.
  :type mouseEvent: const EmscriptenMouseEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_click_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_mousedown_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_mouseup_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_dblclick_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_mousemove_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_mouseenter_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_mouseleave_callback(const char *target, void *userData, EM_BOOL useCapture, em_mouse_callback_func callback)

  Registers a callback function for receiving browser-generated `mouse input events <https://developer.mozilla.org/en/DOM/MouseEvent>`_.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_mouse_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



.. c:function:: EMSCRIPTEN_RESULT emscripten_get_mouse_status(EmscriptenMouseEvent *mouseState)

  Returns the most recently received mouse event state.

  Note that for this function call to succeed, :c:func:`emscripten_set_xxx_callback <emscripten_set_click_callback>` must have first been called with one of the mouse event types and a non-zero callback function pointer to enable the Mouse state capture.

  :param EmscriptenMouseEvent* mouseState: The most recently received mouse event state.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Wheel
=====

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_WHEEL

    Emscripten wheel event.

.. c:macro:: DOM_DELTA_PIXEL

  The units of measurement for the delta must be pixels (from `spec <http://www.w3.org/TR/DOM-Level-3-Events/#constants-1)>`_).

.. c:macro:: DOM_DELTA_LINE

  The units of measurement for the delta must be individual lines of text (from `spec <http://www.w3.org/TR/DOM-Level-3-Events/#constants-1)>`_).

.. c:macro:: DOM_DELTA_PAGE

  The units of measurement for the delta must be pages, either defined as a single screen or as a demarcated page (from `spec <http://www.w3.org/TR/DOM-Level-3-Events/#constants-1)>`_).


Struct
------

.. c:type:: EmscriptenWheelEvent

  The event structure passed in `mousewheel events <http://www.w3.org/TR/DOM-Level-3-Events/#event-type-wheel>`_.

  .. c:member:: EmscriptenMouseEvent mouse

    Specifies general mouse information related to this event.

  .. c:member:: double deltaX
    double deltaY
    double deltaZ

    Movement of the wheel on each of the axis. Note that these values may be fractional, so you should avoid simply casting them to integer, or it might result
    in scroll values of 0. The positive Y scroll direction is when scrolling the page downwards (page CSS pixel +Y direction), which corresponds to scrolling
    the mouse wheel downwards (away from the screen) on Windows, Linux, and also on macOS when the 'natural scroll' option is disabled.

  .. c:member:: unsigned long deltaMode

    One of the :c:data:`DOM_DELTA_<DOM_DELTA_PIXEL>` values that indicates the units of measurement for the delta values.


Callback functions
------------------

.. c:type:: em_wheel_callback_func

  Function pointer for the :c:func:`wheel event callback functions <emscripten_set_wheel_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_wheel_callback_func)(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData);

  :param int eventType: The type of wheel event (:c:data:`EMSCRIPTEN_EVENT_WHEEL`).
  :param wheelEvent: Information about the wheel event that occurred.
  :type wheelEvent: const EmscriptenWheelEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_wheel_callback(const char *target, void *userData, EM_BOOL useCapture, em_wheel_callback_func callback)

  Registers a callback function for receiving browser-generated `mousewheel events <http://www.w3.org/TR/DOM-Level-3-Events/#event-type-wheel>`_.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_wheel_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



UI
==

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_RESIZE
  EMSCRIPTEN_EVENT_SCROLL

    Emscripten UI events.


Struct
------

.. c:type:: EmscriptenUiEvent

  The event structure passed in DOM element `UIEvent <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#interface-UIEvent>`_ events: `resize <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-resize>`_ and `scroll <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-scroll>`_.


  .. c:member:: long detail

    Specifies additional detail/information about this event.

  .. c:member:: int documentBodyClientWidth
    int documentBodyClientHeight

    The clientWidth/clientHeight of the ``document.body`` element.

  .. c:member:: int windowInnerWidth
    int windowInnerHeight

    The innerWidth/innerHeight of the browser window.

  .. c:member:: int windowOuterWidth
    int windowOuterHeight;

    The outerWidth/outerHeight of the browser window.

  .. c:member:: int scrollTop
    int scrollLeft

    The page scroll position.


Callback functions
------------------

.. c:type:: em_ui_callback_func

  Function pointer for the :c:func:`UI event callback functions <emscripten_set_resize_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_ui_callback_func)(int eventType, const EmscriptenUiEvent *uiEvent, void *userData);

  :param int eventType: The type of UI event (:c:data:`EMSCRIPTEN_EVENT_RESIZE`).
  :param uiEvent: Information about the UI event that occurred.
  :type uiEvent: const EmscriptenUiEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|


Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_resize_callback(const char *target, void *userData, EM_BOOL useCapture, em_ui_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_scroll_callback(const char *target, void *userData, EM_BOOL useCapture, em_ui_callback_func callback)

  Registers a callback function for receiving DOM element `resize <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-resize>`_ and `scroll <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-scroll>`_ events.

  .. note::

    - For the ``resize`` callback, pass in target = 0 to get ``resize`` events from the ``Window`` object.
    - The DOM3 Events specification only requires that the ``Window`` object sends resize events. It is valid to register a ``resize`` callback on other DOM elements, but the browser is not required to fire ``resize`` events for these.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_ui_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|




Focus
=====

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_BLUR
  EMSCRIPTEN_EVENT_FOCUS
  EMSCRIPTEN_EVENT_FOCUSIN
  EMSCRIPTEN_EVENT_FOCUSOUT

    Emscripten focus events.


Struct
------

.. c:type:: EmscriptenFocusEvent

  The event structure passed in DOM element `blur <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-blur>`_, `focus <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-focus>`_, `focusin <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-focusin>`_ and `focusout <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-focusout>`_ events.

  .. c:member:: EM_UTF8 nodeName

    The `nodeName <https://developer.mozilla.org/en-US/docs/Web/API/Node.nodeName>`_ of the target HTML Element.

    Maximum size 128 ``char`` (i.e. ``EM_UTF8 nodeName[128]``).

  .. c:member:: EM_UTF8 id

    The ID of the target element.

    Maximum size 128 ``char`` (i.e. ``EM_UTF8 id[128]``).



Callback functions
------------------

.. c:type:: em_focus_callback_func

  Function pointer for the :c:func:`focus event callback functions <emscripten_set_blur_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_focus_callback_func)(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData);

  :param int eventType: The type of focus event (:c:data:`EMSCRIPTEN_EVENT_BLUR`).
  :param focusEvent: Information about the focus event that occurred.
  :type focusEvent: const EmscriptenFocusEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_blur_callback(const char *target, void *userData, EM_BOOL useCapture, em_focus_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_focus_callback(const char *target, void *userData, EM_BOOL useCapture, em_focus_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_focusin_callback(const char *target, void *userData, EM_BOOL useCapture, em_focus_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_focusout_callback(const char *target, void *userData, EM_BOOL useCapture, em_focus_callback_func callback)

  Registers a callback function for receiving DOM element `blur <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-blur>`_, `focus <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-focus>`_, `focusin <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-focusin>`_ and `focusout <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-focusout>`_ events.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_focus_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Device orientation
==================

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_DEVICEORIENTATION

    Emscripten ``deviceorientation`` events.

Struct
------

.. c:type:: EmscriptenDeviceOrientationEvent

  The event structure passed in the `deviceorientation <http://dev.w3.org/geo/api/spec-source-orientation.html#deviceorientation>`_ event.


  .. c:member:: double timestamp

    Absolute wallclock time when the event occurred (in milliseconds).

  .. c:member:: double alpha
    double beta
    double gamma

    The `orientation <https://developer.mozilla.org/en-US/Apps/Build/gather_and_modify_data/responding_to_device_orientation_changes#Device_Orientation_API>`_ of the device in terms of the transformation from a coordinate frame fixed on the Earth to a coordinate frame fixed in the device.

    The image (source: `dev.opera.com <http://dev.opera.com/articles/view/w3c-device-orientation-api/>`_) and definitions below illustrate the co-ordinate frame:

      - :c:type:`~EmscriptenDeviceOrientationEvent.alpha`: the rotation of the device around the Z axis.
      - :c:type:`~EmscriptenDeviceOrientationEvent.beta`: the rotation of the device around the X axis.
      - :c:type:`~EmscriptenDeviceOrientationEvent.gamma`: the rotation of the device around the Y axis.

    .. image:: device-orientation-axes.png
      :target: https://developer.mozilla.org/en-US/Apps/Build/gather_and_modify_data/responding_to_device_orientation_changes#Device_Orientation_API
      :alt: Image of device showing X, Y, Z axes


  .. c:member:: EM_BOOL absolute

    If ``false``, the orientation is only relative to some other base orientation, not to the fixed coordinate frame.


Callback functions
------------------

.. c:type:: em_deviceorientation_callback_func

  Function pointer for the :c:func:`orientation event callback functions <emscripten_set_deviceorientation_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_deviceorientation_callback_func)(int eventType, const EmscriptenDeviceOrientationEvent *deviceOrientationEvent, void *userData);

  :param int eventType: The type of orientation event (:c:data:`EMSCRIPTEN_EVENT_DEVICEORIENTATION`).
  :param deviceOrientationEvent: Information about the orientation event that occurred.
  :type deviceOrientationEvent: const EmscriptenDeviceOrientationEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_deviceorientation_callback(void *userData, EM_BOOL useCapture, em_deviceorientation_callback_func callback)

  Registers a callback function for receiving the `deviceorientation <http://dev.w3.org/geo/api/spec-source-orientation.html#deviceorientation>`_ event.

  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_deviceorientation_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_get_deviceorientation_status(EmscriptenDeviceOrientationEvent *orientationState)

  Returns the most recently received ``deviceorientation`` event state.

  Note that for this function call to succeed, :c:func:`emscripten_set_deviceorientation_callback` must have first been called with one of the mouse event types and a non-zero callback function pointer to enable the ``deviceorientation`` state capture.

  :param orientationState: The most recently received ``deviceorientation`` event state.
  :type orientationState: EmscriptenDeviceOrientationEvent*
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Device motion
=============

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_DEVICEMOTION

    Emscripten `devicemotion <http://w3c.github.io/deviceorientation/spec-source-orientation.html#devicemotion>`_ event.


Struct
------

.. c:type:: EmscriptenDeviceMotionEvent

  The event structure passed in the `devicemotion <http://w3c.github.io/deviceorientation/spec-source-orientation.html#devicemotion>`_ event.

  .. c:member:: double timestamp

    Absolute wallclock time when the event occurred (milliseconds).


  .. c:member:: double accelerationX
    double accelerationY
    double accelerationZ

    Acceleration of the device excluding gravity.


  .. c:member:: double accelerationIncludingGravityX
    double accelerationIncludingGravityY
    double accelerationIncludingGravityZ

    Acceleration of the device including gravity.


  .. c:member:: double rotationRateAlpha
    double rotationRateBeta
    double rotationRateGamma

    The rotational delta of the device.


Callback functions
------------------

.. c:type:: em_devicemotion_callback_func

  Function pointer for the :c:func:`devicemotion event callback functions <emscripten_set_devicemotion_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_devicemotion_callback_func)(int eventType, const EmscriptenDeviceMotionEvent *deviceMotionEvent, void *userData);

  :param int eventType: The type of devicemotion event (:c:data:`EMSCRIPTEN_EVENT_DEVICEMOTION`).
  :param deviceMotionEvent: Information about the devicemotion event that occurred.
  :type deviceMotionEvent: const EmscriptenDeviceMotionEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|




Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_devicemotion_callback(void *userData, EM_BOOL useCapture, em_devicemotion_callback_func callback)

  Registers a callback function for receiving the `devicemotion <http://w3c.github.io/deviceorientation/spec-source-orientation.html#devicemotion>`_ event.

  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_devicemotion_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_get_devicemotion_status(EmscriptenDeviceMotionEvent *motionState)

  Returns the most recently received `devicemotion <http://w3c.github.io/deviceorientation/spec-source-orientation.html#devicemotion>`_ event state.

  Note that for this function call to succeed, :c:func:`emscripten_set_devicemotion_callback` must have first been called with one of the mouse event types and a non-zero callback function pointer to enable the ``devicemotion`` state capture.

  :param motionState: The most recently received ``devicemotion`` event state.
  :type motionState: EmscriptenDeviceMotionEvent*
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Orientation
===========

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_ORIENTATIONCHANGE

    Emscripten `orientationchange <https://w3c.github.io/screen-orientation/>`_ event.


.. c:macro:: EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY

  Primary portrait mode orientation.

.. c:macro:: EMSCRIPTEN_ORIENTATION_PORTRAIT_SECONDARY

  Secondary portrait mode orientation.

.. c:macro:: EMSCRIPTEN_ORIENTATION_LANDSCAPE_PRIMARY

  Primary landscape mode orientation.

.. c:macro:: EMSCRIPTEN_ORIENTATION_LANDSCAPE_SECONDARY

  Secondary landscape mode orientation.


Struct
------

.. c:type:: EmscriptenOrientationChangeEvent

  The event structure passed in the `orientationchange <https://w3c.github.io/screen-orientation/>`_ event.


  .. c:member:: int orientationIndex

    One of the :c:type:`EM_ORIENTATION_PORTRAIT_xxx <EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY>` fields, or -1 if unknown.

  .. c:member:: int orientationAngle

    Emscripten-specific extension: Some browsers refer to ``window.orientation``, so report that as well.

    Orientation angle in degrees. 0: "default orientation", i.e. default upright orientation to hold the mobile device in. Could be either landscape or portrait.


Callback functions
------------------

.. c:type:: em_orientationchange_callback_func

  Function pointer for the :c:func:`orientationchange event callback functions <emscripten_set_orientationchange_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_orientationchange_callback_func)(int eventType, const EmscriptenOrientationChangeEvent *orientationChangeEvent, void *userData);

  :param int eventType: The type of orientationchange event (:c:data:`EMSCRIPTEN_EVENT_ORIENTATIONCHANGE`).
  :param orientationChangeEvent: Information about the orientationchange event that occurred.
  :type orientationChangeEvent: const EmscriptenOrientationChangeEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|


Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_orientationchange_callback(void *userData, EM_BOOL useCapture, em_orientationchange_callback_func callback)

  Registers a callback function for receiving the `orientationchange <https://w3c.github.io/screen-orientation/>`_ event.

  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_orientationchange_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_get_orientation_status(EmscriptenOrientationChangeEvent *orientationStatus)

  Returns the current device orientation state.

  :param orientationStatus: The most recently received orientation state.
  :type orientationStatus: EmscriptenOrientationChangeEvent*
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_lock_orientation(int allowedOrientations)

  Locks the screen orientation to the given set of :c:data:`allowed orientations <EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY>`.

  :param int allowedOrientations: A bitfield set of :c:data:`EMSCRIPTEN_ORIENTATION_xxx <EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY>` flags.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_unlock_orientation(void)

  Removes the orientation lock so the screen can turn to any orientation.

  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Fullscreen
==========

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_FULLSCREENCHANGE

    Emscripten `fullscreenchange <https://dvcs.w3.org/hg/fullscreen/raw-file/tip/Overview.html>`_ event.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_SCALE

  An enum-like type which specifies how the Emscripten runtime should treat the CSS size of the target element when displaying it in fullscreen mode via calls to functions
  :c:func:`emscripten_request_fullscreen_strategy` and :c:func:`emscripten_enter_soft_fullscreen`.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT

    Specifies that the DOM element should not be resized by Emscripten runtime when transitioning between fullscreen and windowed modes. The browser will be responsible for
    scaling the DOM element to the fullscreen size. The proper browser behavior in this mode is to stretch the element to fit the full display ignoring aspect ratio, but at the
    time of writing, browsers implement different behavior here. See the discussion at https://github.com/emscripten-core/emscripten/issues/2556 for more information.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH

  Specifies that the Emscripten runtime should explicitly stretch the CSS size of the target element to cover the whole screen when transitioning to fullscreen mode. This
  will change the aspect ratio of the displayed content.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT

  Specifies that the Emscripten runtime should explicitly scale the CSS size of the target element to cover the whole screen, while adding either vertical or horizontal
  black letterbox padding to preserve the aspect ratio of the content. The aspect ratio that is used here is the render target size of the canvas element. To change the
  desired aspect ratio, call :c:func:`emscripten_set_canvas_element_size` before entering fullscreen mode.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE

  An enum-like type which specifies how the Emscripten runtime should treat the pixel size (render target resolution) of the target canvas element when displaying it in
  fullscreen mode via calls to functions :c:func:`emscripten_request_fullscreen_strategy` and :c:func:`emscripten_enter_soft_fullscreen`. To better understand the
  underlying distinction between the CSS size of a canvas element versus the render target size of a canvas element, see https://www.khronos.org/webgl/wiki/HandlingHighDPI.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE

  Specifies that the Emscripten runtime should not do any changes to the render target resolution of the target canvas element that is displayed in fullscreen mode. Use
  this mode when your application is set up to render to a single fixed resolution that cannot be changed under any condition.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF

  Specifies that the Emscripten runtime should resize the render target of the canvas element to match 1:1 with the CSS size of the element in fullscreen mode. On high DPI
  displays (`window.devicePixelRatio` > 1), the CSS size is not the same as the physical screen resolution of the device. Call :c:func:`emscripten_get_device_pixel_ratio`
  to obtain the pixel ratio between CSS pixels and actual device pixels of the screen. Use this mode when you want to render to a pixel resolution that is DPI-independent.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF

  Specifies that the Emscripten runtime should resize the canvas render target size to match 1:1 with the physical screen resolution on the device. This corresponds to high
  definition displays on retina iOS and other mobile and desktop devices with high DPI. Use this mode to match and render 1:1 to the native display resolution.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_FILTERING

  An enum-like type that specifies what kind of image filtering algorithm to apply to the element when it is presented in fullscreen mode.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT

  Specifies that the image filtering mode should not be changed from the existing setting in the CSS style.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST

  Applies a CSS style to the element that displays the content using a nearest-neighbor image filtering algorithm in fullscreen mode.

.. c:macro:: EMSCRIPTEN_FULLSCREEN_FILTERING_BILINEAR

  Applies a CSS style to the element that displays the content using a bilinear image filtering algorithm in fullscreen mode. This is the default browser behavior.

Struct
------

.. c:type:: EmscriptenFullscreenChangeEvent

  The event structure passed in the `fullscreenchange <https://dvcs.w3.org/hg/fullscreen/raw-file/tip/Overview.html>`_ event.

  .. c:member:: EM_BOOL isFullscreen

    Specifies whether an element on the browser page is currently fullscreen.


  .. c:member:: EM_BOOL fullscreenEnabled

    Specifies if the current page has the ability to display elements fullscreen.

  .. c:member:: EM_UTF8 nodeName

    The `nodeName <https://developer.mozilla.org/en-US/docs/Web/API/Node.nodeName>`_ of the target HTML Element that is in full screen mode.

    Maximum size 128 ``char`` (i.e. ``EM_UTF8 nodeName[128]``).

    If ``isFullscreen`` is ``false``, then ``nodeName``, ``id`` and ``elementWidth`` and ``elementHeight`` specify information about the element that just exited fullscreen mode.


  .. c:member:: EM_UTF8 id

    The ID of the target HTML element that is in full screen mode.

    Maximum size 128 ``char`` (i.e. ``EM_UTF8 id[128]``).


  .. c:member:: int elementWidth
    int elementHeight

    The new pixel size of the element that changed fullscreen status.


  .. c:member:: int screenWidth
    int screenHeight

    The size of the whole screen, in pixels.

.. c:type:: EmscriptenFullscreenStrategy

  The options structure that is passed in to functions :c:func:`emscripten_request_fullscreen_strategy` and :c:func:`emscripten_enter_soft_fullscreen` to configure how the target
  element should be displayed in fullscreen mode.

  .. c:member:: EMSCRIPTEN_FULLSCREEN_SCALE scaleMode

    Specifies the rule how the CSS size (the displayed size) of the target element is resized when displayed in fullscreen mode.

  .. c:member:: EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE canvasResolutionScaleMode

    Specifies how the render target size (the pixel resolution) of the target element is adjusted when displayed in fullscreen mode.

  .. c:member:: EMSCRIPTEN_FULLSCREEN_FILTERING filteringMode

    Specifies the image filtering algorithm to apply to the element in fullscreen mode.

  .. c:member:: em_canvasresized_callback_func canvasResizedCallback

    If nonzero, points to a user-provided callback function which will be called whenever either the CSS or the canvas render target size changes. Use this callback to reliably
    obtain information about canvas resize events.

  .. c:member:: void *canvasResizedCallbackUserData

    Stores a custom data field which will be passed to all calls to the user-provided callback function.

Callback functions
------------------

.. c:type:: em_fullscreenchange_callback_func

  Function pointer for the :c:func:`fullscreen event callback functions <emscripten_set_fullscreenchange_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_fullscreenchange_callback_func)(int eventType, const EmscriptenFullscreenChangeEvent *fullscreenChangeEvent, void *userData);

  :param int eventType: The type of fullscreen event (:c:data:`EMSCRIPTEN_EVENT_FULLSCREENCHANGE`).
  :param fullscreenChangeEvent: Information about the fullscreen event that occurred.
  :type fullscreenChangeEvent: const EmscriptenFullscreenChangeEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_fullscreenchange_callback(const char *target, void *userData, EM_BOOL useCapture, em_fullscreenchange_callback_func callback)

  Registers a callback function for receiving the `fullscreenchange <https://dvcs.w3.org/hg/fullscreen/raw-file/tip/Overview.html>`_ event.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_fullscreenchange_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_get_fullscreen_status(EmscriptenFullscreenChangeEvent *fullscreenStatus)

  Returns the current page `fullscreen <https://dvcs.w3.org/hg/fullscreen/raw-file/tip/Overview.html>`_ state.

  :param fullscreenStatus: The most recently received fullscreen state.
  :type fullscreenStatus: EmscriptenFullscreenChangeEvent*
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_request_fullscreen(const char *target, EM_BOOL deferUntilInEventHandler)

  Requests the given target element to transition to full screen mode.

  .. note:: This function can be called anywhere, but for web security reasons its associated *request* can only be raised inside the event handler for a user-generated event (for example a key, mouse or touch press/release). This has implications for porting and the value of ``deferUntilInEventHandler``  — see :ref:`web-security-functions-html5-api` for more information.

  .. note:: This function only performs a fullscreen request without changing any parameters of the DOM element that is to be displayed in fullscreen mode. At the time of writing, there are differences in how browsers present elements in fullscreen mode. For more information, read the discussion at https://github.com/emscripten-core/emscripten/issues/2556. To display an element in fullscreen mode in a way that is consistent across browsers, prefer calling the function :c:func:`emscripten_request_fullscreen_strategy` instead. This function is best called only in scenarios where the preconfigured presets defined by :c:func:`emscripten_request_fullscreen_strategy` conflict with the developer's use case in some way.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param EM_BOOL deferUntilInEventHandler: If ``true`` requests made outside of a user-generated event handler are automatically deferred until the user next presses a keyboard or mouse button. If ``false`` the request will fail if called outside of a user-generated event handler.

  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: **EMSCRIPTEN_RESULT**

.. c:function:: EMSCRIPTEN_RESULT emscripten_request_fullscreen_strategy(const char *target, EM_BOOL deferUntilInEventHandler, const EmscriptenFullscreenStrategy *fullscreenStrategy)

  Requests the given target element to transition to full screen mode, using a custom presentation mode for the element. This function is otherwise the same as :c:func:`emscripten_request_fullscreen`, but this function adds options to control how resizing and aspect ratio, and ensures that the behavior is consistent across browsers.

  .. note:: This function makes changes to the DOM to satisfy consistent presentation across browsers. These changes have been designed to intrude as little as possible, and the changes are cleared once windowed browsing is restored. If any of these changes are conflicting, see the function :c:func:`emscripten_request_fullscreen` instead, which performs a bare fullscreen request without any modifications to the DOM.

  :param fullscreenStrategy: [in] Points to a configuration structure filled by the caller which specifies display options for the fullscreen mode.
  :type fullscreenStrategy: const EmscriptenFullscreenStrategy*

.. c:function:: EMSCRIPTEN_RESULT emscripten_exit_fullscreen(void)

  Returns back to windowed browsing mode from a proper fullscreen mode.

  Do not call this function to attempt to return to windowed browsing mode from a soft fullscreen mode, or vice versa.

  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|

.. c:function:: EMSCRIPTEN_RESULT emscripten_enter_soft_fullscreen(const char *target, const EmscriptenFullscreenStrategy *fullscreenStrategy)

  Enters a "soft" fullscreen mode, where the given target element is displayed in the whole client area of the page and all other elements are hidden, but does not actually request fullscreen mode for the browser. This function is useful in cases where the actual Fullscreen API is not desirable or needed, for example in packaged apps for Firefox OS, where applications essentially already cover the whole screen.

  Pressing the esc button does not automatically exit the soft fullscreen mode. To return to windowed presentation mode, manually call the function :c:func:`emscripten_exit_soft_fullscreen`.

.. c:function:: EMSCRIPTEN_RESULT emscripten_exit_soft_fullscreen()

  Returns back to windowed browsing mode from a soft fullscreen mode. Do not call this function to attempt to return to windowed browsing mode from a real fullscreen mode, or vice versa.

Pointerlock
===========

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_POINTERLOCKCHANGE

    Emscripten `pointerlockchange <http://www.w3.org/TR/pointerlock/#pointerlockchange-and-pointerlockerror-events>`_ event.

.. c:macro:: EMSCRIPTEN_EVENT_POINTERLOCKERROR

    Emscripten `pointerlockerror <http://www.w3.org/TR/pointerlock/#pointerlockchange-and-pointerlockerror-events>`_ event.

Struct
------

.. c:type:: EmscriptenPointerlockChangeEvent

  The event structure passed in the `pointerlockchange <http://www.w3.org/TR/pointerlock/#pointerlockchange-and-pointerlockerror-events>`_ event.


  .. c:member:: EM_BOOL isActive

    Specifies whether an element on the browser page currently has pointer lock enabled.

  .. c:member:: EM_UTF8 nodeName

    The `nodeName <https://developer.mozilla.org/en-US/docs/Web/API/Node.nodeName>`_ of the target HTML Element that has the pointer lock active.

    Maximum size 128 ``char`` (i.e. ``EM_UTF8 nodeName[128]``).

  .. c:member:: EM_UTF8 id

    The ID of the target HTML element that has the pointer lock active.

    Maximum size 128 ``char`` (i.e. ``EM_UTF8 id[128]``).


Callback functions
------------------

.. c:type:: em_pointerlockchange_callback_func

  Function pointer for the :c:func:`pointerlockchange event callback functions <emscripten_set_pointerlockchange_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_pointerlockchange_callback_func)(int eventType, const EmscriptenPointerlockChangeEvent *pointerlockChangeEvent, void *userData);

  :param int eventType: The type of pointerlockchange event (:c:data:`EMSCRIPTEN_EVENT_POINTERLOCKCHANGE`).
  :param pointerlockChangeEvent: Information about the pointerlockchange event that occurred.
  :type pointerlockChangeEvent: const EmscriptenPointerlockChangeEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|

.. c:type:: em_pointerlockerror_callback_func

  Function pointer for the :c:func:`pointerlockerror event callback functions <emscripten_set_pointerlockerror_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_pointerlockerror_callback_func)(int eventType, const void *reserved, void *userData);

  :param int eventType: The type of pointerlockerror event (:c:data:`EMSCRIPTEN_EVENT_POINTERLOCKERROR`).
  :param const void* reserved: Reserved for future use; pass in 0.
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_pointerlockchange_callback(const char *target, void *userData, EM_BOOL useCapture, em_pointerlockchange_callback_func callback)

  Registers a callback function for receiving the `pointerlockchange <http://www.w3.org/TR/pointerlock/#pointerlockchange-and-pointerlockerror-events>`_ event.

  Pointer lock hides the mouse cursor and exclusively gives the target element relative mouse movement events via the `mousemove <https://dvcs.w3.org/hg/dom3events/raw-file/tip/html/DOM3-Events.html#event-type-mousemove>`_ event.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_pointerlockchange_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



.. c:function:: EMSCRIPTEN_RESULT emscripten_set_pointerlockerror_callback(const char *target, void *userData, EM_BOOL useCapture, em_pointerlockerror_callback_func callback)

  Registers a callback function for receiving the `pointerlockerror <http://www.w3.org/TR/pointerlock/#pointerlockchange-and-pointerlockerror-events>`_ event.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_pointerlockerror_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



.. c:function:: EMSCRIPTEN_RESULT emscripten_get_pointerlock_status(EmscriptenPointerlockChangeEvent *pointerlockStatus)

  Returns the current page pointerlock state.

  :param EmscriptenPointerlockChangeEvent* pointerlockStatus: The most recently received pointerlock state.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_request_pointerlock(const char *target, EM_BOOL deferUntilInEventHandler)

  Requests the given target element to grab pointerlock.

  .. note:: This function can be called anywhere, but for web security reasons its associated *request* can only be raised inside the event handler for a user-generated event (for example a key, mouse or touch press/release). This has implications for porting and the value of ``deferUntilInEventHandler``  — see :ref:`web-security-functions-html5-api` for more information.


  :param target: |target-parameter-doc|
  :type target: const char*
  :param EM_BOOL deferUntilInEventHandler: If ``true`` requests made outside of a user-generated event handler are automatically deferred until the user next presses a keyboard or mouse button. If ``false`` the request will fail if called outside of a user-generated event handler.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_exit_pointerlock(void)

  Exits pointer lock state and restores the mouse cursor to be visible again.

  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Visibility
==========

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_VISIBILITYCHANGE

  Emscripten `visibilitychange <http://www.w3.org/TR/page-visibility>`__ event.

.. c:macro:: EMSCRIPTEN_VISIBILITY_HIDDEN

  The document is `hidden <http://www.w3.org/TR/page-visibility/#pv-page-hidden>`_ (not visible).

.. c:macro:: EMSCRIPTEN_VISIBILITY_VISIBLE

  The document is at least partially `visible <http://www.w3.org/TR/page-visibility/#pv-page-visible>`_.

.. c:macro:: EMSCRIPTEN_VISIBILITY_PRERENDER

  The document is loaded off screen and not visible (`prerender <http://www.w3.org/TR/page-visibility/#pv-prerender>`_).

.. c:macro:: EMSCRIPTEN_VISIBILITY_UNLOADED

  The document is to be `unloaded <http://www.w3.org/TR/page-visibility/#pv-unloaded>`_.


Struct
------

.. c:type:: EmscriptenVisibilityChangeEvent

  The event structure passed in the `visibilitychange <http://www.w3.org/TR/page-visibility/>`__ event.

  .. c:member:: EM_BOOL hidden

    If true, the current browser page is now hidden.


  .. c:member:: int visibilityState

    Specifies a more fine-grained state of the current page visibility status. One of the :c:type:`EMSCRIPTEN_VISIBILITY_ <EMSCRIPTEN_VISIBILITY_HIDDEN>` values.


Callback functions
------------------

.. c:type:: em_visibilitychange_callback_func

  Function pointer for the :c:func:`visibilitychange event callback functions <emscripten_set_visibilitychange_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_visibilitychange_callback_func)(int eventType, const EmscriptenVisibilityChangeEvent *visibilityChangeEvent, void *userData);

  :param int eventType: The type of ``visibilitychange`` event (:c:data:`EMSCRIPTEN_VISIBILITY_HIDDEN`).
  :param visibilityChangeEvent: Information about the ``visibilitychange`` event that occurred.
  :type visibilityChangeEvent: const EmscriptenVisibilityChangeEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|


Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_visibilitychange_callback(void *userData, EM_BOOL useCapture, em_visibilitychange_callback_func callback)

  Registers a callback function for receiving the `visibilitychange <http://www.w3.org/TR/page-visibility/>`_ event.

  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_visibilitychange_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_get_visibility_status(EmscriptenVisibilityChangeEvent *visibilityStatus)

  Returns the current page visibility state.

  :param EmscriptenVisibilityChangeEvent* visibilityStatus: The most recently received page visibility state.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Touch
=====

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_TOUCHSTART
  EMSCRIPTEN_EVENT_TOUCHEND
  EMSCRIPTEN_EVENT_TOUCHMOVE
  EMSCRIPTEN_EVENT_TOUCHCANCEL

    Emscripten touch events.


Struct
------

.. c:type:: EmscriptenTouchPoint

  Specifies the status of a single `touch point <http://www.w3.org/TR/touch-events/#touch-interface>`_ on the page.

  .. c:member:: long identifier

    An identification number for each touch point.

  .. c:member:: long screenX
    long screenY

    The touch coordinate relative to the whole screen origin, in pixels.

  .. c:member:: long clientX
    long clientY

    The touch coordinate relative to the viewport, in pixels.

  .. c:member:: long pageX
    long pageY

    The touch coordinate relative to the viewport, in pixels, and including any scroll offset.

  .. c:member:: EM_BOOL isChanged

    Specifies whether the touch point changed during this event.

  .. c:member:: EM_BOOL onTarget

    Specifies whether this touch point is still above the original target on which it was initially pressed.

  .. c:member:: long targetX
     long targetY

    These fields give the touch coordinates mapped relative to the coordinate space of the target DOM element receiving the input events (Emscripten-specific extension).

  .. c:member:: long canvasX
    long canvasY

    The touch coordinates mapped to the Emscripten canvas client area, in pixels (Emscripten-specific extension).



.. c:type:: EmscriptenTouchEvent

  Specifies the data of a single `touchevent <http://www.w3.org/TR/touch-events/#touchevent-interface>`_.

  .. c:member:: int numTouches

    The number of valid elements in the touches array.


  .. c:member:: EM_BOOL ctrlKey
    EM_BOOL shiftKey
    EM_BOOL altKey
    EM_BOOL metaKey

    Specifies which modifiers were active during the touch event.

  .. c:member:: EmscriptenTouchPoint touches[32]

    An array of currently active touches, one for each finger.



Callback functions
------------------


.. c:type:: em_touch_callback_func

  Function pointer for the :c:func:`touch event callback functions <emscripten_set_touchstart_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_touch_callback_func)(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);

  :param int eventType: The type of touch event (:c:data:`EMSCRIPTEN_EVENT_TOUCHSTART`).
  :param touchEvent: Information about the touch event that occurred.
  :type touchEvent: const EmscriptenTouchEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_touchstart_callback(const char *target, void *userData, EM_BOOL useCapture, em_touch_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_touchend_callback(const char *target, void *userData, EM_BOOL useCapture, em_touch_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_touchmove_callback(const char *target, void *userData, EM_BOOL useCapture, em_touch_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_touchcancel_callback(const char *target, void *userData, EM_BOOL useCapture, em_touch_callback_func callback)

  Registers a callback function for receiving `touch events <http://www.w3.org/TR/touch-events/)>`__ : `touchstart <http://www.w3.org/TR/touch-events/#the-touchstart-event>`_, `touchend <http://www.w3.org/TR/touch-events/#dfn-touchend>`_, `touchmove <http://www.w3.org/TR/touch-events/#dfn-touchmove>`_ and `touchcancel <http://www.w3.org/TR/touch-events/#dfn-touchcancel>`_.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_touch_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Gamepad
=======

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_GAMEPADCONNECTED
  EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED

    Emscripten gamepad_ events.


Struct
------

.. c:type:: EmscriptenGamepadEvent

  Represents the current snapshot state of a gamepad_.


  .. c:member:: double timestamp

    Absolute wallclock time when the data was recorded (milliseconds).

  .. c:member:: int numAxes

    The number of valid axis entries in the ``axis`` array.

  .. c:member:: int numButtons

    The number of valid button entries in the analogButton and digitalButton arrays.

  .. c:member:: double axis[64]

    The analog state of the gamepad axes, in the range [-1, 1].


  .. c:member:: double analogButton[64]

    The analog state of the gamepad buttons, in the range [0, 1].


  .. c:member:: EM_BOOL digitalButton[64]

    The digital state of the gamepad buttons, either 0 or 1.

  .. c:member:: EM_BOOL connected

    Specifies whether this gamepad is connected to the browser page.

  .. c:member:: long index

    An ordinal associated with this gamepad, zero-based.

  .. c:member:: EM_UTF8 id

    An ID for the brand or style of the connected gamepad device. Typically, this will include the USB vendor and a product ID.

    Maximum size 64 ``char`` (i.e. ``EM_UTF8 id[128]``).

  .. c:member:: EM_UTF8 mapping

    A string that identifies the layout or control mapping of this device.

    Maximum size 128 ``char`` (i.e. ``EM_UTF8 mapping[128]``).



Callback functions
------------------

.. c:type:: em_gamepad_callback_func

  Function pointer for the :c:func:`gamepad event callback functions <emscripten_set_gamepadconnected_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_gamepad_callback_func)(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData)

  :param int eventType: The type of gamepad event (:c:data:`EMSCRIPTEN_EVENT_GAMEPADCONNECTED`).
  :param gamepadEvent: Information about the gamepad event that occurred.
  :type gamepadEvent: const EmscriptenGamepadEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_gamepadconnected_callback(void *userData, EM_BOOL useCapture, em_gamepad_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_gamepaddisconnected_callback(void *userData, EM_BOOL useCapture, em_gamepad_callback_func callback)

  Registers a callback function for receiving the gamepad_ events: `gamepadconnected <http://www.w3.org/TR/gamepad/#the-gamepadconnected-event>`_ and `gamepaddisconnected <http://www.w3.org/TR/gamepad/#the-gamepaddisconnected-event>`_.

  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_gamepad_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_sample_gamepad_data(void)

  This function samples a new state of connected Gamepad data, and returns either
  EMSCRIPTEN_RESULT_SUCCESS if Gamepad API is supported by the current browser,
  or EMSCRIPTEN_RESULT_NOT_SUPPORTED if Gamepad API is not supported. Note that
  even if EMSCRIPTEN_RESULT_SUCCESS is returned, there may not be any gamepads
  connected yet to the current browser tab.

  Call this function before calling either of the functions
  emscripten_get_num_gamepads() or emscripten_get_gamepad_status().

.. c:function:: int emscripten_get_num_gamepads(void)

  After having called emscripten_sample_gamepad_data(), this function
  returns the number of gamepads connected to the system or
  :c:type:`EMSCRIPTEN_RESULT_NOT_SUPPORTED` if the current browser does not
  support gamepads.

  .. note:: A gamepad does not show up as connected until a button on it is pressed.

  .. note::

     Gamepad API uses an array of gamepad state objects to return the state of
     each device. The devices are identified via the index they are present in in
     this array. Because of that, if one first connects gamepad A, then gamepad
     B, and then disconnects gamepad A, the gamepad B shall not take the place of
     gamepad A, so in this scenario, this function will still keep returning two
     for the count of connected gamepads, even though gamepad A is no longer
     present. To find the actual number of connected gamepads, listen for the
     gamepadconnected and gamepaddisconnected events.  Consider the return value
     of function emscripten_get_num_gamepads() minus one to be the largest index
     value that can be passed to the function emscripten_get_gamepad_status().

  :returns: The number of gamepads connected to the browser tab.
  :rtype: int


.. c:function:: EMSCRIPTEN_RESULT emscripten_get_gamepad_status(int index, EmscriptenGamepadEvent *gamepadState)

  After having called emscripten_sample_gamepad_data(), this function returns a
  snapshot of the current gamepad state for the gamepad controller located at
  given index of the controllers array.

  :param int index: The index of the gamepad to check (in the `array of connected gamepads <https://developer.mozilla.org/en-US/docs/Web/API/Navigator.getGamepads>`_).
  :param EmscriptenGamepadEvent* gamepadState: The most recently received gamepad state.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Battery
=======

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_BATTERYCHARGINGCHANGE
  EMSCRIPTEN_EVENT_BATTERYLEVELCHANGE

    Emscripten `batterymanager <http://www.w3.org/TR/battery-status/#batterymanager-interface>`_ events.


Struct
------

.. c:type:: EmscriptenBatteryEvent

  The event structure passed in the `batterymanager <http://www.w3.org/TR/battery-status/#batterymanager-interface>`_ events: ``chargingchange`` and ``levelchange``.


  .. c:member:: double chargingTime

    Time remaining until the battery is fully charged (seconds).

  .. c:member:: double dischargingTime

    Time remaining until the battery is empty and the system will be suspended (seconds).

  .. c:member:: double level

    Current battery level, on a scale of 0 to 1.0.

  .. c:member::  EM_BOOL charging;

    ``true`` if the battery is charging, ``false`` otherwise.


Callback functions
------------------

.. c:type:: em_battery_callback_func

  Function pointer for the :c:func:`batterymanager event callback functions <emscripten_set_batterychargingchange_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_battery_callback_func)(int eventType, const EmscriptenBatteryEvent *batteryEvent, void *userData);

  :param int eventType: The type of ``batterymanager`` event (:c:data:`EMSCRIPTEN_EVENT_BATTERYCHARGINGCHANGE`).
  :param batteryEvent: Information about the ``batterymanager`` event that occurred.
  :type batteryEvent: const EmscriptenBatteryEvent*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------

.. c:function:: EMSCRIPTEN_RESULT emscripten_set_batterychargingchange_callback(void *userData, em_battery_callback_func callback)
  EMSCRIPTEN_RESULT emscripten_set_batterylevelchange_callback(void *userData, em_battery_callback_func callback)

  Registers a callback function for receiving the `batterymanager <http://www.w3.org/TR/battery-status/#batterymanager-interface>`_ events: ``chargingchange`` and ``levelchange``.

  :param void* userData: |userData-parameter-doc|
  :param em_battery_callback_func callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_get_battery_status(EmscriptenBatteryEvent *batteryState)

  Returns the current battery status.

  :param batteryState: The most recently received battery state.
  :type batteryState: EmscriptenBatteryEvent*
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



Vibration
=========

Functions
---------


.. c:function:: EMSCRIPTEN_RESULT emscripten_vibrate(int msecs)

  Produces a `vibration <http://dev.w3.org/2009/dap/vibration/>`_ for the specified time, in milliseconds.

  :param int msecs: The amount of time for which the vibration is required (milliseconds).
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_vibrate_pattern(int *msecsArray, int numEntries)

  Produces a complex vibration feedback pattern.

  :param int* msecsArray: An array of timing entries [on, off, on, off, on, off, ...] where every second one specifies a duration of vibration, and every other one specifies a duration of silence.
  :param int numEntries: The number of integers in the array ``msecsArray``.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


Page unload
===========

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_BEFOREUNLOAD

    Emscripten `beforeunload <http://www.whatwg.org/specs/web-apps/current-work/multipage/history.html#beforeunloadevent>`_ event.


Callback functions
------------------

.. c:type:: em_beforeunload_callback

  Function pointer for the :c:func:`beforeunload event callback functions <emscripten_set_beforeunload_callback>`, defined as:

  .. code-block:: cpp

    typedef const char *(*em_beforeunload_callback)(int eventType, const void *reserved, void *userData);

  :param int eventType: The type of ``beforeunload`` event (:c:data:`EMSCRIPTEN_EVENT_BEFOREUNLOAD`).
  :param reserved: Reserved for future use; pass in 0.
  :type reserved: const void*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: Return a string to be displayed to the user.
  :rtype: char*



Functions
---------


.. c:function:: EMSCRIPTEN_RESULT emscripten_set_beforeunload_callback(void *userData, em_beforeunload_callback callback)

  Registers a callback function for receiving the page `beforeunload <http://www.whatwg.org/specs/web-apps/current-work/multipage/history.html#beforeunloadevent>`_ event.

  Hook into this event to perform actions immediately prior to page close (for example, to display a notification to ask if the user really wants to leave the page).

  :param void* userData: |userData-parameter-doc|
  :param em_beforeunload_callback callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



WebGL context
=============

Defines
-------

.. c:macro:: EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST
  EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED

    Emscripten `WebGL context`_ events.

.. c:type:: EMSCRIPTEN_WEBGL_CONTEXT_HANDLE

  Represents a handle to an Emscripten WebGL context object. The value 0 denotes an invalid/no context (this is a typedef to an ``int``).


Struct
------

.. c:type:: EmscriptenWebGLContextAttributes

  Specifies `WebGL context creation parameters <http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.2>`_.

  .. c:member:: EM_BOOL alpha

    If ``true``, request an alpha channel for the context. If you create an alpha channel, you can blend the canvas rendering with the underlying web page contents. Default value: ``true``.

  .. c:member:: EM_BOOL depth

    If ``true``, request a depth buffer of at least 16 bits. If ``false``, no depth buffer will be initialized. Default value: ``true``.

  .. c:member:: EM_BOOL stencil

    If ``true``, request a stencil buffer of at least 8 bits. If ``false``, no stencil buffer will be initialized. Default value: ``false``.

  .. c:member:: EM_BOOL antialias

    If ``true``, antialiasing will be initialized with a browser-specified algorithm and quality level. If ``false``, antialiasing is disabled. Default value: ``true``.


  .. c:member:: EM_BOOL premultipliedAlpha

    If ``true``, the alpha channel of the rendering context will be treated as representing premultiplied alpha values. If ``false``, the alpha channel represents non-premultiplied alpha. Default value: ``true``.


  .. c:member:: EM_BOOL preserveDrawingBuffer

    If ``true``, the contents of the drawing buffer are preserved between consecutive ``requestAnimationFrame()`` calls. If ``false``, color, depth and stencil are cleared at the beginning of each ``requestAnimationFrame()``. Generally setting this to ``false`` gives better performance. Default value: ``false``.


  .. c:member:: EM_WEBGL_POWER_PREFERENCE powerPreference

    Specifies a hint to the WebGL canvas implementation to how it should choose the use of available GPU resources. One of EM_WEBGL_POWER_PREFERENCE_DEFAULT, EM_WEBGL_POWER_PREFERENCE_LOW_POWER, EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE.

  .. c:member:: EM_BOOL failIfMajorPerformanceCaveat

    If ``true``, requests context creation to abort if the browser is only able to create a context that does not give good hardware-accelerated performance. Default value: ``false``.


  .. c:member:: int majorVersion
    int minorVersion

    Emscripten-specific extensions which specify the WebGL context version to initialize.

    For example, pass in ``majorVersion=1``, ``minorVersion=0`` to request a WebGL 1.0 context, and ``majorVersion=2``, ``minorVersion=0`` to request a WebGL 2.0 context.

    Default value: ``majorVersion=1``, ``minorVersion=0``


  .. c:member:: EM_BOOL enableExtensionsByDefault

    If ``true``, all GLES2-compatible non-performance-impacting WebGL extensions will automatically be enabled for you after the context has been created. If ``false``, no extensions are enabled by default, and you need to manually call :c:func:`emscripten_webgl_enable_extension` to enable each extension that you want to use. Default value: ``true``.


  .. c:member:: EM_BOOL explicitSwapControl

    By default, when ``explicitSwapControl`` is in its default state ``false``, rendered WebGL content is implicitly presented (displayed to the user) on the canvas when the event handler that renders with WebGL returns back to the browser event loop. If ``explicitSwapControl`` is set to ``true``, rendered content will not be displayed on screen automatically when event handler function finishes, but the control of swapping is given to the user to manage, via the ``emscripten_webgl_commit_frame()`` function.

    In order to be able to set ``explicitSwapControl==true``, support for it must explicitly be enabled either 1) via adding the ``-s OFFSCREEN_FRAMEBUFFER=1`` Emscripten linker flag, and enabling ``renderViaOffscreenBackBuffer==1``, or 2) via adding the the linker flag ``-s OFFSCREENCANVAS_SUPPORT=1``, and running in a browser that supports OffscreenCanvas.


  .. c:member:: EM_BOOL renderViaOffscreenBackBuffer

    If ``true``, an extra intermediate backbuffer (offscreen render target) is allocated to the created WebGL context, and rendering occurs to this backbuffer instead of directly onto the WebGL "default backbuffer". This is required to be enabled if 1) ``explicitSwapControl==true`` and the browser does not support OffscreenCanvas, 2) when performing WebGL rendering in a worker thread and the browser does not support OffscreenCanvas, and 3) when performing WebGL context accesses from multiple threads simultaneously (independent of whether OffscreenCanvas is supported or not).

    Because supporting offscreen framebuffer adds some amount of extra code to the compiled output, support for it must explicitly be enabled via the ``-s OFFSCREEN_FRAMEBUFFER=1`` Emscripten linker flag. When building simultaneously with both ``-s OFFSCREEN_FRAMEBUFFER=1`` and ``-s OFFSCREENCANVAS_SUPPORT=1`` linker flags enabled, offscreen backbuffer can be used as a polyfill-like compatibility fallback to enable rendering WebGL from a pthread when the browser does not support the OffscreenCanvas API.


  .. c:member:: EM_BOOL proxyContextToMainThread

    This member specifies the threading model that will be used for the created WebGL context, when the WebGL context is created in a pthread. Three values are possible: ``EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW``, ``EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK`` or ``EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS``. If ``EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW`` is specified, the WebGLRenderingContext object will be created inside the pthread that is calling the ``emscripten_webgl_create_context()`` function as an OffscreenCanvas-based rendering context. This is only possible if 1) current browser supports OffscreenCanvas specification, 2) code was compiled with ``-s OFFSCREENCANVAS_SUPPORT=1`` linker flag enabled, 3) the Canvas object that the context is being created on was transferred over to the calling pthread with function ``emscripten_pthread_attr_settransferredcanvases()`` when the pthread was originally created, and 4) no OffscreenCanvas-based context already exists from the given Canvas at the same time.

    If a WebGL rendering context is created as an OffscreenCanvas-based context, it will have the limitation that only the pthread that created the context can enable access to it (via ``emscripten_webgl_make_context_current()`` function). Other threads will not be able to activate rendering to the context, i.e. OffscreenCanvas-based contexts are essentially "pinned" to the pthread that created them.

    If the current browser does not support OffscreenCanvas, you can specify the ``EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS`` WebGL context creation flag. If this flag is passed, and code was compiled with ``-s OFFSCREEN_FRAMEBUFFER=1`` enabled, the WebGL context will be created as a "proxied context". In this context mode, the WebGLRenderingContext object will actually be created on the main browser thread, and all WebGL API calls will be proxied as asynchronous messages from the pthread into the main thread. This will have a performance and latency impact in comparison to OffscreenCanvas contexts, however unlike OffscreenCanvas-based contexts, proxied contexts can be shared across any number of pthreads: you can use the ``emscripten_webgl_make_context_current()`` function in any pthread to activate and deactivate access to the WebGL context: for example, you could have one WebGL loading thread, and another WebGL rendering thread that coordinate shared access to the WebGL rendering context by cooperatively acquiring and releasing access to the WebGL rendering context via the ``emscripten_webgl_make_context_current()`` function. Proxied contexts do not require any special support from the browser, so any WebGL capable browser can create a proxied WebGL context.

    The ``EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS`` WebGL context creation flag will always create a proxied context, even if the browser did support OffscreenCanvas. If you would like to prefer to create a higher performance OffscreenCanvas context whenever suppported by the browser, but only fall back to a proxied WebGL context to keep compatibility with browsers that do not yet have OffscreenCanvas support, you can specify the ``EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK`` context creation flag. In order to use this flag, code should be compiled with both ``-s OFFSCREEN_FRAMEBUFFER=1`` and ``-s OFFSCREENCANVAS_SUPPORT=1`` linker flags.

    Default value of ``proxyContextToMainThread`` after calling ``emscripten_webgl_init_context_attributes()`` is ``EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW``, if the WebGL context is being created on the main thread. This means that by default WebGL contexts created on the main thread are not shareable between multiple threads (to avoid accidental performance loss from enabling proxying when/if it is not needed). To create a context that can be shared between multiple pthreads, set the ``proxyContextToMainThread`` flag ``EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS``.

Callback functions
------------------


.. c:type:: em_webgl_context_callback

  Function pointer for the :c:func:`WebGL Context event callback functions <emscripten_set_webglcontextlost_callback>`, defined as:

  .. code-block:: cpp

    typedef EM_BOOL (*em_webgl_context_callback)(int eventType, const void *reserved, void *userData);

  :param int eventType: The type of :c:data:`WebGL context event <EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST>`.
  :param reserved: Reserved for future use; pass in 0.
  :type reserved: const void*
  :param void* userData: The ``userData`` originally passed to the registration function.
  :returns: |callback-handler-return-value-doc|
  :rtype: |EM_BOOL|



Functions
---------


.. c:function:: EMSCRIPTEN_RESULT emscripten_set_webglcontextlost_callback(const char *target, void *userData, EM_BOOL useCapture, em_webgl_context_callback callback)
  EMSCRIPTEN_RESULT emscripten_set_webglcontextrestored_callback(const char *target, void *userData, EM_BOOL useCapture, em_webgl_context_callback callback)

  Registers a callback function for the canvas `WebGL context`_ events: ``webglcontextlost`` and ``webglcontextrestored``.

  :param target: |target-parameter-doc|
  :type target: const char*
  :param void* userData: |userData-parameter-doc|
  :param EM_BOOL useCapture: |useCapture-parameter-doc|
  :param em_webgl_context_callback callback: |callback-function-parameter-doc|
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EM_BOOL emscripten_is_webgl_context_lost(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)

  Queries the given WebGL context if it is in a lost context state.

  :param target: Specifies a handle to the context to test.
  :type target: EMSCRIPTEN_WEBGL_CONTEXT_HANDLE
  :returns: ``true`` if the WebGL context is in a lost state (or the context does not exist)
  :rtype: |EM_BOOL|


.. c:function:: void emscripten_webgl_init_context_attributes(EmscriptenWebGLContextAttributes *attributes)

  Populates all fields of the given :c:type:`EmscriptenWebGLContextAttributes` structure to their default values for use with WebGL 1.0.

  Call this function as a forward-compatible way to ensure that if there are new fields added to the ``EmscriptenWebGLContextAttributes`` structure in the future, that they also will get default-initialized without having to change any code.

  :param attributes: The structure to be populated.
  :type attributes: EmscriptenWebGLContextAttributes*



.. c:function:: EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_create_context(const char *target, const EmscriptenWebGLContextAttributes *attributes)

  Creates and returns a new `WebGL context <http://www.khronos.org/registry/webgl/specs/latest/1.0/#2.1>`_.

  .. note::

    - A successful call to this function will not immediately make that rendering context active. Call :c:func:`emscripten_webgl_make_context_current` after creating a context to activate it.
    - This function will try to initialize the context version that was *exactly* requested. It will not e.g. initialize a newer backwards-compatible version or similar.

  :param target: The DOM canvas element in which to initialize the WebGL context. If 0 is passed, the element specified by ``Module.canvas`` will be used.
  :type target: const char*
  :param attributes: The attributes of the requested context version.
  :type attributes: const EmscriptenWebGLContextAttributes*
  :returns: On success, a strictly positive value that represents a handle to the created context. On failure, a negative number that can be cast to an |EMSCRIPTEN_RESULT| field to get the reason why the context creation failed.
  :rtype: |EMSCRIPTEN_WEBGL_CONTEXT_HANDLE|


.. c:function:: EMSCRIPTEN_RESULT emscripten_webgl_make_context_current(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)

  Activates the given WebGL context for rendering. After calling this function, all OpenGL functions (``glBindBuffer()``, ``glDrawArrays()``, etc.) can be applied to the given GL context.

  :param EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context: The WebGL context to activate.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_get_current_context()

  Returns the currently active WebGL rendering context, or 0 if no context is active. Calling any WebGL functions when there is no active rendering context is undefined and may throw a JavaScript exception.

  :returns: The currently active WebGL rendering context, or 0 if no context is active.
  :rtype: |EMSCRIPTEN_WEBGL_CONTEXT_HANDLE|


.. c:function:: EMSCRIPTEN_RESULT emscripten_webgl_get_context_attributes(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, EmscriptenWebGLContextAttributes *outAttributes)

  Fetches the context creation attributes that were used to initialize the given WebGL context.

  :param EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context: The WebGL context to query.
  :param EmscriptenWebGLContextAttributes \*outAttributes: The context creation attributes of the specified context will be filled in here. This pointer cannot be null.
  :returns: On success, EMSCRIPTEN_RESULT_SUCCESS.


.. c:function:: EMSCRIPTEN_RESULT emscripten_webgl_commit_frame()

  Presents ("swaps") the content rendered on the currently active WebGL context to be visible on the canvas. This function is available on WebGL contexts that were created with the ``explicitSwapControl==true`` context creation attribute. If ``explicitSwapControl==false``, then the rendered content is displayed on the screen "implicitly" when yielding back to the browser from the calling event handler.

  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values, denoting a reason for failure.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_webgl_get_drawing_buffer_size(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, int *width, int *height)

  Gets the ``drawingBufferWidth`` and ``drawingBufferHeight`` of the specified WebGL context.

  :param EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context: The WebGL context to get width/height of.
  :param int \*width: The context's ``drawingBufferWidth``.
  :param int \*height: The context's ``drawingBufferHeight``.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_webgl_destroy_context(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)

  Deletes the given WebGL context. If that context was active, then the no context is set to active.

  :param EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context: The WebGL context to delete.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EM_BOOL emscripten_webgl_enable_extension(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, const char *extension)

  Enables the given extension on the given context.

  :param EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context: The WebGL context on which the extension is to be enabled.
  :param extension: A string identifying the `WebGL extension <http://www.khronos.org/registry/webgl/extensions/>`_. For example "OES_texture_float".
  :type extension: const char*
  :returns: EM_TRUE if the given extension is supported by the context, and EM_FALSE if the extension was not available.
  :rtype: |EM_BOOL|


.. c:function:: EMSCRIPTEN_RESULT emscripten_set_canvas_element_size(const char *target, int width, int height)

  Resizes the pixel width and height of the given Canvas element in the DOM.

  :param target: Specifies a selector for the canvas to resize.
  :param width: New pixel width of canvas element.
  :param height: New pixel height of canvas element.
  :returns: EMSCRIPTEN_RESULT_SUCCESS if resizing succeeded, and one of the EMSCRIPTEN_RESULT_* error values on failure.


.. c:function:: EMSCRIPTEN_RESULT emscripten_get_canvas_element_size(const char *target, int *width, int *height)

  Gets the current pixel width and height of the given Canvas element in the DOM.

  :param target: Specifies a selector for the canvas to resize.
  :param width: A pointer to memory location where the width of the canvas element is received. This pointer may not be null.
  :param height: A pointer to memory location where the height of the canvas element is received. This pointer may not be null.
  :returns: EMSCRIPTEN_RESULT_SUCCESS if resizing succeeded, and one of the EMSCRIPTEN_RESULT_* error values on failure.


CSS
===

Functions
---------


.. c:function:: EMSCRIPTEN_RESULT emscripten_set_element_css_size(const char * target, double width, double height)

  Resizes the CSS width and height of the element specified by ``target`` on the Emscripten web page.

  :param target: Element to resize. If 0 is passed, the element specified by ``Module.canvas`` will be used.
  :type target: const char*
  :param double width: New width of the element.
  :param double height: New height of the element.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|


.. c:function:: EMSCRIPTEN_RESULT emscripten_get_element_css_size(const char * target, double * width, double * height)

  Gets the current CSS width and height of the element specified by ``target``.

  :param target: Element to get size of. If 0 is passed, the element specified by ``Module.canvas`` will be used.
  :type target: const char*
  :param double* width: Width of the element.
  :param double* height: Height of the element.
  :returns: :c:data:`EMSCRIPTEN_RESULT_SUCCESS`, or one of the other result values.
  :rtype: |EMSCRIPTEN_RESULT|



.. COMMENT (not rendered): Section below is automated copy and replace text.

.. COMMENT (not rendered): The replace function return values with links (not created automatically)

.. |EMSCRIPTEN_RESULT| replace:: :c:type:`EMSCRIPTEN_RESULT`
.. |EM_BOOL| replace:: :c:type:`EM_BOOL`
.. |EMSCRIPTEN_WEBGL_CONTEXT_HANDLE| replace:: :c:type:`EMSCRIPTEN_WEBGL_CONTEXT_HANDLE`


.. COMMENT (not rendered): Following values are common to many functions, and currently only updated in one place (here).
.. COMMENT (not rendered): These can be properly replaced if required either wholesale or on an individual basis.

.. |target-parameter-doc| replace:: :ref:`Target HTML element id <target-parameter-html5-api>`.
.. |userData-parameter-doc| replace:: :ref:`User-defined data <userdata-parameter-html5-api>` to be passed to the callback (opaque to the API).
.. |useCapture-parameter-doc| replace:: Set ``true`` to :ref:`use capture <usecapture-parameter-html5-api>`.
.. |callback-handler-return-value-doc| replace:: ``true`` (non zero) to indicate that the event was consumed by the :ref:`callback handler <callback-handler-return-em_bool-html5-api>`.
.. |callback-function-parameter-doc| replace:: A callback function. The function is called with the type of event, information about the event, and user data passed from this registration function. The callback should return ``true`` if the event is consumed.

.. _gamepad: http://www.w3.org/TR/gamepad/#gamepad-interface
.. _webgl_context: http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2

Animation and Timing
====================

The API provided here are low-level functions that directly call the relevant Web APIs and nothing more. They don't integrate with the emscripten runtime, such as checking if the program has halted and cancelling a callback if so. For that purpose,
see the function ``emscripten_set_main_loop()``.

Functions
---------

.. c:function:: long emscripten_set_timeout(void (*cb)(void *userData), double msecs, void *userData)

  Performs a setTimeout() callback call on the given function on the calling thread.

  :param cb: The callback function to call.
  :param msecs: Millisecond delay until the callback should fire.
  :param userData: Specifies a pointer sized field of custom data that will be passed in to the callback function.
  :returns: An ID to the setTimeout() call that can be passed to emscripten_clear_timeout() to cancel the pending timeout timer.


.. c:function:: void emscripten_clear_timeout(long setTimeoutId)

  Cancels a pending setTimeout() call on the calling thread. This function must be called on the same
  thread as the emscripten_set_timeout() call that registered the callback.

  :param setTimeoutId: An ID returned by function emscripten_set_timeout().


.. c:function:: void emscripten_set_timeout_loop(EM_BOOL (*cb)(double time, void *userData), double intervalMsecs, void *userData)

  Initializes a setTimeout() loop on the given function on the calling thread. The specified callback
  function 'cb' needs to keep returning EM_TRUE as long as the animation loop should continue to run.
  When the function returns false, the setTimeout() loop will stop.
  Note: The loop will start immediately with a 0 msecs delay - the passed in intervalMsecs time specifies
  the interval that the consecutive callback calls should fire at.

  :param cb: The callback function to call.
  :param intervalMsecs: Millisecond interval at which the callback should keep firing.
  :param userData: Specifies a pointer sized field of custom data that will be passed in to the callback function.


.. c:function:: long emscripten_request_animation_frame(EM_BOOL (*cb)(double time, void *userData), void *userData)

  Performs a single requestAnimationFrame() callback call on the given function on the calling thread.

  :param cb: The callback function to call. This function will receive the current high precision timer value
    (value of performance.now()) at the time of the call.
  :param userData: Specifies a pointer sized field of custom data that will be passed in to the callback function.
  :returns: An ID to the requestAnimationFrame() call that can be passed to emscripten_cancel_animation_frame() to
    cancel the pending requestAnimationFrame timer.


.. c:function:: void emscripten_cancel_animation_frame(long requestAnimationFrameId)

  Cancels a registered requestAnimationFrame() callback on the calling thread before it is run. This
  function must be called on the same thread as the emscripten_request_animation_frame() call that
  registered the callback.

  :param requestAnimationFrameId: An ID returned by function emscripten_request_animation_frame().


.. c:function:: void emscripten_request_animation_frame_loop(EM_BOOL (*cb)(double time, void *userData), void *userData)

  Initializes a requestAnimationFrame() loop on the given function on the calling thread. The specified
  callback function 'cb' needs to keep returning EM_TRUE as long as the animation loop should continue
  to run. When the function returns false, the animation frame loop will stop.

  :param cb: The callback function to call. This function will receive the current high precision timer value
    (value of performance.now()) at the time of the call.
  :param userData: Specifies a pointer sized field of custom data that will be passed in to the callback function.


.. c:function:: long emscripten_set_immediate(void (*cb)(void *userData), void *userData)

  Performs a setImmediate() callback call on the given function on the calling thread. Returns an ID
  to the setImmediate() call that can be passed to emscripten_clear_immediate() function to cancel
  a pending setImmediate() invocation.
  TODO: Currently the polyfill of setImmediate() only works in the main browser thread, but not in pthreads.

  :param cb: The callback function to call.
  :param userData: Specifies a pointer sized field of custom data that will be passed in to the callback function.
  :returns: An ID to the setImmediate() call that can be passed to emscripten_clear_immediate() to cancel the pending callback.


.. c:function:: void emscripten_clear_immediate(long setImmediateId)

  Cancels a pending setImmediate() call on the calling thread. This function must be called on the same
  thread as the emscripten_set_immediate() call that registered the callback.

  :param setImmediateId: An ID returned by function emscripten_set_immediate().


.. c:function:: void emscripten_set_immediate_loop(EM_BOOL (*cb)(void *userData), void *userData)

  Initializes a setImmediate() loop on the given function on the calling thread. The specified callback
  function 'cb' needs to keep returning EM_TRUE as long as the loop should continue to run.
  When the function returns false, the setImmediate() loop will stop.
  TODO: Currently the polyfill of setImmediate() only works in the main browser thread, but not in pthreads.

  :param cb: The callback function to call.
  :param userData: Specifies a pointer sized field of custom data that will be passed in to the callback function.


.. c:function:: long emscripten_set_interval(void (*cb)(void *userData), double intervalMsecs, void *userData)

  Initializes a setInterval() loop on the given function on the calling thread. Returns an ID to the
  initialized loop. Call emscripten_clear_interval() with this ID to terminate the loop.
  Note that this function will cause the given callback to be called repeatedly. Do not call
  emscripten_set_interval() again on the same callback function from inside the callback, as that would
  register multiple loops to run simultaneously.

  :param cb: The callback function to call.
  :param intervalMsecs: Millisecond interval at which the callback should keep firing.
  :param userData: Specifies a pointer sized field of custom data that will be passed in to the callback function.
  :returns: An ID to the setInterval() call that can be passed to emscripten_clear_interval() to cancel the
    currently executing interval timer.


.. c:function:: void emscripten_clear_interval(long setIntervalId)

  Cancels a currently executing setInterval() loop on the calling thread. This function must be called on
  the same thread as the emscripten_set_interval() call that registered the callback.

  :param setIntervalId: An ID returned by function emscripten_set_interval().


.. c:function:: double emscripten_date_now(void)

  Calls JavaScript Date.now() function in the current thread.

  :returns: The number of msecs elapsed since the UNIX epoch. (00:00:00 Thursday, 1 January 1970)


.. c:function:: double emscripten_performance_now(void)

  Calls JavaScript performance.now() function in the current thread. Note that the returned value of
  this function is based on different time offset depending on which thread this function is called in.
  That is, do not compare absolute time values returned by performance.now() across threads. (comparing
  relative timing values is ok). On the main thread this function returns the number of milliseconds elapsed
  since page startup time. In a Web Worker/pthread, this function returns the number of milliseconds since
  the Worker hosting that pthread started up. (Workers are generally not torn down when a pthread quits and
  a second one starts, but they are kept alive in a pool, so this function is not guaranteed to start
  counting time from 0 at the time when a pthread starts)

  :returns: A high precision wallclock time value in msecs.


Console
=======

Functions
---------

.. c:function:: void emscripten_console_log(const char *utf8String)

  Prints a string using console.log().

  :param utf8String: A string encoded as UTF-8.


.. c:function:: void emscripten_console_warn(const char *utf8String)

  Prints a string using console.warn().

  :param utf8String: A string encoded as UTF-8.


.. c:function:: void emscripten_console_error(const char *utf8String)

  Prints a string using console.error().

  :param utf8String: A string encoded as UTF-8.


Throw
=====

Functions
---------

.. c:function:: void emscripten_throw_number(double number)

  Invokes JavaScript throw statement and throws a number.


.. c:function:: void emscripten_throw_string(const char *utf8String)

  Invokes JavaScript throw statement and throws a string.
