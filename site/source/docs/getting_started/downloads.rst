.. _sdk-download-and-install:

====================
Download and install
====================

.. tip:: If you are :ref:`contributing <contributing>` to Emscripten you should :ref:`build Emscripten from source <installing-from-source>`.

.. _sdk-installation-instructions:

Installation instructions
=========================

First check the :ref:`Platform-specific notes <platform-notes-installation_instructions-SDK>` below and install any prerequisites.

The core Emscripten SDK (emsdk) driver is a Python script. You can get it for the first time with

  ::

    # Get the emsdk repo
    git clone https://github.com/emscripten-core/emsdk.git

    # Enter that directory
    cd emsdk

.. note:: You can also get the emsdk without git, by selecting "Clone or download => Download ZIP" on the `emsdk GitHub page <https://github.com/emscripten-core/emsdk>`_.

Run the following :ref:`emsdk <emsdk>` commands to get the latest tools from GitHub and set them as :term:`active <Active Tool/SDK>`:

  ::

    # Fetch the latest version of the emsdk (not needed the first time you clone)
    git pull

    # Download and install the latest SDK tools.
    ./emsdk install latest

    # Make the "latest" SDK "active" for the current user. (writes ~/.emscripten file)
    ./emsdk activate latest

    # Activate PATH and other environment variables in the current terminal
    source ./emsdk_env.sh

  .. note:: On Windows, run ``emsdk`` instead of ``./emsdk``, and ``emsdk_env.bat`` instead of ``source ./emsdk_env.sh``.

  .. note:: ``git pull`` will fetch the current list of tags, but very recent ones may not yet be present there. You can run ``./emsdk update-tags`` to update the list of tags directly.

If you change the location of the SDK (e.g. take it to another computer on an USB), re-run the ``./emsdk activate latest`` and ``source ./emsdk_env.sh`` commands.

.. _platform-notes-installation_instructions-SDK:

Platform-specific notes
----------------------------

Windows
+++++++

#. Install Python 2.7.12 or newer (older versions may not work due to `a GitHub change with SSL <https://github.com/emscripten-core/emscripten/issues/6275>`_).

  .. note:: Instead of running emscripten on Windows directly, you can use the Windows Subsystem for Linux to run it in a Linux environment.

macOS
+++++

If you use MacOS 10.13.3 or later then you should have a new enough version of Python installed (older versions may not work due to `a GitHub change with SSL <https://github.com/emscripten-core/emscripten/issues/6275>`_). Otherwise you can manually install and use Python 2.7.12 or newer.

These instructions explain how to install **all** the :ref:`required tools <toolchain-what-you-need>`. You can :ref:`test whether some of these are already installed <toolchain-test-which-dependencies-are-installed>` on the platform and skip those steps.

#. Install the *Xcode Command Line Tools*. These are a precondition for *git*.

  -  Install Xcode from the `macOS App Store <http://superuser.com/questions/455214/where-is-svn-on-os-x-mountain-lion>`_.
  -  In **Xcode | Preferences | Downloads**, install *Command Line Tools*.

#. Install *git*:

  - `Allow installation of unsigned packages <https://www.my-private-network.co.uk/knowledge-base/apple-related-questions/osx-unsigned-apps.html>`_, or installing the git package won't succeed.
  - Install Xcode and the Xcode Command Line Tools (should already have been done). This will provide *git* to the system PATH (see `this stackoverflow post <http://stackoverflow.com/questions/9329243/xcode-4-4-command-line-tools>`_).
  - Download and install git directly from http://git-scm.com/.

#. Install *cmake* if you do not have it yet:

  -  Download and install latest CMake from `Kitware CMake downloads <http://www.cmake.org/download/>`_.

#. Install *node.js* from http://nodejs.org/

  .. _getting-started-on-macos-install-python2:

Linux
++++++++

