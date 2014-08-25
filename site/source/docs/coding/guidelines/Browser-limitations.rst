.. _Browser-limitations:

======================================
Browser limitations (ready-for-review)
======================================

This page lists some of the most common differences between the latest versions of the major browsers and features that are relevant to Emscripten-compiled applications and games:

-  The function :c:func:`emscripten_get_now` returns a wallclock time as a ``float`` in milliseconds. *Opera 12.16* and on `Windows Google Chrome 28.0.1500.95 <https://code.google.com/p/chromium/issues/detail?id=158234>`_ have a limitation that the timer precision is only in milliseconds. On other major browsers (*IE10*, *Firefox 22*, *Chrome 28* on non-Windows) it has sub-millisecond precision.
-  *Internet Explorer 10* and older do not support WebGL, which means trying to initialize a GL context via EGL, GLUT, SDL or similar will fail. Emscripten applications that do not depend on OpenGL can still be compiled for this browser however. See `WebGL support in different browsers <http://caniuse.com/#feat=webgl>`_.
-  *Internet Explorer 11* supports WebGL, but not all of WebGL 1.0, so certain commands, shaders etc. may not work. You may be able to limit your app to using the subset that is supported by IE11. Hopefully IE12 will support everything.
-  *Opera 12.16* has limited support for the W3C File API, in particular it does not support `createObjectURL functionality <http://www.opera.com/docs/specs/presto2.12/apis/#file>`_. As a result, it is not possible to use the browser image codecs on this version of Opera to decode preloaded files in the Emscripten virtual files system.
-  OpenAL and SDL audio support in Emscripten depend on the Web Audio API. See `Web Audio API support in different browsers <http://caniuse.com/#feat=audio-api>`_.

