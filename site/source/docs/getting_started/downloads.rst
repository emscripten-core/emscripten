.. _sdk-download-and-install:

====================
Download and install
====================

.. note:: You can also :ref:`build Emscripten from source <installing-from-source>` if you prefer that to downloading binaries using the emsdk.

.. note:: There are additional ways to install Emscripten than the instructions
    below, for example, using Homebrew on MacOS, the package manager on your linux
    distro. However, the emsdk is the only officially supported way to use
    Emscripten that is supported by the Emscripten project, and the only one
    that we constantly test
    (`emsdk CI <https://github.com/emscripten-core/emsdk/blob/master/.circleci/config.yml>`_,
    `Emscripten GitHub CI <https://github.com/emscripten-core/emscripten/blob/master/.circleci/config.yml>`_,
    `Chromium CI <https://ci.chromium.org/p/emscripten-releases>`_). (While we
    don't officially support other ways of getting Emscripten, we definitely
    appreciate the efforts by third parties to
    `package Emscripten <https://github.com/emscripten-core/emscripten/blob/master/docs/packaging.md>`_
    for users' convenience, and we'd like to help out, please get in touch if
    you are such a packager!)

.. _sdk-installation-instructions:

Installation instructions
=========================

First check the :ref:`Platform-specific notes <platform-notes-installation_instructions-SDK>` below and install any prerequisites.

The core Emscripten SDK (emsdk) driver is a Python script. You can get it for
the first time with

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

    # Make the "latest" SDK "active" for the current user. (writes .emscripten file)
    ./emsdk activate latest

    # Activate PATH and other environment variables in the current terminal
    source ./emsdk_env.sh

  .. note:: On Windows, run ``emsdk`` instead of ``./emsdk``, and ``emsdk_env.bat`` instead of ``source ./emsdk_env.sh``.

  .. note:: ``git pull`` will fetch the current list of tags, but very recent ones may not yet be present there. You can run ``./emsdk update-tags`` to update the list of tags directly.

If you change the location of the SDK (e.g. take it to another computer on an USB), re-run the ``./emsdk activate latest`` and ``source ./emsdk_env.sh`` commands.

Emsdk install targets
---------------------

In the description above we asked the emsdk to install and activate ``latest``,
which is the latest tagged release. That is often what you want.

You can also install a specific version by specifying it, for example,

  ::

    ./emsdk install 1.38.45


.. note:: When installing old versions from before the build infrastructure rewrite (anything before ``1.38.33``), you need to write something like ``./emsdk install sdk-1.38.20-64bit`` (add ``sdk-`` and ``-64bit``) as that was the naming convention at the time.

You can also specify which backend you want to use, either ``fastcomp`` or ``upstream`` (without specifying the backend, the current default is used), for example,

  ::

    # Get a specific version using the upstream backend.
    ./emsdk install latest-upstream

    # Get a specific version using the fastcomp backend.
    ./emsdk install 1.38.45-fastcomp


There are also "tip-of-tree builds", which are the very latest code that passes integration tests on `Chromium CI <https://ci.chromium.org/p/emscripten-releases>`_. This is updated much more frequently than tagged releases, but may be less stable (we `tag releases manually <https://github.com/emscripten-core/emscripten/blob/master/docs/process.md#minor-version-updates-1xy-to-1xy1>`_ using a more careful procedure). Tip-of-tree builds may be useful for continuous integration that uses the emsdk (as Emscripten's GitHub CI does), and you may want to use it in your own CI as well, so that if you find a regression on your project you can report it and prevent it from reaching a tagged release. Tip-of-builds may also be useful if you want to test a feature that just landed but didn't reach a release yet. To use a tip-of-tree build, use the ``tot`` target, and note that you must specify the backend explicitly,

  ::

    # Get a tip-of-tree using the upstream backend.
    ./emsdk install tot-upstream

    # Get a tip-of-tree using the fastcomp backend.
    ./emsdk install tot-fastcomp

(In the above examples we installed the various targets; remember to also ``activate`` them as in the full example from earlier.)

.. _platform-notes-installation_instructions-SDK:

Platform-specific notes
-----------------------

Windows
+++++++

#. Install Python 2.7.12 or newer (older versions may not work due to `a GitHub change with SSL <https://github.com/emscripten-core/emscripten/issues/6275>`_).

  .. note:: Instead of running emscripten on Windows directly, you can use the Windows Subsystem for Linux to run it in a Linux environment.

macOS
+++++

If you use the Emscripten SDK it includes a bundled version of Python 3.  Otherwise
you will need to manually install and use Python 3.5 or newer.

These instructions explain how to install **all** the :ref:`required tools <toolchain-what-you-need>`. You can :ref:`test whether some of these are already installed <toolchain-test-which-dependencies-are-installed>` on the platform and skip those steps.

#. Install the *Xcode Command Line Tools*. These are a precondition for *git*.

  -  Install Xcode from the `macOS App Store <http://superuser.com/questions/455214/where-is-svn-on-os-x-mountain-lion>`_.
  -  In **Xcode | Preferences | Downloads**, install *Command Line Tools*.

#. Install *git*:

  - `Make sure the OS allows installing git <https://support.apple.com/en-gb/HT202491>`_.
  - Install Xcode and the Xcode Command Line Tools (should already have been done). This will provide *git* to the system PATH (see `this stackoverflow post <http://stackoverflow.com/questions/9329243/xcode-4-4-command-line-tools>`_).
  - Download and install git directly from http://git-scm.com/.

#. Install *cmake* if you do not have it yet:

  -  Download and install latest CMake from `Kitware CMake downloads <http://www.cmake.org/download/>`_.

Linux
+++++

.. note:: *Emsdk* does not install any tools to the system, or otherwise interact with Linux package managers. All file changes are done inside the **emsdk/** directory.

- *Python*, *CMake*, and *Java* are not provided by *emsdk*. The user is expected to install these beforehand with the *system package manager*:

  ::

    # Install Python
    sudo apt-get install python3

    # Install CMake (optional, only needed for tests and building Binaryen)
    sudo apt-get install cmake

    # Install Java (optional, only needed for Closure Compiler minification)
    sudo apt-get install default-jre

.. note:: If you want to use your system's Node.js instead of the emsdk's, it may be ``node`` instead of ``nodejs``, and you can adjust the ``NODE_JS`` attribute of your ``.emscripten`` file to point to it.

- *Git* is not installed automatically. Git is only needed if you want to use tools from one of the development branches **emscripten-incoming** or **emscripten-master**:

  ::

    # Install git
    sudo apt-get install git-core


Verifying the installation
==========================

The easiest way to verify the installation is to compile some code using Emscripten.

You can jump ahead to the :ref:`Tutorial`, but if you have any problems building
you should run through the basic tests and troubleshooting instructions in
:ref:`verifying-the-emscripten-environment`.


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
===============================

If you want to remove the whole SDK, just delete the directory containing the
SDK.

It is also possible to :ref:`remove specific tools in the SDK using emsdk
<emsdk-remove-tool-sdk>`.

Using the Docker image
======================

The entire Emscripten SDK is also available in the form of a `docker image
<https://hub.docker.com/r/emscripten/emsdk>`_.  For example::

  docker run --rm -v $(pwd):/src -u $(id -u):$(id -g) \
    emscripten/emsdk emcc helloworld.cpp -o helloworld.js

See the Docker Hub page for more details and examples.
