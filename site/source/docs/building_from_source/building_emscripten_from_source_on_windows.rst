.. _building-emscripten-on-windows-from-source:

=======================================
Manually Building Emscripten on Windows
=======================================

This page contains basic instructions on how to manually build and configure Emscripten from source on a clean Windows box. 

.. note:: The instructions clone from the main Emscripten repository (https://github.com/kripken/emscripten). :ref:`Contributors <contributing>` should instead clone from their own Emscripten fork, and submit changes as pull requests.

.. tip:: You can also build Emscripten from source :ref:`using the SDK <building-emscripten-from-source-using-the-sdk>`. This is recommended if you need to easily switch between SDK and source builds.


What you'll need
=================

The specific versions of tools that are needed are listed in the :ref:`Emscripten Toolchain Requirements <toolchain-what-you-need>`.

.. note:: 64-bit versions of all needed dependencies are preferred, and may be required if you are building large projects. However, if you plan to :ref:`integrate with Visual Studio <emscripten-and-vs2010-manual-integration-on-windows>`, do NOT install the 64-bit versions of the tools.

Installing required tools
==========================

These instructions explain how to install **all** the :ref:`required tools <toolchain-what-you-need>`. You can :ref:`test whether some of these are already installed <toolchain-test-which-dependencies-are-installed>` on the platform and skip those steps.


#. Install `Python 2.x <http://www.python.org/>`_ (not 3.x):

	- For example `python-2.7.5.amd64.msi <http://python.org/ftp/python/2.7.5/python-2.7.5.amd64.msi>`_ (64-bit version) or `python-2.7.5.msi <http://python.org/ftp/python/2.7.5/python-2.7.5.msi>`_ (32-bit version).
	- Add the path to the Python directory containing **Python.exe** to your PATH. 
	
		- Paths are set by opening **System Settings | Advanced system properties**, clicking **Environment Variables** and selecting **PATH**. 
		- Add the path to python, separated by semicolons: e.g. ``;C:/Python27/;`` or ``;C:/Python27/bin;`` (depending on the location of the exe).
		
		
#. Install `node.js <http://nodejs.org/>`_:

	- For example `node-v0.10.17-x64.msi <http://nodejs.org/dist/v0.10.17/x64/node-v0.10.17-x64.msi>`_ (64-bit version) or `node-v0.10.17-x86.msi <http://nodejs.org/dist/v0.10.17/node-v0.10.17-x86.msi>`_ (32-bit version).


#. Install `Visual Studio 2010 <http://go.microsoft.com/?linkid=9709949>`_ (and possibly `Microsoft Windows SDK for Windows 7 and .NET Framework 4 <http://www.microsoft.com/en-us/download/details.aspx?id=8279>`_).

#. Install `cmake <http://www.cmake.org/cmake/resources/software.html>`_.

#. Install `Github for Windows <http://windows.github.com/>`_ (or any other git client).

#. Install `Java <http://java.com/en/download/index.jsp>`_.

#. Build :ref:`Fastcomp <LLVM-Backend>` (LLVM + Clang) from source using :ref:`these instructions <building-fastcomp-from-source>`. 
	
#. Clone the `kripken/emscripten <https://github.com/kripken/emscripten>`_ repository from Github. This repository contains the main compiler tool for compiling C/C++ programs to JavaScript:

	- Using *Github for Windows*:
		-  Launch the *GitHub for Windows* client. Click **Skip Setup** if you don't have a Github account.
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

The best way to validate the environment is to build some code. Open the terminal in your *Emscripten* directory — the directory that contains *emcc* (if you installed using *Github for Windows* this will be **C:/Users/username/Documents/GitHub/emscripten**) and enter: ::

	./emcc tests/hello_world.cpp

If this builds **a.out.js** in the current directory, and you don't see any build errors in the terminal, Emscripten is good to go! 

There are additional validation and troubleshooting instructions in the topic :ref:`verifying-the-emscripten-environment`.



