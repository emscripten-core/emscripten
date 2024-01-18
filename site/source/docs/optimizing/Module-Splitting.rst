.. _Module-Splitting:

================
Module Splitting
================

*wasm-split and the SPLIT_MODULE Emscripten integration are both in active
development and may change and gain new features frequently. This page will be
kept up-to-date with the latest changes.*

Large codebases often contain a lot of code that is very rarely used in practice
or is never used early in the application's life cycle. Loading that unused code
can noticeably delay application startup, so it would be good to defer loading
that code until after the application has already started. One excellent
solution for this is to use dynamic linking, but that requires refactoring an
application into shared libraries and also comes with some performance overhead,
so it is not always feasible. Module splitting is another approach where a
module is split into separate pieces, the primary and secondary modules, after
it is built normally. The primary module is loaded first and contains the code
necessary to start the application, while the secondary module contains code
that will be needed later or not at all. The secondary module will automatically
be loaded on demand.

wasm-split is a Binaryen tool that performs module splitting. After running
wasm-split, the primary module has all the same imports and exports as the
original module and is meant to be a drop-in replacement for it. However, it
also imports a placeholder function for each secondary function that was split
out into the secondary module. Before the secondary module is loaded, calls of
secondary functions will call the appropriate placeholder function instead. The
placeholder functions are responsible for loading and instantiating the
secondary module, which automatically replaces all the placeholder functions
with the original secondary functions when it is instantiated. After the
secondary module is loaded, the placeholder function that loaded it is also
responsible for calling its corresponding newly-loaded secondary function and
returning the result to its caller. The loading of the secondary module is
therefore completely transparent to the primary module; it just looks like a
function call took a long time to return.

Currently the only workflow for splitting modules involves instrumenting the
original module to collect a profile of what functions are run, running that
instrumented module with a number of interesting workloads, and using the
resulting profiles to determine how to split the module. wasm-split will leave
any function that was run during any of the profiled workloads in the primary
module and will split all other functions out into the secondary module.

Emscripten has a prototype integration with wasm-split enabled by the
``-sSPLIT_MODULE`` option. This option will emit the original module with the
wasm-split instrumentation applied so it is ready to collect profiles. It will
also insert the placeholder functions responsible for loading a secondary module
into the emitted JS. The developer is then responsible for running appropriate
workloads, collecting the profiles, and using the wasm-split tool to perform the
splitting. After the module is split, everything will work correctly with no
further changes to the JS produced by the initial compilation.

Basic Example
-------------

Let’s run through a basic example of using SPLIT_MODULE with Node. Later in the
"Running on the Web" section we will discuss how to adapt the example to run on
the Web as well.

Here’s our application code::

  // application.c

  #include <stdio.h>
  #include <emscripten.h>

  void foo() {
    printf("foo\n");
  }

  void bar() {
    printf("bar\n");
  }

  void unsupported(int i) {
    printf("%d is not supported!\n", i);
  }

  EM_JS(int, get_number, (), {
    if (typeof prompt === 'undefined') {
      prompt = require('prompt-sync')();
    }
    return parseInt(prompt('Give me 0 or 1: '));
  });

  int main() {
    int i = get_number();
    if (i == 0) {
      foo();
    } else if (i == 1) {
      bar();
    } else {
      unsupported(i);
    }
  }

This application prompts the user for some input and executes different
functions depending on what the user provides. It uses the prompt-sync npm
module to make the prompting behavior portable between Node and the Web. We will
see that the input we provide during profiling will determine how our functions
are split between the primary and secondary modules.

We can compile our application with ``-sSPLIT_MODULE``::

  $ emcc application.c -o application.js -sSPLIT_MODULE

In addition to the typical application.wasm and application.js files, this also
produces an application.wasm.orig file. application.wasm.orig is the original,
unmodified module that a normal Emscripten build would produce, while
application.wasm has been instrumented by wasm-split to collect profiles.

The instrumented module has an additional exported function,
``__write_profile``, that takes as arguments a pointer and length for an
in-memory buffer to which it will write the profile. ``__write_profile`` returns
the length of the profile, and only writes the data if the supplied buffer is
large enough. ``__write_profile`` can be called externally from JS or
internally, from the application itself. For simplicity, we will just call it at
the end of our main function here, but note that this will mean that any
functions called after main, such as destructors for global objects, will not be
included in the profile.

