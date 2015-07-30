.. _api-limitations:

===============
API Limitations
===============

The browser environment and JavaScript are different from the native environments that C and C++ typically run in. These differences impose some limitations on how native APIs can be called and used. This section lists some of the more obvious limitations.

Networking
==========

Emscripten supports *libc* networking functions but you must limit yourself to asynchronous (non-blocking) operations. This is required because the underlying JavaScript networking functions are asynchronous.

File Systems
============

Emscripten supports *libc* file system functions and C/C++ code can be written in the normal way. 

Code run in a :ref:`browser environment <emscripten-runtime-environment>` is sandboxed, and does not have direct access to the local file system. Instead, Emscripten creates a virtual file system that may be preloaded with data or linked to URLs for lazy loading. This affects when synchronous file system functions can be called and how a project is compiled. See the :ref:`file-system-overview` for more information. 


Application Main Loop
=====================

The browser event model uses *co-operative multitasking* — each event has a "turn" to run, and must then return control to the browser event loop so that other events can be processed. A common cause of HTML pages hanging is JavaScript that does not complete and return control to the browser.

This can affect how an application using an infinite main loop is written. See :ref:`emscripten-runtime-environment` for more information.

Other APIs
==========

Support for other **portable** C/C++ code is :ref:`fairly comprehensive <about-emscripten-porting-code>`. 

