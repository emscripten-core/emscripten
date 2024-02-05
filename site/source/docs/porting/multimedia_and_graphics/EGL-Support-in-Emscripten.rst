.. _egl-support-in-emscripten:

=========================
EGL Support in Emscripten
=========================

.. note:: This article is under construction.

Khronos Group publishes a specification called EGL, which is an API that handles (among other tasks) graphics context creation, rendering surface management, and interop between different Khronos Group graphics APIs (OpenGL, OpenGL ES, OpenVG). For detailed information, see the `Khronos EGL webpage <http://www.khronos.org/egl>`_.

Currently, EGL is not very widely used across operating systems/graphics driver vendors. The most notable adoption is in the Android architecture, where EGL is the primary method for creating rendering contexts for OpenGL ES 1&2 when using the Android NDK. Also, Mesa has an implementation of the EGL specification in its `graphics driver <http://www.mesa3d.org/egl.html>`_.

Emscripten also supplies an implementation of the EGL v1.4 specification. This allows C/C++ client code to use a (nearly) unified codebase for creating a GLES2 (WebGL) rendering context across Web, Linux (with Mesa) and Android NDK. The implementation of the EGL specification in Emscripten is not perfect, see the end of this page for a status chart.

What EGL is not?
================

Somewhat disappointingly, EGL is not a self-sufficient complete solution for initializing GLES2 graphics rendering (on any platform, not just Emscripten) and overseeing various associated tasks. The specification is limited in its scope and lacks some features. In particular, EGL cannot help with the following tasks:

- Creating a render window. The EGL specification does not specify how a target window is created to which to render to. One must use platform-specific native window system functions (X11, Win32 API, ANativeWindow) to first create a render window.
- Specifying render window size in arbitrary pixel increments. EGL does not have any functionality to request a desired size for the main render window, or to resize it.
- Specifying a fullscreen video mode/screen resolution. EGL cannot be used to control whether to render in a windowed or fullscreen mode, or to toggle between these at runtime.

Therefore, for each platform, including Emscripten, there exists platform-specific means to perform these tasks.

How to create a WebGL context using EGL?
========================================

In the web environment, WebGL is the technology used for 3D-accelerated rendering. WebGL is almost identical to GLES2, and because EGL does not apply at all for WebGL, for all purposes in this page, the terms WebGL and GLES2 are used interchangeably. Therefore to create a WebGL context,
one uses EGL, and according to its wording, creates a GLES2 context.

Initialization
--------------

Perform the following steps to create a GLES2 context using EGL:

#. Obtain a handle to an ``EGLDisplay`` object by calling ``eglGetDisplay``.
#. Initialize EGL on that display by calling ``eglInitialize``.
#. Call ``eglGetConfigs`` and/or ``eglChooseConfig`` one or multiple times to find the ``EGLConfig`` that represents the desired main render target parameters. To examine the attributes of an ``EGLConfig``, call ``eglGetConfigAttrib``.
#. At this point, one would use whatever platform-specific functions available (*X11*, *Win32 API*, *ANativeWindow*) to set up a native window to render to. For Emscripten, this step does not apply, and can be skipped.
#. Create a main render target surface (``EGLSurface``) by calling ``eglCreateWindowSurface`` with a valid display and config parameters. Set window and attribute list parameters to null.
#. Create a GLES2 rendering context (``EGLContext``) by calling ``eglCreateContext``, followed by a call to ``eglMakeCurrent`` to activate the rendering context. When creating the context, specify the context attribute ``EGL_CONTEXT_CLIENT_VERSION == 2``.

After these steps, you have a set of EGL objects ``EGLDisplay``, ``EGLConfig``, ``EGLSurface`` and ``EGLContext`` that represent the main GLES2 rendering context.

Cleanup
-------

The sequence to clean up at de-initialization is as follows:

#. Free up the currently active rendering context by calling ``eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)``.
#. Deinitialize the ``EGLContext`` object by calling ``eglDestroyContext`` on it.
#. Destroy all initialized ``EGLSurface`` objects by calling ``eglDestroySurface`` on them.
#. Deinitialize EGL altogether by calling ``eglTerminate(display)``.
#. Delete the native rendering window. This step does not apply for Emscripten.

Sample code
-----------

Example code for using EGL to initialize a WebGL context can be found in the sample applications in the `emscripten/test/third_party/glbook <https://github.com/emscripten-core/emscripten/tree/main/test/third_party/glbook>`_ directory, more specifically in the file `esUtil.c <https://github.com/emscripten-core/emscripten/blob/main/test/third_party/glbook/Common/esUtil.c>`_.

