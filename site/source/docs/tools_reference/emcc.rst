.. _emccdoc:

===================================
Emscripten Compiler Frontend (emcc)
===================================

The Emscripten Compiler Frontend (``emcc``) is used to call the Emscripten compiler from the command line. It is effectively a drop-in replacement for a standard compiler like *gcc* or *clang*.


Command line syntax
===================

::

  emcc [options] file...

(Note that you will need ``./emcc`` if you want to run emcc from your current directory.)

The input file(s) can be either source code files that *Clang* can handle (C or
C++), object files (produced by `emcc -c`), or LLVM assembly files.


Arguments
---------

Most `clang options <http://linux.die.net/man/1/clang>`_ will work, as will `gcc options <https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html#Option-Summary>`_, for example: ::

  # Display this information
  emcc --help

  # Display compiler version information
  emcc --version


To see the full list of *Clang* options supported on the version of *Clang* used by Emscripten, run ``clang --help``.

Options that are modified or new in *emcc* are listed below:

.. _emcc-compiler-optimization-options:

.. _emcc-O0:

``-O0``
  [compile+link]
  No optimizations (default). This is the recommended setting for starting to port a project, as it includes various assertions.

  This and other optimization settings are meaningful both during compile and
  during link. During compile it affects LLVM optimizations, and during link it
  affects final optimization of the code in Binaryen as well as optimization of
  the JS. (For fast incremental builds ``-O0`` is best, while for release you
  should link with something higher.)

.. _emcc-O1:

``-O1``
  [compile+link]
  Simple optimizations. During the compile step these include LLVM ``-O1`` optimizations. During the link step this does not include various runtime assertions in JS that `-O0` would do.

.. _emcc-O2:

``-O2``
  [compile+link]
  Like ``-O1``, but enables more optimizations. During link this will also enable various JavaScript optimizations.

  .. note:: These JavaScript optimizations can reduce code size by removing things that the compiler does not see being used, in particular, parts of the runtime may be stripped if they are not exported on the ``Module`` object. The compiler is aware of code in :ref:`--pre-js <emcc-pre-js>` and :ref:`--post-js <emcc-post-js>`, so you can safely use the runtime from there. Alternatively, you can use ``EXPORTED_RUNTIME_METHODS``, see `src/settings.js <https://github.com/emscripten-core/emscripten/blob/main/src/settings.js>`_.

.. _emcc-O3:

``-O3``
  [compile+link]
  Like ``-O2``, but with additional optimizations that may take longer to run.

  .. note:: This is a good setting for a release build.

.. _emcc-Og:

``-Og``
  [compile+link]
  Like ``-O1``. In future versions, this option might disable different
  optimizations in order to improve debuggability.

.. _emcc-Os:

``-Os``
  [compile+link]
  Like ``-O3``, but focuses more on code size (and may make tradeoffs with speed). This can affect both Wasm and JavaScript.

.. _emcc-Oz:

``-Oz``
  [compile+link]
  Like ``-Os``, but reduces code size even further, and may take longer to run. This can affect both Wasm and JavaScript.

  .. note:: For more tips on optimizing your code, see :ref:`Optimizing-Code`.

.. _emcc-s-option-value:

