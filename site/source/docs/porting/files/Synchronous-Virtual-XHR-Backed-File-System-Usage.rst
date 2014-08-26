.. _Synchronous-Virtual-XHR-Backed-File-System-Usage:

==============================================================
Synchronous Virtual XHR Backed File System Usage (wiki-import)
==============================================================
.. note:: This article was migrated from the wiki (Mon, 04 Aug 2014 23:20) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

Emscripten supports lazily loading binary data from HTTP servers using
XHR's (ala AJAX but no XML or JSON).

**Restriction: only possible in Web Workers** (due to browser
limitations)

It is recommended to use a web server with `byte
serving <http://en.wikipedia.org/wiki/Byte_serving>`__ support. This
will enable Emscripten to only load the parts of the file that are
actually read. If it is not available, the runtime will of course be
forced to load the whole file, even if you only read a single byte from
a 100 MB file.

Instructions for use
--------------------

1. You will need a page that spawns the web worker. See the page inlined
   in ``tests/runner.py/test_chunked_synchronous_xhr``. The ``prejs`` we
   use below will cause the program running in the Web Worker to
   ``postMessage`` it's stdout back. If you use that solution, the
   mother page should probably contain your handwritten glue code (not
   Emscriptened) to handle the stdout data.
2. The tests use ``checksummer.c`` as the actual Emscriptened program.
   As you can see, it is simply a vanilla C program using
   ``fopen``/``fread``/``fclose`` and no Emscripten specific code at
   all.
3. You will need at ``prejs`` that sets up the mapping between the file
   path in your equivalent of ``checksummer.c`` and the server to
   download from. Remember CORS! The test case also contains an HTTP
   server that shows some CORS headers that might need to be set. Of
   course, if the resources are hosted from the same domain Emscripten
   runs from, there is no issue.