Implementation status and notes
===============================

This section lists all EGL v1.4 functions and describes their current implementation status in Emscripten.

Fully implemented
-----------------

- ``eglInitialize``, ``eglGetConfigs``, ``eglQueryContext``, ``eglQueryString``, ``eglQuerySurface``, ``eglGetCurrentContext``, ``glGetCurrentSurface``, ``eglGetCurrentDisplay``, ``eglReleaseThread``, ``eglDestroySurface``, ``eglDestroyContext``: Implemented and should work according to the EGL v1.4 specification.

- ``eglSwapBuffers``: Implemented, but this function cannot really control the swap behavior under WebGL. Calling this function is optional under Emscripten. In WebGL, the contents of the display are always presented to the screen only after the code yields its execution back to the browser, that is, when you return from the tick callback handler you passed to :c:func:`emscripten_set_main_loop`. The ``eglSwapBuffers`` function can however still be used to detect when a GL context loss event occurs.

- ``eglGetDisplay``: Implemented according to the specification. Emscripten does not utilize multiple ``EGLNativeDisplayType`` objects, so pass in ``EGL_DEFAULT_DISPLAY`` here. Emscripten currently actually ignores any value passed in here for Linux emulation purposes, but you should not rely on this in the future.

- ``eglGetError``: Implemented according to the specification.

  .. important:: According to the specification, ``eglGetError`` reports the single most recent error rather than the list of all previous errors. Don't call this function in a loop in the same way you would call ``glGetError``.


Partially implemented
---------------------

- ``eglChooseConfig``: Implemented as a stub, but this function does not do searching/filtering, and is at the moment identical to ``eglGetConfigs`` (`issue #643 <https://github.com/emscripten-core/emscripten/issues/643>`_).

- ``eglGetConfigAttrib``: Implemented. Querying for the attributes ``EGL_BUFFER_SIZE``, ``EGL_ALPHA_SIZE``, ``EGL_BLUE_SIZE``, ``EGL_GREEN_SIZE``, ``EGL_RED_SIZE``, ``EGL_DEPTH_SIZE`` and ``EGL_STENCIL_SIZE`` currently return hardcoded default values (`issue #644 <https://github.com/emscripten-core/emscripten/issues/644>`_). The attributes ``EGL_MIN_SWAP_INTERVAL`` and ``EGL_MAX_SWAP_INTERVAL`` don't currently have any function. Instead, call :c:func:`emscripten_set_main_loop` to specify the main loop update rate.

- ``eglCreateWindowSurface``: Implemented, except it is not possible to call this function multiple times to create multiple render windows.

- ``eglCreateContext``: Implemented as a stub. It is not possible to call this function multiple times to create multiple contexts.

- ``eglBindAPI``, ``eglQueryAPI``: Implemented, although these functions have little utility on Emscripten, since only the GLES2 client API is supported.

- ``eglWaitClient``, ``eglWaitNative``: Implemented as no-op functions. These have no meaning on Emscripten.

- ``eglSwapInterval``: Implemented as a no-op stub. Currently this function cannot set the vsync interval, or enable/disable it.

- ``eglMakeCurrent``: Implemented as a no-op stub.

- ``eglTerminate``: Implemented as a no-op function stub. JavaScript apps are not often shut down manually, but when closing the browser or switching the web page, the browser manages all teardown automatically. Therefore this function does not have a critical importance in Emscripten.

- ``eglGetProcAddress``: Implemented, experimental.

Missing functionality
---------------------

The following functions are currently not implemented:

- ``eglCreatePbufferSurface``, ``eglCreatePixmapSurface``, ``eglCreatePbufferFromClientBuffer``, ``eglSurfaceAttrib``, ``eglBindTexImage``, ``eglReleaseTexImage``, ``eglWaitGL``, ``eglCopyBuffers``.

.. important:: **Do not call**  these functions in Emscripten code, or the application will halt on trying to execute an undefined function.

EGL extensions
--------------

Currently, Emscripten does not implement any extensions in the `EGL Extension Registry <http://www.khronos.org/registry/egl/>`_.

EGL-related bugs and todos
---------------------------

The `Emscripten issue tracker <https://github.com/emscripten-core/emscripten/issues?labels=EGL&state=open>`_ lists EGL-relates issues using the label EGL. Check that page to report or find issues in Emscripten related to EGL.
