.. _emscripten-toolchain-top:

=================================
Emscripten Toolchain Requirements
=================================

The instructions below list the main tools and dependencies in an Emscripten environment, along with instructions on how to test which dependencies are installed.

.. tip:: The :ref:`SDK <sdk-download-and-install>` provides the **easiest** and **most reliable** method for getting, using, updating and managing Emscripten environments. If you're using the SDK you won't *need* these instructions — they are provided for information only.

	The instructions below are useful if you're :ref:`manually <installing-from-source>` building from source.

.. _toolchain-what-you-need:

What you'll need
================

.. _central-list-of-emscripten-tools-and-dependencies:

Emscripten tools and dependencies
---------------------------------

A complete Emscripten environment requires the following tools. First test to see if they are already installed using the :ref:`instructions below <toolchain-test-which-dependencies-are-installed>`. Then install any missing tools using the instructions in the appropriate platform-specific build topic (:ref:`building-emscripten-on-linux`, :ref:`building-emscripten-on-windows-from-source`, :ref:`building-emscripten-on-mac-osx-from-source`):

	- :term:`Node.js` (0.8 or above; 0.10.17 or above to run websocket-using servers in node):
	- :term:`Python` 2.x (2.7.3 or above preferred)
	- :term:`Java` (1.6.0_31 or later).  Java is optional. It is required to use the :term:`Closure Compiler` (in order to minify your code).
	- :term:`Git` client. Git required if building tools from source. 
	- :term:`Fastcomp` (Emscripten's fork of LLVM and Clang)
	- The `Emscripten code <https://github.com/kripken/emscripten>`_, from GitHub 

.. note: 64-bit versions of all needed dependencies are preferred, and may be required if you are building large projects. 

.. note:: The `Spidermonkey shell <https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Introduction_to_the_JavaScript_shell>`_ is also required if you want to run **100%** of the tests in the test suite. Most developers will not need this, and should instead use *node.js*.

.. _compiler-toolchain:

Compiler toolchain
------------------

When building Emscripten from source code, whether "manually" or using the SDK, you will also need a *compiler toolchain* (including *gcc*, *g++*, *cmake*, etc.):

- Windows: Install `gcc 4.6 <http://sourceforge.net/projects/mingw-w64/files/>`_
- Linux: Install *gcc* from the **build-essential** package:
	
	::
		
		sudo apt-get install build-essential
	
- Mac OS X: Install the *XCode Command Line Tools*:

	-  Install XCode from the `Mac OS X App Store <http://superuser.com/questions/455214/where-is-svn-on-os-x-mountain-lion>`_.
	-  In **XCode | Preferences | Downloads**, install *Command Line Tools*.


.. _toolchain-test-which-dependencies-are-installed:

Test which tools are installed
==============================

Some of the tools are pre-installed on the various platforms (for example, Python is always available on Linux builds). 

You can check which tools are already present using the following commands:

::

	# Check for Python
	python --version
		
	# Check for node.js on Linux
	nodejs --version
	
	# Check for node.js on Windows
	node --version 	#
	
	# Check for node.js on Mac OS X
	node -v
	
	# Check for git
	git --version
		
	# Check for Java
	java -version

	# Check for gcc / g++
	gcc --version
	g++
	
	# Check for cmake
	cmake
	
