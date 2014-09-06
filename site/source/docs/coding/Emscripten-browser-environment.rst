.. _Emscripten-browser-environment:

============================================
Emscripten browser environment (wiki-import)
============================================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

The browser environment is different than the environment a normal C/C++ application expects. The main differences are how input and output work, and the fact that the main loop must be asynchronous.

For input and output, if you use SDL etc. then everything should basically be taken care of for you. Emscripten implements the familiar SDL API for the browser environment. Typically you should need to refactor nothing or a very small amount of code for this.

The main loop being asynchronous, however, is trickier.

Browser main loop
============================================

Graphical C++ apps typically have a main loop that is an infinite loop, in which event handling is done, processing and rendering, then a wait (SDL\_Delay, for example) to keep the frame rate right. However, in JS there is no way for something like SDL\_Delay to actually return control to the browser event loop, and returning control is important because the browser event model is asynchronous - your JavaScript must finish its "turn", by completing execution and returning control to the browser itself. The browser will then asynchronously call your JavaScript later according to how you asked it to do that.

This is inherent in how browsers work. If you do not finish your turn, the page will 'hang' and the browser will eventually tell the user the page is stuck and offer to halt it or close it. Also, that things like WebGL will only actually render when your JS "turn" is over - so while a normal C++ GL app would swap buffers manually, in JS you just finish your turn and the browser renders and swaps.

Implementing an asynchronous main loop in C/C++
===============================================

The standard way to do this is make a C function that runs one iteration of your main loop. Then call it from JS at the proper frequency. This is very simple to do manually (just call it from JS, all you need is an underscore at the beginning of the name), but you can also use :c:func:`emscripten_set_main_loop` (see emscripten.h) for something a little more convenient. For a regular native build, you can just call that function in an infinite loop (likely with SDL\_Delay etc.), so typically you will have a small section with ``#ifdef EMSCRIPTEN`` for the two cases, for example

::

    int main() {
      ...
    #ifdef EMSCRIPTEN
      // void emscripten_set_main_loop(void (*func)(), int fps, int simulate_infinite_loop);
      emscripten_set_main_loop(one_iter, 60, 1);
    #else
      while (1) {
        one_iter();
        SDL_Delay(time_to_next_frame());
      }
    #endif
    }

    void one_iter() {
      // process input
      // render to screen
    }

:c:func:`emscripten_set_main_loop` is documented in ``emscripten.h`` (under ``system/include/emscripten/``). There are also several other useful functions there. As mentioned above, we use existing familiar APIs when present, like SDL, so the things in the additional Emscripten API in that header are capabilities specific to the JS or the browser environment, that we couldn't find an existing API for. Documentation for the Emscripten API is all in that file. Below is a general overview of some important parts.

Execution lifecycle
===================

When an emscripten-compiled application is loaded, it starts by with preparing data in the ``preloading`` phase. Files you marked for preloading (``emcc --preload-file``, or manually from JS using createPreloadedFile) are set up at this stage. You can add additional operations with :js:func:`addRunDependency` (remember to call :js:func:`removeRunDependency`), which is a counter of all dependencies preventing run. When all dependencies are met, we will call ``run``, which proceeds to call your ``main()`` function.

After the application runs, as mentioned above you will typically want to define a main loop. You can affect how the main loop works in several ways:

- :c:func:`emscripten_pause_main_loop` pauses the main loop, and ``emscripten_resume_main_loop`` resumes it. This might be useful if you want to manually freeze the application for some reason. This is fairly low-level, there are more convenient alternatives below for common tasks.
- ``emscripten_push_main_loop_blocker`` adds a function that **blocks** the main loop: The main loop will not run until the blocker completes. For example, if when the user finishes a level in your game you want to load the next one, you can push blockers for each action involved in that (unpack the file, generate the data structures, etc.), and when they are done the main loop will resume (and if things are set up properly, it will run the new level). The blockers are a simple FIFO queue. Basically, blockers are a small simplification of manually pausing and resuming the main loop. They also are useful for marking progress: If you all ``emscripten_set_main_loop_expected_blockers(10)`` and then push 10 blockers, progress will be shown to the user as the blockers execute.
- :c:func:`emscripten_async_call` lets you call a function after some specific interval (basically a wrapper around ``setTimout/requestAnimationFrame``).
- :c:func:`emscripten_async_wget` asynchronously loads a file from the network. Synchronous XHRs cannot load binary data, so an asynchronous function is necessary. It will call a callback that you give it when the file arrives. You can use this to fetch the next level in your game, for example (note that it will do the same operation we do on preloaded files, setting them up as image or audio elements as necessary for execution later).

Notes
=====

- SDL_QUIT is tricky to implement in browsers. The current Emscripten implementation of it will work if you use :c:func:`emscripten_set_main_loop`: As the page is shut, it will force a final direct call to the main loop, giving it a chance to notice the SDL_QUIT event. So if you do not use a main loop, you will not notice it - your app will close before your next event handling. Note also that there are limitations on what you can do as the page shuts (in onunload), some actions like showing alerts are banned by browsers.