Here’s the function to write the profile and our new main function::

  EM_JS(void, write_profile, (), {
    var __write_profile = wasmExports.__write_profile;
    if (!__write_profile) {
      return;
    }

    // Get the size of the profile and allocate a buffer for it.
    var len = __write_profile(0, 0);
    var ptr = _malloc(len);

    // Write the profile data to the buffer.
    __write_profile(ptr, len);

    // Write the profile file.
    var profile_data = HEAPU8.subarray(ptr, ptr + len);
    const fs = require("fs");
    fs.writeFileSync('profile.data', profile_data);

    // Free the buffer.
    _free(ptr);
  });

  int main() {
    int i = get_number();
    if (i == 0) {
      foo();
    } else if (i == 1) {
      bar();
    } else {
      unsupported(i);
    }
    write_profile();
  }

Note that we only try to write the profile if the ``__write_profile`` export
exists. This is important because only the instrumented, unsplit module exports
``__write_profile``. The split modules will not include the profiling
instrumentation or this export.

Our new write_profile function depends on malloc and free being available to JS,
so we need to explicitly export them on the command line::

  $ emcc application.c -o application.js -sSPLIT_MODULE -sEXPORTED_FUNCTIONS=_malloc,_free,_main

Now we can run our application, which produces a profile.data file. The next
step is to use wasm-split and the profile to split the original module,
application.wasm::

  $ wasm-split --enable-mutable-globals --export-prefix=% application.wasm.orig -o1 application.wasm -o2 application.deferred.wasm --profile=profile.data

Let’s break down what all those options are for.

``--enable-mutable-globals``
  This option enables the mutable-global target feature, which allows mutable
  Wasm globals (as opposed to C/C++ globals) to be imported and exported.
  wasm-split has to share mutable globals between the primary and secondary
  modules, so it requires this feature to be enabled.

``--export-prefix=%``
  This is a prefix added to all the new exports wasm-split creates to share
  module elements from the primary module to the secondary module. The prefix
  can be used to differentiate "true" exports from those that only exist to be
  consumed by the secondary module. Emscripten’s wasm-split integration expects
  “%” in particular to be used as the prefix.

``-o1 application.wasm``
  Write the primary module to application.wasm. Note that this will overwrite
  the instrumented module previously produced by Emscripten, so the application
  will now use the split modules rather than the instrumented module.

``-o2 application.deferred.wasm``
  Write the secondary module to application.deferred.wasm. Emscripten expects
  the name of the secondary module to be the same as the name of the primary
  module with “.wasm” replaced with “.deferred.wasm”.

``--profile=profile.data``
  Directs wasm-split to use the profile in profile.data to guide the splitting.

Running application.js in node again, we can see that the application works just
as it did before, but if we execute any code path besides the one used in the
profiled workload, the application will print a console message about a
placeholder function being called and the deferred module being loaded.

Profiling Multiple Workloads
----------------------------

wasm-split supports merging profiles from multiple profiling workloads into a
single profile to guide splitting. Any function that was run in any of the
workloads will be kept in the primary module and all other functions will be
split out into the secondary module.

This command will merge any number of profiles (here just profile1.data and
profile2.data) into a single profile::

  $ wasm-split --merge-profiles profile1.data profile2.data -o profile.data

Multithreaded Programs
----------------------

By default, the data gathered by the wasm-split instrumentation is stored in
Wasm globals, so it is thread local. But in a multithreaded program, it is
important to collect profile information from all threads. To do so, you can
tell wasm-split to collect shared profile information in shared memory using the
``--in-memory`` wasm-split flag. This will use memory starting at address zero
to store the profile information, so you must also pass ``-sGLOBAL_BASE=N`` to
Emscripten, where ``N`` is at least the number of functions in the module, to
prevent the program from clobbering that memory region.

After splitting, multithreaded applications will currently fetch and compile the
secondary module separately on each thread. The compiled secondary module is not
postmessaged to each thread the way the Emscripten postmessages the primary
module to the threads. This is not as bad as it sounds because downloads of the
secondary module from workers will be serviced from the cache if the appropriate
Cache-Control headers are set, but improving this is an area for future work.

Running on the Web
------------------

One complication to keep in mind when using SPLIT_MODULE for Web applications is
that the secondary module cannot be loaded both lazily and asynchronously, which
means it cannot be loaded lazily on the main browser thread. The reason is that
the placeholder functions need to be completely transparent to the functions in
the primary module, so they can’t return until they have synchronously loaded
and called the correct secondary function.

One workaround for this limitation would be to eagerly load and instantiate the
secondary module and ensure that no secondary functions can possibly be called
before it has been instantiated on the main browser thread. This may be
difficult to ensure, though. Another fix would be to run the Asyncify
transformation on the primary module to allow placeholder functions to return to
the JS event loop while waiting for the secondary module to load asynchronously.
This is on the wasm-split roadmap, although we do not yet know what the size and
performance overhead of this solution will be.

