.. _Browser-limitations:

============================
Specific Browser Limitations
============================

This page lists some of the differences between the latest versions of the major browsers that are relevant to Emscripten-compiled applications and games:

-  The function :c:func:`emscripten_get_now` returns a wallclock time as a ``float`` in milliseconds. *Opera 12.16* and `Windows Google Chrome 28.0.1500.95 <https://code.google.com/p/chromium/issues/detail?id=158234>`_ have a limitation that the timer precision is only in milliseconds. On other major browsers (*IE10*, *Firefox 22*, *Chrome 28* on non-Windows) it has sub-millisecond precision.
- WebGL is not fully supported on *Internet Explorer* (at least prior to IE12):

  -  *Internet Explorer 10* and older do not support WebGL. Trying to initialize a GL context via EGL, GLUT, SDL or similar will fail. Emscripten applications that do not depend on OpenGL can still run on this browser.
  -  *Internet Explorer 11* supports only part of WebGL 1.0. Some commands, shaders etc. may not work. You may be able to limit your app to using the subset that is supported by IE11.
  - WebGL support on other major browsers is fairly good (see `WebGL support in different browsers <http://caniuse.com/#feat=webgl>`_).

-  *Opera 12.16* has limited support for the W3C File API. In particular it does not support `createObjectURL functionality <http://www.opera.com/docs/specs/presto2.12/apis/#file>`_, which means that it is not possible to use the browser's image codecs to decode preloaded files in the Emscripten virtual file system.
-  OpenAL and SDL audio support in Emscripten depend on the Web Audio API (see `Web Audio API support in different browsers <http://caniuse.com/#feat=audio-api>`_).

