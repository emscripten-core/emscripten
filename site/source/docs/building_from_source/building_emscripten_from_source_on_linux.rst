.. _building-emscripten-on-linux:

============================================================
Building Emscripten on Linux (under-construction)
============================================================

This page contains basic instructions on how to build and configure Emscripten from source on a clean (Ubuntu 14.04.1 LTS 64bit) Linux box.

.. tip:: The :ref:`Portable SDK for Linux <portable-emscripten-sdk-linux-osx>` (and Mac OS X) also builds the tools from source, and may provide a simpler alternative to the process outlined here. The benefit of the SDK is that it helps automate getting the source and configuring the environment.

What you'll need
=================

The previous topic (:ref:`Emscripten Toolchain <toolchain-what-you-need>`) lists the specific versions of tools that are needed.


Installing required tools
============================================================

These instructions explain how to install **all** the :ref:`required tools <toolchain-what-you-need>`. You can :ref:`test whether some of these are already installed <toolchain-test-which-dependencies-are-installed>` on the platform and skip those steps.

1. Update the *system package manager* to ensure the package lists are up to date: 

	::
		
		sudo apt-get update


#. Install *Python* using the *system package manager*: 

	::
	
		sudo apt-get install python2.7


#. Install *node.js* using the *system package manager*: 

	::
	
		sudo apt-get install nodejs


#. Install *gcc* (and related dependencies) using the *system package manager*: 

	::
	
		sudo apt-get install build-essential


#. Install *git* using the *system package manager*: 

	::
	
		sudo apt-get install git-core

#. Install *Java* using the *system package manager*: 

	::
	
		sudo apt-get install default-jre

#. Build :term:`Fastcomp` (LLVM + Clang) from source using :ref:`these instructions <building-fastcomp-from-source>`. 
	
#. Clone the `kripken/emscripten <https://github.com/kripken/emscripten>`_ repository from Github. This repository contains the main compiler tool for compiling C/C++ programs to JavaScript:

	-  Create a directory (with no spaces in the name) to contain the clone. 
	-  Enter the following command into the terminal: ::
	
		git clone https://github.com/kripken/emscripten.git

	
Configuring Emscripten
=======================

**HamishW** What do to with this section depends on how specific it is to particular platforms.  May be best to have very thin links here and link to the parent document

1. Navigate to the directory where you cloned the Emscripten repository.
2. Enter the command: 

	::
	
		./emcc --help

	You should get a ``Welcome to Emscripten!`` message. Behind the scenes, Emscripten generated a file called ``.emscripten`` in your home folder.

3. Edit **.emscripten**:

   1. Edit the variable ``EMSCRIPTEN_ROOT`` to point to the Emscripten root folder, e.g.:
   
	::
   
		EMSCRIPTEN_ROOT = 'C:/Users/username/Documents/GitHub/emscripten'`` 
   
	.. note:: Use forward slashes!
		
   2. Edit the variable ``LLVM_ROOT`` to point directly to the path where you built *Fastcomp*, e.g.: 
   
   ::
   
		LLVM_ROOT = '**HamishW**'
	  
   3. Edit the variable TEMP\_DIR to point to a valid Windows path on your local system, e.g. ``TEMP_DIR = 'C:/tmp'``, and create that folder on the local filesystem if it doesn't exist.




.. COMMENT - HAMISHW Check if can sensibly or useful integrate any of this. 

	Also COMMENT ON ACTUALLY USING THE Emscripten on Linux. This MIGHT actually live in the tutorial since it will change where you navigate. 
	
	On Linux+OSX, you need to use the 'source' command to alter the current terminal. I.e. Linux+OSX call 'source ./emsdk_env.sh' when Windows calls 'emsdk_env.bat'. When you change the active tools with the 'activate' command, there should be a help message that describes that syntax.

	On Linux+OSX there is no support for permanent path activation, but one needs to e.g. add the line 'source ~/emsdk/emsdk_env.sh' to home bash profile files or similar to bootstrap it to the console. On Windows one can use the --global flag for emsdk_env.bat.
	

	Configuring the basic Emscripten settings file
	============================================
	
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


   

Validating the environment
===============================

The best way to validate the environment is to build some code. 

There are some basic validation and troubleshooting instructions in the topic: :ref:`verifying-the-emscripten-environment`.

.. comment: This is a comment that might go in the "verifying" section.

	**HamishW** Also see what can come out of the Downloads validating section. May be some scope for reuse.
	
	**HamishW** confirm that you can't use paths set up by emscripten - you have to actually specify which emcc you want to run? on Linux.

	You can also validate the individual parts of the toolchain have been installed, as described above: ::
		

		
		# Test emcc. This should print out the *emcc* version number as well as run some basic tests
	

	



