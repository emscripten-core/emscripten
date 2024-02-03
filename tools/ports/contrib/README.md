Emscripten "Contrib" Ports
==========================

Ports in this directory are contributed by the wider community and are
supported on a "best effort" basis.  Since they are not run as part of
emscripten CI they are not always guaranteed to build or function.

If you want to add a contrib port, please use another contrib port as 
an example. In particular, each contrib port must provide 3 extra piece
of information (provided as functions in the port file):

* `url`: the url where the user can find more information about 
  the project/port
* `description`: a (short) description of what the project/port 
  is about
* `license`: the license used by the project/port

After adding a contrib port, you should consider modifying the documentation 
under `site/source/docs/compiling/Contrib-Ports.rst`.