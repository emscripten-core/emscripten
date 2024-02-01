Emscripten "Contrib" Ports
==========================

Ports in this directory are contributed by the wider community and are
supported on a "best effort" basis.  Since they are not run as part of
emscripten CI they are not always guaranteed to build or function.

If you want to add a contrib port, please use another contrib port as 
an example. In particular, each contrib port must provide 3 extra piece
of information (provided as functions in the port file):

* `project_url`: the url where the user can find more information about 
  the project/port
* `project_description`: a (short) description of what the project/port 
  is about
* `project_license`: the license used by the project/port

After adding (resp. modifying) a contrib port, you must run the 
`./tools/maint/update_settings_docs.py` command to add (resp. update) 
the new port to the documentation.