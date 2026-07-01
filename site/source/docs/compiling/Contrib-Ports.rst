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

This project is an Emscripten port of GLFW written in C++ for the
web/webassembly platform.

.. note::
  Emscripten includes support for both GLFW 2 and 3 written in Javascript.
  These can be activated with the :ref:`settings <use_glfw>` ``-sUSE_GLFW=2``
  and ``-sUSE_GLFW=3``. This non-official contribution, written in C++,
  provides a more extensive and up-to-date implementation of the GLFW 3 API
  than the built-in port. It is enabled with the option
  ``--use-port=contrib.glfw3``.

`Project information <https://github.com/pongasoft/emscripten-glfw>`__

License: Apache 2.0 license

.. _contrib.lua:

contrib.lua
===========

Lua is a powerful, efficient, lightweight, embeddable scripting language.

Example usage:

.. code-block:: text

  // main.c
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
  #include <stdio.h>

  int main() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    if (luaL_dostring(L, "print('hello world')") != LUA_OK) {
      printf("Error running Lua code %s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
    lua_close(L);
    return 0;
  }

  // compile with
  emcc --use-port=contrib.lua main.c -o /tmp/index.html


`Project information <https://www.lua.org/>`__

License: MIT License