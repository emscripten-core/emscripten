=============================
Connecting C++ and JavaScript
=============================

Emscripten provides various options for connecting "normal" JavaScript and compiled code, which range from functions to call compiled C/C++ from JavaScript (and vice versa) through to accessing environment variables from compiled code. 

The topic :ref:`Interacting-with-code` provides an overview of all the methods. The other two topics provide additional detail on the :ref:`Embind` and :ref:`WebIDL-Binder` tools.

.. note:: For information on how compiled code interacts with the browser environment, see :ref:`Emscripten-Browser-Environment`. For file system-related manners, see the :ref:`Filesystem-Guide`. 

.. toctree::
	:maxdepth: 1
   
	Interacting-with-code
	embind
	WebIDL-Binder
   
