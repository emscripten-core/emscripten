.. _building-emscripten-on-linux:

=====================================
Manually Building Emscripten on Linux
=====================================

This page contains basic instructions on how to manually build and configure Emscripten from source on a clean (Ubuntu 14.04.1 LTS 64bit) Linux box.

.. note:: The instructions clone from the main Emscripten repository (https://github.com/emscripten-core/emscripten). :ref:`Contributors <contributing>` should instead clone from their own Emscripten fork, and submit changes as pull requests.

.. tip:: You can also build Emscripten from source :ref:`using the SDK <building-emscripten-from-source-using-the-sdk>`. This is recommended if you need easily switch between SDK and source builds.

What you'll need
=================

The specific versions of tools that are needed are listed in the :ref:`Emscripten Toolchain Requirements <toolchain-what-you-need>`.


Installing required tools
==========================

These instructions explain how to install **all** the :ref:`required tools <toolchain-what-you-need>`. You can :ref:`test whether some of these are already installed <toolchain-test-which-dependencies-are-installed>` on the platform and skip those steps.

1. Update the *system package manager* to ensure the package lists are up to date::

    sudo apt-get update


#. Install *Python* using the *system package manager*::

    sudo apt-get install python2.7


#. Install `node.js <http://nodejs.org/>`_ using the *system package manager*::

    sudo apt-get install nodejs


#. Install *gcc* and *cmake* using the *system package manager*::

    sudo apt-get install build-essential
    sudo apt-get install cmake


#. Install *git* using the *system package manager*::

    sudo apt-get install git-core

#. Install *Java* using the *system package manager*::

    sudo apt-get install default-jre

#. Build :ref:`Fastcomp <LLVM-Backend>` (LLVM + Clang) from source using :ref:`these instructions <building-fastcomp-from-source>`.

#. Clone the `emscripten-core/emscripten <https://github.com/emscripten-core/emscripten>`_ repository from GitHub. This repository contains the main compiler tool for compiling C/C++ programs to JavaScript:

  - Create a directory (with no spaces in the name) to contain the clone.
  - Enter the following command into the terminal::

      git clone https://github.com/emscripten-core/emscripten.git


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



