.. _emscripten-toolchain-top:

============================================================
Emscripten Toolchain (under-construction)
============================================================

The :ref:`SDKs <sdk-download-and-install>` deliver (or explain how to obtain) everything you need for developing with Emscripten. However if you're building from source you'll need to gather the tools below yourself.

.. _toolchain-what-you-need:

What you'll need
================

A complete Emscripten environment includes the following tools:

	- :term:`Node.js` (0.8 or above; 0.10.17 or above to run websocket-using servers in node):
	- :term:`Python` 2.x (2.7.3 or above preferred)
	- :term:`Java` (1.6.0_31 or later).  Java is optional. It is required to use the :term:`Closure Compiler` (in order to minify your code).
	- :term:`Git` client. Git required if building tools from source. 
	- :term:`Fastcomp` (Emscripten's LLVM and Clang)
	- The `Emscripten code <https://github.com/kripken/emscripten>`_, from GitHub 

.. note: 64-bit versions of all needed dependencies are preferred, and may be required if you are building large projects. 

When building from sources you will also need a compiler environment (to build :ref:`Fastcomp <LLVM-Backend>`):

	- *gcc* (4.6 or later)
	
.. note:: The *Spidermonkey* shell is also required if you want to run **all** the test code. Most developers will not need this, and should use *node.js*.

.. _toolchain-test-which-dependencies-are-installed:

Test which tools are installed
====================================

Some of the tools are pre-installed on the various platforms (for example, Python is always available on Linux builds). 

You can check which tools are already present and only install those files that are needed: 

::

	# Check for Python
	python --version
		
	# Check for node.js on Linux
	nodejs --version
	
	# Check for node.js on Windows
	node --version 	# 
	
	# Check for git
	git --version
		
	# Check for Java
	java -version

	# Check for gcc / g++
	gcc --version
	g++
