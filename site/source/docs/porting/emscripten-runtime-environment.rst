.. _emscripten-runtime-environment:

==============================
Emscripten Runtime Environment
==============================

The Emscripten runtime environment is different to that expected by most C/C++ applications. Emscripten works hard to abstract and mitigate these differences, so that in general code can be compiled with little or no change.

This article expands on some of the differences and the resulting :ref:`api-limitations`, and outlines the few changes you may need to make to your C/C++ code.

Input/output
============

Emscripten implements the Simple DirectMedia Layer API (:term:`SDL`) for the browser environment, which provides low level access to audio, keyboard, mouse, joystick, and graphics hardware. Applications that use *SDL* typically require no input/output changes to run in the browser.

In addition, we have more limited support for *glut*, *glfw*, *glew* and *xlib*.

Applications that do not use *SDL* or the other APIs can use the Emscripten-specific APIs for input and output:

- :ref:`html5-h`, which defines the Emscripten low-level glue bindings to interact with HTML5 events from native code, including access to keys, mouse, wheel, device orientation, battery levels, vibration, etc.
- :ref:`multimedia-and-graphics-index` APIs, including :ref:`OpenGL <OpenGL-support>` and :ref:`EGL <egl-support-in-emscripten>`.


File Systems
============

A lot of C/C++ code uses the synchronous file system APIs in *libc* and *libcxx* to access code in the local file system. This is problematic because the browser prevents code from directly accessing files on the host system, and because JavaScript only supports asynchronous file access outside of web workers.

Emscripten provides an implementation of *libc* and *libcxx* and a *virtual file system* so that normal C/C++ code can be compiled and run without change. Most developers need only specify the set of files to be :ref:`packaged <packaging-files>` for preloading into the virtual file system at runtime.

.. note:: Using a virtual file system bypasses the limitations listed above. The file data is packaged at compile time and downloaded into the file system using *asynchronous* JavaScript APIs before the compiled code is allowed to run. The compiled code then makes "file" calls that are really just calls into program memory.

The default file system (:ref:`MEMFS <filesystem-api-memfs>`) stores files in-memory, so that any changes are lost when the page is reloaded. If file changes need to be stored more permanently then developers can mount the :ref:`IDBFS <filesystem-api-idbfs>` file system, which allows data to be persisted in the browser. When running code in *node.js*, developers can mount :ref:`NODEFS <filesystem-api-nodefs>` to give code direct access to the local file system.

Emscripten also has an API to support :ref:`asynchronous file access <emscripten-h-asynchronous-file-system-api>`.

For more information and examples see :ref:`packaging-code-index`.


.. _emscripten-runtime-environment-main-loop:

Browser main loop
=================

The browser event model uses co-operative multitasking — each event has a “turn” to run, and must then return control to the browser so that other events can be processed. A common cause of HTML pages hanging is JavaScript that does not complete and return control to the browser.

Graphical C++ apps typically run in an infinite loop. Within each iteration of the loop the app performs event handling, processing and rendering, followed by a delay ("wait") to keep the frame rate constant. This infinite loop is a problem in the browser environment because there is no way for control to return to the browser so other code can run. After a period the browser will notify the user that the page is stuck and offer to halt or close it.

Similarly, JavaScript APIs like WebGL can only run when the current "turn" is over, and will automatically render and swap buffers at that point. This contrasts with OpenGL C++ apps where you would need to swap the buffers manually.

.. _emscripten-runtime-environment-howto-main-loop:

Implementing an asynchronous main loop in C/C++
------------------------------------------------

The standard solution for this problem is to define a C function that performs one iteration of your main loop (not including the "delay"). For a native build this function can be called in an infinite loop, leaving the behaviour effectively unchanged.

Within Emscripten compiled code we use
:c:func:`emscripten_request_animation_frame_loop` to get the environment to call
this same function at the proper frequency for rendering a frame (that is, if
the browser renders at 60fps, it will call this 60 times per second). The
iteration is still run "infinitely" but now other code can run between
iterations and the browser does not hang.

Typically you will have a small section with ``#ifdef __EMSCRIPTEN__`` for the two cases. For example:

.. code-block:: cpp

  #include <emscripten.h>
  #include <emscripten/html5.h>
  #include <stdio.h>

  // Our "main loop" function. This callback receives the current time as
  // reported by the browser, and the user data we provide in the call to
  // emscripten_request_animation_frame_loop().
  bool one_iter(double time, void* userData) {
    // Can render to the screen here, etc.
    puts("one iteration");
    // Return true to keep the loop running.
    return true;
  }

  int main() {
  #ifdef __EMSCRIPTEN__
    // Receives a function to call and some user data to provide it.
    emscripten_request_animation_frame_loop(one_iter, 0);
  #else
    while (1) {
      one_iter();
      // Delay to keep frame rate constant (using SDL).
      SDL_Delay(time_to_next_frame());
    }
  #endif
  }

.. note:: A more full-featured API is provided in
          :c:func:`emscripten_set_main_loop`, which lets you specify the
          frequency at which to call the function, and other things.

.. note:: When using SDL you will often need to set the main loop, unless you
          are just rendering a single frame and halting. You should also note:

  - The current Emscripten implementation of ``SDL_QUIT`` will work if you use :c:func:`emscripten_set_main_loop`. As the page is shut, it will force a final direct call to the main loop, giving it a chance to notice the ``SDL_QUIT`` event. If you do not use a main loop, your app will close before you have had an opportunity to notice this event.
  - There are limitations to what you can do as the page shuts (in ``onunload``). Some actions like showing alerts are banned by browsers at this point.


Using Asyncify to yield to the browser
--------------------------------------

Another option is to use :ref:`Asyncify <yielding_to_main_loop>` which will
rewrite the program so that it can return to the browser's main event loop
by just calling ``emscripten_sleep()``. Note that this rewriting causes size and speed overhead
while ``emscripten_request_animation_frame_loop / emscripten_set_main_loop``
as described earlier do not.

Execution lifecycle
===================

When an Emscripten-compiled application is loaded, it starts by preparing data in the ``preloading`` phase. Files you marked for :ref:`preloading <emcc-preload-file>` (using ``emcc --preload-file``, or manually from JavaScript with :js:func:`FS.createPreloadedFile`) are set up at this stage.

You can add additional operations with :js:func:`addRunDependency`, which is a counter of all dependencies to be executed before compiled code can run. As these are completed you can call :js:func:`removeRunDependency` to remove the completed dependencies.

.. note:: Generally it is not necessary to add additional operations — preloading is suitable for almost all use cases.

When all dependencies are met, Emscripten will call ``run()``, which proceeds to call your ``main()`` function. The ``main()`` function should be used to perform initialization tasks, and will often call :c:func:`emscripten_set_main_loop` (as :ref:`described above <emscripten-runtime-environment-howto-main-loop>`). The main loop function will be then be called at the requested frequency.

You can affect the operation of the main loop in several ways:

-
  :c:func:`emscripten_push_main_loop_blocker` adds a function that **blocks** the main loop until the blocker completes.

  This is useful, for example, to manage loading new game levels. After a level completes, you can push blockers for each action involved (unpacking the file, generate the data structures, etc.) When all the blockers have completed the main loop will resume and the game should run the new level. You can also use this function in conjunction with :c:func:`emscripten_set_main_loop_expected_blockers` to keep the user informed of progress.

- :c:func:`emscripten_pause_main_loop` pauses the main loop, and :c:func:`emscripten_resume_main_loop` resumes it. These are low level (less recommended) alternatives to the blocker functions.

- :c:func:`emscripten_async_call` lets you call a function after some specific interval. This will use ``requestAnimationFrame`` (by default) or ``setTimeout`` if a specific interval was requested.

The :ref:`browser execution environment reference (emscripten.h) <emscripten-h-browser-execution-environment>` describes a number of other methods for controlling execution.


.. _emscripten-memory-model:

Emscripten memory representation
================================

In both asm.js and WebAssembly Emscripten represents memory in a way similar to native architectures. Pointers represent offsets into memory, structs use the same amount of address space as they normally do, and so forth.

In WebAssembly, this is done using a ``WebAssembly.Memory`` which was designed for that purpose. In asm.js, Emscripten uses a single `typed array <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Typed_arrays>`_, with different *views* providing access to different types (:js:data:`HEAPU32` for 32-bit unsigned integers, etc.).

Emscripten experimented with other memory representations in the past, ending up on the "typed arrays mode 2" approach for JS and then asm.js, as described above, and then WebAssembly implemented something similar.
