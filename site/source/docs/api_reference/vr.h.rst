.. _vr-h:

====
vr.h
====

The C APIs in `vr.h <https://github.com/kripken/emscripten/blob/master/system/include/emscripten/vr.h>`_ provide basic interfaces for interacting with WebVR from Emscripten.

.. contents:: Table of Contents
	:local:
	:depth: 1

Initialization
==============

.. c:function:: int emscripten_vr_init()

	Initialize the emscripten VR API. This will `navigator.getVRDisplays()
	<https://w3c.github.io/webvr/spec/1.1/#navigator-getvrdisplays-attribute>`_
	and when completed, set the return of :c:func:`emscripten_vr_ready` to be `true`.

.. tip:: This call succeeding is not sufficient for use of the rest of the API. Please
	make sure to wait until the return value of :c:func:`emscripten_vr_ready` is true.

	:returns: `1` on success, `0` if the browsers WebVR support is insufficient.
	:rtype: int

.. c:function:: int emscripten_vr_ready()

	Check whether the VR API has finished initializing VR displays.
	See also :c:func:`emscripten_vr_init`.

	This function may return `1` event if WebVR is not supported in the
	running browser.

	:returns: `1` if ready, `0` otherwise.
	:rtype: int

API Queries
===========

All of the following functions require :c:func:`emscripten_vr_init` to have been
called but do not require VR displays and can therefore be called before the return
value of :c:func:`emscripten_vr_ready` is `true`.

.. c:function:: int emscripten_vr_version_minor()

	Minor version of the WebVR API currently supported by the browser.

	:returns: minor version of WebVR, or `-1` if not supported or API not initialized.
	:rtype: int

.. c:function:: int emscripten_vr_version_major()

	Major version of the WebVR API currently supported by the browser.

	:returns: major version of WebVR, or `-1` if not supported or API not initialized.
	:rtype: int

Display Functions
=================

All of the following functions require :c:func:`emscripten_vr_init` to have been
called the return value of :c:func:`emscripten_vr_ready` to be `true`.

.. c:function:: int emscripten_vr_count_displays()

	:returns: Number of displays connected.
	:rtype: int

.. c:function: VRDisplayHandle emscripten_vr_get_display_handle(int displayIndex)

	:param int displayIndex: index of display (inclusive 0 to exclusive :c:func:`emscripten_vr_count_displays`).
	:returns: handle for a VR display.
	:rtype: VRDisplayHandle

.. c:function: char* emscripten_vr_get_display_name(VRDisplayHandle handle)

	Get a user-readable name which identifies the VR display.

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:returns: name of the VR display or `0 (NULL)` if the handle is invalid.
	:rtype: char*

.. c:function: bool emscripten_vr_get_display_connected(VRDisplayHandle handle)

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:returns: `true` if the display is connected, `false` otherwise or when
		the handle is invalid.
	:rtype: bool

.. c:function: bool emscripten_vr_get_display_presenting(VRDisplayHandle handle)

	See also :c:func:`emscripten_vr_request_present`.

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:returns: `true` if the display is currently presenting, `false` otherwise
		or when the handle is invalid.
	:rtype: bool

.. c:function: int emscripten_vr_get_display_capabilities(VRDisplayHandle handle, VRDisplayCapabilities* displayCaps)

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:param VRDisplayCapabilities displayCaps: receives capabilities of the VR display.
	:returns: |display-function-return-doc|
	:rtype: bool

.. c:function: int emscripten_vr_get_eye_parameters(VRDisplayHandle handle, VREye whichEye, VREyeParameters* eyeParams)

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:param VREye whichEye: which eye to query parameters for.
	:param VREyeParameters eyeParam: receives the parameters for requested eye.
	:returns: |display-function-return-doc|
	:rtype: bool

Render Loop
===========

