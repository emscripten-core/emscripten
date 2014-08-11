.. _building-emscripten-on-windows-from-source:

===================================================
Building Emscripten on Windows (ready-for-review)
===================================================

.. tip:: The :ref:`Emscripten Windows SDKs <emscripten-sdk-windows-installers>` provide the **easiest** and **most reliable** method for **building from source**. Just :ref:`download and install the SDK <sdk-download-and-install>` and follow the brief instructions :ref:`here <emsdk-master-or-incoming-sdk>`.

	These instructions are provided for developers who, for whatever reason, prefer a manual approach.

This page contains basic instructions on how to manually build and configure Emscripten from source on a clean Windows box. 


What you'll need
=================

The topic :ref:`Emscripten Toolchain <toolchain-what-you-need>` lists the specific versions of tools that are needed.

.. note:: 64-bit versions of all needed dependencies are preferred, and may be required if you are building large projects. However, if you plan to use the Visual Studio integration, do NOT install the 64-bit versions of the tools.

Installing required tools
==========================

These instructions explain how to install **all** the :ref:`required tools <toolchain-what-you-need>`. You can :ref:`test whether some of these are already installed <toolchain-test-which-dependencies-are-installed>` on the platform and skip those steps.


#. Install `Python 2.x <http://www.python.org/>`_ (not 3.x)

	- For example `python-2.7.5.amd64.msi <http://python.org/ftp/python/2.7.5/python-2.7.5.amd64.msi>`_ (64-bit version) or `python-2.7.5.msi <http://python.org/ftp/python/2.7.5/python-2.7.5.msi>`_ (32-bit version).
	- Add the path to the Python directory containing **Python.exe** to your PATH. 
	
		- Paths are set by opening **System Settings | Advanced system properties**, clicking **Environment Variables** and select **PATH**. 
		- Add the path to python, separated by semicolons: e.g. ``;C:/Python27/;`` or ``;C:/Python27/bin;`` (depending on the location of the exe).
		
		
#. Install `node.js <http://nodejs.org/>`_

	- For example `node-v0.10.17-x64.msi <http://nodejs.org/dist/v0.10.17/x64/node-v0.10.17-x64.msi>`_ (64-bit version) or `node-v0.10.17-x86.msi <http://nodejs.org/dist/v0.10.17/node-v0.10.17-x86.msi>`_. (32-bit version).


#. Install `gcc 4.6 <http://sourceforge.net/projects/mingw-w64/files/>`_

#. Install `Github for Windows <http://windows.github.com/>`_ (or any other git client).

#. Install `Java <http://java.com/en/download/index.jsp>`_ 

#. Build :term:`Fastcomp` (LLVM + Clang) from source using :ref:`these instructions <building-fastcomp-from-source>`. 
	
#. Clone the `kripken/emscripten <https://github.com/kripken/emscripten>`_ repository from Github. This repository contains the main compiler tool for compiling C/C++ programs to JavaScript:

	- Using *Github for Windows*:
		-  Launch the *GitHub for Windows* Client. Click **Skip Setup** if you don't have a github account.
		-  (optional) Select **Options** from the gear menu, and customize the default storage directory. Ensure the path has no spaces.
		-  In your web browser, open https://github.com/kripken/emscripten and press the **Clone in Windows** button.
	
	- Using the command line:
		-  Create a directory (with no spaces in the name) to contain the clone. 
		-  Enter the following command into the terminal: ::
		
			git clone https://github.com/kripken/emscripten.git



   
Configuring Emscripten settings
===============================

Almost all the compiler settings used by Emscripten are defined in the :ref:`compiler configuration file (.emscripten) <compiler-configuration-file>`, a user-specific file located in the user's home directory.

Instructions for creating and manually configuring this file are given in :ref:`configuring-emscripten-settings`.

   

Validating the environment
===============================

The best way to validate the environment is to build some code. Open the terminal in your *Emscripten* directory - the directory which contains *emcc* (if you installed using *Github for Windows* this will be **C:/Users/username/Documents/GitHub/emscripten**) and enter: ::

	emcc tests/hello_world.cpp

If this builds **a.out.js** in the current directory, and you don't see any build errors in the terminal, Emscripten is good to go! 

There are additional validation and troubleshooting instructions in the topic: :ref:`verifying-the-emscripten-environment`.



