.. _building-emscripten-on-windows-from-source:

=======================================
Manually Building Emscripten on Windows
=======================================

This page contains basic instructions on how to manually build and configure Emscripten from source on a clean Windows box.

.. note:: The instructions clone from the main Emscripten repository (https://github.com/emscripten-core/emscripten). :ref:`Contributors <contributing>` should instead clone from their own Emscripten fork, and submit changes as pull requests.

.. tip:: You can also build Emscripten from source :ref:`using the SDK <building-emscripten-from-source-using-the-sdk>`. This is recommended if you need to easily switch between SDK and source builds.


What you'll need
=================

The specific versions of tools that are needed are listed in the :ref:`Emscripten Toolchain Requirements <toolchain-what-you-need>`.

.. note:: 64-bit versions of all needed dependencies are preferred, and may be required if you are building large projects.

Installing required tools
==========================

These instructions explain how to install **all** the :ref:`required tools <toolchain-what-you-need>`. You can :ref:`test whether some of these are already installed <toolchain-test-which-dependencies-are-installed>` on the platform and skip those steps.


#. Install `Python <http://www.python.org/>`_:

  - Python v2.7 is currently preferred, although experimental support for Python3 exists.
  - Add the path to the Python directory containing **Python.exe** to your PATH.

    - Paths are set by opening **System Settings | Advanced system properties**, clicking **Environment Variables** and selecting **PATH**.
    - Add the path to python, separated by semicolons: e.g. ``;C:/Python27/;`` or ``;C:/Python27/bin;`` (depending on the location of the exe).


#. Install `node.js <http://nodejs.org/>`_:

  - Version 8.9.1 or newer is needed.


#. Install `Visual Studio 2017 <http://go.microsoft.com/?linkid=9709949>`_.

#. Install `cmake <http://www.cmake.org/cmake/resources/software.html>`_.

#. Install `GitHub for Windows <http://windows.github.com/>`_ (or any other git client).

#. Install `Java <http://java.com/en/download/index.jsp>`_ (Java is optional, you only need it for Closure Compiler minification).

#. Build :ref:`Fastcomp <LLVM-Backend>` (LLVM + Clang) from source using :ref:`these instructions <building-fastcomp-from-source>`.

#. Clone the `emscripten-core/emscripten <https://github.com/emscripten-core/emscripten>`_ repository from GitHub. This repository contains the main compiler tool for compiling C/C++ programs to JavaScript:

  - Using *GitHub for Windows*:
    -  Launch the *GitHub for Windows* client. Click **Skip Setup** if you don't have a GitHub account.
    -  (optional) Select **Options** from the gear menu, and customize the default storage directory. Ensure the path has no spaces.
    -  In your web browser, open https://github.com/emscripten-core/emscripten and press the **Clone in Windows** button.

  - Using the command line:
    -  Create a directory (with no spaces in the name) to contain the clone.
    -  Enter the following command into the terminal: ::

      git clone https://github.com/emscripten-core/emscripten.git




Configuring Emscripten settings
===============================

Almost all the compiler settings used by Emscripten are defined in the :ref:`compiler configuration file (.emscripten) <compiler-configuration-file>`, a user-specific file located in the user's home directory.

Instructions for creating and manually configuring this file are given in :ref:`configuring-emscripten-settings`.



Validating the environment
===============================

The best way to validate the environment is to build some code. Open the terminal in your *Emscripten* directory — the directory that contains *emcc* (if you installed using *GitHub for Windows* this will be **C:/Users/username/Documents/GitHub/emscripten**) and enter: ::

  emcc tests/hello_world.cpp

If this builds **a.out.js** in the current directory, and you don't see any build errors in the terminal, Emscripten is good to go!

There are additional validation and troubleshooting instructions in the topic :ref:`verifying-the-emscripten-environment`.



