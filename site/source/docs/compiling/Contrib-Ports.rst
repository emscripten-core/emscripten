.. _Contrib-Ports:

========================
Emscripten Contrib Ports
========================

Contrib ports are contributed by the wider community and 
supported on a "best effort" basis. Since they are not run as part 
of emscripten CI they are not always guaranteed to build or function.
          
The following is the complete list of the contrib ports that are 
available in emscripten. In order to use a contrib port you use the 
``--use-port=<port_name>`` option (:ref:`emcc <emcc-use-port>`).

.. _contrib.glfw3:

contrib.glfw3
=============

This project is an emscripten port of GLFW written in C++ for the
web/webassembly platform.

.. note::
  emscripten includes support for both GLFW 2 and 3 written in Javascript.
  These can be activated with the :ref:`settings <use_glfw>` ``-sUSE_GLFW=2``
  and ``-sUSE_GLFW=3``. This non-official contribution, written in C++,
  provides a more extensive and up-to-date implementation of the GLFW 3 API
  than the built-in port. It is enabled with the option
  ``--use-port=contrib.glfw3``.

`Project information <https://github.com/pongasoft/emscripten-glfw>`_

License: Apache 2.0 license