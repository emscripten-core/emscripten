Emscripten "Contrib" Ports
==========================

Ports in this directory are contributed by the wider community and are
supported on a "best effort" basis.  Since they are not run as part of
emscripten CI they are not always guaranteed to build or function.

If you want to add a contrib port, please use another contrib port as 
an example. In particular, each contrib port must provide 3 extra piece
of information:

* `URL`: the url where the user can find more information about 
  the project/port
* `DESCRIPTION`: a (short) description of what the project/port 
  is about
* `LICENSE`: the license used by the project/port

A contrib port can have options using the syntax 
`--use-port=name:opt1=v1:opt2=v2`.

If you want to support options, then your port needs to provide 2 
additional components:

1. A handler function defined this way:
```python
def handle_options(options, error_handler):
  # options is of type Dict[str, str]
  # in case of error, use error_handler('error message')
  # note that error_handler is guaranteed to never return
```
2. A dictionary called `OPTIONS` (type `Dict[str, str]`) where each key is the 
   name of the option and the value is a short description of what it does 

When emscripten detects that options have been provided, it parses them and
check that they are valid option names for this port (using `OPTIONS`). It then
calls the handler function with these (valid) options. If you detect an error
with a value, you should use the error handler provided to report the 
failure.

Since emscripten uses `:` as the option separator, `::` is the escape syntax 
(ex: `--use-port=name:opt1=abc::def`)

> ### Note
> If the options influence the way the library produced by the port is built, 
> you must ensure that the library name accounts for these options. Check 
> `glfw3.py` for an example of ports with options.

After adding a contrib port, you should consider modifying the documentation 
under `site/source/docs/compiling/Contrib-Ports.rst`.