.. _OpenGL-support:

============================
OpenGL support in Emscripten
============================

Emscripten provides three OpenGL modes:

- :ref:`opengl-support-webgl-subset` (default) — supports the set of OpenGL ES 2.0/3.0 commands that map directly to WebGL 1/2.
- :ref:`opengl-support-opengl-es2-0-emulation` — support for some emulated OpenGL ES 2.0/3.0 features that are not present in WebGL.
- :ref:`opengl-support-legacy_and_mobile` — support for a number of legacy GL 1.x features and commands.

This topic provides information about the modes, and how they are enabled.

.. tip:: We highly recommend using the :ref:`opengl-support-webgl-subset` for new code, and porting existing code to this subset if possible. The other two modes are less efficient, and should only be considered for codebases that rely heavily on those features.

.. _opengl-support-webgl-subset:

WebGL-friendly subset of OpenGL ES 2.0/3.0
==========================================

By default, Emscripten targets the WebGL-friendly subset of OpenGL ES 2.0. This is the set of GL ES commands that map directly to WebGL, so that each GL command has a roughly direct mapping to WebGL. It includes almost all of OpenGL ES 2.0, with the notable exception of client-side arrays, and some other features that are listed in `WebGL 1.0 Specification/Chapter 6 <https://www.khronos.org/registry/webgl/specs/1.0/#6>`_.

To program against the WebGL subset of OpenGL ES, one uses the GL ES 2.0 header files and the GL ES 2.0 API, while adhering to the limitations specified in Chapter 6 of the WebGL specification.

This mode is used by default because it best matches the WebGL features brovided by browsers.

To target WebGL 2, pass the linker flag ``-s USE_WEBGL2=1``. Specifying this flag enables (and defaults to, unless otherwise specified at context creation time) the creation of WebGL 2 contexts at runtime, but it is still possible to create WebGL 1 contexts, so applications can choose whether to require WebGL 2 or whether to support a fallback to WebGL 1.

.. _opengl-support-opengl-es2-0-emulation:

OpenGL ES 2.0/3.0 emulation
===========================

This build mode emulates some features of OpenGL ES 2.0/3.0 that are not part of the core WebGL 1 specification.

In particular, this mode emulates client-side arrays that are missing [#f1]_ from the :ref:`opengl-support-webgl-subset`.

This allows you to use functions `glDrawArrays <https://www.opengl.org/sdk/docs/man3/xhtml/glDrawArrays.xml>`_ and `glDrawElements <https://www.opengl.org/sdk/docs/man/html/glDrawElements.xhtml>`_ etc. without a bound buffer, and Emscripten's GL bindings will set up the buffer automatically (WebGL requires that a buffer be bound).

.. note:: This build mode has a limitation that the largest index in client-side index buffer must be smaller than the total number of indices in that buffer. See `issue #4214 <https://github.com/emscripten-core/emscripten/issues/4214>`_ for more details.

To enable *OpenGL ES 2.0 emulation*, specify the :ref:`emcc <emcc-s-option-value>` option ``-s FULL_ES2=1`` when linking the final executable (.js/.html) of the project.

To enable *OpenGL ES 3.0 emulation*, specify the :ref:`emcc <emcc-s-option-value>` option ``-s FULL_ES3=1`` when linking the final executable (.js/.html) of the project. This adds emulation for mapping memory blocks to client side memory. The flags ``-s FULL_ES2=1`` and ``-s FULL_ES3=1`` are orthogonal, so either one or both can be specified to emulate different features.

.. _opengl-support-legacy_and_mobile:

Emulation of older Desktop OpenGL API features
==============================================

This OpenGL mode enables support for a number of legacy desktop OpenGL 1.x features and commands (for example "immediate mode" and `glNormalPointer <https://www.opengl.org/sdk/docs/man2/xhtml/glNormalPointer.xml>`_).

While the emulation is by no means complete, it has been sufficient to port the Sauerbraten 3D game (`BananaBread <https://github.com/kripken/BananaBread>`_ project) and some other real-world codebases using Emscripten.

To enable this mode, specify the :ref:`emcc <emcc-s-option-value>` option ``-s LEGACY_GL_EMULATION=1`` when linking the final executable (.js/.html) of the project.

Optimization settings
----------------------

In this mode (``-s LEGACY_GL_EMULATION=1``), there are a few extra flags that can be used to tune the performance of the GL emulation layer:

- ``-s GL_UNSAFE_OPTS=1`` attempts to skip redundant GL work and cleanup. This optimization is unsafe, so is not enabled by default.
- ``-s GL_FFP_ONLY=1`` tells the GL emulation layer that your code will not use the programmable pipeline/shaders at all. This allows the GL emulation code to perform extra optimizations when it knows that it is safe to do so.
- Add the ``Module.GL_MAX_TEXTURE_IMAGE_UNITS`` integer to your shell **.html** file to signal the maximum number of texture units used by the code. This ensures that the GL emulation layer does not waste clock cycles iterating over unused texture units when examining which Fixed Function Pipeline (FFP) emulation shader to run.

What if my codebase depends on an desktop OpenGL feature that is currently unsupported?
---------------------------------------------------------------------------------------

You can consider building the codebase against the `Regal <https://github.com/p3/regal>`_ Desktop OpenGL emulation library, which aims to support Desktop OpenGL features on top of OpenGL ES 2.0. This may work better or worse than Emscripten's GL emulation depending on the project.

OpenGL ES extensions
====================

When porting code, it should be noted that desktop OpenGL, OpenGL ES and WebGL each have their own extension registries. This means that neither desktop OpenGL or OpenGL ES extensions are automatically also WebGL extensions, although some amount of parity does exist. See the `WebGL 1.0 extension registry <https://www.khronos.org/registry/webgl/extensions/>`_ for the full list of registered extensions.

Additionally, in WebGL, unlike in desktop or mobile OpenGL, extensions must be activated first before the features they expose take effect. If you use one of the native APIs SDL, EGL, GLUT or GLFW to create your GL context, this will be done automatically for most extensions. If instead you use the HTML5 WebGL context creation API, you must explicitly choose whether to autoenable WebGL extensions. If an extension was not automatically enabled at context creation time, the HTML5 API function `emscripten_webgl_enable_extension` can be used to activate it. Debugging related extensions, draft extensions and vendor-prefixed extensions (MOZ_*, WEBKIT_*) are never enabled automatically at context creation time, but must always be activated manually.

When migrating from WebGL 1 to WebGL 2, take note that some WebGL 1 extensions are migrated to core WebGL 2, and therefore their functionality is no longer advertised as GL extensions. This does not mean that the features would be missing, but that it is possible to utilize these features in WebGL 2 without needing to feature test the presence of a GL extension first.

Test code/examples
==================

The files in `tests/glbook <https://github.com/emscripten-core/emscripten/tree/master/tests/glbook>`_ provide a number of simple examples that use only the :ref:`opengl-support-webgl-subset`.

The other modes are covered in various tests, including several in `tests/test_browser.py <https://github.com/emscripten-core/emscripten/blob/master/tests/test_browser.py>`_. The best way to locate the tests is to search the source code for the appropriate compiler flags: ``FULL_ES2``, ``LEGACY_GL_EMULATION`` etc.

Bug Reports
===========

The Emscripten :ref:`bug tracker <bug-reports>` has labels specific to OpenGL and OpenGL Emulation for tracking various GL-related issues.

**Footnotes:**

.. [#f1] Client-side arrays are missing from WebGL because they are less efficient than properly using GPU-side data.
