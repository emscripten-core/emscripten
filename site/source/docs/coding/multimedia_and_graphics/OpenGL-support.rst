.. _OpenGL-support:

============================
OpenGL support (wiki-import)
============================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

Emscripten supports GL in three ways:

-  The **WebGL-friendly subset of OpenGL**, basically what maps directly
   to WebGL, and is approximately OpenGL ES 2.0 minus client-side data
   and a few other things. This is stable, and recommended since it is
   the most efficient - each GL command basically becomes a direct WebGL
   command, so performance is predictable and fast. If you write new
   code, we recommend you write to this subset of OpenGL.
-  **OpenGL ES 2.0 emulation**. Compile with ``-s FULL_ES2=1`` to get
   this (there is no compile-time way for us to automatically know if
   you need it, sadly), and it adds client-side data. That means you can
   do glDrawArrays etc. without a bound buffer, and our GL bindings will
   set up the buffer automatically (since WebGL requires that a buffer
   be bound). This is not as efficient as the WebGL-friendly subset,
   since while we can bind buffers for you, we don't know what the
   optimal pattern of buffer creation/sizing/etc. is. But this option is
   very useful if you have a lot of code that is OpenGL ES 2.0.
-  **OpenGL emulation of older desktop and mobile versions**. This
   includes support for GL 1.x features like immediate mode and various
   commands like ``glNormalPointer`` etc. Emscripten support for such
   emulation is sufficient to run Sauerbraten (BananaBread project) and
   several other real-world codebases, but is definitely not complete in
   the sense of supporting all older GL features (which would be an
   enormous task). It also adds significant emulation overhead, so this
   is not recommended (but you might want to try it if your codebase
   currently requires GL 1.x). To enable it, compile with
   ``-s LEGACY_GL_EMULATION=1``.

Emulation levels
----------------

As mentioned above, we can directly support a subset of ES2 (the
"WebGL-friendly subset"), and have emulation for the rest of ES2, as
well as emulation for various desktop GL features.

By default we assume the WebGL-friendly subset. If you want ES2
emulation, use ``-s FULL_ES2=1``.

The desktop GL emulation can be enabled with
``-s LEGACY_GL_EMULATION=1``.

Settings for optimizing emulation code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When specifying ``-s LEGACY_GL_EMULATION=1``, there are a few extra
flags that can be used to tune the performance of the GL emulation
layer.

-  Setting ``-s GL_UNSAFE_OPTS=1`` tries to simplify the emulation by
   skipping GL work and cleanup that should be redundant. This is not
   guaranteed to be correct, so is not enabled by default.

-  Passing the option ``-s GL_FFP_ONLY=1`` tells the GL emulation layer
   that your code will not use the programmable pipeline/shaders at all.
   This allows the GL emulation code to perform extra optimizations when
   it knows that it is safe to do so.

-  You can add an int parameter ``Module.GL_MAX_TEXTURE_IMAGE_UNITS`` to
   your shell .html file to signal the GL emulation layer how many
   texture units your code is using at maximum. This avoids wasting
   clock cycles on iterating over unused texture units when examining
   which FFP emulation shader to run.

What is the "WebGL-friendly subset of OpenGL"?
----------------------------------------------

The WebGL-friendly subset of OpenGL is basically what maps directly to
WebGL, call to call. That includes almost all of OpenGL ES 2.0, except
for client-side arrays. The reason they are missing from WebGL is
because they are less efficient than properly using GPU-side data.
Similarly, if we emulated client-side arrays here, we would end up with
very bad performance, since we would need to upload the data on each
call to glDrawArrays etc. (Even if there are two calls one after the
other, we can't know the data did not change!) So in practical terms
full OpenGL ES 2.0 emulation would be convenient, but lead to bad
performance so in practice you would need to properly rewrite your code
to a WebGL-friendly subset anyhow.

Writing for the WebGL-friendly subset of OpenGL
-----------------------------------------------

See the files in tests/glbook and their git history for some simple
examples ported to that subset.

A very useful tool is the ``webgl.verbose`` option in Firefox. If you
compile code that uses clientside arrays, that option will give you a
warning when there isn't a bound buffer and so forth. It will also warn
you of other differences between OpenGL and WebGL.

What if I need additional GL features not supported yet?
--------------------------------------------------------

Feel free to file issues with testcases that do not work, and we will
look into them.