This limitation on lazy loading means that the best way to run applications with
SPLIT_MODULE is in a worker thread, for example using ``-sPROXY_TO_PTHREAD``. In
PROXY_TO_PTHREAD mode, it is important to collect a profile for the browser main
thread in addition to the application main thread because the browser main
thread runs some functions not run in the application main thread, such as the
shim that wraps the proxied main function and the functions involved in handling
calls proxied back to the main browser thread. See the previous section for how
to collect profiles from multiple threads.

Another minor complication is that the profile data cannot be immediately
written to a file from inside the browser. The data must instead be transmitted
to developer machines some other way, such as posting it to the dev server or
copying a base64 encoding of it from the console.

Here’s code implementing the base64 solution::

  var profile_data = HEAPU8.subarray(ptr, ptr + len);
  var binary = '';
  for (var i = 0; i < profile_data.length; i++) {
      binary += String.fromCharCode(profile_data[i]);
  }
  console.log("===BEGIN===");
  console.log(window.btoa(binary));
  console.log("===END===");

Then the profile file can be created by by running::

  $ echo [pasted base64] | base64 --decode > profile.data

or::

  $ base64 --decode [base64 file] > profile.data

Usage with Dynamic Linking
--------------------------

Module splitting can be used in conjunction with dynamic linking, but making the
two features work correctly together requires some developer intervention.
wasm-split often needs to grow the table to make space for placeholder
functions, but that means that the instrumented and split modules would have
different table sizes. Normally this is not a problem, but
MAIN_MODULE/SIDE_MODULE dynamic linking support currently requires the table
size to be baked into the JS Emscripten emits, so the table size needs to be
stable.

To ensure that the table size is the same between the instrumented module and
the split modules, use the ``-sINITIAL_TABLE=N`` Emscripten setting, where ``N``
is the desired table size. Then, when using wasm-split to perform the splitting,
pass ``--initial-table=N`` to wasm-split to ensure that the split modules have
the correct table size as well.

If the specified table size is too small, you will get an error message when the
primary module is loaded after splitting. Adjust the table size you specify
until it is large enough. Besides taking up extra space at runtime, there is no
downside to specifying a table size that is larger than necessary.

Custom Loading of the Secondary Module
--------------------------------------

The default logic for lazily loading the secondary module can be overridden by
implementing the "loadSplitModule" custom hook function. The hook is called from
placeholder functions and is responsible for returning the [instance, module]
pair for the secondary module. The hook takes as arguments the name of the file
to load (e.g. “my_program.deferred.wasm”), the imports object to instantiate the
module with, and the property corresponding to the called placeholder function.
Here is an example implementation that does the same thing as the default
implementation with some extra logging::

  Module["loadSplitModule"] = function(deferred, imports, prop) {
      console.log('Custom handler for loading split module.');
      console.log('Called with placeholder ', prop);

      return instantiateSync(deferred, imports);
  }

If the module was eagerly loaded, then this hook could simply instantiate the
module rather than fetching and compiling it as well. However, if the eagerly
loaded module is instantiated eagerly as well, the placeholder functions will be
patched out and never called in the first place, so this custom hook will never
be called either.

When eagerly instantiating the secondary module, the imports object should be::

  {'primary': wasmExports}

Debugging
---------

wasm-split has several options to make debugging split modules easier.

``-v``
  When splitting, print the primary and secondary functions. When merging
  profiles, print profiles that do not contribute to the merged profile.

``-g``
  Preserve names in both the primary and secondary modules. Without this option,
  wasm-split will strip the names instead.

``--emit-module-names``
  Generate and emit module names to differentiate the primary and secondary
  module in stack traces, even if -g is not used.

``--symbolmap``
  Emit separate map files for the primary and secondary modules, mapping
  function indices to function names. When combined with --emit-module-names,
  these maps can be used to re-symbolify stack traces. To ensure that the
  function names are available for wasm-split to emit into the maps,
  pass --profiling-funcs to Emscripten.

``--placeholdermap``
  Emit a map file mapping placeholder function indices to their corresponding
  secondary functions. This can be useful for figuring out what function caused
  the secondary module to be loaded.


Upcoming Changes
----------------

*A list of changes and new features that have not yet been incorporated into
this documentation.*

Work is planned on an integration with the Asyncify instrumentation that will
allow the secondary module to be asynchronously loaded on the main browser
thread.
