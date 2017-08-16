.. _vr-h:

====
vr.h
====

The C APIs in `vr.h <https://github.com/kripken/emscripten/blob/master/system/include/emscripten/vr.h>`_ provide basic interfaces for interacting with WebVR from Emscripten.

.. contents:: Table of Contents
	:local:
	:depth: 1



Render loop
===========

In contrast to the usual emscripten main loop (see :ref:`_emscripten-h-browser-execution-environment`),
VR displays require their own rendering loop which is independent from the main loop. The rendering
loop can be set per display and will act like a main loop with timing mode ``EM_TIMING_RAF`` until the
display is requested to present, as of which it will run at the VR display's refresh rate.

.. c:function:: void emscripten_vr_set_display_render_loop(WebVRDeviceId deviceId, em_vr_callback_func callback)

	Set a C function as the per frame rendering callback of a VR display.

.. tip:: There can be only *one* render loop function per VR display. To change the render loop function, first :c:func:`cancel <emscripten_vr_cancel_display_render_loop>` the current loop, and then call this function to set another.

	:param WebVRDeviceId deviceId: id of the display to set the render loop for.
	:param em_vr_callback_func callback: C function to set as per frame rendering callback.
	:rtype: void

.. c:function:: void emscripten_vr_set_display_render_loop_arg(WebVRDeviceId deviceId, em_vr_callback_func callback, void* arg)

	Set a C function as the per frame rendering callback of a VR display.

	:param WebVRDeviceId deviceId: id of the display to set the render loop for.
	:param em_vr_callback_arg_func callback: C function to set as per frame rendering callback. The function signature must have a ``void*`` parameter for passing the ``arg`` value.
	:param void* arg: User-defined data passed to the render loop function, untouched by the API itself.
	:rtype: void

.. c:function:: void emscripten_vr_cancel_display_render_loop(WebVRDeviceId deviceId)

	Cancels the render loop of a VR display should there be one running for it.

	See also :c:func:`emscripten_vr_set_display_render_loop` and :c:func:`emscripten_vr_set_display_render_loop_arg` for information about setting and using the render loop.

	:param WebVRDeviceId deviceId: id of the display to set the render loop for.
	:rtype: void
