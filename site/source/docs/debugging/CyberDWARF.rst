.. _CyberDWARF:

====================
CyberDWARF Debugging
====================

Building
========

To add CyberDWARF support to a build, pass ``-s CYBERDWARF=1`` to ``emcc``. This generates a ``.cd`` file containing type information for debugging and adds a debugging toolkit to the output JavaScript.

Using
=====

The CyberDWARF debugger is designed to be used from the JavaScript devtool console available in most modern browsers.

Heap Pretty Printer
-------------------

This small example will show how to use CyberDWARF to visualize a simple struct.

.. code-block:: cpp

  #include <emscripten.h>
  #include <stdio.h>
  #include <string.h>

  struct TinyStruct {
    short len;
    char * chars;
  };

  extern "C" {

  int example() {
    TinyStruct example;
    example.chars = "Hello World";
    example.len = strlen(example.chars);
    
    printf("%p\n", &example);
    EM_ASM({ debugger });  
    
    return 0;
  }

  }

  int main(int argc, char *argv[]) {
    return 0;
  }

**Compile the code**

.. code-block:: bash

  em++ -O1 -s CYBERDWARF=1 example.cpp -o example.html -s EXPORTED_FUNCTIONS="['_example']"

**Visualizing**

After the page loads, open a JavaScript console.

.. code-block:: bash

  > Module['cyberdwarf'].initialize_debugger()
  Debugger ready
  > Module['_example']()
  0x1078
  > Module['cyberdwarf'].set_current_function("_example")
  > JSON.stringify(Module['cyberdwarf'].decode_var_by_var_name(0x1078, "example", 10), null, "\t")
  '{
    "struct TinyStruct": {
      "short : len": 5,
      "char * : chars": 87
    }
  }'

API
---

.. js:function:: Module['cyberdwarf'].initialize_debugger

	Called to load the CyberDWARF file for the script.
	
.. js:function:: Module['cyberdwarf'].set_current_function(name)

	Sets the function to lookup variable type by ``name``. Supports either C/mangled name or minified name from symbol file.
  
.. js:function:: Module['cyberdwarf'].decode_var_by_var_name(address, name, depth)

  Looks up the type given by variable ``name`` in the current function, then dumps a JSON formatted representation of that type at ``address`` up to ``depth`` *(defaults to 1)*

.. js:function:: Module['cyberdwarf'].decode_var_by_type_name(address, type, depth)

  Using the type given in ``type``, then dumps a JSON formatted representation of that type at ``address`` up to depth *(defaults to 1)*