``-sOPTION[=VALUE]``
  [different OPTIONs affect at different stages, most at link time]
  Emscripten build options. For the available options, see `src/settings.js <https://github.com/emscripten-core/emscripten/blob/main/src/settings.js>`_.

  .. note:: If no value is specified it will default to ``1``.

  .. note:: It is possible, with boolean options, to use the ``NO_`` prefix to reverse their meaning. For example, ``-sEXIT_RUNTIME=0`` is the same as ``-sNO_EXIT_RUNTIME=1`` and vice versa.  This is not recommended in most cases.

  .. note:: Lists can be specified as comma separated strings:

    ::

      -sEXPORTED_FUNCTIONS=foo,bar

  .. note:: We also support older list formats that involve more quoting.  Lists can be specified with or without quotes around each element and with or without brackets around the list.  For example, all the following are equivalent:

    ::

      -sEXPORTED_FUNCTIONS="foo","bar"
      -sEXPORTED_FUNCTIONS=["foo","bar"]
      -sEXPORTED_FUNCTIONS=[foo,bar]

  .. note:: For lists that include brackets or quote, you need quotation marks (") around the list in most shells (to avoid errors being raised). Two examples are shown below:

    ::

      -sEXPORTED_FUNCTIONS="['liblib.so']"
      -s"EXPORTED_FUNCTIONS=['liblib.so']"

  You can also specify that the value of an option will be read from a file. For example, the following will set ``EXPORTED_FUNCTIONS`` based on the contents of the file at **path/to/file**.

  ::

    -sEXPORTED_FUNCTIONS=@/path/to/file

  .. note::

    - In this case the file should contain a list of symbols, one per line.  For legacy use cases JSON-formatted files are also supported: e.g. ``["_func1", "func2"]``.
    - The specified file path must be absolute, not relative.
    - The file may contain comments where the first character of the line is ``'#'``.


  .. note:: Options can be specified as a single argument with or without a space
            between the ``-s`` and option name.  e.g. ``-sFOO`` or ``-s FOO``.
            It's `highly recommended <https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-specify-s-options-in-a-cmake-project>`_ you use the notation without space.

.. _emcc-g:

``-g``
  [compile+link]
  Preserve debug information.

  - When compiling to object files, this is the same as in *Clang* and *gcc*, it
    adds DWARF debug information to the object files.
  - When linking, this is equivalent to :ref:`-g3 <emcc-g3>`.

.. _emcc-gseparate-dwarf:

``-gseparate-dwarf[=FILENAME]``
  [same as -g3 if passed at compile time, otherwise applies at link]
  Preserve debug information, but in a separate file on the side. This is the
  same as ``-g``, but the main file will contain no debug info. Instead, debug
  info will be present in a file on the side, in ``FILENAME`` if provided,
  otherwise the same as the Wasm file but with suffix ``.debug.wasm``. While
  the main file contains no debug info, it does contain a URL to where the
  debug file is, so that devtools can find it. You can use
  ``-sSEPARATE_DWARF_URL=URL`` to customize that location (this is useful if
  you want to host it on a different server, for example).

.. _emcc-gsplit-dwarf:

``-gsplit-dwarf``
  Enable debug fission, which creates split DWARF object files alongside the
  wasm object files. This option must be used together with ``-c``.

.. _emcc-gsource-map:

``-gsource-map``
  [link]
  Generate a source map using LLVM debug information (which must
  be present in object files, i.e., they should have been compiled with ``-g``).
  When this option is provided, the **.wasm** file is updated to have a
  ``sourceMappingURL`` section. The resulting URL will have format:
  ``<base-url>`` + ``<wasm-file-name>`` + ``.map``. ``<base-url>`` defaults
  to being empty (which means the source map is served from the same directory
  as the Wasm file). It can be changed using :ref:`--source-map-base <emcc-source-map-base>`.

.. _emcc-gN:

``-g<level>``
  [compile+link]
  Controls the level of debuggability. Each level builds on the previous one:

    -
      .. _emcc-g0:

      ``-g0``: Make no effort to keep code debuggable.

    -
      .. _emcc-g1:

      ``-g1``: When linking, preserve whitespace in JavaScript.

    -
      .. _emcc-g2:

      ``-g2``: When linking, preserve function names in compiled code.

    -
      .. _emcc-g3:

      ``-g3``: When compiling to object files, keep debug info, including JS whitespace, function names, and LLVM debug info (DWARF) if any (this is the same as :ref:`-g <emcc-g>`).

.. _emcc-profiling:

``--profiling``
  [same as -g2 if passed at compile time, otherwise applies at link]
  Use reasonable defaults when emitting JavaScript to make the build readable but still useful for profiling. This sets ``-g2`` (preserve whitespace and function names) and may also enable optimizations that affect performance and otherwise might not be performed in ``-g2``.

.. _emcc-profiling-funcs:

``--profiling-funcs``
  [link]
  Preserve function names in profiling, but otherwise minify whitespace and names as we normally do in optimized builds. This is useful if you want to look at profiler results based on function names, but do *not* intend to read the emitted code.

``--tracing``
  [link]
  Enable the :ref:`Emscripten Tracing API <trace-h>`.

``--reproduce=<file.tar>``
  [compile+link]
  Write tar file containing inputs and command to reproduce invocation.  When
  sharing this file be aware that it will any object files, source files and
  libraries that that were passed to the compiler.

.. _emcc-emit-symbol-map:

``--emit-symbol-map``
  [link]
  Save a map file between function indexes in the Wasm and function names. By
  storing the names on a file on the side, you can avoid shipping the names, and
  can still reconstruct meaningful stack traces by translating the indexes back
  to the names.

  .. note:: When used with ``-sWASM=2``, two symbol files are created. ``[name].js.symbols`` (with WASM symbols) and ``[name].wasm.js.symbols`` (with ASM.js symbols)

.. _emcc-emit-minification-map:

``--emit-minification-map <file>``
  [link]
  In cases where emscripten performs import/export minificiton this option can
  be used to output a file that maps minified names back to their original
  names.  The format of this file is single line per import/export of the form
  ``<minname>:<origname>``.

.. _emcc-lto:

``-flto``
  [compile+link]
  Enables link-time optimizations (LTO).

.. _emcc-closure:

``--closure 0|1|2``
  [link]
  Runs the :term:`Closure Compiler`. Possible values are:

    - ``0``: No closure compiler (default in ``-O2`` and below).
    - ``1``: Run closure compiler. This greatly reduces the size of the support JavaScript code (everything but the WebAssembly or asm.js). Note that this increases compile time significantly.
    - ``2``: Run closure compiler on *all* the emitted code, even on **asm.js** output in **asm.js** mode. This can further reduce code size, but does prevent a significant amount of **asm.js** optimizations, so it is not recommended unless you want to reduce code size at all costs.

  .. note::

    - Consider using ``-sMODULARIZE`` when using closure, as it minifies globals to names that might conflict with others in the global scope. ``MODULARIZE`` puts all the output into a function (see ``src/settings.js``).
    - Closure will minify the name of `Module` itself, by default! Using ``MODULARIZE`` will solve that as well. Another solution is to make sure a global variable called `Module` already exists before the closure-compiled code runs, because then it will reuse that variable.
    - Closure is only run if JavaScript opts are being done (``-O2`` or above).

``--closure-args=<args>``
   [link]
   Pass arguments to the :term:`Closure compiler`. This is an alternative to ``EMCC_CLOSURE_ARGS``.

   For example, one might want to pass an externs file to avoid minifying JS functions defined in ``--pre-js`` or ``--post-js`` files.
   To pass to Closure the ``externs.js`` file containing those public APIs that should not be minified, one would add the flag:
   ``--closure-args=--externs=path/to/externs.js``

.. _emcc-pre-js:

``--pre-js <file>``
  [link]
  Specify a file whose contents are added before the emitted code and optimized together with it. Note that this might not literally be the very first thing in the JS output, for example if ``MODULARIZE`` is used (see ``src/settings.js``). If you want that, you can just prepend to the output from emscripten; the benefit of ``--pre-js`` is that it optimizes the code with the rest of the emscripten output, which allows better dead code elimination and minification, and it should only be used for that purpose. In particular, ``--pre-js`` code should not alter the main output from emscripten in ways that could confuse the optimizer, such as using ``--pre-js`` + ``--post-js`` to put all the output in an inner function scope (see ``MODULARIZE`` for that).

  `--pre-js` (but not `--post-js`) is also useful for specifying things on the ``Module`` object, as it appears before the JS looks at ``Module`` (for example, you can define ``Module['print']`` there).

.. _emcc-post-js:

``--post-js <file>``
  [link]
  Like ``--pre-js``, but emits a file *after* the emitted code.

``--extern-pre-js <file>``
  [link]
  Specify a file whose contents are prepended to the JavaScript output. This
  file is prepended to the final JavaScript output, *after* all other
  work has been done, including optimization, optional ``MODULARIZE``-ation,
  instrumentation like ``SAFE_HEAP``, etc. This is the same as prepending
  this file after ``emcc`` finishes running, and is just a convenient
  way to do that. (For comparison, ``--pre-js`` and ``--post-js`` optimize the
  code together with everything else, keep it in the same scope if running
  `MODULARIZE`, etc.).

``--extern-post-js <file>``
  [link]
  Like ``--extern-pre-js``, but appends to the end.

.. _emcc-embed-file:

``--embed-file <file>``
  [link]
  Specify a file (with path) to embed inside the generated WebAssembly module.
  The path is relative to the current directory at compile time. If a directory
  is passed here, its entire contents will be embedded.

  For example, if the command includes ``--embed-file dir/file.dat``, then
  ``dir/file.dat`` must exist relative to the directory where you run *emcc*.

  .. note:: Embedding files is generally more efficient than :ref:`preloading
     <emcc-preload-file>` as it avoids copying the file data at runtime.

  For more information about the ``--embed-file`` options, see
  :ref:`packaging-files`.

.. _emcc-preload-file:

``--preload-file <name>``
  [link]
  Specify a file to preload before running the compiled code asynchronously. The
  path is relative to the current directory at compile time. If a directory is
  passed here, its entire contents will be embedded.

  Preloaded files are stored in **filename.data**, where **filename.html** is
  the main file you are compiling to. To run your code, you will need both the
  **.html** and the **.data**.

  .. note:: This option is similar to :ref:`--embed-file <emcc-embed-file>`,
     except that it is only relevant when generating HTML (it uses asynchronous
     binary :term:`XHRs <XHR>`), or JavaScript that will be used in a web page.

  *emcc* runs `tools/file_packager
  <https://github.com/emscripten-core/emscripten/blob/main/tools/file_packager.py>`_
  to do the actual packaging of embedded and preloaded files. You can run the
  file packager yourself if you want (see :ref:`packaging-files-file-packager`).
  You should then put the output of the file packager in an emcc ``--pre-js``,
  so that it executes before your main compiled code.

  For more information about the ``--preload-file`` options, see
  :ref:`packaging-files`.

.. _emcc-exclude-file:

``--exclude-file <name>``
  [link]
  Files and directories to be excluded from :ref:`--embed-file <emcc-embed-file>` and :ref:`--preload-file <emcc-preload-file>`. Wildcards (*) are supported.

``--use-preload-plugins``
  [link]
  Tells the file packager to run preload plugins on the files as they are loaded. This performs tasks like decoding images and audio using the browser's codecs.

.. _emcc-shell-file:

``--shell-file <path>``
  [link]
  The path name to a skeleton HTML file used when generating HTML output. The shell file used needs to have this token inside it: ``{{{ SCRIPT }}}``.

  .. note::

    - See `src/shell.html <https://github.com/emscripten-core/emscripten/blob/main/src/shell.html>`_ and `src/shell_minimal.html <https://github.com/emscripten-core/emscripten/blob/main/src/shell_minimal.html>`_ for examples.
    - This argument is ignored if a target other than HTML is specified using the ``-o`` option.

.. _emcc-source-map-base:

``--source-map-base <base-url>``
  [link]
  The base URL for the location where WebAssembly source maps will be published. Must be used
  with :ref:`-gsource-map <emcc-gsource-map>`.

.. _emcc-minify:

``--minify 0``
  [same as -g1 if passed at compile time, otherwise applies at link]
  Identical to ``-g1``.

``--js-transform <cmd>``
  [link]
  Specifies a ``<cmd>`` to be called on the generated code before it is optimized. This lets you modify the JavaScript, for example adding or removing some code, in a way that those modifications will be optimized together with the generated code.

  ``<cmd>`` will be called with the file name of the generated code as a parameter. To modify the code, you can read the original data and then append to it or overwrite it with the modified data.

  ``<cmd>`` is interpreted as a space-separated list of arguments, for example, ``<cmd>`` of **python processor.py** will cause a Python script to be run.

.. _emcc-bind:

``--bind``
  [link]
  Links against embind library.  Deprecated: Use ``-lembind`` instead.

.. _emcc-embind-emit-tsd:

``--embind-emit-tsd <path>``
  [link]
  Generates TypeScript definition file.  Deprecated: Use ``--emit-tsd`` instead.

.. _emcc-emit-tsd:

``--emit-tsd <path>``
  [link]
  Generate a TypeScript definition file for the emscripten module. The definition
  file will include exported Wasm functions, runtime exports, and exported
  embind bindings (if used). In order to generate bindings from embind, the
  program will be instrumented and run in node.

``--ignore-dynamic-linking``
  [link]
  Tells the compiler to ignore dynamic linking (the user will need to manually link to the shared libraries later on).

  Normally *emcc* will simply link in code from the dynamic library as though it were statically linked, which will fail if the same dynamic library is linked more than once. With this option, dynamic linking is ignored, which allows the build system to proceed without errors.

.. _emcc-js-library:

``--js-library <lib>``
  [link]
  A JavaScript library to use in addition to those in Emscripten's core libraries (src/library_*).

.. _emcc-verbose:

``-v``
  [general]
  Turns on verbose output.

  This will print the internal sub-commands run by emscripten as well as ``-v``
  to *Clang*.

  .. tip:: ``emcc -v`` is a useful tool for diagnosing errors. It works with or without other arguments.

``--check``
  [general]
  Runs Emscripten's internal sanity checks and reports any issues with the
  current configuration.

.. _emcc-cache:

``--cache <directory>``
  [general]
  Sets the directory to use as the Emscripten cache. The Emscripten cache
  is used to store pre-built versions of ``libc``, ``libcxx`` and other
  libraries.

  If using this in combination with ``--clear-cache``, be sure to specify
  this argument first.

  The Emscripten cache defaults to ``emscripten/cache`` but can be overridden
  using the ``EM_CACHE`` environment variable or ``CACHE`` config setting.

.. _emcc-clear-cache:

``--clear-cache``
  [general]
  Manually clears the cache of compiled Emscripten system libraries (libc++,
  libc++abi, libc).

  This is normally handled automatically, but if you update LLVM in-place
  (instead of having a different directory for a new version), the caching
  mechanism can get confused. Clearing the cache can fix weird problems related
  to cache incompatibilities, like *Clang* failing to link with library files.
  This also clears other cached data. After the cache is cleared, this process
  will exit.

  By default this will also clear any download ports since the ports directory
  is usually within the cache directory.

.. _emcc-use-port:

``--use-port=<port>``
  [compile+link]
  Use the specified port. If you need to use more than one port you can use
  this option multiple times (ex: ``--use-port=sdl2 --use-port=bzip2``). A port
  can have options separated by ``:``
  (ex: ``--use-port=sdl2_image:formats=png,jpg``). To use an  external port,
  you provide the path to the port directly
  (ex: ``--use-port=/path/to/my_port.py``). To get more information about a
  port, use the ``help`` option (ex: ``--use-port=sdl2_image:help``).
  To get the list of available ports, use ``--show-ports``.

.. _emcc-clear-ports:

``--clear-ports``
  [general]
  Manually clears the local copies of ports from the Emscripten Ports repos
  (sdl2, etc.). This also clears the cache, to remove their builds.

  You should only need to do this if a problem happens and you want all ports
  that you use to be downloaded and built from scratch. After this operation is
  complete, this process will exit.

.. _emcc-show-ports:

``--show-ports``
  [general]
  Shows the list of available projects in the Emscripten Ports repos. After this operation is complete, this process will exit.

``-Wwarn-absolute-paths``
  [compile+link]
  Enables warnings about the use of absolute paths in ``-I`` and ``-L`` command line directives. This is used to warn against unintentional use of absolute paths, which is sometimes dangerous when referring to nonportable local system headers.

.. _proxy-to-worker:

``--proxy-to-worker``
  [link]
  Runs the main application code in a worker, proxying events to it and output from it. If emitting HTML, this emits a **.html** file, and a separate **.js** file containing the JavaScript to be run in a worker. If emitting JavaScript, the target file name contains the part to be run on the main thread, while a second **.js** file with suffix ".worker.js" will contain the worker portion.

.. _emcc-emrun:

``--emrun``
  [link]
  Enables the generated output to be aware of the :ref:`emrun <Running-html-files-with-emrun>` command line tool. This allows ``stdout``, ``stderr`` and ``exit(returncode)`` capture when running the generated application through *emrun*. (This enables `EXIT_RUNTIME=1`, allowing normal runtime exiting with return code passing.)

``--cpuprofiler``
  [link]
  Embeds a simple CPU profiler onto the generated page. Use this to perform cursory interactive performance profiling.

``--memoryprofiler``
  [link]
  Embeds a memory allocation tracker onto the generated page. Use this to profile the application usage of the Emscripten HEAP.

``--threadprofiler``
  [link]
  Embeds a thread activity profiler onto the generated page. Use this to profile the application usage of pthreads when targeting multithreaded builds (-pthread).

.. _emcc-config:

``--em-config <path>``
  [general]
  Specifies the location of the **.emscripten** configuration file.  If not
  specified emscripten will search for ``.emscripten`` first in the emscripten
  directory itself, and then in the user's home directory (``~/.emscripten``).
  This can be overridden using the ``EM_CONFIG`` environment variable.

``--valid-abspath <path>``
  [compile+link]
  Note an allowed absolute path, which we should not warn about (absolute
  include paths normally are warned about, since they may refer to the
  local system headers etc. which we need to avoid when cross-compiling).

.. _emcc-o-target:

``-o <target>``
  [link]
  When linking an executable, the ``target`` file name extension defines the output type to be generated:

    - <name> **.js** : JavaScript (+ separate **<name>.wasm** file if emitting WebAssembly). (default)
    - <name> **.mjs** : ES6 JavaScript module (+ separate **<name>.wasm** file if emitting WebAssembly).
    - <name> **.html** : HTML + separate JavaScript file (**<name>.js**; + separate **<name>.wasm** file if emitting WebAssembly).
    - <name> **.wasm** : WebAssembly without JavaScript support code ("standalone Wasm"; this enables ``STANDALONE_WASM``).

  These rules only apply when linking.  When compiling to object code (See `-c`
  below) the name of the output file is irrelevant.

.. _emcc-c:

``-c``
  [compile]
  Tells *emcc* to emit an object file which can then be linked with other object files to produce an executable.

``--output_eol windows|linux``
  [link]
  Specifies the line ending to generate for the text files that are outputted. If "--output_eol windows" is passed, the final output files will have Windows \r\n line endings in them. With "--output_eol linux", the final generated files will be written with Unix \n line endings.

``--cflags``
  [other]
  Prints out the flags ``emcc`` would pass to ``clang`` to compile source code to object form. You can use this to invoke clang yourself, and then run ``emcc`` on those outputs just for the final linking+conversion to JS.

.. _emcc-environment-variables:

Environment variables
=====================
*emcc* is affected by several environment variables, as listed below:

  - ``EMMAKEN_JUST_CONFIGURE`` [other]
  - ``EMCC_AUTODEBUG`` [compile+link]
  - ``EMCC_CFLAGS`` [compile+link]
  - ``EMCC_CORES`` [general]
  - ``EMCC_DEBUG`` [general]
  - ``EMCC_DEBUG_SAVE`` [general]
  - ``EMCC_FORCE_STDLIBS`` [link]
  - ``EMCC_ONLY_FORCED_STDLIBS`` [link]
  - ``EMCC_LOCAL_PORTS`` [compile+link]
  - ``EMCC_STDERR_FILE`` [general]
  - ``EMCC_CLOSURE_ARGS`` [link] arguments to be passed to *Closure Compiler*
  - ``EMCC_STRICT`` [general]
  - ``EMCC_SKIP_SANITY_CHECK`` [general]
  - ``EM_IGNORE_SANITY`` [general]
  - ``EM_CONFIG`` [general]
  - ``EM_LLVM_ROOT`` [compile+link]
  - ``_EMCC_CCACHE`` [general] Internal setting that is set to 1 by emsdk when integrating with ccache compiler frontend

Search for 'os.environ' in `emcc.py <https://github.com/emscripten-core/emscripten/blob/main/emcc.py>`_ to see how these are used. The most interesting is possibly ``EMCC_DEBUG``, which forces the compiler to dump its build and temporary files to a temporary directory where they can be reviewed.


.. todo:: In case we choose to document them properly in future, below are some of the :ref:`-s<emcc-s-option-value>` options that are documented in the site are listed below. Note that this is not exhaustive by any means:

  - ``-sFULL_ES2``
  - ``-sLEGACY_GL_EMULATION``:

    - ``-sGL_UNSAFE_OPTS``
    - ``-sGL_FFP_ONLY``

  - ASSERTIONS
  - SAFE_HEAP
  - -sDISABLE_EXCEPTION_CATCHING=0
  - INLINING_LIMIT=
