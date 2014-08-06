.. _installing-from-source:

============================================
Installing from Source (under-construction) 
============================================

.. COMMENT : Cut out of the Downloading Page

Instead of using the SDK, you can grab the code and dependencies yourself. This is a little more technical but lets you use the very latest development code.


Things you'll need
============================================

- The Emscripten code, from GitHub (git clone git://github.com/kripken/emscripten.git. The master branch is fine, it is guaranteed to always be stable. We merge to master only after all tests pass.)
- Emscripten's LLVM and Clang. Emscripten now has an LLVM backend ("fastcomp"), which means you need to use our LLVM+Clang. See `Getting Fastcomp <LLVM-Backend#getting-fastcomp>`_ in the :doc:`LLVM Backend` page. 

	.. note::
		
		- The `LLVM Backend <https://github.com/kripken/emscripten/wiki/LLVM-Backend>`__ page has instructions for building Emscripten's LLVM+Clang. After you build it, run ``emcc -v``, which should print out the version number as well as run some basic tests.
		- It is possible but not recommended to disable  fastcomp and use a stock version of LLVM (see the link for more information).
		
- **Node.js** (0.8 or above; 0.10.17 or above to run websocket-using servers in node)
- Python 2.7.3
- Optionally, if you want to use Closure Compiler to minify your code as much as possible, you will also need Java.



Additional Notes
============================================

- Python is probably already installed if you are on Linux or OS X.
- Node.js and LLVM should have convenient binaries for your OS, but installing them from source is easy, just compile them in their directories, you don't need to bother with installing them systemwide (you will point Emscripten to them in the next step, where you set up directories).




Windows and OSX: guides for manual building
============================================

.. COMMENT - HAMISHW from the MDN - and I suspect no longer needed now we have LLVM Backend. Check and delete if needed. 

The following guides explain setting up Emscripten and its prerequisites manually on Windows and Mac OSX:

- :doc:`Build Clang on Mac OS X`.
- :doc:`Download a prebuilt Clang on Mac OS X`.
- :doc:`Get Emscripten and Clang via brew` by nathanhammond.
- :doc:`Manual Emscripten setup on Windows`.

Linux: guides for manual building
============================================

.. COMMENT - HAMISHW from the MDN - and I suspect no longer needed now we have LLVM Backend. Check and delete if needed. 


The following guides explain setting up Emscripten and its prerequisites manually on Linux:

- For help on Ubuntu, you can follow the :doc:`Getting Started on Ubuntu 12.10` guide for instructions on how to obtain the prerequisites and build Clang manually using CMake.
- For help on Debian, see this :doc:`guide by EarthServer`.
- rhelmer has provided a Vagrant VM for Emscripten, see :doc:`emscripten-vagrant`.
- Dirk Krause created an :doc:`Amazon EC2 image` for Emscripten.


Configuring the basic Emscripten settings file
==============================================

.. COMMENT - HAMISHW from the MDN - and I suspect no longer needed now we have LLVM Backend. Check and delete if needed. 

The first time you run emcc (or any of the other Emscripten tools, for that matter), it will create a settings file at ``~/.emscripten`` (~ is your user's home directory) and exit. This file contains a number of settings that provide Emscripten with the Paths to all its requirements (LLVM, clang, etc.) amongst other things, and looks something like so: ::

	import os
	LLVM_ROOT='C:/Program Files/Emscripten/clang/3.2_64bit/bin'
	NODE_JS='C:/Program Files/Emscripten/node/0.10.17_64bit/node.exe'
	PYTHON='C:/Program Files/Emscripten/python/2.7.5.3_64bit/python.exe'
	JAVA='C:/Program Files/Emscripten/java/7.45_64bit/bin/java.exe'
	EMSCRIPTEN_ROOT='C:/Program Files/Emscripten/emscripten/1.7.8'
	CRUNCH='C:/Program Files/Emscripten/crunch/1.03/crunch.exe'
	MINGW_ROOT='C:/Program Files/Emscripten/mingw/4.6.2_32bit'
	SPIDERMONKEY_ENGINE = ''
	V8_ENGINE = ''
	TEMP_DIR = 'c:/users/cmills/appdata/local/temp'
	COMPILER_ENGINE = NODE_JS
	JS_ENGINES = [NODE_JS]

If you used an emsdk installer to install Emscripten everything would be set up automatically, but since you probably built everything manually (since you are reading this section) you'll need to set the Emscripten settings yourself.

1. If you haven't run Emscripten before, run it now with (assumes you are in ``emsdk`` and haven't set it's location in your PATH):

	::
	
	./Emscripten/1.7.8/emcc
	
	.. note:: This is for Mac/Linux; on Windows you would use emcc instead.
	.. note:: If you are having trouble with python versions, you can also explicitly invoke 
		::
		
			python emcc

		especially if python2 is not defined in your system. python2 allows python 2 and 3 to be installed together on one system, which is increasingly common; as an alternative to python emcc, you can also add a symlink to python from python2). In that case you should also update the PYTHON line in the ~/.emscripten settings file.
	
#. Edit the ``~/.emscripten`` file now using your favourite text editor.

#. Change the directory locations of ``LLVM_ROOT`` and ``NODE_JS`` to the right places in your setup (specifically, edit ``LLVM_ROOT`` and ``NODE_JS``). If those paths are not right, Emscripten will not find LLVM, Clang or Node.js and return a failure message. Look at the comments in the file that explain what the settings are and which ones you need to change.

#. After setting those paths, run ``./Emscripten/1.7.8/emcc`` again. It should again perform the sanity checks to test the specified paths. If they don't all pass, you might have a typo somewhere. When everything is set up properly, running ``./Emscripten/1.7.8/emcc`` should return ``emcc: no input files``, and you should be ready to use it.
