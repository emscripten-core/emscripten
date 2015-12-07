.. _building-emscripten-on-mac-osx-from-source:

========================================
Manually Building Emscripten on Mac OS X
========================================

This page contains basic instructions on how to manually build and configure Emscripten from source on a clean Mac OS X box (tested on OS X version 10.8.2).

.. note:: The instructions clone from the main Emscripten repository (https://github.com/kripken/emscripten). :ref:`Contributors <contributing>` should instead clone from their own Emscripten fork, and submit changes as pull requests.

.. tip:: You can also build Emscripten from source :ref:`using the SDK <building-emscripten-from-source-using-the-sdk>`. This is recommended if you need easily switch between SDK and source builds.

What you'll need
================

The specific versions of tools that are needed are listed in the :ref:`Emscripten Toolchain Requirements <toolchain-what-you-need>`.


Installing required tools
=========================

These instructions explain how to install **all** the :ref:`required tools <toolchain-what-you-need>`. You can :ref:`test whether some of these are already installed <toolchain-test-which-dependencies-are-installed>` on the platform and skip those steps.

#. Install the *XCode Command Line Tools*. These include the toolchain to build :term:`Fastcomp`, and are a precondition for *git*.

	-  Install XCode from the `Mac OS X App Store <http://superuser.com/questions/455214/where-is-svn-on-os-x-mountain-lion>`_.
	-  In **XCode | Preferences | Downloads**, install *Command Line Tools*.

#. Install *git*:

	- `Allow installation of unsigned packages <https://www.my-private-network.co.uk/knowledge-base/apple-related-questions/osx-unsigned-apps.html>`_, or installing the git package won't succeed.
	- Install XCode and the XCode Command Line Tools (should already have been done). This will provide *git* to the system PATH (see `this stackoverflow post <http://stackoverflow.com/questions/9329243/xcode-4-4-command-line-tools>`_).
	- Download and install git directly from http://git-scm.com/.	

#. Install *CMake* if you do not have it yet:

	-  Download and install `CMake <http://www.cmake.org/cmake/resources/software.html>`_, and make sure it is available in PATH after installation.
	
#. Install *node.js* from http://nodejs.org/ 


	.. _getting-started-on-osx-install-python2:

#. Build :ref:`Fastcomp <LLVM-Backend>` (LLVM + Clang) from source using :ref:`these instructions <building-fastcomp-from-source-building>`. 
	
#. Clone the `kripken/emscripten <https://github.com/kripken/emscripten>`_ repository from Github. This repository contains the main compiler tool for compiling C/C++ programs to JavaScript:

	-  Create a directory (with no spaces in the name) to contain the clone. 
	-  Enter the following command into the terminal: ::
	
		git clone https://github.com/kripken/emscripten.git
	
	


Configuring Emscripten settings
===============================

Almost all the compiler settings used by Emscripten are defined in the :ref:`compiler configuration file (~/.emscripten) <compiler-configuration-file>`, a user-specific file located in the user's home directory.

Instructions for creating and manually configuring up this file are given in :ref:`configuring-emscripten-settings`. 
   

Validating the environment
===============================

The best way to validate the environment is to build some code. Open the terminal in your *Emscripten* directory (where *emcc* is located) and enter: ::

	./emcc tests/hello_world.cpp

If this builds **a.out.js** in the current directory, and you don't see any build errors in the terminal, Emscripten is good to go! 

There are additional validation and troubleshooting instructions in the topic: :ref:`verifying-the-emscripten-environment`.





