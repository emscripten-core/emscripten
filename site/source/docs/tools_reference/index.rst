.. _tools-reference:

===============
Tools Reference
===============

This section provides reference for the main tools in the :ref:`Emscripten toolchain <about-emscripten-toolchain>`:

- :ref:`emccdoc` 
	*Emcc* is used to call the Emscripten compiler from the command line. It is effectively a drop-in replacement for a standard compiler like *gcc* or *clang*.
	
- :ref:`emsdk`: 
	*Emsdk* is used to perform all SDK maintenance. You can use it to download, install, activate, and remove SDKs and tools, and even to build and use the latest compiler from source on Github.
	
- :ref:`emcmdprompt` 
	This prompt is used to call *Emscripten* from the command line on Windows. It is configured with the correct system paths and settings to point to the active Emscripten SDK/tools.



.. toctree:: 
   :hidden:
   
   emsdk
   emcc
   emcmdprompt

