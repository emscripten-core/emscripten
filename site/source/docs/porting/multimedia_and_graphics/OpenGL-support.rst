.. _OpenGL-support:

============================
OpenGL support in Emscripten
============================

Emscripten provides three OpenGL modes:

- :ref:`opengl-support-webgl-subset` (default) — supports the set of OpenGL commands that map directly to WebGL.
- :ref:`opengl-support-opengl-es2-0-emulation` — full support for OpenGL ES 2.0.
- :ref:`opengl-support-legacy_and_mobile` — support for a number of legacy GL 1.x features and commands.

This topic provides information about the modes, and how they are enabled.

.. tip:: We highly recommend using the :ref:`opengl-support-webgl-subset` for new code, and porting existing code to this subset if possible. The other two modes are much less efficient, and should only be considered for large codebases that rely heavily on those features. 

.. _opengl-support-webgl-subset:

WebGL-friendly subset of OpenGL
===============================

Emscripten supports the WebGL-friendly subset of OpenGL *by default*. This is the set of commands that map directly to WebGL, so that each GL command becomes a direct call to WebGL. It includes almost all of OpenGL ES 2.0, with the notable exception of client-side arrays.

This mode is used by default because it is stable, predictable and fast. 


.. _opengl-support-opengl-es2-0-emulation:

OpenGL ES 2.0 emulation
=======================

This mode provides a full OpenGL ES 2.0 environment — specifically it emulates client-side arrays that were missing [#f1]_ from the :ref:`opengl-support-webgl-subset`.

This allows you to use `glDrawArrays <https://www.opengl.org/sdk/docs/man3/xhtml/glDrawArrays.xml>`_ etc. without a bound buffer, and Emscripten's GL bindings will set up the buffer automatically (WebGL requires that a buffer be bound). 

This mode is not as efficient as the WebGL-friendly subset, because Emscripten cannot predict the optimal pattern for buffer creation/sizing/etc. We therefore have to make more (costly) data uploads from the CPU to the GPU than are actually needed.

To enable *OpenGL ES 2.0 emulation*, specify the :ref:`emcc <emcc-s-option-value>` option ``-s FULL_ES2=1`` when compiling the project.


.. _opengl-support-legacy_and_mobile:

OpenGL emulation of older desktop and mobile versions
=====================================================

This OpenGL mode enables support for a number of legacy GL 1.x features and commands (for example "immediate mode" and `glNormalPointer <https://www.opengl.org/sdk/docs/man2/xhtml/glNormalPointer.xml>`_). 

While the emulation is by no means "complete", it has been sufficient to port the Sauerbraten 3D game (`BananaBread <https://github.com/kripken/BananaBread>`_ project) and several other real-world codebases using Emscripten. 

To enable this mode, specify the :ref:`emcc <emcc-s-option-value>` option ``-s LEGACY_GL_EMULATION=1`` when compiling the project.

.. warning:: This mode adds **significant** emulation overhead. 


OpenGL extensions
=================

When porting code, it should be noted that desktop OpenGL, OpenGL ES and WebGL each have their own extension registries. This means that neither desktop OpenGL or OpenGL ES extensions are automatically also WebGL extensions, although some amount of parity does exist. See the `WebGL 1.0 extension registry <https://www.khronos.org/registry/webgl/extensions/>`_ for the full list of registered extensions.

Additionally, in WebGL, unlike in desktop or mobile OpenGL, extensions must be activated first before the features they expose take effect. If you use one of the native APIs SDL, EGL, GLUT or GLFW to create your GL context, this will be done automatically for most extensions. If instead you use the HTML5 WebGL context creation API, you must explicitly choose whether to autoenable WebGL extensions. If an extension was not automatically enabled at context creation time, the HTML5 API function `emscripten_webgl_enable_extension` can be used to activate it. Debugging related extensions, draft extensions and vendor-prefixed extensions (MOZ_*, WEBKIT_*) are never enabled automatically at context creation time, but must always be activated manually.

Optimization settings
----------------------

In this mode (``-s LEGACY_GL_EMULATION=1``), there are a few extra flags that can be used to tune the performance of the GL emulation layer:

- ``-s GL_UNSAFE_OPTS=1`` attempts to skip redundant GL work and cleanup. This optimization is unsafe, so is not enabled by default.
- ``-s GL_FFP_ONLY=1`` tells the GL emulation layer that your code will not use the programmable pipeline/shaders at all. This allows the GL emulation code to perform extra optimizations when it knows that it is safe to do so.
- Add the ``Module.GL_MAX_TEXTURE_IMAGE_UNITS`` integer to your shell **.html** file to signal the maximum number of texture units used by the code. This ensures that the GL emulation layer does not waste clock cycles iterating over unused texture units when examining which Fixed Function Pipeline (FFP) emulation shader to run.



Test code/examples
==================

The files in `tests/glbook <https://github.com/kripken/emscripten/tree/master/tests/glbook>`_ provide a number of simple examples that use only the :ref:`opengl-support-webgl-subset`.

The other modes are covered in various tests, including several in `tests/test_browser.py <https://github.com/kripken/emscripten/blob/master/tests/test_browser.py>`_. The best way to locate the tests is to search the source code for the appropriate compiler flags: ``FULL_ES2``, ``LEGACY_GL_EMULATION`` etc.


What if I need unsupported GL features?
=======================================

File a :ref:`bug report <bug-reports>` (with test case) and we will review.


**Footnotes:**

.. [#f1] Client-side arrays are missing from WebGL because they are less efficient than properly using GPU-side data.