In contrast to the usual emscripten main loop (see :ref:`_emscripten-h-browser-execution-environment`),
VR displays require their own rendering loop which is independent from the main loop. The rendering
loop can be set per display and will act like a main loop with timing mode ``EM_TIMING_RAF`` until the
display is requested to present, as of which it will run at the VR display's refresh rate.

.. c:function:: void emscripten_vr_set_display_render_loop(VRDisplayHandle handle, em_vr_callback_func callback)

	Set a C function as the per frame rendering callback of a VR display.

.. tip:: There can be only *one* render loop function per VR display. To change the render loop function, first :c:func:`cancel <emscripten_vr_cancel_display_render_loop>` the current loop, and then call this function to set another.

	:param VRDisplayHandle handle: |display-handle-parameter-doc|: id of the display to set the render loop for.
	:param em_vr_callback_func callback: C function to set as per frame rendering callback.
	:rtype: |display-function-return-doc|

.. c:function:: void emscripten_vr_set_display_render_loop_arg(VRDisplayHandle handle, em_vr_callback_func callback, void* arg)

	Set a C function as the per frame rendering callback of a VR display.

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:param em_vr_callback_arg_func callback: C function to set as per frame rendering callback. The function signature must have a ``void*`` parameter for passing the ``arg`` value.
	:param void* arg: User-defined data passed to the render loop function, untouched by the API itself.
	:rtype: |display-function-return-doc|

.. c:function:: void emscripten_vr_cancel_display_render_loop(VRDisplayHandle handle: |display-handle-parameter-doc|)

	Cancels the render loop of a VR display should there be one running for it.

	|render-loop-info|

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:rtype: |display-function-return-doc|

.. c:function:: int emscripten_vr_get_frame_data(VRDisplayHandle handle)

	Get view matrix, projection matrix, timestamp and head pose for current frame.
	Only valid when called from within a render loop callback.

	|render-loop-info|

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:rtype: |display-function-return-doc|

.. c:function:: int emscripten_vr_submit_frame(VRDisplayHandle handle)

	Submit the current state of canvases passed via VRLayerInit to
	:c:func:`emscripten_vr_request_present` to be rendered to the VR display.
	Only valid when called from within a render loop callback.

	|render-loop-info|

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:rtype: |display-function-return-doc|

.. c:function:: int emscripten_vr_request_present(VRDisplayHandle handle, VRLayerInit* layerInit, int layerCount, em_vr_arg_callback_func callback, void* userData)

	Request present for the VR display using canvases specified in the `layerInit` array.
	If the request is successful `callback` will be called with `userData` and the render
	loop will continue rendering at the refresh rate of the VR display.

	Must be called from a user callback (see :ref:`HTML5 API <_html5-h>`_).

	See the :ref:`specification of VRDisplay.requestPresent <https://w3c.github.io/webvr/spec/1.1/#dom-vrdisplay-requestpresent>` for detailed information.

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:param VRLayerInit layers: array of layers which will be rendered to.
	:param int layerCount: number of layers in `layers`.
	:param em_vr_arg_callback_func callback: optional function that will be called when the requst has succeeded.
	:param void* userData: optional data to pass to the callback when the request succeeds. Is not modified by the API.
	:rtype: |display-function-return-doc|

.. c:function:: int emscripten_vr_exit_present(VRDisplayHandle handle)

	Request present exit.

	:param VRDisplayHandle handle: |display-handle-parameter-doc|
	:rtype: |display-function-return-doc|


.. COMMENT (not rendered): Following values are common to many functions, and currently only updated in one place (here).
.. COMMENT (not rendered): These can be properly replaced if required either wholesale or on an individual basis.

.. |display-handle-parameter-doc| replace:: a display handle.

.. |display-function-return-doc| replace:: `1` on success, `0` if handle was invalid.

.. |render-loop-info| replace:: See also :c:func:`emscripten_vr_set_display_render_loop` and :c:func:`emscripten_vr_set_display_render_loop_arg` for information about setting and using the render loop.