.. note:: *Emsdk* does not install any tools to the system, or otherwise interact with Linux package managers. All file changes are done inside the **emsdk/** directory.

- *Python*, *node.js*, *CMake*, and *Java* are not provided by *emsdk*. The user is expected to install these beforehand with the *system package manager*:

  ::

    # Install Python
    sudo apt-get install python2.7

    # Install node.js
    sudo apt-get install nodejs

    # Install CMake (optional, only needed for tests and building Binaryen)
    sudo apt-get install cmake

    # Install Java (optional, only needed for Closure Compiler minification)
    sudo apt-get install default-jre

.. note:: You need Python 2.7.12 or newer because older versions may not work due to `a GitHub change with SSL <https://github.com/emscripten-core/emscripten/issues/6275>`_).

.. note:: Your system may provide Node.js as ``node`` instead of ``nodejs``. In that case, you may need to also update the ``NODE_JS`` attribute of your ``~/.emscripten`` file.

- *Git* is not installed automatically. Git is only needed if you want to use tools from one of the development branches **emscripten-incoming** or **emscripten-master**:

  ::

    # Install git
    sudo apt-get install git-core

More detailed instructions on the toolchain are provided in: :ref:`building-emscripten-on-linux`.


Verifying the installation
==========================

The easiest way to verify the installation is to compile some code using Emscripten.

You can jump ahead to the :ref:`Tutorial`, but if you have any problems building you should run through the basic tests and troubleshooting instructions in :ref:`verifying-the-emscripten-environment`.


.. _updating-the-emscripten-sdk:

Updating the SDK
================

.. tip:: You only need to install the SDK once! After that you can update to the latest SDK at any time using :ref:`Emscripten SDK (emsdk) <emsdk>`.

Type the following in a command prompt ::

  # Fetch the latest registry of available tools.
  ./emsdk update

  # Download and install the latest SDK tools.
  ./emsdk install latest

  # Set up the compiler configuration to point to the "latest" SDK.
  ./emsdk activate latest

  # Activate PATH and other environment variables in the current terminal
  source ./emsdk_env.sh

The package manager can do many other maintenance tasks ranging from fetching specific old versions of the SDK through to using the :ref:`versions of the tools on GitHub <emsdk-master-or-incoming-sdk>` (or even your own fork). Check out all the possibilities in the :ref:`emsdk_howto`.

.. _downloads-uninstall-the-sdk:

Uninstalling the Emscripten SDK
========================================================

If you want to remove the whole SDK, just delete the directory containing the SDK.

It is also possible to :ref:`remove specific tools in the SDK using emsdk <emsdk-remove-tool-sdk>`.


.. _archived-nsis-windows-sdk-releases:

Archived releases
=================

You can always install old SDK and compiler toolchains using a *current SDK*. See :ref:`emsdk-install-old-tools` for more information.

On Windows, you can also install one of the **old versions** via an offline NSIS installer. These NSIS installers are now deprecated, but still available for archived reference:

- `emsdk-1.35.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.35.0-full-64bit.exe>`_
- `emsdk-1.34.1-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.34.1-full-64bit.exe>`_ (first release based on Clang 3.7)
- `emsdk-1.30.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.30.0-full-64bit.exe>`_ (first and last release based on Clang 3.5)
- `emsdk-1.29.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.29.0-full-64bit.exe>`_ (first and last release based on Clang 3.4)
- `emsdk-1.27.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.27.0-full-64bit.exe>`_
- `emsdk-1.25.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.25.0-full-64bit.exe>`_
- `emsdk-1.22.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.22.0-full-64bit.exe>`_
- `emsdk-1.21.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.21.0-full-64bit.exe>`_
- `emsdk-1.16.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.16.0-full-64bit.exe>`_ (first stable fastcomp release)
- `emsdk-1.13.0-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.13.0-full-64bit.exe>`_ (a unstable first fastcomp release with Clang 3.3)
- `emsdk-1.12.0-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.12.0-full-64bit.exe>`_ (the last non-fastcomp version with Clang 3.2)
- `emsdk-1.12.0-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.12.0-full-32bit.exe>`_
- `emsdk-1.8.2-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.8.2-full-64bit.exe>`_
- `emsdk-1.8.2-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.8.2-full-32bit.exe>`_
- `emsdk-1.7.8-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.7.8-full-64bit.exe>`_
- `emsdk-1.7.8-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.7.8-full-32bit.exe>`_
- `emsdk-1.5.6.2-full-64bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.5.6.2-full-64bit.exe>`_
- `emsdk-1.5.6.2-full-32bit.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.5.6.2-full-32bit.exe>`_
- `emsdk-1.5.6.1-full.exe <https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.5.6.1-full.exe)>`_ (32-bit, first emsdk release)


A snapshot of all tagged Emscripten compiler releases (not full SDKs) can be found at `emscripten/releases <https://github.com/emscripten-core/emscripten/releases>`_.

